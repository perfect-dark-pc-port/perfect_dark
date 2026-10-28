#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <SDL.h>
#include <PR/ultratypes.h>
#include <PR/os_thread.h>
#include "platform.h"
#include "input.h"
#include "gyro.h"
#include "../gamepadmotionhelper/GamepadMotion.h"
#include "system.h"
#include "game/menu.h"
#include "bss.h"

#define GMH_STILLNESS_CORRECTION_TIME 1.0f
#define GMH_STILLNESS_EASE_TIME       1.5f
#define MANUAL_CALIB_HOLD_MS 500
#define GMH_DEG2RAD 0.01745329252f
#define GYRO_TIGHTENING_DEGPS 10.0f

typedef struct {
	// Manual calibration
	bool     manualCalibActive;
	Uint32   manualCalibStartTime;
	bool     prevManualCalibPressed;

	// Auto-calibration
	bool     justFinishedCalibrating;
	s32      lastDesiredCalibMode;  // -1 = uninitialised, tracks last applied CalibrationMode

	// Axis mode change detection
	s32      previousAxisMode;      // -1 = uninitialised
	bool     axisModeInitialized;

	// Modifier toggle
	bool     gyroToggleState;       // true = gyro on
	s32      prevGyroModPressed;

	// Smoothing EMA
	f32      emaX, emaY, emaZ;

} GyroCalibState;

static GamepadMotionHandle gpadMotion[INPUT_MAX_CONTROLLERS] = { NULL };
static f32  gyroOrientYaw[INPUT_MAX_CONTROLLERS];
static f32  gyroOrientPitch[INPUT_MAX_CONTROLLERS];
static f32  gyroOrientRoll[INPUT_MAX_CONTROLLERS];
static float gyroData[INPUT_MAX_CONTROLLERS][3]  = {0};
static float accelData[INPUT_MAX_CONTROLLERS][3] = {0};
static bool  hasSensorData[INPUT_MAX_CONTROLLERS] = {false};
static GyroCalibState gyroCalibState[INPUT_MAX_CONTROLLERS] = {0};

static void applyGyroAxisMapping(s32 cidx, float calibratedGyro[3], f32 *deltaX, f32 *deltaY, f32 *deltaZ);
static void applyGyroModifier(f32 *deltaX, f32 *deltaY, f32 *deltaZ, s32 activationMode, s32 idx);

static void applyGyroTightening(f32 *dx, f32 *dy, f32 *dz, f32 tightening);
static void applyGyroSmoothing(f32 *deltaX, f32 *deltaY, f32 *deltaZ, f32 smoothing, s32 cidx);
static void inputConfigureGamepadMotionSettings(GamepadMotionHandle handle);
static void inputUpdateGyroCalibrationHandle(void);
static void inputUpdateGyroManualCalibration(s32 cidx);
static void inputGyroManualCalibrationActivation(s32 cidx, bool start);
static void inputUpdateGyroCalibrationMode(s32 cidx);
static void inputResetGyroCalibration(s32 cidx);

static void inputInitGyroCalibState(s32 cidx)
{
	memset(&gyroCalibState[cidx], 0, sizeof(GyroCalibState));
	gyroCalibState[cidx].lastDesiredCalibMode = -1;
	gyroCalibState[cidx].previousAxisMode     = -1;
	gyroCalibState[cidx].gyroToggleState      = true;
}

void gyroInitController(s32 cidx)
{
	if (!gpadMotion[cidx]) {
		gpadMotion[cidx] = gmhCreateGamepadMotion();
	}
	if (gpadMotion[cidx]) {
		inputInitGyroCalibState(cidx);
		inputConfigureGamepadMotionSettings(gpadMotion[cidx]);
		gmhResetMotion(gpadMotion[cidx]);
		inputUpdateGyroCalibrationMode(cidx);
	}
}

void gyroCloseController(s32 cidx)
{
	if (gpadMotion[cidx]) {
		gmhDeleteGamepadMotion(gpadMotion[cidx]);
		gpadMotion[cidx] = NULL;
	}
	gyroOrientYaw[cidx]   = 0.f;
	gyroOrientPitch[cidx] = 0.f;
	gyroOrientRoll[cidx]  = 0.f;
	gyroData[cidx][0]  = gyroData[cidx][1]  = gyroData[cidx][2]  = 0.f;
	accelData[cidx][0] = accelData[cidx][1] = accelData[cidx][2] = 0.f;
	hasSensorData[cidx] = false;
	inputInitGyroCalibState(cidx);
}

void gyroFeedAngVel(s32 cidx, float gx, float gy, float gz)
{
	gyroData[cidx][0] = gx;
	gyroData[cidx][1] = gy;
	gyroData[cidx][2] = gz;
	hasSensorData[cidx] = true;
}

