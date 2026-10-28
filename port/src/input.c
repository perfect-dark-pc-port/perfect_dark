#include <string.h>
#include <math.h>
#include <ctype.h>
#include <SDL.h>
#include <SDL_sensor.h>
#include <PR/ultratypes.h>
#include <PR/os_thread.h>
#include <PR/os_cont.h>
#include "platform.h"
#include "input.h"
#include "gyro.h"
#include "video.h"
#include "config.h"
#include "utils.h"
#include "system.h"
#include "fs.h"
#include "game/menu.h"
#include "bss.h"

#if !SDL_VERSION_ATLEAST(2, 0, 14)
// this was added in 2.0.14
#define SDL_CONTROLLER_TYPE_VIRTUAL SDL_CONTROLLER_TYPE_UNKNOWN
#endif

#define CONTROLLERDB_FNAME "gamecontrollerdb.txt"
#define MAX_BIND_STR 256

#define TRIG_THRESHOLD (30 * 256)
#define DEFAULT_DEADZONE 4096
#define DEFAULT_DEADZONE_RY 6144

#define WHEEL_UP_MASK SDL_BUTTON(VK_MOUSE_WHEEL_UP - VK_MOUSE_BEGIN + 1)
#define WHEEL_DN_MASK SDL_BUTTON(VK_MOUSE_WHEEL_DN - VK_MOUSE_BEGIN + 1)

#define CURSOR_HIDE_THRESHOLD 1
#define CURSOR_HIDE_TIME 3000000 // us

// Gravity constant for converting accelerometer data from m/s^2 to units of gravity (g)
#define SDL_STANDARD_GRAVITY 9.80665f



static SDL_GameController *pads[INPUT_MAX_CONTROLLERS];

#define CONTROLLERCFG_DEFAULT { \
	.rumbleOn = 0, \
	.rumbleScale = 0.5f, \
	.axisMap = { \
		{ SDL_CONTROLLER_AXIS_LEFTX,  SDL_CONTROLLER_AXIS_LEFTY  }, \
		{ SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY }, \
	}, \
	.sens = { 1.f, 1.f, 1.f, 1.f }, \
	.deadzone = { DEFAULT_DEADZONE, DEFAULT_DEADZONE, DEFAULT_DEADZONE, DEFAULT_DEADZONE_RY }, \
	.stickCButtons = 0, \
	.swapSticks = 1, \
	.deviceIndex = -1, \
	.cancelCButtons = 0, \
	.gyroEnabled = 1, \
	.gyroAxisMode = GYRO_YAW, \
	.gyroAimMode = GYRO_AIM_CROSSHAIR, \
	.gyroModifier = GYRO_ALWAYS_ON, \
	.gyroAdvanced = 0, \
	.gyroSpeedX = 2.5f, \
	.gyroSpeedY = 2.5f, \
	.gyroAimSpeedX = 5.0f, \
	.gyroAimSpeedY = 5.0f, \
	.gyroVHMixer = 0.0f, \
	.gyroInvertX = 0, \
	.gyroInvertY = 0, \
	.gyroAimInvertX = 0, \
	.gyroAimInvertY = 0, \
	.gyroSmoothing = 0.18f, \
	.gyroTightening = 0.5f, \
	.gyroAutoCalibration = 1, \
}

static struct controllercfg {
	s32 rumbleOn;
	f32 rumbleScale;
	u32 axisMap[2][2];
	f32 sens[4];
	s32 deadzone[4];
	s32 gyroSensorActive;
	s32 stickCButtons;
	s32 swapSticks;
	s32 deviceIndex;
	s32 cancelCButtons;
	s32 gyroEnabled;
	s32 gyroAxisMode;
	s32 gyroAimMode;
	s32 gyroModifier;
	s32 gyroAdvanced;
	f32 gyroSpeedX;
	f32 gyroSpeedY;
	f32 gyroAimSpeedX;
	f32 gyroAimSpeedY;
	f32 gyroVHMixer;
	s32 gyroInvertX;
	s32 gyroInvertY;
	s32 gyroAimInvertX;
	s32 gyroAimInvertY;
	f32 gyroSmoothing;
	f32 gyroTightening;
	s32 gyroAutoCalibration;
} padsCfg[INPUT_MAX_CONTROLLERS] = {
	CONTROLLERCFG_DEFAULT,
	CONTROLLERCFG_DEFAULT,
	CONTROLLERCFG_DEFAULT,
	CONTROLLERCFG_DEFAULT
};

static u32 binds[MAXCONTROLLERS][CK_TOTAL_COUNT][INPUT_MAX_BINDS];
static char bindStrs[MAXCONTROLLERS][CK_TOTAL_COUNT][MAX_BIND_STR];

static s32 fakeControllers = 0;
static s32 firstController = 0;
static s32 connectedMask = 0;

static s32 numJoysticks = 0;

static s32 useHIDAPI = 1;
static s32 useRawInput = 0;

static s32 mouseEnabled = 1;
static s32 mouseX, mouseY;
static s32 mouseDX, mouseDY;
static u32 mouseButtons;
static s32 mouseWheel = 0;

static s32 mouseLocked = 0;
static s32 mouseLockMode = MLOCK_AUTO;
static u64 mouseCursorTime = 0;
static s32 mouseShowCursor = 1;

static f32 mouseSensX = 2.5f;
static f32 mouseSensY = 2.5f;

static s32 lastKey = 0;
static char lastChar = 0;
static s32 textInput = 0;

static char *clipboardText = NULL;



static const char *ckNames[CK_TOTAL_COUNT] = {
	"R_CBUTTONS",
	"L_CBUTTONS",
	"D_CBUTTONS",
	"U_CBUTTONS",
	"R_TRIG",
	"L_TRIG",
	"X_BUTTON",
	"Y_BUTTON",
	"R_JPAD",
	"L_JPAD",
	"D_JPAD",
	"U_JPAD",
	"START_BUTTON",
	"Z_TRIG",
	"B_BUTTON",
	"A_BUTTON",
	"STICK_XNEG",
	"STICK_XPOS",
	"STICK_YNEG",
	"STICK_YPOS",
	"ACCEPT_BUTTON",
	"CANCEL_BUTTON",
	"CK_0040",
	"CK_0080",
	"CK_0100",
	"CK_0200",
	"CK_0400",
	"CK_0800",
	"CK_1000",
	"CK_2000",
	"CK_4000",
	"CK_8000"
};

static const char *vkPunctNames[] = {
	"MINUS", "EQUALS", "LEFTBRACKET", "RIGHTBRACKET", "BACKSLASH",
	"HASH", "SEMICOLON", "APOSTROPHE", "GRAVE", "COMMA", "PERIOD", "SLASH"
};

static const char *vkMouseNames[] = {
	"MOUSE_LEFT",
	"MOUSE_MIDDLE",
	"MOUSE_RIGHT",
	"MOUSE_X1",
	"MOUSE_X2",
	"MOUSE_WHEEL_UP",
	"MOUSE_WHEEL_DN",
};

static const char *vkJoyNames[] = {
	"JOY1_A",
	"JOY1_B",
	"JOY1_X",
	"JOY1_Y",
	"JOY1_BACK",
	"JOY1_GUIDE",
	"JOY1_START",
	"JOY1_LSTICK",
	"JOY1_RSTICK",
	"JOY1_LSHOULDER",
	"JOY1_RSHOULDER",
	"JOY1_DPAD_UP",
	"JOY1_DPAD_DOWN",
	"JOY1_DPAD_LEFT",
	"JOY1_DPAD_RIGHT",
	"JOY1_BUTTON_15",
	"JOY1_BUTTON_16",
	"JOY1_BUTTON_17",
	"JOY1_BUTTON_18",
	"JOY1_BUTTON_19",
	"JOY1_TOUCHPAD",
	"JOY1_BUTTON_21",
	"JOY1_BUTTON_22",
	"JOY1_BUTTON_23",
	"JOY1_BUTTON_24",
	"JOY1_BUTTON_25",
	"JOY1_BUTTON_26",
	"JOY1_BUTTON_27",
	"JOY1_BUTTON_28",
	"JOY1_BUTTON_29",
	"JOY1_LTRIGGER",
	"JOY1_RTRIGGER",
};

static char vkNames[VK_TOTAL_COUNT][64];

static s8 vkPrevState[VK_TOTAL_COUNT];

