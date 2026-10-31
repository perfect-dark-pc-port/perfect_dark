#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <PR/os_internal.h>
#include <PR/rcp.h>
#include "platform.h"
#include "system.h"
#include "input.h"
#include "video.h"
#include "audio.h"
#include "fs.h"
#include "gbcam.h"
#include "mempak.h"

#define EEPROM_SIZE (EEP16K_MAXBLOCKS * 8)
#define EEPROM_FNAME "eeprom.bin"
#define EEPROM_PATH "$S/" EEPROM_FNAME

#define OS_COUNTER_RATE 46875000ULL
#define OS_COUNTER_NUM (OS_COUNTER_RATE / 1000ULL)
#define OS_COUNTER_DEN (1000000ULL / 1000ULL)

u64 osClockRate = OS_CLOCK_RATE;
u32 osMemSize = 16 * 1024 * 1024; /* expansion pak installed plus some extra */
s32 osTvType = OS_TV_NTSC;        /* 0 = PAL, 1 = NTSC, 2 = MPAL */
s32 osResetType = 0;              /* 0 = cold reset */
s32 osViClock = VI_NTSC_CLOCK;

static u8 eeprom[EEPROM_SIZE];
static char eepromPath[FS_MAXPATH + 1];
static s32 eepromLoaded = 0;

/* Time */

OSTime osGetTime(void)
{
	// u64 should be enough to last a while
	return (sysGetMicroseconds() * OS_COUNTER_NUM) / OS_COUNTER_DEN;
}

u32 osGetCount(void)
{
	return (u32)osGetTime();
}

/* Thread */

void osCreateThread(OSThread *thrd, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p)
{
	thrd->id = id;
	thrd->state = OS_STATE_STOPPED;
}

void osDestroyThread(OSThread *thrd)
{
	thrd->id = 0;
	thrd->state = 0;
}

void osYieldThread(void)
{

}

void osStartThread(OSThread *thrd)
{
	thrd->state = OS_STATE_RUNNING;
}

void osStopThread(OSThread *thrd)
{
	thrd->state = OS_STATE_STOPPED;
}

OSPri osGetThreadPri(OSThread *thrd)
{
	return 0;
}

void osSetThreadPri(OSThread *thrd, OSPri pri)
{

}

/* Mesg */

void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msgBuf, s32 count)
{
	mq->validCount = 0;
	mq->first = 0;
	mq->msgCount = count;
	mq->msg = msgBuf;
}

void osSetEventMesg(OSEvent e, OSMesgQueue *mq, OSMesg msg)
{

}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag)
{
	return 0;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag)
{
	return 0;
}

/* Vi */

void osCreateViManager(OSPri pri)
{

}

void osViSetMode(OSViMode *mode)
{

}

void osViSetEvent(OSMesgQueue *mq, OSMesg msg, u32 retraceCount)
{

}

void osViBlack(u8 active)
{
	if (active) {
		videoClearScreen();
	}
}

void osViSetSpecialFeatures(u32 func)
{

}

void osViSwapBuffer(void *vaddr)
{

}

void osViSetXScale(f32 value)
{

}

void osViSetYScale(f32 value)
{

}

/* Ai */

s32 osAiSetFrequency(u32 frequency)
{
	// we can allow for any freq
	return (s32)frequency;
}

u32 osAiGetLength(void)
{
	return audioGetBytesBuffered();
}

s32 osAiSetNextBuffer(void *bufPtr, u32 size)
{
	audioSetNextBuffer(bufPtr, size);
	return 0;
}

/* Cont */

s32 osContInit(OSMesgQueue *mesgq, u8 *bitpattern, OSContStatus *data)
{
	if (bitpattern) {
		*bitpattern = inputControllerMask();
	}
	if (data) {
		osContGetQuery(data);
	}
	return 0;
}

s32 osContStartReadData(OSMesgQueue *mesgq)
{
	osSendMesg(mesgq, (OSMesg)0, OS_MESG_NOBLOCK);
	return 0;
}

void osContGetReadData(OSContPad *pad)
{
	// game always passes in an array of 4 OSContPads
	for (s32 i = 0; i < MAXCONTROLLERS; ++i, ++pad) {
		pad->button = 0;
		pad->stick_x = 0;
		pad->stick_y = 0;
		pad->rstick_x = 0;
		pad->rstick_y = 0;
		if (inputReadController(i, pad) < 0) {
			pad->errnum = CONT_NO_RESPONSE_ERROR;
		} else {
			pad->errnum = 0;
		}
	}
}

s32 osContStartQuery(OSMesgQueue *mq)
{
	return 0;
}