void gyroFeedAccel(s32 cidx, float ax, float ay, float az)
{
	accelData[cidx][0] = ax;
	accelData[cidx][1] = ay;
	accelData[cidx][2] = az;
	hasSensorData[cidx] = true;
}

static void inputConfigureGamepadMotionSettings(GamepadMotionHandle handle)
{
	if (!handle) return;
	gmhSetMinStillnessCorrectionTime(handle, GMH_STILLNESS_CORRECTION_TIME);
	gmhSetStillnessCalibrationEaseInTime(handle, GMH_STILLNESS_EASE_TIME);
}

static void applyGyroAxisMapping(s32 cidx, float calibratedGyro[3], f32 *deltaX, f32 *deltaY, f32 *deltaZ)
{
	if (!gpadMotion[cidx]) {
		*deltaX = *deltaY = *deltaZ = 0.f;
		return;
	}

	GyroCalibState *state = &gyroCalibState[cidx];
	const s32 currentAxisMode = inputGyroGetAxisMode(cidx);

	if (!state->axisModeInitialized) {
		state->previousAxisMode    = currentAxisMode;
		state->axisModeInitialized = true;
	}

	if (state->previousAxisMode != currentAxisMode) {
		gmhResetMotion(gpadMotion[cidx]);
		state->previousAxisMode = currentAxisMode;
	}

	switch (currentAxisMode) {
	case GYRO_YAW:
		*deltaX = -calibratedGyro[1];
		*deltaY = -calibratedGyro[0];
		break;
	case GYRO_ROLL:
		*deltaX =  calibratedGyro[2];
		*deltaY = -calibratedGyro[0];
		break;
	case GYRO_LOCAL: {
		// Local Space code based on 
		// http://gyrowiki.jibbsmart.com/blog:player-space-gyro-and-alternatives-explained
		*deltaX = -copysignf(hypotf(calibratedGyro[1], calibratedGyro[2]), calibratedGyro[1] - calibratedGyro[2]);
		*deltaY = -calibratedGyro[0];
		break;
	}
	case GYRO_PLAYER: {
		float playerX = 0.f, playerY = 0.f;
		gmhGetPlayerSpaceGyro(gpadMotion[cidx], &playerX, &playerY, 1.41f);
		*deltaX = -playerY;
		*deltaY = -playerX;
		*deltaZ =  calibratedGyro[2];
		break;
	}
	case GYRO_WORLD: {
		float worldX = 0.f, worldY = 0.f;
		gmhGetWorldSpaceGyro(gpadMotion[cidx], &worldX, &worldY, 0.125f);
		*deltaX = -worldY;
		*deltaY = -worldX;
		*deltaZ =  0.f;
		break;
	}
	default:
		*deltaX = *deltaY = *deltaZ = 0.f;
		break;
	}
}

static void applyGyroModifier(f32 *deltaX, f32 *deltaY, f32 *deltaZ, s32 activationMode, s32 idx)
{
	GyroCalibState *state = &gyroCalibState[idx];

	const int modPressed  = inputBindPressed(idx, CK_0080);
	const int justPressed = modPressed && !state->prevGyroModPressed;
	state->prevGyroModPressed = modPressed;

	bool gyroActive = false;

	switch (activationMode) {
	case GYRO_ALWAYS_ON:
		gyroActive = true;
		break;
	case GYRO_TOGGLE:
		if (justPressed) {
			state->gyroToggleState = !state->gyroToggleState;
		}
		gyroActive = state->gyroToggleState;
		break;
	case GYRO_ENABLE_HELD:
		gyroActive = modPressed;
		break;
	case GYRO_DISABLE_HELD:
		gyroActive = !modPressed;
		break;
	default:
		gyroActive = false;
		break;
	}

	if (!gyroActive) {
		*deltaX = 0.f;
		*deltaY = 0.f;
		*deltaZ = 0.f;
	}
}

static void applyGyroSmoothing(f32 *deltaX, f32 *deltaY, f32 *deltaZ, f32 smoothing, s32 cidx)
{
	if (smoothing <= 0.0f) return;

	const f32 s     = smoothing >= 1.0f ? 0.99f : smoothing;
	const f32 alpha = 1.0f - s;
	GyroCalibState *state = &gyroCalibState[cidx];

	state->emaX = alpha * (*deltaX) + s * state->emaX;
	state->emaY = alpha * (*deltaY) + s * state->emaY;
	state->emaZ = alpha * (*deltaZ) + s * state->emaZ;

	*deltaX = state->emaX;
	*deltaY = state->emaY;
	*deltaZ = state->emaZ;
}