void inputSetDefaultKeyBinds(s32 cidx, s32 n64mode)
{
	// TODO: make VK constants for all these
	static const u32 pckbbinds[][3] = {
		{ CK_B,             SDL_SCANCODE_E,      0                   },
		{ CK_X,             SDL_SCANCODE_R,      0                   },
		{ CK_RTRIG,         VK_MOUSE_RIGHT,      SDL_SCANCODE_Z      },
		{ CK_LTRIG,         SDL_SCANCODE_F,      SDL_SCANCODE_X      },
		{ CK_ZTRIG,         VK_MOUSE_LEFT,       SDL_SCANCODE_SPACE  },
		{ CK_START,         SDL_SCANCODE_TAB,    0                   },
		{ CK_DPAD_D,        SDL_SCANCODE_Q,      VK_MOUSE_MIDDLE     },
		{ CK_DPAD_U,        0,                   0                   },
		{ CK_Y,             VK_MOUSE_WHEEL_DN,   0                   },
		{ CK_DPAD_L,        VK_MOUSE_WHEEL_UP,   0                   },
		{ CK_C_D,           SDL_SCANCODE_S,      0                   },
		{ CK_C_U,           SDL_SCANCODE_W,      0                   },
		{ CK_C_R,           SDL_SCANCODE_D,      0                   },
		{ CK_C_L,           SDL_SCANCODE_A,      0                   },
		{ CK_STICK_XNEG,    SDL_SCANCODE_LEFT,   0                   },
		{ CK_STICK_XPOS,    SDL_SCANCODE_RIGHT,  0                   },
		{ CK_STICK_YNEG,    SDL_SCANCODE_DOWN,   0                   },
		{ CK_STICK_YPOS,    SDL_SCANCODE_UP,     0                   },
		{ CK_0100,          SDL_SCANCODE_F10,    0                   },
		{ CK_4000,          SDL_SCANCODE_LSHIFT, 0                   },
		{ CK_2000,          SDL_SCANCODE_LCTRL,  0                   },
		{ CK_ACCEPT,        SDL_SCANCODE_RETURN, SDL_SCANCODE_E      },
		{ CK_CANCEL,        VK_MOUSE_RIGHT,      0                   },
	};

	static const u32 pcjoybinds[][2] = {
		{ CK_A,      SDL_CONTROLLER_BUTTON_A             },
		{ CK_X,      SDL_CONTROLLER_BUTTON_X             },
		{ CK_Y,      SDL_CONTROLLER_BUTTON_Y             },
		{ CK_DPAD_L, SDL_CONTROLLER_BUTTON_B             },
		{ CK_DPAD_D, SDL_CONTROLLER_BUTTON_LEFTSHOULDER  },
		{ CK_LTRIG,  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
		{ CK_RTRIG,  VK_JOY1_LTRIG - VK_JOY1_BEGIN       },
		{ CK_ZTRIG,  VK_JOY1_RTRIG - VK_JOY1_BEGIN       },
		{ CK_START,  SDL_CONTROLLER_BUTTON_START         },
		{ CK_C_D,    SDL_CONTROLLER_BUTTON_DPAD_DOWN     },
		{ CK_C_U,    SDL_CONTROLLER_BUTTON_DPAD_UP       },
		{ CK_C_R,    SDL_CONTROLLER_BUTTON_DPAD_RIGHT    },
		{ CK_C_L,    SDL_CONTROLLER_BUTTON_DPAD_LEFT     },
		{ CK_ACCEPT, SDL_CONTROLLER_BUTTON_A             },
		{ CK_CANCEL, SDL_CONTROLLER_BUTTON_B             },
		{ CK_0100,   SDL_CONTROLLER_BUTTON_BACK          },
		{ CK_8000,   SDL_CONTROLLER_BUTTON_LEFTSTICK     },
	};

	static const u32 n64kbbinds[][3] = {
		{ CK_A,          SDL_SCANCODE_Q,      0                  },
		{ CK_B,          SDL_SCANCODE_E,      0                  },
		{ CK_RTRIG,      VK_MOUSE_RIGHT,      SDL_SCANCODE_LALT  },
		{ CK_LTRIG,      SDL_SCANCODE_F,      0                  },
		{ CK_ZTRIG,      VK_MOUSE_LEFT,       SDL_SCANCODE_SPACE },
		{ CK_START,      SDL_SCANCODE_RETURN, 0                  },
		{ CK_C_D,        SDL_SCANCODE_S,      0                  },
		{ CK_C_U,        SDL_SCANCODE_W,      0                  },
		{ CK_C_R,        SDL_SCANCODE_D,      0                  },
		{ CK_C_L,        SDL_SCANCODE_A,      0                  },
		{ CK_DPAD_L,     SDL_SCANCODE_LEFT,   0                  },
		{ CK_DPAD_R,     SDL_SCANCODE_RIGHT,  0                  },
		{ CK_DPAD_D,     SDL_SCANCODE_DOWN,   0                  },
		{ CK_DPAD_U,     SDL_SCANCODE_UP,     0                  },
		{ CK_STICK_YNEG, SDL_SCANCODE_K,      0                  },
		{ CK_STICK_YPOS, SDL_SCANCODE_I,      0                  },
		{ CK_STICK_XNEG, SDL_SCANCODE_J,      0                  },
		{ CK_STICK_XPOS, SDL_SCANCODE_L,      0                  },
		{ CK_0100,       SDL_SCANCODE_F10,    0                  },
	};

	static const u32 n64joybinds[][2] = {
		{ CK_A,      SDL_CONTROLLER_BUTTON_A             },
		{ CK_B,      SDL_CONTROLLER_BUTTON_B             },
		{ CK_LTRIG,  SDL_CONTROLLER_BUTTON_LEFTSHOULDER  },
		{ CK_RTRIG,  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
		{ CK_ZTRIG,  VK_JOY1_RTRIG - VK_JOY1_BEGIN       },
		{ CK_START,  SDL_CONTROLLER_BUTTON_START         },
		{ CK_DPAD_D, SDL_CONTROLLER_BUTTON_DPAD_DOWN     },
		{ CK_DPAD_U, SDL_CONTROLLER_BUTTON_DPAD_UP       },
		{ CK_DPAD_L, SDL_CONTROLLER_BUTTON_DPAD_LEFT     },
		{ CK_DPAD_R, SDL_CONTROLLER_BUTTON_DPAD_RIGHT    },
		{ CK_0100,   SDL_CONTROLLER_BUTTON_BACK          },
	};

	memset(binds[cidx], 0, sizeof(binds[cidx]));

	const u32 (*kbbinds)[3];
	const u32 (*joybinds)[2];
	u32 numkbbinds;
	u32 numjoybinds;
	if (n64mode) {
		kbbinds = n64kbbinds;
		joybinds = n64joybinds;
		numkbbinds = sizeof(n64kbbinds) / sizeof(n64kbbinds[0]);
		numjoybinds = sizeof(n64joybinds) / sizeof(n64joybinds[0]);
	} else {
		kbbinds = pckbbinds;
		joybinds = pcjoybinds;
		numkbbinds = sizeof(pckbbinds) / sizeof(pckbbinds[0]);
		numjoybinds = sizeof(pcjoybinds) / sizeof(pcjoybinds[0]);
	}

	if (cidx == 0) {
		for (u32 i = 0; i < numkbbinds; ++i) {
			for (s32 j = 1; j < 3; ++j) {
				if (kbbinds[i][j]) {
					inputKeyBind(cidx, kbbinds[i][0], j - 1, kbbinds[i][j]);
				}
			}
		}
	}

	for (u32 i = 0; i < numjoybinds; ++i) {
		inputKeyBind(cidx, joybinds[i][0], -1, VK_JOY_BEGIN + cidx * INPUT_MAX_CONTROLLER_BUTTONS + joybinds[i][1]);
	}
}

static inline s32 inputDeviceIndexFromId(const SDL_JoystickID id) {
	for (s32 jidx = 0; jidx < numJoysticks; ++jidx) {
		if (SDL_JoystickGetDeviceInstanceID(jidx) == id) {
			return jidx;
		}
	}
	return -1;
}

static inline SDL_JoystickID inputControllerGetId(SDL_GameController *ctrl)
{
	return SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
}

static inline void inputInitController(const s32 cidx, const s32 jidx)
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
	// SDL_GameControllerHasRumble() appeared in 2.0.18 even though SDL_GameControllerRumble() is in 2.0.9
	padsCfg[cidx].rumbleOn = SDL_GameControllerHasRumble(pads[cidx]);
#else
	// assume that all joysticks with haptic feedback support will support rumble
	padsCfg[cidx].rumbleOn = SDL_JoystickIsHaptic(SDL_GameControllerGetJoystick(pads[cidx]));
	if (!padsCfg[cidx].rumbleOn) {
		// at least on Windows some controllers will report no haptics, but rumble will still function
		// just assume it's supported if the controller is of known type
		const SDL_GameControllerType ctype = SDL_GameControllerGetType(pads[cidx]);
		padsCfg[cidx].rumbleOn = ctype && (ctype != SDL_CONTROLLER_TYPE_VIRTUAL);
	}
#endif

	// make the LEDs on the controller indicate which player it's for
	SDL_GameControllerSetPlayerIndex(pads[cidx], cidx);

	// remember the joystick index
	padsCfg[cidx].deviceIndex = jidx;

	connectedMask |= (1 << cidx);

	sysLogPrintf(LOG_NOTE, "input: assigned controller '%d: (%s)' (id %d) to player %d",
		jidx, SDL_GameControllerName(pads[cidx]), inputControllerGetId(pads[cidx]), cidx);

	SDL_Joystick* joy = SDL_GameControllerGetJoystick(pads[cidx]);
	if (joy) {
		char guidStr[1024] = "";
		SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
		SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
		sysLogPrintf(LOG_NOTE, "input: GUID for controller %d: %s", jidx, guidStr);
	}

#if SDL_VERSION_ATLEAST(2, 0, 14)
	// initialize motion sensors
	padsCfg[cidx].gyroSensorActive = 0;

	if (!SDL_GameControllerHasSensor(pads[cidx], SDL_SENSOR_GYRO) || 
	    !SDL_GameControllerHasSensor(pads[cidx], SDL_SENSOR_ACCEL)) {
		return;
	}

	// while hotswapping: nintendo switch controllers sensors under bluetooth will not work properly,
	// this can be fixed by either changing/reset the controller id order (within controller options), or restarting the game while connected.
	sysLogPrintf(LOG_NOTE, "input: assigned controller's motion sensors detected for player %d", cidx);

	if (SDL_GameControllerSetSensorEnabled(pads[cidx], SDL_SENSOR_GYRO, SDL_TRUE) != 0 ||
	    SDL_GameControllerSetSensorEnabled(pads[cidx], SDL_SENSOR_ACCEL, SDL_TRUE) != 0) {
		return;
	}

	padsCfg[cidx].gyroSensorActive = 1;

	gyroInitController(cidx);
#endif
}

static inline void inputCloseController(const s32 cidx)
{
	sysLogPrintf(LOG_NOTE, "input: removed controller '%d: (%s)' (id %d) from player %d",
		padsCfg[cidx].deviceIndex, SDL_GameControllerName(pads[cidx]), inputControllerGetId(pads[cidx]), cidx);

	// reset player LEDs
	SDL_GameControllerSetPlayerIndex(pads[cidx], -1);

	SDL_GameControllerClose(pads[cidx]);

	pads[cidx] = NULL;
	padsCfg[cidx].rumbleOn = 0;
	gyroCloseController(cidx);
	padsCfg[cidx].gyroSensorActive = 0;

	if (cidx) {
		connectedMask &= ~(1 << cidx);
	}
}

static inline s32 inputControllerGetIndex(SDL_GameController *ctrl)
{
	if (ctrl) {
		for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
			if (pads[i] == ctrl) {
				return i;
			}
		}
	}
	return -1;
}

