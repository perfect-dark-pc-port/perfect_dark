#ifndef _IN_MEMPAK_H
#define _IN_MEMPAK_H

#include <PR/ultratypes.h>
#include <PR/os_internal.h>

/**
 * Virtual Controller Pak (32 KiB), saved as a standard .mpk image so the files
 * can be shared with N64 emulators and real paks (via dumps). Enabled per
 * controller with Input.PlayerN.ControllerPak. The osPfs* functions in
 * libultra.c call into this module when the pak's channel has one.
 */

// whether controller channel <channel> has a virtual Controller Pak
s32 mempakIsPlugged(s32 channel);

s32 mempakInit(OSMesgQueue *queue, OSPfs *pfs, s32 channel);
s32 mempakChecker(OSPfs *pfs);
s32 mempakFreeBlocks(OSPfs *pfs, s32 *remaining);
s32 mempakNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used);
s32 mempakAllocateFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int num_bytes, s32 *file_no);
s32 mempakFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, s32 *file_no);
s32 mempakDeleteFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name);
s32 mempakReSizeFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int length);
s32 mempakFileState(OSPfs *pfs, s32 file_no, OSPfsState *state);
s32 mempakReadWriteFile(OSPfs *pfs, s32 file_no, u8 flag, int offset, int size, u8 *data);

#endif
