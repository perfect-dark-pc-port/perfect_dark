#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PR/ultratypes.h>
#include <PR/os_internal.h>
#include "platform.h"
#include "config.h"
#include "system.h"
#include "fs.h"
#include "mempak.h"

/**
 * Virtual Controller Pak.
 *
 * The pak is kept as a 32 KiB image in the standard layout, the same as an
 * emulator .mpk file or a dump of a real pak:
 *
 *   page 0      ID sector (ID block at 0x20, backups at 0x60, 0x80, 0xc0)
 *   page 1      allocation table: 2 bytes per page, 1 = end of chain,
 *               3 = free, >= 5 = next page; byte 1 is the checksum
 *   page 2      backup of page 1
 *   pages 3-4   note table, 16 entries of 32 bytes
 *   pages 5-127 data
 *
 * All values are big-endian. The osPfs* calls follow libultra's semantics
 * (see src/lib/ultra/io/pfs*.c), but work on the image directly, so that the
 * image stays byte-compatible on little-endian hosts.
 *
 * Reference: https://n64brew.dev/wiki/Controller_Pak/Filesystem
 */

#define MEMPAK_SIZE        0x8000
#define MEMPAK_PAGESIZE    256
#define MEMPAK_NUMPAGES    128
#define MEMPAK_FIRSTPAGE   5
#define MEMPAK_NUMNOTES    16
#define MEMPAK_FAT         (1 * MEMPAK_PAGESIZE)
#define MEMPAK_FATBACKUP   (2 * MEMPAK_PAGESIZE)
#define MEMPAK_NOTES       (3 * MEMPAK_PAGESIZE)
#define MEMPAK_NOTESIZE    32

#define FAT_END   0x0001
#define FAT_FREE  0x0003

struct mempak {
	u8 *image;
	s32 loaded;
};

static s32 mempakEnabled[MAXCONTROLLERS];
static struct mempak mempaks[MAXCONTROLLERS];

static const char *mempakPath(s32 channel)
{
	static char path[32];
	snprintf(path, sizeof(path), "$S/controllerpak%d.mpk", channel + 1);
	return path;
}

static u16 mempakGet16(const u8 *p)
{
	return (p[0] << 8) | p[1];
}

static void mempakSet16(u8 *p, u16 value)
{
	p[0] = value >> 8;
	p[1] = value & 0xff;
}

static u32 mempakGet32(const u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}

static void mempakSet32(u8 *p, u32 value)
{
	p[0] = value >> 24;
	p[1] = (value >> 16) & 0xff;
	p[2] = (value >> 8) & 0xff;
	p[3] = value & 0xff;
}

static void mempakWriteIdBlock(u8 *image)
{
	static const u16 offsets[] = { 0x20, 0x60, 0x80, 0xc0 };
	u8 block[32] = { 0 };
	u16 sum = 0;

	for (s32 i = 0; i < 24; i++) {
		block[i] = rand() & 0xff; // serial
	}

	block[0x19] = 0x01; // device ID: Controller Pak
	block[0x1a] = 0x01; // banks

	for (s32 i = 0; i < 0x1c; i += 2) {
		sum += mempakGet16(&block[i]);
	}

	mempakSet16(&block[0x1c], sum);
	mempakSet16(&block[0x1e], 0xfff2 - sum);

	for (s32 i = 0; i < 4; i++) {
		memcpy(image + offsets[i], block, sizeof(block));
	}
}

static s32 mempakIdIsValid(const u8 *image)
{
	const u8 *block = image + 0x20;
	u16 sum = 0;

	for (s32 i = 0; i < 0x1c; i += 2) {
		sum += mempakGet16(&block[i]);
	}

	return mempakGet16(&block[0x1c]) == sum && mempakGet16(&block[0x1e]) == (u16)(0xfff2 - sum);
}

static u16 mempakFatGet(u8 *image, s32 page)
{
	return mempakGet16(image + MEMPAK_FAT + page * 2);
}

static void mempakFatSet(u8 *image, s32 page, u16 value)
{
	mempakSet16(image + MEMPAK_FAT + page * 2, value);
}