static inline s32 inputControllerGetIndexByDeviceIndex(const s32 jidx)
{
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		if (pads[cidx] && padsCfg[cidx].deviceIndex == jidx) {
			return cidx;
		}
	}
	return -1;
}

static inline s32 inputControllerGetIndexById(const SDL_JoystickID jid)
{
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		if (pads[cidx]) {
			if (inputControllerGetId(pads[cidx]) == jid) {
				return cidx;
			}
		}
	}
	return -1;
}

static inline void inputCloseAllControllers(void)
{
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		if (pads[cidx]) {
			inputCloseController(cidx);
			pads[cidx] = NULL;
		}
	}

	connectedMask = 1; // always report first controller as connected
}

static inline s32 inputTryController(const s32 cidx, const s32 jidx)
{
	if (!pads[cidx]) {
		pads[cidx] = SDL_GameControllerOpen(jidx);
		if (pads[cidx]) {
			inputInitController(cidx, jidx);
			return 1;
		}
	}
	return 0;
}

static inline void inputInitAllControllers(void)
{
	SDL_GameControllerUpdate();

	numJoysticks = SDL_NumJoysticks();

	connectedMask = 1; // always report first controller as connected

	// first try to assign the controllers that we had last time
	// we're still free to check by device index before any controller device events fire
	for (s32 cidx = 0; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
		const s32 jidx = padsCfg[cidx].deviceIndex;
		if (jidx >= 0 && jidx < numJoysticks) {
			if (SDL_IsGameController(jidx) && inputControllerGetIndexByDeviceIndex(jidx) < 0) {
				// using the full assign function in case user sets same index for several players
				if (inputTryController(cidx, jidx)) {
					// success
					continue;
				}
			}
			// nothing was there, forget it
			padsCfg[cidx].deviceIndex = -1;
		}
	}

	// now try autofilling the rest, starting with firstController
	for (s32 jidx = 0; jidx < numJoysticks; ++jidx) {
		if (SDL_IsGameController(jidx) && inputControllerGetIndexByDeviceIndex(jidx) < 0) {
			for (s32 cidx = firstController; cidx < INPUT_MAX_CONTROLLERS; ++cidx) {
				if (inputTryController(cidx, jidx)) {
					break;
				}
			}
		}
	}

	const s32 overrideMask = (1 << fakeControllers) - 1;
	if (overrideMask) {
		connectedMask = overrideMask;
	}
}

