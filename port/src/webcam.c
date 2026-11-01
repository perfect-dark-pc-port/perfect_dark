#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>
#include <PR/ultratypes.h>
#include "versions.h"
#include "platform.h"
#include "config.h"
#include "system.h"
#include "webcam.h"

#if PERFECTHEAD

/**
 * OS camera input for the virtual Game Boy Camera (PerfectHead).
 *
 * SDL3 has a camera API with a backend for each desktop OS (Media Foundation,
 * AVFoundation, V4L2/PipeWire). The port is built on SDL2, and SDL2 and SDL3
 * can't be linked into the same program (their symbols have the same names),
 * so SDL3 is loaded at run time with SDL_LoadObject and only its camera
 * functions are used, the same way sdl2-compat runs SDL2 programs on SDL3.
 * Without SDL3 (or a camera) the virtual camera keeps its test card.
 *
 * SDL3 delivers frames from its own camera thread and updates the permission
 * state without its event loop being pumped (SDL 3.2, src/camera/SDL_camera.c),
 * so nothing here needs SDL3's events.
 *
 * Config ([PerfectHead]):
 *   CameraDevice  which camera to use, 0 = the first one
 *   CameraMirror  1 = flip left/right, like a mirror (default)
 *   SDL3Library   path to the SDL3 library, if it isn't found by name
 */

/* Just enough of SDL3's types, from SDL 3.2's SDL_camera.h and SDL_surface.h */
typedef Uint32 SDL3_CameraID;
typedef struct SDL3_Camera SDL3_Camera;

typedef struct SDL3_CameraSpec {
	Uint32 format;     // SDL_PixelFormat
	Uint32 colorspace; // SDL_Colorspace
	int width;
	int height;
	int framerate_numerator;
	int framerate_denominator;
} SDL3_CameraSpec;

typedef struct SDL3_Surface {
	Uint32 flags;  // SDL_SurfaceFlags
	Uint32 format; // SDL_PixelFormat
	int w;
	int h;
	int pitch;
	void *pixels;
	int refcount;
	void *reserved;
} SDL3_Surface;

#define SDL3_INIT_CAMERA 0x00010000u

// SDL3 kept SDL2's pixel format values
#define SDL3_PIXELFORMAT_XRGB8888 SDL_PIXELFORMAT_RGB888

#define WEBCAM_IDLE_MS 3000 // close the camera this long after the last frame

static struct {
	void *lib;
	bool (SDLCALL *InitSubSystem)(Uint32 flags);
	const char *(SDLCALL *GetError)(void);
	void (SDLCALL *free)(void *mem);
	SDL3_CameraID *(SDLCALL *GetCameras)(int *count);
	const char *(SDLCALL *GetCameraName)(SDL3_CameraID id);
	SDL3_CameraSpec **(SDLCALL *GetCameraSupportedFormats)(SDL3_CameraID id, int *count);
	SDL3_Camera *(SDLCALL *OpenCamera)(SDL3_CameraID id, const SDL3_CameraSpec *spec);
	int (SDLCALL *GetCameraPermissionState)(SDL3_Camera *camera);
	bool (SDLCALL *GetCameraFormat)(SDL3_Camera *camera, SDL3_CameraSpec *spec);
	SDL3_Surface *(SDLCALL *AcquireCameraFrame)(SDL3_Camera *camera, Uint64 *timestamp);
	void (SDLCALL *ReleaseCameraFrame)(SDL3_Camera *camera, SDL3_Surface *frame);
	void (SDLCALL *CloseCamera)(SDL3_Camera *camera);
	const char *(SDLCALL *GetCurrentCameraDriver)(void);
	SDL3_Surface *(SDLCALL *ConvertSurface)(SDL3_Surface *surface, Uint32 format);
	void (SDLCALL *DestroySurface)(SDL3_Surface *surface);
} sdl3;

static s32 webcamDevice = 0;
static s32 webcamMirror = 1;
static char webcamLibPath[512] = "";

static s32 webcamState = 0; // 0 = SDL3 not loaded yet, 1 = loaded, -1 = unavailable
static SDL3_Camera *webcamCamera = NULL;
static bool webcamDenied = false;
static SDL_mutex *webcamLock = NULL;
static SDL_TimerID webcamTimer = 0;
static u32 webcamLastUse = 0;

static u8 *webcamCache = NULL; // the last frame, for when no new one has arrived
static s32 webcamCacheW = 0;
static s32 webcamCacheH = 0;
static bool webcamCacheValid = false;