void osContGetQuery(OSContStatus *status)
{
	// also always 4 status structs here
	for (s32 i = 0; i < MAXCONTROLLERS; ++i, ++status) {
		if (inputControllerConnected(i)) {
			status->errnum = 0;
			status->type = CONT_ABSOLUTE;
			status->status = CONT_CARD_ON;
		} else {
			status->errnum = CONT_NO_RESPONSE_ERROR;
			status->type = 0;
			status->status = 0;
		}
	}
}

/* Motor */

s32 osMotorProbe(OSMesgQueue *ctrlrqueue, OSPfs* pfs, s32 channel)
{
	if (channel == gbcamGetChannel() || mempakIsPlugged(channel)) {
		// a Transfer Pak or Controller Pak answers the rumble probe with a device error
		return PFS_ERR_DEVICE;
	}

	if (pfs && inputRumbleSupported(channel)) {
		pfs->queue = ctrlrqueue;
		pfs->channel = channel;
		pfs->activebank = 0xff;
		pfs->status = PFS_MOTOR_INITIALIZED;
		return 0;
	}
	return PFS_ERR_NOPACK;
}

s32 __osMotorAccess(OSPfs *pfs, s32 cmd)
{
	if (!pfs || pfs->channel < 0 || pfs->channel >= INPUT_MAX_CONTROLLERS) {
		return PFS_ERR_NOPACK;
	}

	const f32 strength = (f32)(cmd == MOTOR_START);
	inputRumble(pfs->channel, strength, 5.f); // hope someone turns it off in those 5 seconds

	return 0;
}

/* Eeprom */

static inline void osEepromSetPath(void)
{
	const char *extPath = sysArgGetString("--eeprom-file");
	if (extPath && extPath[0]) {
		if (extPath[0] == '$' || fsPathIsAbsolute(extPath) || fsPathIsCwdRelative(extPath)) {
			strncpy(eepromPath, extPath, FS_MAXPATH);
		} else {
			// just a filename, look for it in the save dir
			snprintf(eepromPath, FS_MAXPATH, "$S/%s", extPath);
		}
	} else {
		strncpy(eepromPath, EEPROM_PATH, FS_MAXPATH);
	}
}

static inline void osEeepromLoad(const char *fname)
{
	if (!eepromLoaded) {
		eepromLoaded = 1;
		FILE *fp = fsFileOpenRead(fname);
		if (fp) {
			fread(eeprom, 1, EEPROM_SIZE, fp);
			fsFileFree(fp);
		} else {
			sysLogPrintf(LOG_NOTE, "could not read EEPROM from `%s`: %s", fsFullPath(fname), strerror(errno));
		}
	}
}

static inline void osEeepromSave(const char *fname)
{
	FILE* fp = fsFileOpenWrite(fname);
	if (fp) {
		fwrite(eeprom, 1, EEPROM_SIZE, fp);
		fsFileFree(fp);
	} else {
		sysLogPrintf(LOG_ERROR, "could not save EEPROM to `%s`: %s", fsFullPath(fname), strerror(errno));
	}
}

s32 osEepromProbe(OSMesgQueue *mq)
{
	return EEPROM_TYPE_16K;
}

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(buffer, eeprom + address * 8, nbytes);

	return 0;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(eeprom + address * 8, buffer, nbytes);

	osEeepromSave(eepromPath);

	return 0;
}

/* Pfs */

s32 osPfsIsPlug(OSMesgQueue *queue, u8 *pattern)
{
	if (pattern) {
		*pattern = 0;
		for (s32 i = 0; i < MAXCONTROLLERS; ++i) {
			if (inputRumbleSupported(i) || mempakIsPlugged(i) || (i == gbcamGetChannel() && inputControllerConnected(i))) {
				*pattern |= 1 << i;
			}
		}
	}
	return 0;
}

s32 osPfsInitPak(OSMesgQueue *queue, OSPfs *pfs, s32 channel, s32 *arg3)
{
	// if rumble is supported or the virtual Game Boy Camera is plugged in here,
	// indicate that we have a different kind of pak instead
	if (channel == gbcamGetChannel()) {
		return PFS_ERR_DEVICE;
	}
	if (mempakIsPlugged(channel)) {
		return mempakInit(queue, pfs, channel);
	}
	return inputRumbleSupported(channel) ? PFS_ERR_DEVICE : PFS_ERR_NOPACK;
}

/* Controller Pak file system: only the virtual Controller Pak is emulated, see mempak.c */

#define PFS_IS_MEMPAK(pfs) ((pfs) && mempakIsPlugged((pfs)->channel))

s32 osPfsChecker(OSPfs *pfs)
{
	return PFS_IS_MEMPAK(pfs) ? mempakChecker(pfs) : PFS_ERR_NOPACK;
}