static void applyGyroTightening(f32 *dx, f32 *dy, f32 *dz, f32 tightening)
{
	if (tightening <= 0.0f) return;

	const f32 threshold = tightening * GYRO_TIGHTENING_DEGPS;
	const f32 mag = sqrtf((*dx) * (*dx) + (*dy) * (*dy) + (*dz) * (*dz));
	if (mag > 0.0f && mag < threshold) {
		const f32 scale = mag / threshold;
		*dx *= scale;
		*dy *= scale;
		*dz *= scale;
	}
}

static void inputProcessMotionSensorData(s32 cidx, float deltaTime, f32 *deltaX, f32 *deltaY, f32 *deltaZ)
{
	if (!gpadMotion[cidx] || !hasSensorData[cidx]) {
		return;
	}

	gmhProcessMotion(gpadMotion[cidx],
		gyroData[cidx][0], gyroData[cidx][1], gyroData[cidx][2],
		accelData[cidx][0], accelData[cidx][1], accelData[cidx][2],
		deltaTime);

	float calibratedGyro[3] = {0.f};
	gmhGetCalibratedGyro(gpadMotion[cidx], &calibratedGyro[0], &calibratedGyro[1], &calibratedGyro[2]);
	applyGyroAxisMapping(cidx, calibratedGyro, deltaX, deltaY, deltaZ);
}

static void inputApplyGyroProcessing(s32 cidx, f32 *deltaX, f32 *deltaY, f32 *deltaZ)
{
	if (gyroCalibState[cidx].justFinishedCalibrating) {
		*deltaX = 0.f;
		*deltaY = 0.f;
		*deltaZ = 0.f;
		gyroCalibState[cidx].justFinishedCalibrating = false;
		return;
	}

	applyGyroModifier(deltaX, deltaY, deltaZ, inputGetGyroModifier(cidx), cidx);
	applyGyroSmoothing(deltaX, deltaY, deltaZ, inputGetGyroSmoothing(cidx), cidx);
	applyGyroTightening(deltaX, deltaY, deltaZ, inputGyroGetTightening(cidx));
}

void inputUpdateGyro(s32 cidx)
{
	float deltaTime = g_Vars.diffframe60f / 60.0f;
	float frameTime = g_Vars.lvupdate60freal;

	f32 deltaX = 0.f, deltaY = 0.f, deltaZ = 0.f;
	inputProcessMotionSensorData(cidx, deltaTime, &deltaX, &deltaY, &deltaZ);
	inputApplyGyroProcessing(cidx, &deltaX, &deltaY, &deltaZ);

	gyroOrientYaw[cidx]   = deltaX * GMH_DEG2RAD * frameTime;
	gyroOrientPitch[cidx] = deltaY * GMH_DEG2RAD * frameTime;
	gyroOrientRoll[cidx]  = deltaZ * GMH_DEG2RAD * frameTime;
}

static void inputUpdateGyroCalibrationMode(s32 cidx)
{
	if (!gpadMotion[cidx]) return;

	if (inputIsMenuGyroCalibrationActive(cidx)) {
		gyroCalibState[cidx].lastDesiredCalibMode = -1;
		return;
	}

	const s32 mode = inputGyroGetAutoCalibration(cidx);
	int desired;

	switch (mode) {
	case GYRO_AUTOCALIBRATION_MENU_ONLY:
		desired = (g_Menus[cidx].curdialog != NULL)
			? CALIBRATIONMODE_STILLNESS
			: CALIBRATIONMODE_MANUAL;
		break;
	case GYRO_AUTOCALIBRATION_ALWAYS:
		desired = CALIBRATIONMODE_STILLNESS | CALIBRATIONMODE_SENSORFUSION;
		break;
	default: // OFF
		desired = CALIBRATIONMODE_MANUAL;
		break;
	}

	if (desired == gyroCalibState[cidx].lastDesiredCalibMode) return;
	gyroCalibState[cidx].lastDesiredCalibMode = desired;

	// Preserve calibrated offset and confidence across mode changes.
	float ox = 0.f, oy = 0.f, oz = 0.f;
	gmhGetCalibrationOffset(gpadMotion[cidx], &ox, &oy, &oz);
	float confidence = gmhGetAutoCalibrationConfidence(gpadMotion[cidx]);

	gmhSetCalibrationMode(gpadMotion[cidx], desired);
	gmhSetCalibrationOffset(gpadMotion[cidx], ox, oy, oz, 1);
	gmhSetAutoCalibrationConfidence(gpadMotion[cidx], confidence);
}

