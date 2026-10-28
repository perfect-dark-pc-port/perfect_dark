#include <string.h>
#include <strings.h>
#include "patch.h"
#include "fs.h"
#include "mod.h"
#include "zlib.h"
#include "lib/rzip.h"
#include "system.h"
#include "romdata.h"

#define PATCH_DIR "$H/autopatch"
#define PATCH_CONFIG PATCH_DIR "/" MOD_CONFIG_FNAME
#define PATCH_FILES_DIR PATCH_DIR "/files"
#define PATCH_BGDATA_DIR PATCH_FILES_DIR "/bgdata"

u8 dl_begin[] = {
  0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // G_RDPPIPESYNC
  0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, // G_CLEARGEOMETRYMODE
  0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, // G_SETENVCOLOR
  0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // G_RDPPIPESYNC
  0xba, 0x00, 0x14, 0x02, 0x00, 0x10, 0x00, 0x00, // G_SETOTHERMODE_H
  0xba, 0x00, 0x0c, 0x02, 0x00, 0x00, 0x20, 0x00, // G_SETOTHERMODE_H
  0xba, 0x00, 0x10, 0x01, 0x00, 0x01, 0x00, 0x00, // G_SETOTHERMODE_H
  0xba, 0x00, 0x11, 0x02, 0x00, 0x00, 0x00, 0x00, // G_SETOTHERMODE_H
  0xb9, 0x00, 0x03, 0x1d, 0x0c, 0x18, 0x49, 0xd8, // G_SETOTHERMODE_L
  0xfc, 0x26, 0xa0, 0x04, 0x1f, 0x10, 0x93, 0xff, // G_SETCOMBINE
  0xb7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, // G_SETGEOMETRYMOD
};

u8 dl_end[] = {
  0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // G_ENDDL
};

s32 patchDeflate1173(u8 *src, u32 srclen, u8 *dst, u32 dstlen)
{
	/**
	 * Method to compress src using 1173 compression format
	 *
	 * Args:
	 * 	src (u8 *): Pointer to uncrompressed source data
	 * 	srclen (u32): Size of the source data
	 * 	dst (u8 *): Pointer to destination for compressed data
	 * 	dstlen (u32): Size of the destination
	 *
	 * Returns:
	 * 	(s32): Length of data after compression
	 */
	u8 status;
	s32 len;

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	status = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);

	if (status != Z_OK) return -1;

	strm.next_in = src;
	strm.avail_in = srclen;
	strm.next_out = dst + 5; // compressed data begins after 5 header bytes
	strm.avail_out = dstlen - 5;

	status = deflate(&strm, Z_FINISH);

	if (status != Z_STREAM_END) return -1;

	len = strm.total_out + 5; // total length of compressed data + 5 header bytes
	deflateEnd(&strm);

	// complete the 1173 header bytes
	dst[0] = 0x11;
	dst[1] = 0x73;
	dst[2] = (srclen & 0xff0000) >> 16;
	dst[3] = (srclen & 0x00ff00) >> 8;
	dst[4] = (srclen & 0x0000ff);

	return len;
}

void patchLightDir(u8 *src, s16 lightnum, s16 dx, s16 dy, s16 dz)
{
	/**
	 * Method to patch background light direction in place
	 *
	 * Args:
	 * 	src (u8 *): Pointer to primary background data
	 * 	lightnum (s16): Light number in the background light array
	 * 	dx (s16): Offset to apply along x-axis
	 * 	dy (s16): Offset to apply along y-axis
	 * 	dz (s16): Offset to apply along z-axis
	 */
	// lights are stored in 34 byte (0x22) blocks with direction at offset 0x7
	s8 *light_dir = (s8 *)&src[lightnum * 0x22 + 0x7];

	light_dir[0] += dx; // x
	light_dir[1] += dy; // y
	light_dir[2] += dz; // z
}

