#include <PR/ultratypes.h>
#include "glyph.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Generic display names for controller buttons (maps to same indices as vkJoyNames)
const char *vkJoyDisplayNames[] = {
	"SOUTH_BTN",        // A button (Bottom face button)
	"EAST_BTN",         // B button (Right face button)
	"WEST_BTN",         // X button (Left face button)
	"NORTH_BTN",        // Y button (Top face button)
	"BACK_BTN",
	"GUIDE_BTN",
	"START_BTN",
	"LEFT_STICK_CLICK",
	"RIGHT_STICK_CLICK",
	"LEFT_SHOULDER",
	"RIGHT_SHOULDER",
	"D-PAD_UP",
	"D-PAD_DOWN",
	"D-PAD_LEFT",
	"D-PAD_RIGHT",
	"MISC1_BTN",        // Additional button (e.g. Xbox Series X share button, PS5 microphone button, Switch capture button)
	"RIGHT_PADDLE_1",   // Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1, DualSense Edge RB button, Right Joy-Con SR button)
	"LEFT_PADDLE_1",    // Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3, DualSense Edge LB button, Left Joy-Con SL button)
	"RIGHT_PADDLE_2",   // Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2, DualSense Edge right Fn button, Right Joy-Con SR button)
	"LEFT_PADDLE_2",    // Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4, DualSense Edge left Fn button, Left Joy-Con SL button)
	"TOUCHPAD_BTN",     // PS4/PS5 touchpad button
	"MISC2_BTN",
	"MISC3_BTN",        // Additional button (e.g. Nintendo GameCube Left Trigger click)
	"MISC4_BTN",        // Additional button (e.g. Nintendo GameCube Right Trigger click)
	"MISC5_BTN",
	"MISC6_BTN",
	"BTN_26",
	"BTN_27",
	"BTN_28",
	"BTN_29",
	"LEFT_TRIG",
	"RIGHT_TRIG",
};

// Controller-specific overrides
// These are used to map specific controller buttons to Controller-specific glyphs

struct button_override {
	int button_index;
	const char *name;
};

// Standard glyph overrides (shared across controllers)
static const struct button_override glyph_standard[] = {
	{  0, "A_BTN" },          // Bottom face button
	{  1, "B_BTN" },          // Right face button
	{  2, "X_BTN" },          // Left face button
	{  3, "Y_BTN" },          // Top face button
	{  7, "L3_STICK_CLICK" }, // Left stick press (L3)
	{  8, "R3_STICK_CLICK" }, // Right stick press (R3)
	{  9, "L1_SHOULDER" },    // Left shoulder (L1)
	{ 10, "R1_SHOULDER" },    // Right shoulder (R1)
	{ 15, "M1_BTN" },         // M1 button
	{ 16, "R4_BTN" },         // Upper right paddle (R4)
	{ 17, "L4_BTN" },         // Upper left paddle (L4)
	{ 18, "R5_BTN" },         // Below right paddle (R5)
	{ 19, "L5_BTN" },         // Below left paddle (L5)
	{ 21, "M2_BTN" },         // M2 button
	{ 22, "M3_BTN" },         // M3 button
	{ 23, "M4_BTN" },         // M4 button
	{ 30, "L2_TRIG" },        // Left trigger
	{ 31, "R2_TRIG" },        // Right trigger
};

// Xbox-specific overrides
static const struct button_override xbox_overrides[] = {
	{ 5,  "XBOX_BTN" },
	{ 9,  "LB_SHOULDER" },
	{ 10, "RB_SHOULDER" },
	{ 30, "LT_TRIG" },
	{ 31, "RT_TRIG" },
};

// Xbox 360-specific button overrides
static const struct button_override xbox360_overrides[] = {
	{ 4, "BACK_BTN" },
	{ 6, "START_BTN" },
};

// Xbox One/Series X|S-specific button overrides
static const struct button_override xboxone_overrides[] = {
	{ 4, "VIEW_BTN" },
	{ 6, "MENU_BTN" },
	{ 15, "SHARE_BTN" },
	{ 16, "PADDLE_1" },     // Xbox Elite right upper paddle
	{ 17, "PADDLE_2" },     // Xbox Elite left upper paddle
	{ 18, "PADDLE_3" },     // Xbox Elite right lower paddle
	{ 19, "PADDLE_4" },     // Xbox Elite left lower paddle
};