static bool webcamLoadSdl3(void)
{
	static const char *names[] = {
#if defined(_WIN32)
		"SDL3.dll",
#elif defined(__APPLE__)
		"libSDL3.0.dylib",
		"libSDL3.dylib",
		"SDL3.framework/SDL3",
#else
		"libSDL3.so.0",
		"libSDL3.so",
#endif
		NULL,
	};
	s32 i;

	if (webcamLibPath[0]) {
		sdl3.lib = SDL_LoadObject(webcamLibPath);
	}

	for (i = 0; !sdl3.lib && names[i]; i++) {
		sdl3.lib = SDL_LoadObject(names[i]);
	}

	if (!sdl3.lib) {
		sysLogPrintf(LOG_NOTE, "webcam: SDL3 not found, using the test card (install SDL3 or set PerfectHead.SDL3Library)");
		return false;
	}

#define WEBCAM_LOAD(name) \
	if (!(*(void **)&sdl3.name = SDL_LoadFunction(sdl3.lib, "SDL_" #name))) { \
		sysLogPrintf(LOG_WARNING, "webcam: SDL3 has no SDL_%s (SDL 3.2 or newer is needed)", #name); \
		SDL_UnloadObject(sdl3.lib); \
		sdl3.lib = NULL; \
		return false; \
	}

	WEBCAM_LOAD(InitSubSystem);
	WEBCAM_LOAD(GetError);
	WEBCAM_LOAD(free);
	WEBCAM_LOAD(GetCameras);
	WEBCAM_LOAD(GetCameraName);
	WEBCAM_LOAD(GetCameraSupportedFormats);
	WEBCAM_LOAD(OpenCamera);
	WEBCAM_LOAD(GetCameraPermissionState);
	WEBCAM_LOAD(GetCameraFormat);
	WEBCAM_LOAD(AcquireCameraFrame);
	WEBCAM_LOAD(ReleaseCameraFrame);
	WEBCAM_LOAD(CloseCamera);
	WEBCAM_LOAD(GetCurrentCameraDriver);
	WEBCAM_LOAD(ConvertSurface);
	WEBCAM_LOAD(DestroySurface);

#undef WEBCAM_LOAD

	if (!sdl3.InitSubSystem(SDL3_INIT_CAMERA)) {
		sysLogPrintf(LOG_WARNING, "webcam: SDL3 camera init failed: %s", sdl3.GetError());
		SDL_UnloadObject(sdl3.lib);
		sdl3.lib = NULL;
		return false;
	}

	sysLogPrintf(LOG_NOTE, "webcam: using SDL3 camera driver %s", sdl3.GetCurrentCameraDriver());

	return true;
}

static bool webcamIsLuma8(Uint32 format)
{
	return format == SDL_PIXELFORMAT_YV12 || format == SDL_PIXELFORMAT_IYUV
		|| format == SDL_PIXELFORMAT_NV12 || format == SDL_PIXELFORMAT_NV21;
}

static bool webcamIsLuma16(Uint32 format)
{
	return format == SDL_PIXELFORMAT_YUY2 || format == SDL_PIXELFORMAT_YVYU || format == SDL_PIXELFORMAT_UYVY;
}

/**
 * The smallest mode that still covers the Game Boy Camera's picture, preferring
 * formats whose luma can be read directly. Asking for a small mode keeps the
 * camera thread's copies and our scaling cheap.
 */
static bool webcamChooseSpec(SDL3_CameraID id, SDL3_CameraSpec *out)
{
	SDL3_CameraSpec **specs;
	s32 count = 0;
	s32 best = -1;
	s32 bestscore = 0;
	s32 i;

	specs = sdl3.GetCameraSupportedFormats(id, &count);

	if (!specs) {
		return false;
	}

	for (i = 0; i < count; i++) {
		SDL3_CameraSpec *spec = specs[i];
		s32 score;

		if (spec->width < 160 || spec->height < 120) {
			continue;
		}

		score = spec->width * spec->height;

		if (!webcamIsLuma8(spec->format) && !webcamIsLuma16(spec->format)) {
			score *= 2;
		}

		if (best < 0 || score < bestscore) {
			best = i;
			bestscore = score;
		}
	}

	if (best >= 0) {
		*out = *specs[best];
	}

	sdl3.free(specs);

	return best >= 0;
}

static void webcamCloseLocked(void)
{
	if (webcamCamera) {
		sdl3.CloseCamera(webcamCamera);
		webcamCamera = NULL;
		sysLogPrintf(LOG_NOTE, "webcam: camera closed");
	}

	webcamCacheValid = false;
}

static Uint32 SDLCALL webcamTimerCallback(Uint32 interval, void *param)
{
	Uint32 next = interval;

	SDL_LockMutex(webcamLock);

	if (webcamCamera == NULL || SDL_GetTicks() - webcamLastUse >= WEBCAM_IDLE_MS) {
		webcamCloseLocked();
		webcamTimer = 0;
		next = 0;
	}

	SDL_UnlockMutex(webcamLock);

	return next;
}

static bool webcamOpenLocked(void)
{
	SDL3_CameraID *ids;
	SDL3_CameraSpec spec;
	s32 count = 0;
	s32 index;

	ids = sdl3.GetCameras(&count);

	if (!ids || count == 0) {
		if (ids) {
			sdl3.free(ids);
		}

		return false;
	}

	index = webcamDevice < count ? webcamDevice : 0;

	if (webcamChooseSpec(ids[index], &spec)) {
		webcamCamera = sdl3.OpenCamera(ids[index], &spec);
	}

	if (!webcamCamera) {
		webcamCamera = sdl3.OpenCamera(ids[index], NULL);
	}

	if (webcamCamera) {
		SDL3_CameraSpec actual;

		if (sdl3.GetCameraFormat(webcamCamera, &actual)) {
			sysLogPrintf(LOG_NOTE, "webcam: opened camera %d of %d (%s), %dx%d format 0x%08x",
					index, count, sdl3.GetCameraName(ids[index]), actual.width, actual.height, actual.format);
		}
	} else {
		sysLogPrintf(LOG_WARNING, "webcam: can't open camera %d: %s", index, sdl3.GetError());
	}

	sdl3.free(ids);

	return webcamCamera != NULL;
}

static s32 webcamPixelLuma(const SDL3_Surface *frame, s32 x, s32 y)
{
	const u8 *row = (const u8 *)frame->pixels + y * frame->pitch;
	Uint32 rgb;

	if (webcamIsLuma8(frame->format)) {
		return row[x];
	}

	if (frame->format == SDL_PIXELFORMAT_UYVY) {
		return row[x * 2 + 1];
	}

	if (webcamIsLuma16(frame->format)) {
		return row[x * 2];
	}

	// SDL3_PIXELFORMAT_XRGB8888 (converted)
	memcpy(&rgb, row + x * 4, 4);

	return (((rgb >> 16) & 0xff) * 77 + ((rgb >> 8) & 0xff) * 150 + (rgb & 0xff) * 29) >> 8;
}

/**
 * Crop the middle of the frame to the destination's aspect ratio and scale it
 * down, averaging up to 4x4 samples per destination pixel.
 */
static void webcamScaleFrame(const SDL3_Surface *frame, u8 *dst, s32 width, s32 height)
{
	s32 cropw = frame->w;
	s32 croph = frame->h;
	s32 x0, y0;
	s32 dx, dy;

	if (cropw * height > croph * width) {
		cropw = croph * width / height;
	} else {
		croph = cropw * height / width;
	}

	x0 = (frame->w - cropw) / 2;
	y0 = (frame->h - croph) / 2;

	for (dy = 0; dy < height; dy++) {
		s32 sy0 = y0 + dy * croph / height;
		s32 sy1 = y0 + (dy + 1) * croph / height;
		s32 ystep = (sy1 - sy0) / 4 + 1;

		if (sy1 <= sy0) {
			sy1 = sy0 + 1;
		}

		for (dx = 0; dx < width; dx++) {
			s32 sx0 = x0 + dx * cropw / width;
			s32 sx1 = x0 + (dx + 1) * cropw / width;
			s32 xstep = (sx1 - sx0) / 4 + 1;
			s32 sum = 0;
			s32 n = 0;
			s32 sx, sy;

			if (sx1 <= sx0) {
				sx1 = sx0 + 1;
			}

			for (sy = sy0; sy < sy1; sy += ystep) {
				for (sx = sx0; sx < sx1; sx += xstep) {
					sum += webcamPixelLuma(frame, sx, sy);
					n++;
				}
			}

			dst[dy * width + (webcamMirror ? width - 1 - dx : dx)] = sum / n;
		}
	}
}

static bool webcamConvertFrame(SDL3_Surface *frame, u8 *dst, s32 width, s32 height)
{
	SDL3_Surface *converted = NULL;

	if (frame->pixels == NULL || frame->w <= 0 || frame->h <= 0) {
		return false;
	}

	if (!webcamIsLuma8(frame->format) && !webcamIsLuma16(frame->format)
			&& frame->format != SDL3_PIXELFORMAT_XRGB8888) {
		converted = sdl3.ConvertSurface(frame, SDL3_PIXELFORMAT_XRGB8888);

		if (!converted) {
			return false;
		}

		frame = converted;
	}

	webcamScaleFrame(frame, dst, width, height);

	if (converted) {
		sdl3.DestroySurface(converted);
	}

	return true;
}

s32 webcamGetFrame(u8 *dst, s32 width, s32 height)
{
	SDL3_Surface *frame;
	SDL3_Surface *latest = NULL;
	s32 result;

	if (webcamState == 0) {
		webcamState = webcamLoadSdl3() ? 1 : -1;
		webcamLock = SDL_CreateMutex();
	}

	if (webcamState < 0 || webcamDenied || !webcamLock) {
		return WEBCAM_UNAVAILABLE;
	}

	SDL_LockMutex(webcamLock);

	webcamLastUse = SDL_GetTicks();

	if (!webcamCamera) {
		if (!webcamOpenLocked()) {
			SDL_UnlockMutex(webcamLock);
			sysLogPrintf(LOG_NOTE, "webcam: no camera found, using the test card");
			webcamState = -1;
			return WEBCAM_UNAVAILABLE;
		}

		if (!webcamTimer) {
			webcamTimer = SDL_AddTimer(1000, webcamTimerCallback, NULL);
		}
	}

	switch (sdl3.GetCameraPermissionState(webcamCamera)) {
	case -1:
		sysLogPrintf(LOG_WARNING, "webcam: permission to use the camera was denied, using the test card");
		webcamCloseLocked();
		webcamDenied = true;
		SDL_UnlockMutex(webcamLock);
		return WEBCAM_UNAVAILABLE;
	case 0:
		SDL_UnlockMutex(webcamLock);
		return WEBCAM_WAITING;
	}

	if (webcamCache == NULL || webcamCacheW != width || webcamCacheH != height) {
		free(webcamCache);
		webcamCache = malloc(width * height);
		webcamCacheW = width;
		webcamCacheH = height;
		webcamCacheValid = false;
	}

	// Frames queue up between captures: keep the newest
	while ((frame = sdl3.AcquireCameraFrame(webcamCamera, NULL)) != NULL) {
		if (latest) {
			sdl3.ReleaseCameraFrame(webcamCamera, latest);
		}

		latest = frame;
	}

	if (latest) {
		if (webcamCache && webcamConvertFrame(latest, webcamCache, width, height)) {
			webcamCacheValid = true;
		}

		sdl3.ReleaseCameraFrame(webcamCamera, latest);
	}

	if (webcamCacheValid) {
		memcpy(dst, webcamCache, width * height);
		result = WEBCAM_OK;
	} else {
		result = WEBCAM_WAITING;
	}

	SDL_UnlockMutex(webcamLock);

	return result;
}

/**
 * Close the camera before the program exits, so SDL3's camera thread isn't
 * still delivering frames while the rest of the port shuts down. SDL3 itself
 * stays loaded until the process ends. So does the mutex: the idle timer may
 * already be waiting for it.
 */
void webcamShutdown(void)
{
	if (webcamLock == NULL) {
		return;
	}

	SDL_LockMutex(webcamLock);

	if (webcamTimer) {
		SDL_RemoveTimer(webcamTimer);
		webcamTimer = 0;
	}

	webcamCloseLocked();
	free(webcamCache);
	webcamCache = NULL;

	SDL_UnlockMutex(webcamLock);
}

PD_CONSTRUCTOR static void webcamConfigInit(void)
{
	configRegisterInt("PerfectHead.CameraDevice", &webcamDevice, 0, 15);
	configRegisterInt("PerfectHead.CameraMirror", &webcamMirror, 0, 1);
	configRegisterString("PerfectHead.SDL3Library", webcamLibPath, sizeof(webcamLibPath));
}

#else

s32 webcamGetFrame(u8 *dst, s32 width, s32 height)
{
	return WEBCAM_UNAVAILABLE;
}

void webcamShutdown(void)
{
}

#endif