void patchLightBbox(u8 *src, s16 lightnum, s16 dx, s16 dy, s16 dz, s16 index)
{
	/**
	 * Method to patch background light locations in place
	 *
	 * Args:
	 * 	src (u8 *): Pointer to primary background data
	 * 	lightnum (s16): Light number in the background light array
	 * 	dx (s16): Offset to apply along x-axis
	 * 	dy (s16): Offset to apply along y-axis
	 * 	dz (s16): Offset to apply along z-axis
	 * 	index (s16): Bbox index to patch, 4 will patch all bboxes
	 */
	// lights are stored in 34 byte (0x22) blocks with bbox at offset 0xa
	s16 *light_bbox = (s16 *)&src[lightnum * 0x22 + 0xa];

	s16 istart = index == 4 ? 0 : index;
	s16 istop = index == 4 ? 4 : index + 1;

	for (s16 i = istart;  i < istop; i++) {
		light_bbox[3 * i + 0] = PD_BE16(PD_BE16(light_bbox[3 * i + 0]) + dx); // x
		light_bbox[3 * i + 1] = PD_BE16(PD_BE16(light_bbox[3 * i + 1]) + dy); // y
		light_bbox[3 * i + 2] = PD_BE16(PD_BE16(light_bbox[3 * i + 2]) + dz); // z
	}
}

s32 patchRoomGdl(u8 *src, u8 *dst, u32 room_ptr, u32 cmdnum, u8 cmdbyte)
{
	/**
	 * Method to patch gdl by splitting the opaque display list
	 * into opaque and transparent background passes. This is
	 * used to avoid light blockage in the Dr. Caroll cutscene
	 * on PC when the camera goes out-of-bounds at the end of
	 * the investigation level.
	 *
	 * Args:
	 * 	src (u8 *): Compressed room data
	 * 	dst (u8 *): Recompressed room data after patch
	 *      room_ptr (u32): ROM pointer to start of compressed room data
	 * 	cmdnum (u32): Command number to patch within the graphics display list
	 * 	cmdbyte (u8): Starting byte of the command (for verification)
	 *
	 * Returns:
	 *      (s32): size of recompressed room data
	 */
	// 32k should be enough for bg_ear.seg, may need to adjust
	// in the future if there are larger patches
	u16 scratchsize = 32768;

	// unzip room data
	u8 *unzip = (u8 *)sysMemZeroAlloc(scratchsize);
	s32 size = rzipInflate(src, unzip, NULL);

	// shift data by 16 bytes to create space for a new xlu roomblock
	s32 shift = 16;
	for (u32 i = size - 1; i >= 48; i--)
		unzip[i + shift] = unzip[i];

	u32 *ptrs = (u32 *)unzip;

	// update roomgfxdata
	ptrs[0] = PD_BE32(PD_BE32(ptrs[0]) + shift); // start of vertices
	ptrs[1] = PD_BE32(PD_BE32(ptrs[1]) + shift); // start of colors
	ptrs[3] = PD_BE32(PD_BE32(ptrs[2]) + 20);    // new xlu roomblock starting after 20 byte opa roomblock

	// update opa roomblock
	u32 opa_gdl_ptr = PD_BE32(ptrs[8]) + shift;
	ptrs[8] = PD_BE32(opa_gdl_ptr);              // start of gdl for opa
	ptrs[9] = ptrs[0];                           // start of vertices
	ptrs[10] = ptrs[1];                          // start of colors

	// create xlu roomblock
	u32 xlu_gdl_ptr = opa_gdl_ptr + 8 * cmdnum + sizeof(dl_end);
	ptrs[11] = 0;                                // block type (leaf)
	ptrs[12] = 0;                                // next pointer (null)
	ptrs[13] = PD_BE32(xlu_gdl_ptr);             // start of gdl for xlu
	ptrs[14] = ptrs[0];                          // start of vertices
	ptrs[15] = ptrs[1];                          // start of colors

	// verify split position matches current ROM, return -1 if check fails
	u32 split_pos = opa_gdl_ptr - room_ptr + 8 * cmdnum;
	if (unzip[split_pos] != cmdbyte) {
		sysMemFree(unzip);
		return -1;
	}

	// split gdl commands into opa and xlu halves
	for (u32 i = size + shift - 1; i >= split_pos; i--)
		unzip[i + sizeof(dl_begin) + sizeof(dl_end)] = unzip[i];

	// insert end of opa commands
	for (u32 i = 0; i < sizeof(dl_end); i++)
		unzip[split_pos + i] = dl_end[i];

	// insert beginning of xlu commands
	for (u32 i = 0; i < sizeof(dl_begin); i++)
		unzip[split_pos + sizeof(dl_end) + i] = dl_begin[i];

	s32 new_size = size + shift + sizeof(dl_begin) + sizeof(dl_end);

	// compress to final destination
	s32 zipsize = patchDeflate1173(unzip, new_size, dst, new_size);

	sysMemFree(unzip);

	return zipsize;
}