static int inputEventFilter(void *data, SDL_Event *event)
{
	switch (event->type) {
		case SDL_CONTROLLERDEVICEADDED:
			for (s32 i = firstController; i < INPUT_MAX_CONTROLLERS; ++i) {
				if (!pads[i]) {
					pads[i] = SDL_GameControllerOpen(event->cdevice.which);
					if (pads[i]) {
						inputInitController(i, event->cdevice.which);
					}
					break;
				}
			}
			break;

		case SDL_CONTROLLERDEVICEREMOVED: {
			SDL_GameController *ctrl = SDL_GameControllerFromInstanceID(event->cdevice.which);
			const s32 idx = inputControllerGetIndex(ctrl);
			if (idx >= 0) {
				inputCloseController(idx);
				padsCfg[idx].deviceIndex = -1;
			}
			break;
		}

		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			numJoysticks = SDL_NumJoysticks(); // joystick count has changed
			break;

		case SDL_MOUSEWHEEL:
			mouseWheel = event->wheel.y;
			if (!lastKey && mouseWheel) {
				lastKey = (mouseWheel < 0) + VK_MOUSE_WHEEL_UP;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (!lastKey) {
				lastKey = VK_MOUSE_BEGIN - 1 + event->button.button;
			}
			break;

		case SDL_KEYDOWN:
			if (!lastKey) {
				lastKey = VK_KEYBOARD_BEGIN + event->key.keysym.scancode;
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
			if (!lastKey) {
				lastKey = VK_JOY1_BEGIN + event->cbutton.button;
				SDL_GameController *ctrl = SDL_GameControllerFromInstanceID(event->cdevice.which);
				const s32 idx = inputControllerGetIndex(ctrl);
				if (idx >= 0) {
					lastKey += idx * INPUT_MAX_CONTROLLER_BUTTONS;
				}
			}
			break;

		case SDL_CONTROLLERAXISMOTION:
			if (!lastKey) {
				if (event->caxis.axis >= SDL_CONTROLLER_AXIS_TRIGGERLEFT && event->caxis.value > TRIG_THRESHOLD) {
					lastKey = VK_JOY1_LTRIG + (event->caxis.axis - SDL_CONTROLLER_AXIS_TRIGGERLEFT);
					SDL_GameController *ctrl = SDL_GameControllerFromInstanceID(event->cdevice.which);
					const s32 idx = inputControllerGetIndex(ctrl);
					if (idx >= 0) {
						lastKey += idx * INPUT_MAX_CONTROLLER_BUTTONS;
					}
				}
			}
			break;

		case SDL_CONTROLLERSENSORUPDATE: {
			SDL_GameController *ctrl = SDL_GameControllerFromInstanceID(event->csensor.which);
			const s32 idx = inputControllerGetIndex(ctrl);
			if (idx >= 0 && idx < INPUT_MAX_CONTROLLERS) {
				if (event->csensor.sensor == SDL_SENSOR_GYRO) {
					const float r2d = 180.f / (float)M_PI;
					gyroFeedAngVel(idx,
						event->csensor.data[0] * r2d,
						event->csensor.data[1] * r2d,
						event->csensor.data[2] * r2d);
				} else if (event->csensor.sensor == SDL_SENSOR_ACCEL) {
					gyroFeedAccel(idx,
						event->csensor.data[0] / SDL_STANDARD_GRAVITY,
						event->csensor.data[1] / SDL_STANDARD_GRAVITY,
						event->csensor.data[2] / SDL_STANDARD_GRAVITY);
				}
			}
			break;
		}

		case SDL_TEXTINPUT:
			if (!lastChar && event->text.text[0] && (u8)event->text.text[0] < 0x80) {
				lastChar = event->text.text[0];
			}
			break;

		default:
			break;
	}

	return 0;
}

static inline void inputGetScancodeName(const SDL_Scancode sc, char *out, size_t len)
{
		const char *scname = SDL_GetScancodeName(sc);
		if (scname) {
			strncpy(out, scname, len - 1);
			for (u32 i = 0; i < len && out[i]; ++i) {
				if (out[i] == ' ') {
					out[i] = '_';
				} else {
					out[i] = toupper(out[i]);
				}
			}
		} else {
			snprintf(out, len, "KEY%d", (s32)sc);
		}
}

static inline void inputInitKeyNames(void)
{
	for (SDL_Scancode key = SDL_SCANCODE_A; key <= SDL_SCANCODE_SPACE; ++key) {
		inputGetScancodeName(key, vkNames[key], sizeof(vkNames[key]));
	}

	// special characters
	for (SDL_Scancode key = SDL_SCANCODE_MINUS; key < SDL_SCANCODE_CAPSLOCK; ++key) {
		strcpy(vkNames[key], vkPunctNames[key - SDL_SCANCODE_MINUS]);
	}

	for (SDL_Scancode key = SDL_SCANCODE_CAPSLOCK; key <= SDL_SCANCODE_NUMLOCKCLEAR; ++key) {
		inputGetScancodeName(key, vkNames[key], sizeof(vkNames[key]));
	}

	// keypad names
	strcpy(vkNames[SDL_SCANCODE_KP_DIVIDE], "KP_DIVIDE");
	strcpy(vkNames[SDL_SCANCODE_KP_MULTIPLY], "KP_MULTIPLY");
	strcpy(vkNames[SDL_SCANCODE_KP_MINUS], "KP_MINUS");
	strcpy(vkNames[SDL_SCANCODE_KP_PLUS], "KP_PLUS");
	strcpy(vkNames[SDL_SCANCODE_KP_ENTER], "KP_ENTER");
	strcpy(vkNames[SDL_SCANCODE_KP_PERIOD], "KP_PERIOD");
	strcpy(vkNames[SDL_SCANCODE_KP_EQUALS], "KP_EQUALS");
	for (SDL_Scancode key = SDL_SCANCODE_KP_1; key < SDL_SCANCODE_KP_0; ++key) {
		char tmp[8] = "KP_1";
		tmp[3] = '1' + (key - SDL_SCANCODE_KP_1);
		strcpy(vkNames[key], tmp);
	}

	for (SDL_Scancode key = SDL_SCANCODE_LCTRL; key <= SDL_SCANCODE_RGUI; ++key) {
		inputGetScancodeName(key, vkNames[key], sizeof(vkNames[key]));
	}

	// mouse names
	for (u32 vk = VK_MOUSE_BEGIN; vk < VK_JOY1_BEGIN; ++vk) {
		strcpy(vkNames[vk], vkMouseNames[vk - VK_MOUSE_BEGIN]);
	}

	// joystick names
	for (u32 vk = VK_JOY1_BEGIN; vk < VK_TOTAL_COUNT; ++vk) {
		const u32 jidx = (vk - VK_JOY1_BEGIN) / INPUT_MAX_CONTROLLER_BUTTONS;
		const u32 jbtn = (vk - VK_JOY1_BEGIN) % INPUT_MAX_CONTROLLER_BUTTONS;
		strcpy(vkNames[vk], vkJoyNames[jbtn]);
		vkNames[vk][3] = '1' + jidx;
	}
}

void inputSaveBinds(void)
{
	char *bindstr;

	for (s32 i = 0; i < MAXCONTROLLERS; ++i) {
		for (u32 ck = 0; ck < CK_TOTAL_COUNT; ++ck) {
			bindstr = bindStrs[i][ck];
			bindstr[0] = '\0';
			for (s32 b = 0; b < INPUT_MAX_BINDS; ++b) {
				if (binds[i][ck][b]) {
					if (b) {
						strncat(bindstr, ", ", MAX_BIND_STR - 1);
					}
					strncat(bindstr, inputGetKeyName(binds[i][ck][b]), MAX_BIND_STR - 1);
				}
			}
			if (!bindstr[0]) {
				strcpy(bindstr, "NONE");
			}
		}
	}
}

static inline void inputParseBindString(const s32 ctrl, const u32 ck, char *bindstr)
{
	if (!bindstr[0]) {
		// empty string, keep defaults
		return;
	}

	// unbind all first
	memset(binds[ctrl][ck], 0, sizeof(binds[ctrl][ck]));

	if (!strcasecmp(bindstr, "NONE")) {
		// explicitly nothing bound
		return;
	}

	const char *tok = strtok(bindstr, ", ");
	while (tok) {
		if (tok[0]) {
			const s32 vk = inputGetKeyByName(tok);
			if (vk > 0) {
				inputKeyBind(ctrl, ck, -1, vk);
			}
		}
		tok = strtok(NULL, ", ");
	}
}

static inline void inputLoadBinds(void)
{
	for (s32 i = 0; i < MAXCONTROLLERS; ++i) {
		for (u32 ck = 0; ck < CK_TOTAL_COUNT; ++ck) {
			inputParseBindString(i, ck, bindStrs[i][ck]);
		}
	}
}

s32 inputInit(void)
{
	// Set SDL hints before initializing the controller subsystem.
	if (useHIDAPI) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 14)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 22)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "1");
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
		// the two hints below enable Rumble and Motion Sensor for PS4/5 pads connected via bluetooth
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAM, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 23, 2)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_COMBINE_JOY_CONS, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 25, 1)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 26, 0)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 30, 0)
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");
#endif
	}
	if (useRawInput) {
		SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "1");
#ifdef SDL_HINT_JOYSTICK_RAWINPUT_CORRELATE_XINPUT
		SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT_CORRELATE_XINPUT, "1");
#elif defined(SDL_HINT_JOYSTICK_HIDAPI_CORRELATE_XINPUT)
		// old name for SDL_HINT_JOYSTICK_RAWINPUT_CORRELATE_XINPUT before SDL2.0.16
		SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_CORRELATE_XINPUT, "1");
#endif
	}

	if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC)) {
		SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
	}

	// try to load controller db from an external file in the save folder
	if (fsFileSize("$S/" CONTROLLERDB_FNAME)) {
		const char *dbpath = fsFullPath("$S/" CONTROLLERDB_FNAME);
		const s32 dbcount = SDL_GameControllerAddMappingsFromFile(dbpath);
		if (dbcount >= 0) {
			sysLogPrintf(LOG_NOTE, "input: added %d controller mappings from %s", dbcount, dbpath);
		}
	}

	inputInitAllControllers();

	// since the main event loop is elsewhere, we can receive some events we need using a watcher
	SDL_AddEventWatch(inputEventFilter, NULL);

	inputInitKeyNames();

	for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
		inputSetDefaultKeyBinds(i, 0);
	}

	if (mouseLockMode != MLOCK_AUTO) {
		inputLockMouse(mouseLockMode);
	}

	// update the axis maps
	// NOTE: by default sticks get swapped for 1.2: "right stick" here means left stick on your controller
	for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
		inputControllerSetSticksSwapped(i, padsCfg[i].swapSticks);
	}

	inputLoadBinds();
  
  	// Explicitly stop text input to prevent key events being swallowed on certain Wayland systems
	inputStopTextInput();

	// if GyroAdvanced is disabled: GyroSpeed/GyroAimSpeed will sync to whatever is higher between X and Y
	for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
		if (!padsCfg[i].gyroAdvanced) {
			padsCfg[i].gyroSpeedX = padsCfg[i].gyroSpeedY = fmaxf(padsCfg[i].gyroSpeedX, padsCfg[i].gyroSpeedY);
			padsCfg[i].gyroAimSpeedX = padsCfg[i].gyroAimSpeedY = fmaxf(padsCfg[i].gyroAimSpeedX, padsCfg[i].gyroAimSpeedY);
		}
	}

	return connectedMask;
}

s32 inputBindPressed(const s32 idx, const u32 ck)
{
	for (s32 i = 0; i < INPUT_MAX_BINDS; ++i) {
		if (binds[idx][ck][i]) {
			if (inputKeyPressed(binds[idx][ck][i])) {
				return 1;
			}
		}
	}
	return 0;
}

static inline s32 inputAxisScale(s32 x, const s32 deadzone, const f32 scale)
{
	if (abs(x) < deadzone) {
		return 0;
	} else {
		// rescale to fit the non-deadzone range
		if (x < 0) {
			x += deadzone;
		} else {
			x -= deadzone;
		}
		x = x * 32768 / (32768 - deadzone);
		// scale with sensitivity
		x *= scale;
		return (x > 32767) ? 32767 : ((x < -32768) ? -32768 : x);
	}
}

