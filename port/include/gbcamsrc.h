#ifndef _IN_GBCAMSRC_H
#define _IN_GBCAMSRC_H

#include <PR/ultratypes.h>

#define GBCAMSRC_WIDTH  128
#define GBCAMSRC_HEIGHT 112

/**
 * Frame source for the virtual Game Boy Camera.
 * Fills dst with GBCAMSRC_WIDTH * GBCAMSRC_HEIGHT 8-bit luma values
 * (0 = black, 255 = white), already cropped and scaled.
 * Returns 1 if a frame was produced.
 */
s32 gbcamsrcGetFrame(u8 *dst);

#endif