s32 patchRoomPtr(u8 *src, u8 *dst, s32 shift)
{
	/**
	 * Method to update pointer values within the roomgfxdata
	 * and roomblock sections of roomdata.
	 *
	 * Args:
	 * 	src (u8 *): Compressed room data
	 * 	dst (u8 *): Recompressed room data after patch
	 * 	shift (s32): Relative shift applied to pointer values
	 *
	 * Returns:
	 *      (s32): size of recompressed room data
	 */
	// 32k should be enough for bg_ear.seg, may need to adjust
	// in the future if there are larger patches
	u16 scratchsize = 32768;

	// unzip room data
	u8 *unzip = (u8 *)sysMemZeroAlloc(scratchsize);
	s32 size = rzipInflate(src, unzip, NULL);

	u32 *ptrs = (u32 *)unzip;

	// update roomgfxdata
	for (u32 i = 0; i < 4; i++)
		if (ptrs[i] > 0) ptrs[i] = PD_BE32(PD_BE32(ptrs[i]) + shift);

	// update opa roomblock
	for (u32 i = 7; i < 11; i++)
		if (ptrs[i] > 0) ptrs[i] = PD_BE32(PD_BE32(ptrs[i]) + shift);

	// update xlu roomblock
	for (u32 i = 12; i < 16; i++)
		if (ptrs[3] > 0 && ptrs[i] > 0) ptrs[i] = PD_BE32(PD_BE32(ptrs[i]) + shift);

	// compress to final destination
	s32 zipsize = patchDeflate1173(unzip, size, dst, size);

	sysMemFree(unzip);

	return zipsize;
}

void patchBE32(u8 *src, u32 pos, u32 val)
{
	/**
	 * Update unsigned 32 bit int at a given position in src
	 *
	 * Args:
	 * 	src (u8 *): Pointer to source data
	 * 	pos (u32): Start position of 32 bit int in src
	 * 	val (u32): The updated value
	 */
	for (u32 i = 0; i < 4; i++) {
		src[pos + i] = (val >> 8 * (3 - i)) & 0xff;
	}
}

void patchSetupFile(const char * name, u32 numpatches, struct patchbytes *patches)
{
	/**
	 * Method to apply a series of byte patches to setup file.
	 *
	 * Args:
	 *     name (const char *): ROM file name to patch
	 *     numpatches (u32): Number of patches to apply
	 *     patches (u32): Patch positions and byte values
	 */
	sysLogPrintf(LOG_NOTE, " %s", name);

	// patch for patched file
	char path[FS_MAXPATH + 1];
	sprintf(path, "%s/%s", PATCH_FILES_DIR, name);

	// get file info
	s32 fileid = romdataFileGetNumForName(name);
	u8 *data = romdataFileGetData(fileid);
	u32 size = ((u32)data[2] << 16) | ((u32)data[3] << 8) | (u32)data[4];

	// skip patch construction if file exists
	if (fsFileSize(path) > 0) {
		romdataFileFree(fileid);
		return;
	}

	// inflate
	u8 *unzip = (u8 *)sysMemZeroAlloc(size);
	rzipInflate(data, unzip, NULL);

	// apply patches
	for (u32 i = 0; i < numpatches; ++i) {
		const struct patchbytes *p = &patches[i];
		if (!memcmp(unzip + p->ofs, p->src, p->len)) {
			memcpy(unzip + p->ofs, p->dst, p->len);
		}
	}

	// deflate
	u8 *zip = (u8 *)sysMemZeroAlloc(size);
	s32 zipsize = patchDeflate1173(unzip, size, zip, size);

	// write zipped file
	if (zipsize > 0) {
		FILE *file = fsFileOpenWrite(path);
		fwrite(zip, 1, zipsize, file);
		fsFileFree(file);
	} else sysLogPrintf(LOG_WARNING, "patch failed");

	// clean up memory
	sysMemFree(unzip);
	sysMemFree(zip);

	// free rom file location so we can load it externally as a patch
	romdataFileFree(fileid);
}