s32 inputReadController(s32 idx, OSContPad *npad)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS  || !npad) {
		return -1;
	}

	npad->button = 0;

	if (textInput) {
		npad->stick_x = 0;
		npad->stick_y = 0;
		npad->rstick_x = 0;
		npad->rstick_y = 0;
		return 0;
	}

	for (u32 i = 0; i < CONT_NUM_BUTTONS; ++i) {
		if (inputBindPressed(idx, i)) {
			npad->button |= 1U << i;
		}
	}

	const s32 xdiff = (inputBindPressed(idx, CK_STICK_XPOS) - inputBindPressed(idx, CK_STICK_XNEG));
	const s32 ydiff = (inputBindPressed(idx, CK_STICK_YPOS) - inputBindPressed(idx, CK_STICK_YNEG));
	npad->stick_x = xdiff < 0 ? -0x80 : (xdiff > 0 ? 0x7F : 0);
	npad->stick_y = ydiff < 0 ? -0x80 : (ydiff > 0 ? 0x7F : 0);

	const struct controllercfg *cfg = &padsCfg[idx];

	if (cfg->cancelCButtons) {
		// opposite C buttons cancel each other out
		if ((npad->button & (L_CBUTTONS | R_CBUTTONS)) == (L_CBUTTONS | R_CBUTTONS)) {
			npad->button &= ~(L_CBUTTONS | R_CBUTTONS);
		}
		if ((npad->button & (U_CBUTTONS | D_CBUTTONS)) == (U_CBUTTONS | D_CBUTTONS)) {
			npad->button &= ~(U_CBUTTONS | D_CBUTTONS);
		}
	}

	if (!pads[idx]) {
		return 0;
	}

	s32 leftX = SDL_GameControllerGetAxis(pads[idx], cfg->axisMap[0][0]);
	s32 leftY = SDL_GameControllerGetAxis(pads[idx], cfg->axisMap[0][1]);
	s32 rightX = SDL_GameControllerGetAxis(pads[idx], cfg->axisMap[1][0]);
	s32 rightY = SDL_GameControllerGetAxis(pads[idx], cfg->axisMap[1][1]);

	leftX = inputAxisScale(leftX, cfg->deadzone[cfg->axisMap[0][0]], cfg->sens[cfg->axisMap[0][0]]);
	leftY = inputAxisScale(leftY, cfg->deadzone[cfg->axisMap[0][1]], cfg->sens[cfg->axisMap[0][1]]);
	rightX = inputAxisScale(rightX, cfg->deadzone[cfg->axisMap[1][0]], cfg->sens[cfg->axisMap[1][0]]);
	rightY = inputAxisScale(rightY, cfg->deadzone[cfg->axisMap[1][1]], cfg->sens[cfg->axisMap[1][1]]);

	if (!npad->stick_x && leftX) {
		npad->stick_x = leftX / 0x100;
	}

	s32 stickY = -leftY / 0x100;
	if (!npad->stick_y && stickY) {
		npad->stick_y = (stickY == 128) ? 127 : stickY;
	}

	if (cfg->stickCButtons) {
		// rstick emulates C buttons
		if (rightX < -0x4000) npad->button |= L_CBUTTONS;
		if (rightX > +0x4000) npad->button |= R_CBUTTONS;
		if (rightY < -0x4000) npad->button |= U_CBUTTONS;
		if (rightY > +0x4000) npad->button |= D_CBUTTONS;
		npad->rstick_x = 0;
		npad->rstick_y = 0;
	} else {
		// rstick is an analog input
		if (rightX) {
			npad->rstick_x = rightX / 0x100;
		}
		s32 rStickY = -rightY / 0x100;
		if (rStickY) {
			npad->rstick_y = (rStickY == 128) ? 127 : rStickY;
		}
	}

	return 0;
}

static inline void inputUpdateMouse(void)
{
	s32 mx, my;
	mouseButtons = SDL_GetMouseState(&mx, &my);

	if (mouseWheel > 0) {
		mouseButtons |= WHEEL_UP_MASK;
	} else if (mouseWheel < 0) {
		mouseButtons |= WHEEL_DN_MASK;
	}

	mouseWheel = 0;

	s32 mdx = 0;
	s32 mdy = 0;
	SDL_GetRelativeMouseState(&mdx, &mdy);
	if (mouseLocked) {
		mouseDX = mdx;
		mouseDY = mdy;
	} else {
		mouseDX = mx - mouseX;
		mouseDY = my - mouseY;
	}

	mouseX = mx;
	mouseY = my;

	// if MLOCK_AUTO is enabled, disable cursor if mouse is unlocked
	// and we haven't moved it for a few seconds
	if (mouseLockMode == MLOCK_AUTO && !mouseLocked) {
		if (abs(mouseDX) > CURSOR_HIDE_THRESHOLD || abs(mouseDY) > CURSOR_HIDE_THRESHOLD) {
			if (!mouseShowCursor) {
				inputMouseShowCursor(1);
			}
		} else if (sysGetMicroseconds() > mouseCursorTime) {
			if (mouseShowCursor) {
				inputMouseShowCursor(0);
			}
		}
	}
}

s32 inputControllerMotionSensorsSupported(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	
	return padsCfg[cidx].gyroSensorActive;
}


void inputUpdate(void)
{
	SDL_GameControllerUpdate();

	if (mouseEnabled) {
		inputUpdateMouse();
	}

	gyroUpdateAll();
}

s32 inputControllerConnected(s32 idx)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	return pads[idx] || (connectedMask & (1 << idx));
}

s32 inputRumbleSupported(s32 idx)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	return padsCfg[idx].rumbleOn;
}

void inputRumble(s32 idx, f32 strength, f32 time)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || !pads[idx]) {
		return;
	}

	if (padsCfg[idx].rumbleScale <= 0.f) {
		return;
	}

	if (padsCfg[idx].rumbleOn) {
		strength *= padsCfg[idx].rumbleScale;
		if (strength <= 0.f) {
			strength = 0.f;
			time = 0.f;
		} else {
			strength *= 65535.f;
			time *= 1000.f;
		}
		SDL_GameControllerRumble(pads[idx], (u16)strength, (u16)strength, (u32)time);
	}
}

f32 inputRumbleGetStrength(s32 cidx)
{
	return padsCfg[cidx].rumbleScale;
}

void inputRumbleSetStrength(s32 cidx, f32 val)
{
	padsCfg[cidx].rumbleScale = val;
}

s32 inputControllerMask(void)
{
	return connectedMask;
}

s32 inputControllerGetSticksSwapped(s32 cidx)
{
	return padsCfg[cidx].swapSticks;
}

void inputControllerSetSticksSwapped(s32 cidx, s32 swapped)
{
	padsCfg[cidx].swapSticks = swapped;
	if (swapped) {
		padsCfg[cidx].axisMap[0][0] = SDL_CONTROLLER_AXIS_RIGHTX;
		padsCfg[cidx].axisMap[0][1] = SDL_CONTROLLER_AXIS_RIGHTY;
		padsCfg[cidx].axisMap[1][0] = SDL_CONTROLLER_AXIS_LEFTX;
		padsCfg[cidx].axisMap[1][1] = SDL_CONTROLLER_AXIS_LEFTY;
	} else {
		padsCfg[cidx].axisMap[0][0] = SDL_CONTROLLER_AXIS_LEFTX;
		padsCfg[cidx].axisMap[0][1] = SDL_CONTROLLER_AXIS_LEFTY;
		padsCfg[cidx].axisMap[1][0] = SDL_CONTROLLER_AXIS_RIGHTX;
		padsCfg[cidx].axisMap[1][1] = SDL_CONTROLLER_AXIS_RIGHTY;
	}
}

s32 inputControllerGetDualAnalog(s32 cidx)
{
	return !padsCfg[cidx].stickCButtons;
}

void inputControllerSetDualAnalog(s32 cidx, s32 enable)
{
	padsCfg[cidx].stickCButtons = !enable;
}

s32 inputControllerGetCancelCButtons(s32 cidx)
{
	return padsCfg[cidx].cancelCButtons;
}

void inputControllerSetCancelCButtons(s32 cidx, s32 cancel)
{
	padsCfg[cidx].cancelCButtons = cancel;
}

f32 inputControllerGetAxisScale(s32 cidx, s32 stick, s32 axis)
{
	return padsCfg[cidx].sens[stick * 2 + axis];
}

void inputControllerSetAxisScale(s32 cidx, s32 stick, s32 axis, f32 value)
{
	padsCfg[cidx].sens[stick * 2 + axis] = value;
}

f32 inputControllerGetAxisDeadzone(s32 cidx, s32 stick, s32 axis)
{
	return (f32)padsCfg[cidx].deadzone[stick * 2 + axis] / 32767.f;
}

void inputControllerSetAxisDeadzone(s32 cidx, s32 stick, s32 axis, f32 value)
{
	padsCfg[cidx].deadzone[stick * 2 + axis] = value * 32767.f;
}

