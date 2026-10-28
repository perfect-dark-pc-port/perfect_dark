#ifndef _IN_GLYPH_H
#define _IN_GLYPH_H

#include <SDL.h>

// Controller Icon enumeration for button icon detection
typedef enum {
	CONTROLLER_ICON_GENERIC,
	CONTROLLER_ICON_XBOX360,
	CONTROLLER_ICON_XBOXONE,
	CONTROLLER_ICON_PS3,
	CONTROLLER_ICON_PS4,
	CONTROLLER_ICON_PS5,
	CONTROLLER_ICON_NINTENDO_SWITCH,
	CONTROLLER_ICON_NINTENDO_64,
	CONTROLLER_ICON_STEAM_CONTROLLER,
	CONTROLLER_ICON_STEAM_DECK,
} ControllerIconType;

// Valve Corporation VID/PID definitions
#define VALVE_VENDOR_ID 0x28de

// Steam Controller product IDs
#define STEAM_CONTROLLER_LEGACY_PID     0x1101  // Valve Legacy Steam Controller (CHELL)
#define STEAM_CONTROLLER_WIRED_PID      0x1102  // Valve wired Steam Controller (D0G)
#define STEAM_CONTROLLER_BT_1_PID       0x1105  // Valve Bluetooth Steam Controller (D0G)
#define STEAM_CONTROLLER_BT_2_PID       0x1106  // Valve Bluetooth Steam Controller (D0G)
#define STEAM_CONTROLLER_WIRELESS_PID   0x1142  // Valve wireless Steam Controller
#define STEAM_CONTROLLER_V2_WIRED_PID   0x1201  // Valve wired Steam Controller (HEADCRAB)
#define STEAM_CONTROLLER_V2_BT_PID      0x1202  // Valve Bluetooth Steam Controller (HEADCRAB)

// Other Valve product IDs
#define STEAM_VIRTUAL_GAMEPAD_PID       0x11ff  // Steam Virtual Gamepad
#define STEAM_DECK_BUILTIN_PID          0x1205  // Valve Steam Deck Builtin

// Check if a product ID is any Steam Controller (2015) variant
static inline int isSteamControllerPID(unsigned short productId) {
	return (productId == STEAM_CONTROLLER_LEGACY_PID ||
	        productId == STEAM_CONTROLLER_WIRED_PID ||
	        productId == STEAM_CONTROLLER_BT_1_PID ||
	        productId == STEAM_CONTROLLER_BT_2_PID ||
	        productId == STEAM_CONTROLLER_WIRELESS_PID ||
	        productId == STEAM_CONTROLLER_V2_WIRED_PID ||
	        productId == STEAM_CONTROLLER_V2_BT_PID);
}

// Steam Virtual Gamepad detection 
static inline int getSteamVirtualControllerDetection(SDL_GameController* ctrl, int SDLControllerType) {
	if (!ctrl) {
		return SDLControllerType;
	}
	
	if (SDL_GameControllerGetVendor(ctrl) != VALVE_VENDOR_ID) {
		return SDLControllerType;
	}
	
	unsigned short product = SDL_GameControllerGetProduct(ctrl);
	
	// Steam Virtual Gamepad - pass through the underlying controller type
	if (product == STEAM_VIRTUAL_GAMEPAD_PID) {
		return SDLControllerType;
	}
	
	// Steam Deck
	if (product == STEAM_DECK_BUILTIN_PID) {
		return CONTROLLER_ICON_STEAM_DECK;
	}
	
	// Steam Controller (2015)
	if (isSteamControllerPID(product)) {
		return CONTROLLER_ICON_STEAM_CONTROLLER;
	}
	
	return SDLControllerType;
}

// Generic button names for glyphs 
// this will cover generalized button names for glyphs based on SDL_GamepadButton
extern const char *vkJoyDisplayNames[];

// Controller-specific button glyphs
// this will grab the button glyphs based on the controller type and button index
const char *glyphGetControllerButtonName(int controllerType, int buttonIndex);

// Dynamic glyph input binding detection
// TODO: replace all baked-in button localization text with a dynamic glyph system
char* glyphInputBindingDetect(char* text, int textSize, const char* placeholder,
                                       int controllerIndex, int controlKey, int forceController);

#endif