void patchBgFile(const char * name, u32 numpatches, struct patchlights *patches, struct patchroom *patchroom)
{
	/**
	 * Method to patch background file with light and room data.
	 *
	 * Args:
	 *     name (const char *): ROM file name to patch
	 *     patches (struct patchlights *): Pointer to list of light patches
	 *     patchroom (struct patchroom *): Pointer to room gdl patch (only for bg_ear.seg)
	 */
	sysLogPrintf(LOG_NOTE, " %s", name);

	// path for patched file
	char path[FS_MAXPATH + 1];
	sprintf(path, "%s/%s", PATCH_FILES_DIR, name);

	// get file info
	s32 fileid = romdataFileGetNumForName(name);
	s32 size = romdataFileGetSize(fileid);

	// skip patch construction if file exists
	if (fsFileSize(path) > 0) {
		romdataFileFree(fileid);
		return;
	}

	// load file
	u8 *data = romdataFileGetData(fileid);

	// retrieve sizes from file header (first 12 bytes)
	u32 primsize = PD_BE32(*(u32 *)&data[0x0]);  // inflated primary data size
	u32 sect1size = PD_BE32(*(u32 *)&data[0x4]); // full size of section 1, which contains primary data
	u32 zipsize = PD_BE32(*(u32 *)&data[0x8]);   // compressed primary data size

	// uncompress the primary background data
	u8 *primary = (u8 *)sysMemZeroAlloc(primsize);
	rzipInflate(data + 0xc, primary, NULL); // 1173 compressed data starts after 12 byte header

	// patch lights
	u32 light_offset = PD_BE32(*(u32 *)&primary[0x10]) - 0x0f000000;
	for (u32 i = 0; i < numpatches; ++i) {
		const struct patchlights *p = &patches[i];
		if (p->type == 0)
			patchLightBbox(primary + light_offset, p->lightnum, p->dx, p->dy, p->dz, p->index);
		else if (p->type == 1)
			patchLightDir(primary + light_offset, p->lightnum, p->dx, p->dy, p->dz);
	}

	// patch room to avoid line-of-sight conflicts
	// Note: currently only applies to Dr. Carroll cutscene in the Investigation stage
	u8 *roomdata = NULL;
	s32 length = 0;
	s32 new_length = 0;
	u32 patch_start = 0;
	if (patchroom != NULL) {
		u32 i = patchroom->roomnum;

		// original room data
		u32 room_offset = PD_BE32(*(u32 *)&primary[0x4]) - 0x0f000000;
		u32 room_start = PD_BE32(*(u32 *)&primary[room_offset + 0x14 * i++]);
		u32 room_stop = PD_BE32(*(u32 *)&primary[room_offset + 0x14 * i]);
		s32 room_length = (room_stop - room_start);

		// starting point relative to beginning of zipped data
		patch_start = room_start - 0x0f000000 - primsize + zipsize + 0xc;

		// patch room gdl commands
		roomdata = (u8 *)sysMemZeroAlloc(sect1size);
		s32 new_room_length = patchRoomGdl(data + patch_start, roomdata, room_start, patchroom->cmdnum, patchroom->cmdbyte);

		// stop patch if gdl update fails
		if (new_room_length < 0) {
			sysLogPrintf(LOG_WARNING, "patch failed");
			sysMemFree(primary);
			sysMemFree(roomdata);
			return;
		}

		// update room stop
		u32 new_room_stop = room_start + new_room_length;
		patchBE32(primary, room_offset + 0x14 * i, new_room_stop);

		// update cumulative totals
		length += room_length;
		new_length += new_room_length;

		// update rooms following the patched room
		while (true) {
			// original room data for next room
			room_start += room_length;
			room_stop = PD_BE32(*(u32 *)&primary[room_offset + 0x14 * ++i]);
			room_length = (room_stop - room_start);

			// break on null pointer indicating end of room list
			if (room_stop == 0) break;

			// updated room length with patched pointers
			new_room_length = patchRoomPtr(data + patch_start + length, roomdata + new_length, new_length - length);

			// stop patch if pointer update fails
			if (new_room_length < 0) {
				sysLogPrintf(LOG_WARNING, "patch failed");
				sysMemFree(primary);
				sysMemFree(roomdata);
				return;
			}

			// update room stop
			new_room_stop += new_room_length;
			patchBE32(primary, room_offset + 0x14 * i, new_room_stop);

			// update cumulative changes
			length += room_length;
			new_length += new_room_length;
		}
		// update section 1 size
		sect1size += new_length - length;
	}

	// recompress
	u8 *rezip = (u8 *)sysMemZeroAlloc(primsize);
	s32 rezipsize = patchDeflate1173(primary, primsize, rezip, primsize);

	// stop patch if compression fails
	if (rezipsize < 0) {
		sysLogPrintf(LOG_WARNING, "patch failed");
		sysMemFree(primary);
		sysMemFree(rezip);
		sysMemFree(roomdata);
		return;
	}

	// write patched file
	FILE *file = fsFileOpenWrite(path);
	u32 tmp = PD_BE32(primsize);
	fwrite(&tmp, 4, 1, file);
	tmp = PD_BE32(sect1size - zipsize + rezipsize);
	fwrite(&tmp, 4, 1, file);
	tmp = PD_BE32(rezipsize);
	fwrite(&tmp, 4, 1, file);
	fwrite(rezip, 1, rezipsize, file);
	if (new_length == 0) {
		fwrite(data + 0xc + zipsize, 1, size - 0xc - zipsize, file);
	} else {
		fwrite(data + 0xc + zipsize, 1, patch_start - zipsize - 0xc, file);
		fwrite(roomdata, 1, new_length, file);
		fwrite(data + patch_start + length, 1, size - patch_start - length, file);
		sysMemFree(roomdata);
	}

	// free up memory
	fsFileFree(file);
	sysMemFree(primary);
	sysMemFree(rezip);

	// free rom file location so we can load it externally as a patch
	romdataFileFree(fileid);
}

