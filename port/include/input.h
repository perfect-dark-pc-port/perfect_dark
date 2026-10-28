#ifndef _IN_INPUT_H
#define _IN_INPUT_H

#include <PR/ultratypes.h>
#include <PR/os_cont.h>

#define INPUT_MAX_CONTROLLERS MAXCONTROLLERS
#define INPUT_MAX_CONNECTED_CONTROLLERS 8
#define INPUT_MAX_CONTROLLER_BUTTONS 32
#define INPUT_MAX_BINDS 4

#define CONT_STICK_XNEG 0x10000
#define CONT_STICK_XPOS 0x20000
#define CONT_STICK_YNEG 0x40000
#define CONT_STICK_YPOS 0x80000

#define CONT_NUM_BUTTONS 32 // not including the stick axes

enum virtkey {
	/* same order as SDL scancodes */
	VK_KEYBOARD_BEGIN = 0,
	VK_A = 4,
	VK_Z = 29,
	VK_1 = 30,
	VK_9 = 38,
	VK_0 = 39,
	VK_RETURN = 40,
	VK_ESCAPE = 41,
	VK_BACKSPACE = 42,
	VK_SPACE = 44,
	VK_MINUS = 45,
	VK_LEFTBRACKET = 47,
	VK_RIGHTBRACKET = 48,
	VK_SEMICOLON = 51,
	VK_GRAVE = 53,
	VK_COMMA = 54,
	VK_PERIOD = 55,
	VK_F1 = 58,
	VK_F9 = 66,
	VK_DELETE = 76,
	VK_LCTRL = 224,
	VK_LSHIFT = 225,
	VK_LALT = 226,
	VK_RCTRL = 228,
	VK_RSHIFT = 229,

	/* same order as SDL mouse buttons */
	VK_MOUSE_BEGIN = 512,
	VK_MOUSE_LEFT = VK_MOUSE_BEGIN,
	VK_MOUSE_MIDDLE,
	VK_MOUSE_RIGHT,
	VK_MOUSE_X1,
	VK_MOUSE_X2,
	VK_MOUSE_WHEEL_UP,
	VK_MOUSE_WHEEL_DN,

	/* same order as SDL gamecontroller buttons plus two buttons for triggers */
	VK_JOY_BEGIN,
	VK_JOY1_BEGIN = VK_JOY_BEGIN,
	VK_JOY1_LSHOULDER = VK_JOY1_BEGIN + 9,
	VK_JOY1_RSHOULDER = VK_JOY1_BEGIN + 10,
	VK_JOY1_LTRIG = VK_JOY1_BEGIN + 30,
	VK_JOY1_RTRIG = VK_JOY1_BEGIN + 31,
	VK_JOY2_BEGIN = VK_JOY1_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,
	VK_JOY3_BEGIN = VK_JOY2_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,
	VK_JOY4_BEGIN = VK_JOY3_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,

	VK_TOTAL_COUNT = VK_JOY_BEGIN + INPUT_MAX_CONTROLLERS * INPUT_MAX_CONTROLLER_BUTTONS,
};

enum keymod {
	/* same order as SDL keymods */
	KM_LSHIFT = 0x0001,
	KM_RSHIFT = 0x0002,
	KM_LCTRL = 0x0040,
	KM_RCTRL = 0x0080,
	KM_CAPS = 0x2000,
	KM_CTRL = KM_LCTRL | KM_RCTRL,
	KM_SHIFT = KM_LSHIFT | KM_RSHIFT
};

enum contkey {
	CK_C_R,
	CK_C_L,
	CK_C_D,
	CK_C_U,
	CK_RTRIG,
	CK_LTRIG,
	CK_X, // gap in CONT_
	CK_Y, // gap in CONT_
	CK_DPAD_R,
	CK_DPAD_L,
	CK_DPAD_D,
	CK_DPAD_U,
	CK_START,
	CK_ZTRIG,
	CK_B,
	CK_A,
	CK_STICK_XNEG,
	CK_STICK_XPOS,
	CK_STICK_YNEG,
	CK_STICK_YPOS,
	CK_ACCEPT,
	CK_CANCEL,
	CK_0040,
	CK_0080, // Gyro Modifier action
	CK_0100, // Gyro Calibration (Manual) action
	CK_0200,
	CK_0400,
	CK_0800,
	CK_1000,
	CK_2000,
	CK_4000,
	CK_8000,
	CK_TOTAL_COUNT
};