s32 inputGetConnectedControllers(s32 *out)
{
	s32 count = 0;

	for (s32 jidx = 0; jidx < numJoysticks; ++jidx) {
		if (SDL_IsGameController(jidx)) {
			if (out && count < INPUT_MAX_CONNECTED_CONTROLLERS) {
				out[count] = SDL_JoystickGetDeviceInstanceID(jidx);
			}
			++count;
		}
	}

	return count;
}

s32 inputGetAssignedControllerId(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) {
		return -1;
	}

	if (pads[cidx] == NULL) {
		return -1;
	}

	return inputControllerGetId(pads[cidx]);
}

const char *inputGetConnectedControllerName(s32 id)
{
	static char fullName[256];

	if (id < 0) {
		return "Invalid";
	}

	const s32 jidx = inputDeviceIndexFromId(id);
	if (jidx < 0) {
		return "Invalid";
	}

	const char *name = SDL_GameControllerNameForIndex(jidx);
	if (!name || !name[0]) {
		name = "Unnamed Controller";
	}

	snprintf(fullName, sizeof(fullName), "%d: %s", jidx, name);

	// replace non-ascii chars with spaces
	for (char *p = fullName; *p; ++p) {
		if ((u32)*p >= 0x7f) {
			*p = ' ';
		}
	}

	return fullName;
}

s32 inputAssignController(s32 cidx, s32 id)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}

	if (id < 0) {
		// close current controller, if any
		if (pads[cidx]) {
			inputCloseController(cidx);
			return 1;
		}
		return 0;
	}

	const s32 jidx = inputDeviceIndexFromId(id);
	if (jidx < 0 || jidx >= SDL_NumJoysticks() || !SDL_IsGameController(jidx)) {
		return 0;
	}

	// try to unassign any other instances of this controller
	for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
		if (pads[i] && inputControllerGetId(pads[i]) == id) {
			inputCloseController(i);
			pads[i] = NULL;
			padsCfg[i].deviceIndex = -1;
		}
	}

	SDL_GameController *newpad = SDL_GameControllerOpen(jidx);
	if (!newpad) {
		return 0;
	}

	if (pads[cidx]) {
		inputCloseController(cidx);
	}

	pads[cidx] = newpad;
	inputInitController(cidx, id);

	return 1;
}

void inputKeyBind(s32 idx, u32 ck, s32 bind, u32 vk)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || bind >= INPUT_MAX_BINDS || ck >= CK_TOTAL_COUNT) {
		return;
	}

	if (bind < 0) {
		for (s32 i = 0; i < INPUT_MAX_BINDS; ++i) {
			if (binds[idx][ck][i] == 0) {
				bind = i;
				break;
			}
		}
		if (bind < 0) {
			bind = INPUT_MAX_BINDS - 1; // just overwrite last
		}
	}

	binds[idx][ck][bind] = vk;
}

const u32 *inputKeyGetBinds(s32 idx, u32 ck)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || ck >= CK_TOTAL_COUNT) {
		return NULL;
	}
	return binds[idx][ck];
}

s32 inputKeyPressed(u32 vk)
{
	if (vk >= VK_KEYBOARD_BEGIN && vk < VK_MOUSE_BEGIN) {
		const u8 *state = SDL_GetKeyboardState(NULL);
		return state[vk - VK_KEYBOARD_BEGIN];
	}

	if (vk >= VK_MOUSE_BEGIN && vk < VK_JOY_BEGIN) {
		return (mouseButtons & SDL_BUTTON(vk - VK_MOUSE_BEGIN + 1)) != 0;
	}

	if (vk >= VK_JOY_BEGIN && vk < VK_TOTAL_COUNT) {
		vk -= VK_JOY_BEGIN;
		const s32 idx = vk / INPUT_MAX_CONTROLLER_BUTTONS;
		if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || !pads[idx]) {
			return 0;
		}
		vk = vk % INPUT_MAX_CONTROLLER_BUTTONS;
		// triggers
		if (vk == 30 || vk == 31) {
			const s32 trig = SDL_CONTROLLER_AXIS_TRIGGERLEFT + vk - 30;
			return SDL_GameControllerGetAxis(pads[idx], trig) > TRIG_THRESHOLD;
		}
		return SDL_GameControllerGetButton(pads[idx], vk);
	}

	return 0;
}

s32 inputKeyJustPressed(u32 vk)
{
	const s8 pressed = inputKeyPressed(vk);
	const s32 result = pressed && !vkPrevState[vk];
	vkPrevState[vk] = pressed;
	return result;
}

static inline u32 inputContToContKey(const u32 cont)
{
	if (cont == 0) {
		return 0;
	}
	// just a log2 to convert CONT_* to their indices
	return 32 - __builtin_clz(cont - 1);
}

s32 inputButtonPressed(s32 idx, u32 contbtn)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}

	return inputBindPressed(idx, inputContToContKey(contbtn));
}


void inputLockMouse(s32 lock)
{
	mouseLocked = !!lock;
	SDL_SetRelativeMouseMode(mouseLocked);
}

s32 inputMouseIsLocked(void)
{
	return mouseLocked;
}

s32 inputMouseGetPosition(s32 *x, s32 *y)
{
	if (x) *x = mouseX * videoGetNativeWidth() / videoGetWidth();
	if (y) *y = mouseY * videoGetNativeHeight() / videoGetHeight();
	return (mouseDX != 0 || mouseDY != 0);
}

void inputMouseGetRawDelta(s32 *dx, s32 *dy)
{
	if (dx) *dx = mouseDX;
	if (dy) *dy = mouseDY;
}

void inputMouseGetScaledDelta(f32* dx, f32* dy)
{
	f32 mdx = 0.f, mdy = 0.f;
	if (mouseLocked) {
		mdx = mouseDX * (0.022f / 3.5f) * mouseSensX;
		mdy = mouseDY * (0.022f / 3.5f) * mouseSensY;
	}
	if (dx) *dx = mdx;
	if (dy) *dy = mdy;
}

void inputMouseGetAbsScaledDelta(f32* dx, f32* dy)
{
	f32 mdx = 0.f, mdy = 0.f;
	if (mouseLocked) {
		mdx = mouseDX * (0.022f / 3.5f) * fabsf(mouseSensX);
		mdy = mouseDY * (0.022f / 3.5f) * fabsf(mouseSensY);
	}
	if (dx) *dx = mdx;
	if (dy) *dy = mdy;
}

void inputMouseGetSpeed(f32 *x, f32 *y)
{
	*x = mouseSensX;
	*y = mouseSensY;
}

void inputMouseSetSpeed(f32 x, f32 y)
{
	mouseSensX = x;
	mouseSensY = y;
}

s32 inputMouseIsEnabled(void)
{
	return mouseEnabled;
}

void inputMouseEnable(s32 enabled)
{
	mouseEnabled = !!enabled;
	if (!mouseEnabled && mouseLockMode != MLOCK_ON && mouseLocked) {
		inputLockMouse(0);
	}
}

s32 inputAutoLockMouse(s32 wantlock)
{
	if (mouseEnabled && mouseLockMode == MLOCK_AUTO) {
		inputLockMouse(wantlock);
		return 1;
	}
	return 0;
}

void inputMouseShowCursor(s32 show)
{
	mouseShowCursor = !!show;
	SDL_ShowCursor(mouseShowCursor);
	if (show) {
		mouseCursorTime = sysGetMicroseconds() + CURSOR_HIDE_TIME;
	}
}

s32 inputGetMouseLockMode(void)
{
	return mouseLockMode;
}

void inputSetMouseLockMode(s32 lockmode)
{
	mouseLockMode = lockmode;
	if (lockmode == MLOCK_ON) {
		inputLockMouse(1);
	} else {
		inputLockMouse(0);
	}
}

s32 inputGyroIsEnabled(s32 cidx)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	
	return padsCfg[cidx].gyroEnabled && inputControllerMotionSensorsSupported(cidx);
}

void inputGyroEnable(s32 cidx, s32 enabled)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) {
		return;
	}
	
	padsCfg[cidx].gyroEnabled = (enabled != 0);
}

s32 inputGyroGetAdvanced(s32 cidx)
{
    return padsCfg[cidx].gyroAdvanced;
}

void inputGyroSetAdvanced(s32 cidx, s32 advanced)
{
    padsCfg[cidx].gyroAdvanced = advanced;
}

s32 inputGyroGetAxisMode(s32 cidx)
{
    return padsCfg[cidx].gyroAxisMode;
}

void inputGyroSetAxisMode(s32 cidx, s32 mode)
{
    padsCfg[cidx].gyroAxisMode = mode;
}

s32 inputGetGyroAimMode(s32 cidx)
{
	return padsCfg[cidx].gyroAimMode;
}

void inputSetGyroAimMode(s32 cidx, s32 mode)
{
	padsCfg[cidx].gyroAimMode = mode;
}