// Recalculate the allocation table's checksum and copy it to its backup
static void mempakFatCommit(u8 *image)
{
	u8 sum = 0;

	for (s32 i = MEMPAK_FIRSTPAGE * 2; i < MEMPAK_PAGESIZE; i++) {
		sum += image[MEMPAK_FAT + i];
	}

	image[MEMPAK_FAT + 1] = sum;
	memcpy(image + MEMPAK_FATBACKUP, image + MEMPAK_FAT, MEMPAK_PAGESIZE);
}

static void mempakFormat(u8 *image)
{
	memset(image, 0, MEMPAK_SIZE);
	mempakWriteIdBlock(image);

	for (s32 i = MEMPAK_FIRSTPAGE; i < MEMPAK_NUMPAGES; i++) {
		mempakFatSet(image, i, FAT_FREE);
	}

	mempakFatCommit(image);
}

static void mempakSave(s32 channel)
{
	FILE *f = fsFileOpenWrite(mempakPath(channel));

	if (f) {
		fwrite(mempaks[channel].image, 1, MEMPAK_SIZE, f);
		fsFileFree(f);
	} else {
		sysLogPrintf(LOG_ERROR, "mempak: could not save %s", fsFullPath(mempakPath(channel)));
	}
}

static u8 *mempakGetImage(s32 channel)
{
	struct mempak *pak = &mempaks[channel];

	if (!pak->loaded) {
		const char *path = mempakPath(channel);

		pak->image = sysMemZeroAlloc(MEMPAK_SIZE);

		if (!pak->image) {
			sysFatalError("mempak: could not allocate memory");
		}

		if (fsFileSize(path) == MEMPAK_SIZE && fsFileLoadTo(path, pak->image, MEMPAK_SIZE) > 0) {
			sysLogPrintf(LOG_NOTE, "mempak: loaded Controller Pak %d from %s", channel + 1, fsFullPath(path));
		} else {
			// a new pak comes formatted, like one from the shop
			mempakFormat(pak->image);
			mempakSave(channel);
			sysLogPrintf(LOG_NOTE, "mempak: created Controller Pak %d at %s", channel + 1, fsFullPath(path));
		}

		pak->loaded = 1;
	}

	return pak->image;
}

static u8 *mempakNote(u8 *image, s32 file_no)
{
	return image + MEMPAK_NOTES + file_no * MEMPAK_NOTESIZE;
}

static s32 mempakNoteIsUsed(const u8 *note)
{
	return mempakGet16(note + 4) != 0 && mempakGet32(note) != 0;
}

static s32 mempakCountFreePages(u8 *image)
{
	s32 count = 0;

	for (s32 i = MEMPAK_FIRSTPAGE; i < MEMPAK_NUMPAGES; i++) {
		if (mempakFatGet(image, i) == FAT_FREE) {
			count++;
		}
	}

	return count;
}

// Number of pages in a note's chain, or -1 if the chain is broken
static s32 mempakCountChain(u8 *image, u16 page)
{
	s32 count = 0;

	while (page >= MEMPAK_FIRSTPAGE && page < MEMPAK_NUMPAGES && count <= MEMPAK_NUMPAGES) {
		count++;
		page = mempakFatGet(image, page);
	}

	return page == FAT_END ? count : -1;
}

static void mempakFreeChain(u8 *image, u16 page)
{
	s32 count = 0;

	while (page >= MEMPAK_FIRSTPAGE && page < MEMPAK_NUMPAGES && count++ <= MEMPAK_NUMPAGES) {
		u16 next = mempakFatGet(image, page);
		mempakFatSet(image, page, FAT_FREE);
		page = next;
	}
}

/**
 * Chain <count> free pages together and return the first, or 0 if there are
 * not enough. The pages are taken in ascending order, like libultra does.
 */