enum mouselockmode {
	MLOCK_OFF = 0,
	MLOCK_ON = 1,
	MLOCK_AUTO = 2
};

enum gyroactivation {
	GYRO_ALWAYS_ON = 0,     // Gyro is always enabled
	GYRO_TOGGLE = 1,        // Gyro is enabled/disabled by a button press
	GYRO_ENABLE_HELD = 2,   // Gyro is enabled while a button is held down
	GYRO_DISABLE_HELD = 3   // Gyro is disabled while a button is held down
};

enum gyroaxismode {
	GYRO_YAW = 0,           // Gyro controls yaw axis (turn)
	GYRO_ROLL = 1,          // Gyro controls roll axis (lean)
	GYRO_LOCAL = 2,         // Gyro controls local space orientation
	GYRO_PLAYER = 3,        // Gyro controls player space orientation
	GYRO_WORLD = 4          // Gyro controls world space orientation
};

enum gyroaimmode {
	GYRO_AIM_CAMERA = 0,    // Gyro controls camera movement
	GYRO_AIM_CROSSHAIR = 1, // Gyro controls crosshair movement
	GYRO_AIM_BOTH = 2       // Gyro controls both camera and crosshair movement
};

typedef enum {
	GYRO_CALIB_START,       // Start gyro calibration
	GYRO_CALIB_FINISH,      // Finish gyro calibration
	GYRO_CALIB_RESET,       // Reset gyro calibration
	GYRO_CALIB_QUERY,       // Query gyro calibration status
	GYRO_CALIB_AUTO         // Automatic gyro calibration
} GyroCalibrationOp;

typedef enum {
	CALIBRATIONMODE_MANUAL = 0,       // Manual calibration mode
	CALIBRATIONMODE_STILLNESS = 1,    // Stillness calibration mode
	CALIBRATIONMODE_SENSORFUSION = 2  // Sensor fusion calibration mode
} CalibrationMode;

enum gyroautocalibration {
	GYRO_AUTOCALIBRATION_OFF = 0,         // Disables Auto-Calibration
	GYRO_AUTOCALIBRATION_MENU_ONLY = 1,   // Enables Auto-Calibration only when a menu dialog is opened
	GYRO_AUTOCALIBRATION_ALWAYS = 2       // Enables continuous Auto-Calibration whenever possible.
};

// returns bitmask of connected controllers or -1 if failed
s32 inputInit(void);

// read the specified player's inputs into the N64 pad struct
// returns 0 if read, non-0 if failed
s32 inputReadController(s32 idx, OSContPad *npad);

// returns 1 if rumble is supported for specified player's controller
s32 inputRumbleSupported(s32 idx);

// returns 1 if specified player has a live controller assigned
s32 inputControllerConnected(s32 idx);

// returns bitmask of players with assigned controllers
s32 inputControllerMask(void);

// get/set Input.Player%d.SwapSticks
s32 inputControllerGetSticksSwapped(s32 cidx);
void inputControllerSetSticksSwapped(s32 cidx, s32 swapped);

// get/set Input.Player%d.StickCButtons (DualAnalog is 1 if StickCButtons is 0)
s32 inputControllerGetDualAnalog(s32 cidx);
void inputControllerSetDualAnalog(s32 cidx, s32 enable);

// get/set Input.Player%d.CancelCButtons
s32 inputControllerGetCancelCButtons(s32 cidx);
void inputControllerSetCancelCButtons(s32 cidx, s32 cancel);

