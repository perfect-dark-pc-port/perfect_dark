#ifndef _IN_WEBCAM_H
#define _IN_WEBCAM_H

#include <PR/ultratypes.h>

/**
 * OS camera input for the virtual Game Boy Camera (PerfectHead).
 *
 * Uses the camera API of SDL3 (Media Foundation on Windows, AVFoundation on
 * macOS, V4L2/PipeWire on Linux), loaded at run time so the port keeps
 * building and running against SDL2 alone.
 */

#define WEBCAM_OK          1  // dst holds a frame
#define WEBCAM_WAITING     0  // opening or waiting for the user's permission
#define WEBCAM_UNAVAILABLE -1 // no SDL3, no camera, or permission denied

/**
 * Fill dst with a width * height 8-bit luma image (0 = black), cropped from
 * the middle of the camera image to the same aspect ratio and scaled down.
 * The camera is opened on the first call and closed again a few seconds
 * after the last one.
 */
s32 webcamGetFrame(u8 *dst, s32 width, s32 height);

// Close the camera if it is open (at exit)
void webcamShutdown(void);

#endif