static u16 mempakAllocChain(u8 *image, s32 count)
{
	u16 first = 0;
	u16 prev = 0;

	if (count <= 0 || mempakCountFreePages(image) < count) {
		return 0;
	}

	for (s32 i = MEMPAK_FIRSTPAGE; i < MEMPAK_NUMPAGES && count > 0; i++) {
		if (mempakFatGet(image, i) == FAT_FREE) {
			if (prev) {
				mempakFatSet(image, prev, i);
			} else {
				first = i;
			}

			mempakFatSet(image, i, FAT_END);
			prev = i;
			count--;
		}
	}

	return first;
}

static s32 mempakCheckPfs(OSPfs *pfs)
{
	if (!pfs || !(pfs->status & PFS_INITIALIZED) || pfs->channel < 0
			|| pfs->channel >= MAXCONTROLLERS || !mempakEnabled[pfs->channel]) {
		return PFS_ERR_INVALID;
	}

	return 0;
}

s32 mempakIsPlugged(s32 channel)
{
	return channel >= 0 && channel < MAXCONTROLLERS && mempakEnabled[channel];
}

s32 mempakInit(OSMesgQueue *queue, OSPfs *pfs, s32 channel)
{
	u8 *image;

	if (!mempakIsPlugged(channel)) {
		return PFS_ERR_NOPACK;
	}

	image = mempakGetImage(channel);

	if (!mempakIdIsValid(image)) {
		sysLogPrintf(LOG_WARNING, "mempak: Controller Pak %d has a bad ID block", channel + 1);
		return PFS_ERR_ID_FATAL;
	}

	memset(pfs, 0, sizeof(*pfs));
	pfs->queue = queue;
	pfs->channel = channel;
	pfs->status = PFS_INITIALIZED;
	pfs->version = 0;
	pfs->dir_size = MEMPAK_NUMNOTES;
	pfs->inode_table = 1 * PFS_ONE_PAGE;
	pfs->minode_table = 2 * PFS_ONE_PAGE;
	pfs->dir_table = 3 * PFS_ONE_PAGE;
	pfs->inode_start_page = MEMPAK_FIRSTPAGE;
	pfs->banks = 1;
	pfs->activebank = 0;
	memcpy(pfs->id, image + 0x20, sizeof(pfs->id));
	memcpy(pfs->label, image, sizeof(pfs->label));

	return 0;
}

s32 mempakChecker(OSPfs *pfs)
{
	u8 *image;
	s32 changed = 0;

	if (mempakCheckPfs(pfs)) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);

	// Drop notes whose chains are broken and free any page no note uses
	{
		u8 used[MEMPAK_NUMPAGES] = { 0 };

		for (s32 n = 0; n < MEMPAK_NUMNOTES; n++) {
			u8 *note = mempakNote(image, n);

			if (mempakNoteIsUsed(note)) {
				u16 page = mempakGet16(note + 6);

				if (mempakCountChain(image, page) < 0) {
					memset(note, 0, MEMPAK_NOTESIZE);
					changed = 1;
					continue;
				}

				while (page >= MEMPAK_FIRSTPAGE && page < MEMPAK_NUMPAGES) {
					used[page] = 1;
					page = mempakFatGet(image, page);
				}
			}
		}

		for (s32 i = MEMPAK_FIRSTPAGE; i < MEMPAK_NUMPAGES; i++) {
			if (!used[i] && mempakFatGet(image, i) != FAT_FREE) {
				mempakFatSet(image, i, FAT_FREE);
				changed = 1;
			}
		}
	}

	if (changed) {
		sysLogPrintf(LOG_WARNING, "mempak: repaired Controller Pak %d", pfs->channel + 1);
		mempakFatCommit(image);
		mempakSave(pfs->channel);
	}

	return 0;
}

s32 mempakFreeBlocks(OSPfs *pfs, s32 *remaining)
{
	if (mempakCheckPfs(pfs)) {
		return PFS_ERR_INVALID;
	}

	*remaining = mempakCountFreePages(mempakGetImage(pfs->channel)) * MEMPAK_PAGESIZE;

	return 0;
}