// get/set sensitivity for a given player
f32 inputControllerGetAxisScale(s32 cidx, s32 stick, s32 axis);
void inputControllerSetAxisScale(s32 cidx, s32 stick, s32 axis, f32 value);

// get/set deadzone for a given player
f32 inputControllerGetAxisDeadzone(s32 cidx, s32 stick, s32 axis);
void inputControllerSetAxisDeadzone(s32 cidx, s32 stick, s32 axis, f32 value);

// writes array of up to INPUT_MAX_CONNECTED_CONTROLLERS controller IDs
// for all the controllers available on this machine into out if it's not NULL
// returns number of IDs that would've been written (or were written if out is not NULL)
s32 inputGetConnectedControllers(s32 *out);

// get name of connected controller id
// returns "Invalid" on failure
const char *inputGetConnectedControllerName(s32 id);

// get id of the controller currently assigned to player cidx or -1 if none
s32 inputGetAssignedControllerId(s32 cidx);

// assign connected controller id to player cidx
// if id is -1, unassigns controller from player, if any
// returns true on success, false on failure
s32 inputAssignController(s32 cidx, s32 id);

// vk is a value from the virtkey enum above
s32 inputKeyPressed(u32 vk);
s32 inputKeyJustPressed(u32 vk);

// idx is controller index, contbtn is one of the CONT_ constants
s32 inputButtonPressed(s32 idx, u32 contbtn);

// idx is controller index, ck is one of the CK_ constants
s32 inputBindPressed(s32 idx, u32 ck);

// bind virtkey vk to n64 pad #idx's button/axis ck as represented by its contkey value
// if bind is -1, picks a bind slot automatically
void inputKeyBind(s32 idx, u32 ck, s32 bind, u32 vk);

const u32 *inputKeyGetBinds(s32 idx, u32 ck);

// get VK_ value from human-readable name
s32 inputGetKeyByName(const char *name);

// get human-readable name from VK_ value
const char *inputGetKeyName(s32 vk);

// get CK_ value from human-readable name
s32 inputGetContKeyByName(const char *name);

// get human-readable name from CK_ value
const char *inputGetContKeyName(u32 ck);

// strength is 0 .. 1; 0 strength turns it off
void inputRumble(s32 idx, f32 strength, f32 time);

f32 inputRumbleGetStrength(s32 cidx);
void inputRumbleSetStrength(s32 cidx, f32 val);

// locks the mouse cursor in the window and makes it invisible if argument is true
void inputLockMouse(s32 lock);

// returns the current state of the above
s32 inputMouseIsLocked(void);

// sets x, y to mouse position in native viewport coordinates (ie, 320x240 most of the time)
// returns true if mouse has moved this input frame
s32 inputMouseGetPosition(s32 *x, s32 *y);

// returns changes in mouse position since last frame, in window coordinates
void inputMouseGetRawDelta(s32 *dx, s32 *dy);

// returns changes in mouse position since last frame, scaled by sensitivity
// returns 0, 0 when the mouse is not locked into the window
void inputMouseGetScaledDelta(f32 *dx, f32 *dy);

// returns changes in mouse position since last frame, scaled by absolute sensitivity
// returns 0, 0 when the mouse is not locked into the window
void inputMouseGetAbsScaledDelta(f32 *dx, f32 *dy);

void inputMouseGetSpeed(f32 *x, f32 *y);
void inputMouseSetSpeed(f32 x, f32 y);

s32 inputMouseIsEnabled(void);
void inputMouseEnable(s32 enabled);

// Updates the gyro data for the specified controller index
void inputUpdateGyro(s32 cidx);

// Scaled Gyro Movement Retrieval
void inputGyroGetScaledDelta(s32 cidx, f32* dx, f32* dy, f32* dz);
void inputGyroGetScaledDeltaCrosshair(s32 cidx, f32* dx, f32* dy);

// Raw Gyro Movement Retrieval
void inputGyroGetRawDelta(s32 cidx, s32* dx, s32* dy, s32* dz);