static void inputResetGyroCalibration(s32 cidx)
{
	if (!gpadMotion[cidx]) return;

	gmhResetGamepadMotion(gpadMotion[cidx]);
	inputConfigureGamepadMotionSettings(gpadMotion[cidx]);

	gyroOrientYaw[cidx]   = 0.f;
	gyroOrientPitch[cidx] = 0.f;
	gyroOrientRoll[cidx]  = 0.f;
	inputInitGyroCalibState(cidx);
	inputUpdateGyroCalibrationMode(cidx);
}

static void inputGyroManualCalibrationActivation(s32 cidx, bool start)
{
	GyroCalibState *state = &gyroCalibState[cidx];
	if (!gpadMotion[cidx]) return;

	if (start) {
		state->manualCalibActive       = true;
		state->manualCalibStartTime    = SDL_GetTicks();
		state->justFinishedCalibrating = false;
		gmhSetCalibrationMode(gpadMotion[cidx], CALIBRATIONMODE_MANUAL);
		gmhResetContinuousCalibration(gpadMotion[cidx]);
		gmhStartContinuousCalibration(gpadMotion[cidx]);
	} else {
		state->manualCalibActive       = false;
		state->lastDesiredCalibMode    = -1;
		state->justFinishedCalibrating = true;
		gmhPauseContinuousCalibration(gpadMotion[cidx]);
	}
}

static void inputUpdateGyroManualCalibration(s32 cidx)
{
	if (!inputControllerConnected(cidx) || !gpadMotion[cidx]) {
		gyroCalibState[cidx].manualCalibActive = false;
		return;
	}

	GyroCalibState *state = &gyroCalibState[cidx];

	if (inputIsMenuGyroCalibrationActive(cidx)) {
		if (state->manualCalibActive) {
			inputGyroManualCalibrationActivation(cidx, false);
		}
		return;
	}

	const bool pressed     = inputBindPressed(cidx, CK_0100);
	const bool justPressed = pressed && !state->prevManualCalibPressed;

	if (justPressed && !state->manualCalibActive) {
		inputGyroManualCalibrationActivation(cidx, true);
	}

	if (state->manualCalibActive) {
		const Uint32 elapsed = SDL_GetTicks() - state->manualCalibStartTime;
		if (elapsed >= MANUAL_CALIB_HOLD_MS) {
			inputGyroManualCalibrationActivation(cidx, false);
		}
	}

	state->prevManualCalibPressed = pressed;
}

static void inputUpdateGyroCalibrationHandle(void)
{
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		if (!inputGyroIsEnabled(cidx)) continue;
		inputUpdateGyroManualCalibration(cidx);
		if (!gyroCalibState[cidx].manualCalibActive) {
			inputUpdateGyroCalibrationMode(cidx);
		}
	}
}

void gyroUpdateAll(void)
{
	inputUpdateGyroCalibrationHandle();
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		if (inputGyroIsEnabled(cidx)) {
			inputUpdateGyro(cidx);
		}
	}
}

void gyroReconfigureCalibrationMode(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) return;

	gyroCalibState[cidx].lastDesiredCalibMode = -1;
	if (gpadMotion[cidx]) {
		inputUpdateGyroCalibrationMode(cidx);
	}
}

void inputGyroCalibration(s32 cidx, GyroCalibrationOp op, float *out_confidence, int *out_steady)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) return;

	switch (op) {
	case GYRO_CALIB_START:
		inputGyroManualCalibrationActivation(cidx, true);
		break;
	case GYRO_CALIB_FINISH:
		inputGyroManualCalibrationActivation(cidx, false);
		break;
	case GYRO_CALIB_RESET:
		inputResetGyroCalibration(cidx);
		break;
	case GYRO_CALIB_QUERY:
		if (gpadMotion[cidx]) {
			if (out_confidence) *out_confidence = gmhGetAutoCalibrationConfidence(gpadMotion[cidx]);
			if (out_steady)     *out_steady     = gmhGetAutoCalibrationIsSteady(gpadMotion[cidx]);
		}
		break;
	case GYRO_CALIB_AUTO:
		inputUpdateGyroCalibrationMode(cidx);
		break;
	}
}

s32 inputGyroGetManualCalibration(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) return 0;
	return gyroCalibState[cidx].manualCalibActive ? 1 : 0;
}

void inputGyroSetManualCalibration(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) return;
	if (!gpadMotion[cidx]) return;
	if (inputIsMenuGyroCalibrationActive(cidx)) return;
	inputGyroManualCalibrationActivation(cidx, true);
}

void gyroGetOrientation(s32 cidx, f32 *yaw, f32 *pitch, f32 *roll)
{
	if (yaw)   *yaw   = gyroOrientYaw[cidx];
	if (pitch) *pitch = gyroOrientPitch[cidx];
	if (roll)  *roll  = gyroOrientRoll[cidx];
}