s32 mempakNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used)
{
	u8 *image;

	if (mempakCheckPfs(pfs)) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);

	*max_files = MEMPAK_NUMNOTES;
	*files_used = 0;

	for (s32 n = 0; n < MEMPAK_NUMNOTES; n++) {
		if (mempakNoteIsUsed(mempakNote(image, n))) {
			(*files_used)++;
		}
	}

	return 0;
}

s32 mempakFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, s32 *file_no)
{
	u8 *image;

	if (mempakCheckPfs(pfs)) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);

	for (s32 n = 0; n < MEMPAK_NUMNOTES; n++) {
		u8 *note = mempakNote(image, n);

		if (mempakGet16(note + 4) == company_code && mempakGet32(note) == game_code
				&& (game_name == NULL || memcmp(note + 0x10, game_name, PFS_FILE_NAME_LEN) == 0)
				&& (ext_name == NULL || memcmp(note + 0x0c, ext_name, PFS_FILE_EXT_LEN) == 0)) {
			*file_no = n;
			return 0;
		}
	}

	*file_no = -1;

	return PFS_ERR_INVALID;
}

s32 mempakAllocateFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int num_bytes, s32 *file_no)
{
	u8 *image;
	s32 numpages;
	s32 n;
	u16 start;
	u8 *note;

	if (mempakCheckPfs(pfs) || company_code == 0 || game_code == 0) {
		return PFS_ERR_INVALID;
	}

	numpages = (num_bytes + MEMPAK_PAGESIZE - 1) / MEMPAK_PAGESIZE;

	if (numpages <= 0) {
		return PFS_ERR_INVALID;
	}

	if (mempakFindFile(pfs, company_code, game_code, game_name, ext_name, file_no) == 0) {
		return PFS_ERR_EXIST;
	}

	image = mempakGetImage(pfs->channel);

	for (n = 0; n < MEMPAK_NUMNOTES; n++) {
		if (!mempakNoteIsUsed(mempakNote(image, n))) {
			break;
		}
	}

	if (n == MEMPAK_NUMNOTES) {
		return PFS_DIR_FULL;
	}

	start = mempakAllocChain(image, numpages);

	if (start == 0) {
		return PFS_DATA_FULL;
	}

	note = mempakNote(image, n);
	memset(note, 0, MEMPAK_NOTESIZE);
	mempakSet32(note, game_code);
	mempakSet16(note + 4, company_code);
	mempakSet16(note + 6, start);
	note[8] = 0x02; // occupied
	memcpy(note + 0x0c, ext_name, PFS_FILE_EXT_LEN);
	memcpy(note + 0x10, game_name, PFS_FILE_NAME_LEN);

	mempakFatCommit(image);
	mempakSave(pfs->channel);

	*file_no = n;

	return 0;
}

s32 mempakDeleteFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name)
{
	u8 *image;
	u8 *note;
	s32 file_no;

	if (mempakCheckPfs(pfs) || company_code == 0 || game_code == 0) {
		return PFS_ERR_INVALID;
	}

	if (mempakFindFile(pfs, company_code, game_code, game_name, ext_name, &file_no) != 0) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);
	note = mempakNote(image, file_no);

	mempakFreeChain(image, mempakGet16(note + 6));
	memset(note, 0, MEMPAK_NOTESIZE);

	mempakFatCommit(image);
	mempakSave(pfs->channel);

	return 0;
}

/**
 * Perfect Dark's addition to libultra (src/lib/ultra/io/pfsresizefile.c):
 * grow or shrink a note to <length> bytes, keeping its data.
 */