// Motion Sensor detection
// returns 1 if the controller has motion sensors, 0 otherwise disables Gyro Aiming functionality for the controller.
s32 inputControllerMotionSensorsSupported(s32 cidx);

// Gyro Enable/Disable
s32 inputGyroIsEnabled(s32 cidx);
void inputGyroEnable(s32 cidx, s32 enabled);

// Gyro Aim Mode Management
void inputSetGyroAimMode(s32 cidx, s32 mode);
s32 inputGetGyroAimMode(s32 cidx);

// Gyro Modifier Management
void inputSetGyroModifier(s32 cidx, s32 mode);
s32 inputGetGyroModifier(s32 cidx);

// Gyro Axis Mode Management
void inputGyroSetAxisMode(s32 cidx, s32 mode);
s32 inputGyroGetAxisMode(s32 cidx);

// Gyro Speed Management
void inputGyroGetSpeed(s32 cidx, f32* sensX, f32* sensY);
void inputGyroSetSpeed(s32 cidx, f32 sensX, f32 sensY);
void inputGyroGetAimSpeed(s32 cidx, f32* sensX, f32* sensY);
void inputGyroSetAimSpeed(s32 cidx, f32 sensX, f32 sensY);

// Gyro Advanced Mode Management
s32 inputGyroGetAdvanced(s32 cidx);
void inputGyroSetAdvanced(s32 cidx, s32 advanced);

// Gyro Invert Management
void inputGyroGetInvert(s32 cidx, s32* invertx, s32* inverty);
void inputGyroSetInvert(s32 cidx, s32 invertx, s32 inverty);
void inputGyroGetAimInvert(s32 cidx, s32* invertx, s32* inverty);
void inputGyroSetAimInvert(s32 cidx, s32 invertx, s32 inverty);

// Gyro X/Y Ratio Mixer Management
f32 inputGetGyroVHMixer(s32 cidx);
void inputSetGyroVHMixer(s32 cidx, f32 value);

// Gyro Smoothing Management
f32 inputGetGyroSmoothing(s32 cidx);
void inputSetGyroSmoothing(s32 cidx, f32 value);

// Gyro Tightening Management
f32 inputGyroGetTightening(s32 cidx);
void inputGyroSetTightening(s32 cidx, f32 value);

// Gyro Calibration Management
void inputGyroCalibration(s32 cidx, GyroCalibrationOp op, float* out_confidence, int* out_steady);

// Gyro Auto Calibration
s32 inputGyroGetAutoCalibration(s32 cidx);
void inputGyroSetAutoCalibration(s32 cidx, s32 enabled);

// Gyro Manual Calibration
s32 inputGyroGetManualCalibration(s32 cidx);
void inputGyroSetManualCalibration(s32 cidx);

// Ensures CK_0100 (Manual Calibration bind) is blocked during menu-driven gyro calibration
s32 inputIsMenuGyroCalibrationActive(s32 cidx);

// call this every frame
void inputUpdate(void);

// call this before configSave()
void inputSaveBinds(void);

// reset given player's binds to either PC or N64 defaults
void inputSetDefaultKeyBinds(s32 cidx, s32 n64mode);

// clear or get the last pressed button
void inputClearLastKey(void);
s32 inputGetLastKey(void);

// get/set Input.MouseLockMode
s32 inputGetMouseLockMode(void);
void inputSetMouseLockMode(s32 lockmode);

// same as inputLockMouse but works only if mouse is enabled and lockmode == MLOCK_AUTO
s32 inputAutoLockMouse(s32 wantlock);

// show/hide mouse cursor; if mouse lock is on the cursor is always hidden
void inputMouseShowCursor(s32 show);

void inputStartTextInput(void);
void inputStopTextInput(void);
s32 inputIsTextInputActive(void);

void inputClearLastTextChar(void);
char inputGetLastTextChar(void);

s32 inputTextHandler(char *out, const u32 outSize, s32 *curCol, s32 oskCharsOnly);

void inputClearClipboard(void);
const char *inputGetClipboard(void);

// returns keymod values
u32 inputGetKeyModState(void);

#endif