// PlayStation-specific overrides
static const struct button_override playstation_overrides[] = {
	{ 0, "CROSS_BTN" },
	{ 1, "CIRCLE_BTN" },
	{ 2, "SQUARE_BTN" },
	{ 3, "TRIANGLE_BTN" },
	{ 5, "PS_BTN" },
	{ 20, "TOUCHPAD_BTN" },
};

// PS3-specific button overrides
static const struct button_override ps3_overrides[] = {
	{ 4, "SELECT_BTN" },
	{ 6, "START_BTN" },
};

// PS4-specific button overrides
static const struct button_override ps4_overrides[] = {
	{ 4, "SHARE_BTN" },
	{ 6, "OPTIONS_BTN" },
};

// PS5-specific button overrides
static const struct button_override ps5_overrides[] = {
	{ 4,  "CREATE_BTN" },
	{ 6,  "OPTIONS_BTN" },
	{ 15, "MIC_BTN" },
	{ 16, "RB_PADDLE" },          // DualSense Edge RB Button  
	{ 17, "LB_PADDLE" },          // DualSense Edge LB Button
	{ 18, "RIGHT_FN_BTN" },       // DualSense Edge right Fn button
	{ 19, "LEFT_FN_BTN" },        // DualSense Edge left Fn button
};

// Nintendo Switch-specific overrides
static const struct button_override nintendoswitch_overrides[] = {
	{  4, "MINUS_BTN" },
	{  5, "HOME_BTN" },
	{  6, "PLUS_BTN" },
	{  9, "L_SHOULDER" },
	{ 10, "R_SHOULDER" },
	{ 15, "CAPTURE_BTN" },
	{ 16, "RIGHT_SR_BTN" },    // Right Joy-Con SR
	{ 17, "LEFT_SL_BTN" },    // Left Joy-Con SL
	{ 18, "RIGHT_SL_BTN" },   // Right Joy-Con SL
	{ 19, "LEFT_SR_BTN" },   // Left Joy-Con SR
	{ 30, "ZL_TRIG" },
	{ 31, "ZR_TRIG" },
};

// Steam Deck-specific overrides
static const struct button_override steamdeck_overrides[] = {
	{ 4, "VIEW_BTN" },
	{ 5, "STEAM_BTN" },
	{ 6, "MENU_BTN" },
};

// Steam Controller-specific overrides
static const struct button_override steamcontroller_overrides[] = {
	{ 5, "STEAM_BTN" },
	{ 16, "RG_BTN" }, // Steam Controller right grip
	{ 17, "LG_BTN" }, // Steam Controller left grip
};

// Nintendo 64-specific overrides (based on NSO N64 controller's button layout)
static const struct button_override nintendo64_overrides[] = {
	{ 2, "C-LEFT_BTN" },
	{ 3, "C-UP_BTN" },
	{ 4, "C-RIGHT_BTN" },
	{ 6, "START_BTN" },
	{ 9, "L_BTN" },
	{ 10, "R_BTN" },
	{ 30, "C-DOWN_BTN" },
	{ 31, "Z_BTN" },
};

// Function to override a specific button index
static const char* searchOverrides(const struct button_override* overrides, int count, int buttonIndex) {
	for (const struct button_override* override = overrides; override < overrides + count; override++) {
		if (override->button_index == buttonIndex) {
			return override->name;
		}
	}
	return NULL;
}