s32 mempakReSizeFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int length)
{
	u8 *image;
	u8 *note;
	s32 file_no;
	s32 oldpages;
	s32 newpages;
	u16 page;

	if (mempakCheckPfs(pfs) || company_code == 0 || game_code == 0) {
		return PFS_ERR_INVALID;
	}

	if (mempakFindFile(pfs, company_code, game_code, game_name, ext_name, &file_no) != 0) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);
	note = mempakNote(image, file_no);
	page = mempakGet16(note + 6);
	oldpages = mempakCountChain(image, page);
	newpages = (length + MEMPAK_PAGESIZE - 1) / MEMPAK_PAGESIZE;

	if (oldpages < 0) {
		return PFS_ERR_INCONSISTENT;
	}

	if (newpages <= 0) {
		return PFS_ERR_INVALID;
	}

	if (newpages > oldpages) {
		u16 last = page;
		u16 extra;

		if (mempakCountFreePages(image) < newpages - oldpages) {
			return PFS_DATA_FULL;
		}

		while (mempakFatGet(image, last) != FAT_END) {
			last = mempakFatGet(image, last);
		}

		extra = mempakAllocChain(image, newpages - oldpages);
		mempakFatSet(image, last, extra);
	} else if (newpages < oldpages) {
		u16 last = page;

		for (s32 i = 1; i < newpages; i++) {
			last = mempakFatGet(image, last);
		}

		mempakFreeChain(image, mempakFatGet(image, last));
		mempakFatSet(image, last, FAT_END);
	}

	mempakFatCommit(image);
	mempakSave(pfs->channel);

	return 0;
}

s32 mempakFileState(OSPfs *pfs, s32 file_no, OSPfsState *state)
{
	u8 *image;
	u8 *note;
	s32 pages;

	if (mempakCheckPfs(pfs) || file_no < 0 || file_no >= MEMPAK_NUMNOTES) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);
	note = mempakNote(image, file_no);

	if (!mempakNoteIsUsed(note)) {
		return PFS_ERR_INVALID;
	}

	pages = mempakCountChain(image, mempakGet16(note + 6));

	if (pages < 0) {
		return PFS_ERR_INCONSISTENT;
	}

	state->file_size = pages * MEMPAK_PAGESIZE;
	state->game_code = mempakGet32(note);
	state->company_code = mempakGet16(note + 4);
	memcpy(state->ext_name, note + 0x0c, PFS_FILE_EXT_LEN);
	memcpy(state->game_name, note + 0x10, PFS_FILE_NAME_LEN);

	return 0;
}

s32 mempakReadWriteFile(OSPfs *pfs, s32 file_no, u8 flag, int offset, int size, u8 *data)
{
	u8 *image;
	u8 *note;
	u16 page;
	s32 pages;

	if (mempakCheckPfs(pfs) || file_no < 0 || file_no >= MEMPAK_NUMNOTES
			|| offset < 0 || size <= 0 || (offset % BLOCKSIZE) || (size % BLOCKSIZE)) {
		return PFS_ERR_INVALID;
	}

	image = mempakGetImage(pfs->channel);
	note = mempakNote(image, file_no);

	if (!mempakNoteIsUsed(note)) {
		return PFS_ERR_INVALID;
	}

	page = mempakGet16(note + 6);
	pages = mempakCountChain(image, page);

	if (pages < 0) {
		return PFS_ERR_INCONSISTENT;
	}

	if (offset + size > pages * MEMPAK_PAGESIZE) {
		return PFS_ERR_INVALID;
	}

	// skip to the page holding <offset>
	for (s32 i = 0; i < offset / MEMPAK_PAGESIZE; i++) {
		page = mempakFatGet(image, page);
	}

	offset %= MEMPAK_PAGESIZE;

	while (size > 0) {
		s32 len = MEMPAK_PAGESIZE - offset;
		u8 *ptr = image + page * MEMPAK_PAGESIZE + offset;

		if (len > size) {
			len = size;
		}

		if (flag == PFS_WRITE) {
			memcpy(ptr, data, len);
		} else {
			memcpy(data, ptr, len);
		}

		data += len;
		size -= len;
		offset = 0;
		page = mempakFatGet(image, page);
	}

	if (flag == PFS_WRITE) {
		mempakSave(pfs->channel);
	}

	return 0;
}

PD_CONSTRUCTOR static void mempakConfigInit(void)
{
	char key[] = "Input.Player1.ControllerPak";

	for (s32 c = 0; c < MAXCONTROLLERS; ++c) {
		key[12] = '1' + c;
		configRegisterInt(key, &mempakEnabled[c], 0, 1);
	}
}