static inline void applyGyroInvert(s32 cidx, f32 *dx, f32 *dy, bool useAimInvert)
{
	s32 ix = 0, iy = 0;
	if (useAimInvert) {
		inputGyroGetAimInvert(cidx, &ix, &iy);
	} else {
		inputGyroGetInvert(cidx, &ix, &iy);
	}
	if (ix) *dx = -*dx;
	if (iy) *dy = -*dy;
}

static inline void applyGyroVHMixer(s32 cidx, f32 *dx, f32 *dy)
{
	if (inputGyroGetAdvanced(cidx)) return;

	float mix    = inputGetGyroVHMixer(cidx);
	float hScale = 1.0f - fmaxf(0.0f, mix);
	float vScale = 1.0f + fminf(0.0f, mix);
	*dx *= hScale;
	*dy *= vScale;
}

void inputGyroGetRawDelta(s32 cidx, s32 *dx, s32 *dy, s32 *dz)
{
	f32 yaw, pitch, roll;
	gyroGetOrientation(cidx, &yaw, &pitch, &roll);
	if (dx) *dx = (s32)yaw;
	if (dy) *dy = (s32)pitch;
	if (dz) *dz = (s32)roll;
}

void inputGyroGetScaledDelta(s32 cidx, f32 *dx, f32 *dy, f32 *dz)
{
	f32 gdx = 0.f, gdy = 0.f, gdz = 0.f;

	if (inputGyroIsEnabled(cidx)) {
		f32 yaw, pitch, roll;
		gyroGetOrientation(cidx, &yaw, &pitch, &roll);
		f32 sensX, sensY;
		inputGyroGetSpeed(cidx, &sensX, &sensY);
		gdx = yaw   * sensX;
		gdy = pitch * sensY;
		gdz = roll  * sensX;
	}

	*dx = gdx;
	*dy = gdy;
	*dz = gdz;

	applyGyroInvert(cidx, dx, dy, false);
	applyGyroVHMixer(cidx, dx, dy);
}

void inputGyroGetScaledDeltaCrosshair(s32 cidx, f32 *dx, f32 *dy)
{
	*dx = 0.f;
	*dy = 0.f;

	if (inputGyroIsEnabled(cidx)) {
		f32 yaw, pitch, roll;
		gyroGetOrientation(cidx, &yaw, &pitch, &roll);
		f32 aimX, aimY;
		inputGyroGetAimSpeed(cidx, &aimX, &aimY);
		*dx = yaw   * (0.022f / 2.0f) * aimX;
		*dy = pitch * (0.022f / 2.0f) * aimY;
	}

	applyGyroInvert(cidx, dx, dy, true);
	applyGyroVHMixer(cidx, dx, dy);
}

void inputGyroGetSpeed(s32 cidx, f32* sensX, f32* sensY)
{
    if (sensX) *sensX = padsCfg[cidx].gyroSpeedX;
    if (sensY) *sensY = padsCfg[cidx].gyroSpeedY;
}

void inputGyroSetSpeed(s32 cidx, f32 sensX, f32 sensY)
{
    padsCfg[cidx].gyroSpeedX = sensX;
    padsCfg[cidx].gyroSpeedY = sensY;
}

void inputGyroGetAimSpeed(s32 cidx, f32* sensX, f32* sensY)
{
    if (sensX) *sensX = padsCfg[cidx].gyroAimSpeedX;
    if (sensY) *sensY = padsCfg[cidx].gyroAimSpeedY;
}

void inputGyroSetAimSpeed(s32 cidx, f32 sensX, f32 sensY)
{
    padsCfg[cidx].gyroAimSpeedX = sensX;
    padsCfg[cidx].gyroAimSpeedY = sensY;
}

void inputGyroGetInvert(s32 cidx, s32* out_invertx, s32* out_inverty)
{
    if (out_invertx) *out_invertx = padsCfg[cidx].gyroInvertX;
    if (out_inverty) *out_inverty = padsCfg[cidx].gyroInvertY;
}

void inputGyroSetInvert(s32 cidx, s32 invertx, s32 inverty)
{
    padsCfg[cidx].gyroInvertX = invertx ? 1 : 0;
    padsCfg[cidx].gyroInvertY = inverty ? 1 : 0;
}

void inputGyroGetAimInvert(s32 cidx, s32* out_invertx, s32* out_inverty)
{
    if (out_invertx) *out_invertx = padsCfg[cidx].gyroAimInvertX;
    if (out_inverty) *out_inverty = padsCfg[cidx].gyroAimInvertY;
}

void inputGyroSetAimInvert(s32 cidx, s32 invertx, s32 inverty)
{
    padsCfg[cidx].gyroAimInvertX = invertx ? 1 : 0;
    padsCfg[cidx].gyroAimInvertY = inverty ? 1 : 0;
}

f32 inputGetGyroVHMixer(s32 cidx)
{
	return padsCfg[cidx].gyroVHMixer;
}

void inputSetGyroVHMixer(s32 cidx, f32 value)
{
	if (value < -1.0f) value = -1.0f;
	if (value > 1.0f) value = 1.0f;
	padsCfg[cidx].gyroVHMixer = value;
}

s32 inputGetGyroModifier(s32 cidx)
{
	return padsCfg[cidx].gyroModifier;
}

void inputSetGyroModifier(s32 cidx, s32 mode)
{
	padsCfg[cidx].gyroModifier = mode;
}

f32 inputGetGyroSmoothing(s32 cidx)
{
	return padsCfg[cidx].gyroSmoothing;
}

void inputSetGyroSmoothing(s32 cidx, f32 smoothing)
{
	if (smoothing < 0.0f) smoothing = 0.0f;
	if (smoothing > 1.0f) smoothing = 1.0f;
	padsCfg[cidx].gyroSmoothing = smoothing;
}

f32 inputGyroGetTightening(s32 cidx)
{
    return padsCfg[cidx].gyroTightening;
}

void inputGyroSetTightening(s32 cidx, f32 tightening)
{
	if (tightening < 0.f) tightening = 0.f;
	if (tightening > 1.f) tightening = 1.f;
	padsCfg[cidx].gyroTightening = tightening;
}

s32 inputGyroGetAutoCalibration(s32 cidx)
{
	return (cidx >= 0 && cidx < INPUT_MAX_CONTROLLERS) ? padsCfg[cidx].gyroAutoCalibration : 0;
}

void inputGyroSetAutoCalibration(s32 cidx, s32 enabled)
{
	if (cidx < 0 || cidx >= INPUT_MAX_CONTROLLERS) return;

	s32 wasEnabled = padsCfg[cidx].gyroAutoCalibration;
	padsCfg[cidx].gyroAutoCalibration = enabled;

	if (wasEnabled != padsCfg[cidx].gyroAutoCalibration) {
		const char* modeNames[] = {"Disabled", "In Menus Only", "Always"};
		const char* modeName = (enabled >= 0 && enabled < (s32)ARRAYCOUNT(modeNames)) ? modeNames[enabled] : "Unknown";
		sysLogPrintf(LOG_NOTE, "Input: Gyro auto-calibration set to '%s' for controller %d.", modeName, cidx);
		gyroReconfigureCalibrationMode(cidx);
	}
}

const char *inputGetContKeyName(u32 ck)
{
	if (ck >= CK_TOTAL_COUNT) {
		return "";
	}
	return ckNames[ck];
}

s32 inputGetContKeyByName(const char *name)
{
	for (u32 i = 0; i < CK_TOTAL_COUNT; ++i) {
		if (!strcmp(name, ckNames[i])) {
			return i;
		}
	}
	sysLogPrintf(LOG_WARNING, "unknown bind name: `%s`", name);
	return -1;
}

const char *inputGetKeyName(s32 vk)
{
	if (vk < 0 || vk >= VK_TOTAL_COUNT) {
		vk = 0;
	}
	if (!vkNames[vk][0]) {
		snprintf(vkNames[vk], sizeof(vkNames[vk]), "UNKNOWN%d", vk);
	}
	return vkNames[vk];
}

s32 inputGetKeyByName(const char *name)
{
	s32 start = 0;
	s32 end = 0;

	if (!strncmp(name, "JOY", 3) && isdigit(name[3])) {
		const s32 idx = name[3] - '1';
		if (idx >= 0 && idx < INPUT_MAX_CONTROLLERS) {
			start = VK_JOY1_BEGIN + idx * INPUT_MAX_CONTROLLER_BUTTONS;
			end = start + INPUT_MAX_CONTROLLER_BUTTONS;
		}
	} else if (!strncmp(name, "MOUSE", 5)) {
		start = VK_MOUSE_BEGIN;
		end = VK_JOY1_BEGIN;
	} else if (!strncmp(name, "UNKNOWN", 7) && isdigit(name[7])) {
		const s32 key = atoi(name + 7);
		if (key >= 0 && key < VK_TOTAL_COUNT) {
			return key;
		}
	} else {
		end = VK_MOUSE_BEGIN;
	}

	for (s32 i = start; i < end; ++i) {
		if (!strcmp(vkNames[i], name)) {
			return i;
		}
	}

	sysLogPrintf(LOG_WARNING, "unknown key name: `%s`", name);

	return -1;
}