// Function to search and provide Controller-specific button types
const char *glyphGetControllerButtonName(int controllerType, int buttonIndex)
{
	const char *result = NULL;

	switch (controllerType) {
		case CONTROLLER_ICON_XBOX360:
			// Xbox 360-specific overrides
			result = searchOverrides(xbox360_overrides, ARRAY_SIZE(xbox360_overrides), buttonIndex);
			if (result) return result;
			// Xbox overrides
			result = searchOverrides(xbox_overrides, ARRAY_SIZE(xbox_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face Button only)
			if (buttonIndex >= 0 && buttonIndex <= 3) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;

		case CONTROLLER_ICON_STEAM_CONTROLLER:
			// Steam Controller-specific overrides
			result = searchOverrides(steamcontroller_overrides, ARRAY_SIZE(steamcontroller_overrides), buttonIndex);
			if (result) return result;
			// Xbox overrides (Shoulders and triggers)
			result = searchOverrides(xbox_overrides, ARRAY_SIZE(xbox_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face buttons)
			if (buttonIndex >= 0 && buttonIndex <= 3) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_XBOXONE:
			// Xbox One/Series X|S-specific overrides
			result = searchOverrides(xboxone_overrides, ARRAY_SIZE(xboxone_overrides), buttonIndex);
			if (result) return result;
			// Xbox overrides
			result = searchOverrides(xbox_overrides, ARRAY_SIZE(xbox_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face Button only)
			if (buttonIndex >= 0 && buttonIndex <= 3) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_PS3:
			// PS3-specific overrides
			result = searchOverrides(ps3_overrides, ARRAY_SIZE(ps3_overrides), buttonIndex);
			if (result) return result;
			// PlayStation overrides
			result = searchOverrides(playstation_overrides, ARRAY_SIZE(playstation_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face, Shoulders, Triggers only)
			if ((buttonIndex >= 0 && buttonIndex <= 10) || (buttonIndex >= 30 && buttonIndex <= 31)) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_PS4:
			// PS4-specific overrides
			result = searchOverrides(ps4_overrides, ARRAY_SIZE(ps4_overrides), buttonIndex);
			if (result) return result;
			// PlayStation overrides
			result = searchOverrides(playstation_overrides, ARRAY_SIZE(playstation_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face, Shoulders, Triggers only)
			if ((buttonIndex >= 0 && buttonIndex <= 10) || (buttonIndex >= 30 && buttonIndex <= 31)) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_PS5:
			// PS5-specific overrides
			result = searchOverrides(ps5_overrides, ARRAY_SIZE(ps5_overrides), buttonIndex);
			if (result) return result;
			// PlayStation overrides
			result = searchOverrides(playstation_overrides, ARRAY_SIZE(playstation_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (Face, Shoulders, Triggers only)
			if ((buttonIndex >= 0 && buttonIndex <= 10) || (buttonIndex >= 30 && buttonIndex <= 31)) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_NINTENDO_SWITCH:
			// Nintendo Switch specific overrides
			result = searchOverrides(nintendoswitch_overrides, ARRAY_SIZE(nintendoswitch_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard (swapped Face Buttons positions)
			if (buttonIndex >= 0 && buttonIndex <= 3) {
				int mappedIndex = buttonIndex;
				switch (buttonIndex) {
					case 0: mappedIndex = 1; break; // B (Bottom face button)
					case 1: mappedIndex = 0; break; // A (Right face button)
					case 2: mappedIndex = 3; break; // Y (Left face button)
					case 3: mappedIndex = 2; break; // X (Top face button)
				}
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), mappedIndex);
				if (result) return result;
			}
			break;

		case CONTROLLER_ICON_NINTENDO_64:
			// Nintendo 64-specific overrides
			result = searchOverrides(nintendo64_overrides, ARRAY_SIZE(nintendo64_overrides), buttonIndex);
			if (result) return result;
			// Nintendo Switch specific overrides (for NSO N64 controller's Home and Capture buttons)
			if (buttonIndex == 5 || buttonIndex == 15) {
				result = searchOverrides(nintendoswitch_overrides, ARRAY_SIZE(nintendoswitch_overrides), buttonIndex);
				if (result) return result;
			}
			// Glyph standard (N64 face buttons)
			if (buttonIndex >= 0 && buttonIndex <= 1) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;

		case CONTROLLER_ICON_STEAM_DECK:
			// Steam Deck-specific overrides
			result = searchOverrides(steamdeck_overrides, ARRAY_SIZE(steamdeck_overrides), buttonIndex);
			if (result) return result;
			// Glyph standard
			if ((buttonIndex >= 0 && buttonIndex <= 19) || (buttonIndex >= 30 && buttonIndex <= 31)) {
				result = searchOverrides(glyph_standard, ARRAY_SIZE(glyph_standard), buttonIndex);
				if (result) return result;
			}
			break;
			
		case CONTROLLER_ICON_GENERIC:
		default:
			break;
	}

	// Fallback to Generic type
	if (buttonIndex >= 0 && buttonIndex < 32) {
		return vkJoyDisplayNames[buttonIndex];
	}
	
	return "UNKNOWN BUTTON";
}