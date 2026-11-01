#ifndef _IN_GBCAM_H
#define _IN_GBCAM_H

#include <PR/ultratypes.h>
#include <PR/os_internal.h>

/**
 * Virtual Game Boy Camera cartridge in a virtual Transfer Pak (PerfectHead).
 *
 * The game talks to the camera through osGbpak* exactly as on N64 hardware.
 * This module emulates the cartridge side: MBC (RAM enable, RAM bank / register
 * select), the camera registers at A000-A035, 128 KiB of battery SRAM and the
 * capture itself (exposure, gain, edge enhancement, dither matrix, 2bpp tiles).
 * Frames come from gbcamsrc.c.
 */

// controller channel the camera is plugged into, or -1 if disabled
s32 gbcamGetChannel(void);

s32 gbcamRead(u16 addr, u8 *buf, u16 size);
s32 gbcamWrite(u16 addr, const u8 *buf, u16 size);
void gbcamGetId(OSGbpakId *id);

#endif