void patchInit(void)
{
	/**
	 * Method to create automatic ROM patches on PC.
	 *
	 * Current patches include:
	 *     1. Adjusting lights in Defection level helipad to obtain correct brightness
	 *        and fixing a broken light by the DataDyne statue on the bottom floor
	 *     2. Adjusting lights in Air Base level along the diagonal lift
	 *     3. Adjusting lights in Investigation level to restore missing lights
	 *     4. Adjusting lights in Villa level to restore missing lights
	 *     5. Removing repeated dialog in Infiltration outro
	 */
	if (fsGetModDir()) {
		sysLogPrintf(LOG_NOTE, "skipping auto patches because --moddir is set");
		return;
	} else if (sysArgCheck("--no-patches")) {
		return;
	}
	sysLogPrintf(LOG_NOTE, "applying patches:");

	static char path[FS_MAXPATH + 1];

	// create patch directory as needed
	if (fsFileSize(PATCH_DIR) < 0)
		fsCreateDir(PATCH_DIR);

	// generate mod config for patches
	if (fsFileSize(PATCH_CONFIG) < 0) {
		FILE *f = fsFileOpenWrite(PATCH_CONFIG);
		fprintf(f, "stage 0x22 { bgfile bgdata/bg_ame.seg }\n");
		fprintf(f, "stage 0x27 { bgfile bgdata/bg_cave.seg }\n");
		fprintf(f, "stage 0x2C { bgfile bgdata/bg_eld.seg }\n");
		fprintf(f, "stage 0x2F { setupfile UsetuplueZ }\n");
		fprintf(f, "stage 0x30 { bgfile bgdata/bg_ame.seg }\n");
		fprintf(f, "stage 0x33 { bgfile bgdata/bg_ear.seg }\n");
		fsFileFree(f);
	}

	// create directory for background files
	if (fsFileSize(PATCH_FILES_DIR) < 0)
		fsCreateDir(PATCH_FILES_DIR);
	if (fsFileSize(PATCH_BGDATA_DIR) < 0)
		fsCreateDir(PATCH_BGDATA_DIR);

	// create patched background files
	struct patchlights ame[] = {
		{ 0,  7,  1,   0, -1, 4 }, // helipad light near top of stairs
		{ 0,  8, -1,   0,  1, 4 }, // helipad light left of datadyne sign
		{ 0, 18, 16,   0, 15, 1 }, // light to left of datadyne statue on bottom floor (position patch)
		{ 1, 18,  0, 254,  0, 1 }, // light to left of datadyne statue on bottom floor (direction patch)
	};
	patchBgFile("bgdata/bg_ame.seg", 4, ame, NULL);
	struct patchlights cave[] = {
		{ 0,  4,  0, -1,  0, 4 }, // 12th yellow light from base of diagonal elevator
		{ 0,  5,  0, -1,  0, 4 }, // 11th yellow light from base of diagonal elevator
		{ 0,  6,  0, -1,  0, 4 }, // 10th yellow light from base of diagonal elevator
		{ 0,  7,  0, -1,  0, 4 }, //  9th yellow light from base of diagonal elevator
		{ 0,  8,  0, -1,  0, 4 }, //  8th yellow light from base of diagonal elevator
		{ 0,  9,  0, -1,  0, 4 }, //  7th yellow light from base of diagonal elevator
		{ 0, 10,  0, -1,  0, 4 }, //  6th yellow light from base of diagonal elevator
		{ 0, 11,  0, -1,  0, 4 }, //  5th yellow light from base of diagonal elevator
		{ 0, 12,  0, -1,  0, 4 }, //  4th yellow light from base of diagonal elevator
		{ 0, 13,  0, -1,  0, 4 }, //  3rd yellow light from base of diagonal elevator
		{ 0, 14,  0, -1,  0, 4 }, //  2nd yellow light from base of diagonal elevator
		{ 0, 15,  0, -1,  0, 4 }, //  1st yellow light at bottom of diagonal evelator
		{ 0, 45,  0, -1,  0, 4 }, // 20th yellow light at top of diagonal elevator
		{ 0, 46,  0, -1,  0, 4 }, // 19th yellow light from base of diagonal elevator
		{ 0, 47,  0, -1,  0, 4 }, // 18th yellow light from base of diagonal elevator
		{ 0, 48,  0, -1,  0, 4 }, // 17th yellow light from base of diagonal elevator
		{ 0, 49,  0, -1,  0, 4 }, // 14th yellow light from base of diagonal elevator
		{ 0, 50,  0, -1,  0, 4 }, // 15th yellow light from base of diagonal elevator
		{ 0, 51,  0, -1,  0, 4 }, // 16th yellow light from base of diagonal elevator
		{ 0, 52,  0, -1,  0, 4 }, // 13th yellow light from base of diagonal elevator
	};
	patchBgFile("bgdata/bg_cave.seg", 20, cave, NULL);
	struct patchlights eld[] = {
		{ 0, 20, -1,  0,  0, 4 }, // last pair of lights in tunnel near villa
		{ 0, 21, -1,  0,  0, 4 }, // last pair of lights in tunnel near villa
		{ 0, 22, -1,  0,  0, 4 }, // third to last light in tunnel near villa
		{ 0, 30,  0,  0, -1, 4 }, // first tunnel light near observatory
	};
	patchBgFile("bgdata/bg_eld.seg", 4, eld, NULL);
	struct patchlights ear[] = {
		{ 0, 11,  0, -1,  0, 4 }, // light on left side of isotope chamber
		{ 0, 24,  0, -1,  0, 4 }, // pair of lights in first lab hallway
		{ 0, 25,  0, -1,  0, 4 }, // pair of lights in first lab hallway
		{ 0, 32,  0, -1,  0, 4 }, // corner light in lab with elevator platform
		{ 0, 33,  0, -1,  0, 4 }, // light over the elevator platform
		{ 0, 38,  0, -1,  0, 4 }, // corner light in lab with nightvision goggles
		{ 0, 55,  0, -1,  0, 4 }, // light at end of lab hallway just before final door
		{ 0, 57,  0, -1,  0, 4 }, // light between double doors before lasers
		{ 0, 58,  0, -1,  0, 4 }, // first light in laser hall
		{ 0, 59,  0, -1,  0, 4 }, // second light in laser hall
		{ 0, 60,  0, -1,  0, 4 }, // third light in laser hall
		{ 0, 61,  0, -1,  0, 4 }, // last light in laser hall
		{ 0, 65,  0,  0,  1, 4 }, // light in glass container with plants
		{ 0, 95,  0, -1,  0, 4 }, // light by second minigun in final hallway
	};
	struct patchroom drcaroll = {100, 77, 0xbb}; // render half of room as transparent layer
	                                             // to avoid blocking lights on PC when camera
	                                             // goes out-of-bounds during Dr. Caroll cutscene
	patchBgFile("bgdata/bg_ear.seg", 14, ear, &drcaroll);

	// create patched setup files
	struct patchbytes lue[] = {
		/* fixes Jon's double "if what" in Infiltration outro */
		{ 0x92a2, 1, "\x6c", "\x99" },
		{ 0x92b0, 1, "\x6c", "\x99" },
	};
	patchSetupFile("UsetuplueZ", 2, lue);

	// set the mod directory
	fsSetModDir(fsFullPath(PATCH_DIR));
}
