#include <PR/ultratypes.h>
#include "versions.h"
#include "system.h"
#include "platform.h"
#include "config.h"
#include "gbcamsrc.h"
#include "webcam.h"

#if PERFECTHEAD

/**
 * Frame source for the virtual Game Boy Camera.
 *
 * With PerfectHead.CameraSource = 1 (the default) this is the PC's camera
 * (webcam.c) when there is one. Otherwise it is an animated test card: a
 * face-like oval that sways from side to side (so live preview updates are
 * visible), on a vertical gradient, with a row of grey steps along the bottom
 * for checking exposure and the dither matrix.
 */
static s32 gbcamsrcSource = 1; // 0 = test card, 1 = OS camera if available

static void gbcamsrcTestPattern(u8 *dst)
{
	const f32 t = (f32)(sysGetMicroseconds() % 4000000ull) / 4000000.0f; // 4 s loop
	const s32 cx = GBCAMSRC_WIDTH / 2 + (s32)(16.0f * (t < 0.5f ? (4.0f * t - 1.0f) : (3.0f - 4.0f * t)));
	const s32 cy = 50;
	s32 x, y;

	for (y = 0; y < GBCAMSRC_HEIGHT; y++) {
		for (x = 0; x < GBCAMSRC_WIDTH; x++) {
			const s32 dx = x - cx;
			const s32 dy = y - cy;
			s32 v = 40 + y; // background gradient

			// head: oval 30 wide, 38 tall (radii)
			if (dx * dx * 38 * 38 + dy * dy * 30 * 30 <= 30 * 30 * 38 * 38) {
				v = 200;

				// hair
				if (dy <= -22) {
					v = 50;
				}

				// eyebrows
				if (dy >= -15 && dy <= -13 && ((dx >= -16 && dx <= -5) || (dx >= 5 && dx <= 16))) {
					v = 60;
				}

				// eyes
				if ((dy >= -10 && dy <= -4) && ((dx >= -14 && dx <= -6) || (dx >= 6 && dx <= 14))) {
					v = 30;
				}

				// a mark on the viewer's left cheek, to check left/right mapping
				if (dx >= -20 && dx <= -15 && dy >= 6 && dy <= 11) {
					v = 40;
				}

				// nose
				if (dx >= -2 && dx <= 2 && dy >= -2 && dy <= 8) {
					v = 150;
				}

				// mouth
				if (dy >= 16 && dy <= 19 && dx >= -10 && dx <= 10) {
					v = 60;
				}
			}

			// grey steps
			if (y >= GBCAMSRC_HEIGHT - 8) {
				v = (x / 16) * 255 / 7;
			}

			dst[y * GBCAMSRC_WIDTH + x] = v > 255 ? 255 : v;
		}
	}
}

s32 gbcamsrcGetFrame(u8 *dst)
{
	if (gbcamsrcSource == 1) {
		switch (webcamGetFrame(dst, GBCAMSRC_WIDTH, GBCAMSRC_HEIGHT)) {
		case WEBCAM_OK:
			return 1;
		case WEBCAM_WAITING:
			// Opening, or waiting for the user to allow the camera
			return 0;
		}
	}

	gbcamsrcTestPattern(dst);
	return 1;
}

PD_CONSTRUCTOR static void gbcamsrcConfigInit(void)
{
	configRegisterInt("PerfectHead.CameraSource", &gbcamsrcSource, 0, 1);
}

#else

s32 gbcamsrcGetFrame(u8 *dst)
{
	return 0;
}

#endif