s32 osPfsFreeBlocks(OSPfs *pfs, s32 *remaining)
{
	return PFS_IS_MEMPAK(pfs) ? mempakFreeBlocks(pfs, remaining) : PFS_ERR_NOPACK;
}

s32 osPfsNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used)
{
	return PFS_IS_MEMPAK(pfs) ? mempakNumFiles(pfs, max_files, files_used) : PFS_ERR_NOPACK;
}

s32 osPfsAllocateFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int num_bytes, s32 *file_no)
{
	return PFS_IS_MEMPAK(pfs) ? mempakAllocateFile(pfs, company_code, game_code, game_name, ext_name, num_bytes, file_no) : PFS_ERR_NOPACK;
}

s32 osPfsFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, s32 *file_no)
{
	return PFS_IS_MEMPAK(pfs) ? mempakFindFile(pfs, company_code, game_code, game_name, ext_name, file_no) : PFS_ERR_NOPACK;
}

s32 osPfsDeleteFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name)
{
	return PFS_IS_MEMPAK(pfs) ? mempakDeleteFile(pfs, company_code, game_code, game_name, ext_name) : PFS_ERR_NOPACK;
}

s32 osPfsReSizeFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int length)
{
	return PFS_IS_MEMPAK(pfs) ? mempakReSizeFile(pfs, company_code, game_code, game_name, ext_name, length) : PFS_ERR_NOPACK;
}

s32 osPfsFileState(OSPfs *pfs, s32 fileNo, OSPfsState *state)
{
	return PFS_IS_MEMPAK(pfs) ? mempakFileState(pfs, fileNo, state) : PFS_ERR_NOPACK;
}

s32 osPfsReadWriteFile(OSPfs* pfs, s32 fileNo, u8 flag, int offset, int size, u8* data)
{
	return PFS_IS_MEMPAK(pfs) ? mempakReadWriteFile(pfs, fileNo, flag, offset, size, data) : PFS_ERR_NOPACK;
}

/* Gbpak: only the virtual Game Boy Camera (PerfectHead) is emulated, see gbcam.c */

static inline s32 osGbpakIsCamera(OSPfs *pfs)
{
	return pfs && pfs->channel >= 0 && pfs->channel == gbcamGetChannel();
}

s32 osGbpakInit(OSMesgQueue *queue, OSPfs *pfs, s32 ch)
{
	if (pfs && ch >= 0 && ch == gbcamGetChannel()) {
		sysLogPrintf(LOG_NOTE, "gbpak: Transfer Pak with virtual Game Boy Camera on controller %d", ch + 1);
		pfs->queue = queue;
		pfs->channel = ch;
		pfs->status = 0;
		pfs->activebank = 0xff;
		return 0;
	}
	return PFS_ERR_NOPACK;
}

s32 osGbpakPower(OSPfs *pfs, s32 flag)
{
	return osGbpakIsCamera(pfs) ? 0 : PFS_ERR_NOPACK;
}

s32 osGbpakReadWrite(OSPfs *pfs, u16 flag, u16 addr, u8 *buf, u16 size)
{
	if (!osGbpakIsCamera(pfs)) {
		return PFS_ERR_NOPACK;
	}
	return (flag == PFS_WRITE) ? gbcamWrite(addr, buf, size) : gbcamRead(addr, buf, size);
}

s32 osGbpakReadId(OSPfs *pfs, OSGbpakId *id, u8 *status)
{
	if (!osGbpakIsCamera(pfs)) {
		return PFS_ERR_NOPACK;
	}
	if (id) {
		gbcamGetId(id);
	}
	if (status) {
		*status = OS_GBPAK_GBCART_ON | OS_GBPAK_POWER;
	}
	return 0;
}

/* Misc */

void osWritebackDCacheAll(void)
{

}

void osWritebackDCache(void *a, s32 b)
{

}

void osInvalDCache(void *a, s32 b)
{

}

s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, uintptr_t devAddr, void *vAddr, u32 nbytes, OSMesgQueue *mq)
{
	memcpy(vAddr, (const void *)devAddr, nbytes);
	return 0;
}

s32 osPiReadIo(u32 devaddr, u32 *data)
{
	return 0;
}

uintptr_t osVirtualToPhysical(void *addr)
{
	return (uintptr_t)addr;
}

u32 osGetMemSize(void)
{
	return osMemSize;
}

OSIntMask osGetIntMask(void)
{
	return 0;
}

OSIntMask osSetIntMask(OSIntMask mask)
{
	return 0;
}

/* libc compatibility wrappers */

#ifndef PLATFORM_OSX

void bzero(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

void bcopy(const void *src, void *dst, size_t n)
{
	memcpy(dst, src, n);
}

s32 bcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}

#endif