void inputClearLastKey(void)
{
	lastKey = 0;
}

s32 inputGetLastKey(void)
{
	return lastKey;
}

void inputStartTextInput(void)
{
	lastChar = 0;
	lastKey = 0;
	textInput = 1;
	SDL_StartTextInput();
}

void inputClearLastTextChar(void)
{
	lastChar = 0;
}

char inputGetLastTextChar(void)
{
	return lastChar;
}

static inline s32 filterChar(const char ch)
{
	return isalnum(ch) || ch == ' ' || ch == '?' || ch == '!' || ch == '.';
}

s32 inputTextHandler(char *out, const u32 outSize, s32 *curCol, s32 oskCharsOnly)
{
	const s32 ctrlHeld = inputGetKeyModState() & KM_CTRL;

	if (!ctrlHeld) {
		const char chr = inputGetLastTextChar();
		inputClearLastTextChar();
		const s32 valid = chr && (oskCharsOnly ? filterChar(chr) : isprint(chr));
		if (valid) {
			if (*curCol < outSize - 1) {
				out[(*curCol)++] = chr;
				out[*curCol] = '\0';
			}
		}
	}

	const s32 key = inputGetLastKey();
	inputClearLastKey();
	if (ctrlHeld && (key == VK_A + ('v' - 'a'))) {
		// CTRL+V; paste from clipboard
		const char *clip = inputGetClipboard();
		if (clip) {
			const s32 remain = outSize - *curCol - 1;
			inputClearClipboard();
			*curCol += snprintf(out + *curCol, remain, "%s", clip);
			if (*curCol > outSize) {
				*curCol = outSize;
			}
		}
	} else if (key == VK_BACKSPACE) {
		if (*curCol) {
			out[--*curCol] = '\0';
		} else {
			out[0] = '\0';
		}
	} else if (key == VK_ESCAPE) {
		return -1;
	}

	return 0;
}

void inputClearClipboard(void)
{
	if (clipboardText) {
		SDL_free(clipboardText);
		clipboardText = NULL;
	}
}

const char *inputGetClipboard(void)
{
	if (!clipboardText) {
		char *text = SDL_GetClipboardText();
		if (text) {
			clipboardText = text;
			// remove non-printable and multibyte chars
			for (; *text; ++text) {
				if ((u8)*text < 0x20 || (u8)*text >= 0x7F) {
					*text = '?';
				}
			}
		}
	}
	return clipboardText;
}

void inputStopTextInput(void)
{
	SDL_StopTextInput();
	textInput = 0;
}

s32 inputIsTextInputActive(void)
{
	return textInput;
}

u32 inputGetKeyModState(void)
{
	return SDL_GetModState();
}

PD_CONSTRUCTOR static void inputConfigInit(void)
{
	configRegisterInt("Input.MouseEnabled", &mouseEnabled, 0, 1);
	configRegisterInt("Input.MouseLockMode", &mouseLockMode, MLOCK_OFF, MLOCK_AUTO);
	configRegisterFloat("Input.MouseSpeedX", &mouseSensX, -30.f, 30.f);
	configRegisterFloat("Input.MouseSpeedY", &mouseSensY, -30.f, 30.f);
	configRegisterInt("Input.FakeGamepads", &fakeControllers, 0, 4);
	configRegisterInt("Input.FirstGamepadNum", &firstController, 0, 3);
	configRegisterInt("Input.UseHIDAPI", &useHIDAPI, 0, 1);
	configRegisterInt("Input.UseRawInput", &useRawInput, 0, 1);

	char secname[] = "Input.Player1.Binds";
	char keyname[256] = { 0 };
	for (s32 c = 0; c < MAXCONTROLLERS; ++c) {
		secname[12] = '1' + c;
		secname[13] = '\0';
		configRegisterFloat(strFmt("%s.RumbleScale", secname), &padsCfg[c].rumbleScale, 0.f, 1.f);
		configRegisterInt(strFmt("%s.LStickDeadzoneX", secname), &padsCfg[c].deadzone[0], 0, 32767);
		configRegisterInt(strFmt("%s.LStickDeadzoneY", secname), &padsCfg[c].deadzone[1], 0, 32767);
		configRegisterInt(strFmt("%s.RStickDeadzoneX", secname), &padsCfg[c].deadzone[2], 0, 32767);
		configRegisterInt(strFmt("%s.RStickDeadzoneY", secname), &padsCfg[c].deadzone[3], 0, 32767);
		configRegisterFloat(strFmt("%s.LStickScaleX", secname), &padsCfg[c].sens[0], -10.f, 10.f);
		configRegisterFloat(strFmt("%s.LStickScaleY", secname), &padsCfg[c].sens[1], -10.f, 10.f);
		configRegisterFloat(strFmt("%s.RStickScaleX", secname), &padsCfg[c].sens[2], -10.f, 10.f);
		configRegisterFloat(strFmt("%s.RStickScaleY", secname), &padsCfg[c].sens[3], -10.f, 10.f);
		configRegisterInt(strFmt("%s.GyroEnabled", secname), &padsCfg[c].gyroEnabled, 0, 1);
		configRegisterInt(strFmt("%s.GyroAdvanced", secname), &padsCfg[c].gyroAdvanced, 0, 1);
		configRegisterInt(strFmt("%s.GyroAimMode", secname), &padsCfg[c].gyroAimMode, GYRO_AIM_CAMERA, GYRO_AIM_BOTH);
		configRegisterInt(strFmt("%s.GyroModifier", secname), &padsCfg[c].gyroModifier, GYRO_ALWAYS_ON, GYRO_DISABLE_HELD);
		configRegisterInt(strFmt("%s.GyroAxisMode", secname), &padsCfg[c].gyroAxisMode, GYRO_YAW, GYRO_WORLD);
		configRegisterInt(strFmt("%s.GyroAutoCalibration", secname), &padsCfg[c].gyroAutoCalibration, GYRO_AUTOCALIBRATION_OFF, GYRO_AUTOCALIBRATION_ALWAYS);
		configRegisterFloat(strFmt("%s.GyroSpeedX", secname), &padsCfg[c].gyroSpeedX, -30.f, 30.f);
		configRegisterFloat(strFmt("%s.GyroSpeedY", secname), &padsCfg[c].gyroSpeedY, -30.f, 30.f);
		configRegisterFloat(strFmt("%s.GyroAimSpeedX", secname), &padsCfg[c].gyroAimSpeedX, -10.f, 10.f);
		configRegisterFloat(strFmt("%s.GyroAimSpeedY", secname), &padsCfg[c].gyroAimSpeedY, -10.f, 10.f);
		configRegisterInt(strFmt("%s.GyroInvertX", secname), &padsCfg[c].gyroInvertX, 0, 1);
		configRegisterInt(strFmt("%s.GyroInvertY", secname), &padsCfg[c].gyroInvertY, 0, 1);
		configRegisterInt(strFmt("%s.GyroAimInvertX", secname), &padsCfg[c].gyroAimInvertX, 0, 1);
		configRegisterInt(strFmt("%s.GyroAimInvertY", secname), &padsCfg[c].gyroAimInvertY, 0, 1);
		configRegisterFloat(strFmt("%s.GyroVHMixer", secname), &padsCfg[c].gyroVHMixer, -1.0f, 1.0f);
		configRegisterFloat(strFmt("%s.GyroSmoothing", secname), &padsCfg[c].gyroSmoothing, 0.f, 1.f);
		configRegisterFloat(strFmt("%s.GyroTightening", secname), &padsCfg[c].gyroTightening, 0.f, 1.f);
		configRegisterInt(strFmt("%s.StickCButtons", secname), &padsCfg[c].stickCButtons, 0, 1);
		configRegisterInt(strFmt("%s.CancelCButtons", secname), &padsCfg[c].cancelCButtons, 0, 1);
		configRegisterInt(strFmt("%s.SwapSticks", secname), &padsCfg[c].swapSticks, 0, 1);
		configRegisterInt(strFmt("%s.ControllerIndex", secname), &padsCfg[c].deviceIndex, -1, 0x7FFFFFFF);
		secname[13] = '.';
		for (u32 ck = 0; ck < CK_TOTAL_COUNT; ++ck) {
			snprintf(keyname, sizeof(keyname), "%s.%s", secname, inputGetContKeyName(ck));
			configRegisterString(keyname, bindStrs[c][ck], MAX_BIND_STR);
		}
	}
}
