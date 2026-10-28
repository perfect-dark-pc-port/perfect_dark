#include <ultra64.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/camdraw.h"
#include "game/game_006900.h"
#include "game/body.h"
#include "game/objectives.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/modeldef.h"
#include "game/savebuffer.h"
#include "game/menugfx.h"
#include "game/menuitem.h"
#include "game/menu.h"
#include "game/filelist.h"
#include "game/filemgr.h"
#include "game/credits.h"
#include "game/game_1531a0.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/mplayer/setup.h"
#include "game/music.h"
#include "game/texdecompress.h"
#include "game/mplayer/setup.h"
#include "game/challenge.h"
#include "game/training.h"
#include "game/game_1a78b0.h"
#include "game/gamefile.h"
#include "game/gfxmemory.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/joy.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/str.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#include "input.h"
#include "platform.h"
#define BLUR_OFS 10
#else
#define BLUR_OFS 30
#endif

#if VERSION >= VERSION_PAL_FINAL
char g_CheatMarqueeString[300];
#elif VERSION >= VERSION_NTSC_1_0
char g_CheatMarqueeString[252];
#else
char g_StringPointer[104];
char g_StringPointer2[100];
#endif

u8 *g_BlurBuffer;
s32 var8009dfc0;
u32 var8009dfc4;
struct briefing g_Briefing;
u32 var8009dfe4;
struct missionconfig g_MissionConfig;
struct menu g_Menus[MAX_PLAYERS];
struct menudata g_MenuData;
s32 g_MenuScissorX1;
s32 g_MenuScissorX2;
s32 g_MenuScissorY1;
s32 g_MenuScissorY2;
Vp var800a2048[MAX_PLAYERS][2];

#if VERSION >= VERSION_NTSC_1_0
struct menudialogdef g_PakCannotReadGameBoyMenuDialog;
struct menudialogdef g_PakDamagedMenuDialog;
struct menudialogdef g_PakDataLostMenuDialog;
#else
struct menudialogdef g_PakDamagedMenuDialog;
#endif

struct menudialogdef g_PakFullMenuDialog;
struct menudialogdef g_PakRemovedMenuDialog;
struct menudialogdef g_PakRepairFailedMenuDialog;
struct menudialogdef g_PakRepairSuccessMenuDialog;

#if VERSION >= VERSION_JPN_FINAL
const struct menucolourpalette g_MenuColours[] = {
	{ 0x20202000, 0x20202000, 0x20202000, 0x4f4f4f00, 0x00000000, 0x00000000, 0x4f4f4f00, 0x4f4f4f00, 0x4f4f4f00, 0x4f4f4f00, 0x00000000, 0x00000000, 0x4f4f4f00, 0x00000000, 0x00000000 },
	{ 0x0060bf7f, 0x0000507f, 0x00f0ff7f, 0xffffffff, 0x00002f9f, 0x00006f7f, 0x00ffffff, 0x007f7fff, 0xffffffff, 0x8fffffff, 0x000044ff, 0x000030ff, 0x7f7fffff, 0xffffffff, 0x6644ff7f },
	{ 0xbf00007f, 0x5000007f, 0xff00007f, 0xffff00ff, 0x2f00009f, 0x6f00007f, 0xff9070ff, 0x7f0000ff, 0xffff00ff, 0xffa090ff, 0x440000ff, 0x003000ff, 0xffff00ff, 0xffffffff, 0xff44447f },
	{ 0x00bf007f, 0x0050007f, 0x00ff007f, 0xffff00ff, 0x002f009f, 0x00ff0028, 0x55ff55ff, 0x006f00af, 0xffffffff, 0x00000000, 0x004400ff, 0x003000ff, 0xffff00ff, 0xffffffff, 0x44ff447f },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff9f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
	{ 0xaaaaaaff, 0xaaaaaa7f, 0xaaaaaaff, 0xffffffff, 0xffffff9f, 0xffffffff, 0xffffffff, 0xffffffff, 0xff8888ff, 0xffffffff, 0x00000000, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
};
#else
const struct menucolourpalette g_MenuColours[] = {
	{ 0x20202000, 0x20202000, 0x20202000, 0x4f4f4f00, 0x00000000, 0x00000000, 0x4f4f4f00, 0x4f4f4f00, 0x4f4f4f00, 0x4f4f4f00, 0x00000000, 0x00000000, 0x4f4f4f00, 0x00000000, 0x00000000 },
	{ 0x0060bf7f, 0x0000507f, 0x00f0ff7f, 0xffffffff, 0x00002f7f, 0x00006f7f, 0x00ffffff, 0x007f7fff, 0xffffffff, 0x8fffffff, 0x000044ff, 0x000030ff, 0x7f7fffff, 0xffffffff, 0x6644ff7f },
	{ 0xbf00007f, 0x5000007f, 0xff00007f, 0xffff00ff, 0x2f00007f, 0x6f00007f, 0xff7050ff, 0x7f0000ff, 0xffff00ff, 0xff9070ff, 0x440000ff, 0x003000ff, 0xffff00ff, 0xffffffff, 0xff44447f },
	{ 0x00bf007f, 0x0050007f, 0x00ff007f, 0xffff00ff, 0x002f007f, 0x00ff0028, 0x55ff55ff, 0x006f00af, 0xffffffff, 0x00000000, 0x004400ff, 0x003000ff, 0xffff00ff, 0xffffffff, 0x44ff447f },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
	{ 0xaaaaaaff, 0xaaaaaa7f, 0xaaaaaaff, 0xffffffff, 0xffffff2f, 0xffffffff, 0xffffffff, 0xffffffff, 0xff8888ff, 0xffffffff, 0x00000000, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
};
#endif

const struct menucolourpalette g_MenuWave1Colours[] = {
	{ 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x4f4f4f00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000 },
	{ 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x006f6faf, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000 },
	{ 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x006f6faf, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000 },
	{ 0xffffff00, 0xffffff00, 0xffffff00, 0xff7f0000, 0xffffff00, 0xffffff00, 0x00ffff00, 0x006f6faf, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000 },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
};

const struct menucolourpalette g_MenuWave2Colours[] = {
	{ 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x4f4f4f00, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x00000000 },
	{ 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x006f6faf, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x00000000 },
	{ 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x006f6faf, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x00000000 },
	{ 0x44444400, 0x44444400, 0x44444400, 0x00ff0000, 0x44444400, 0x44444400, 0xffff0000, 0x006f6faf, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x44444400, 0x00000000 },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
	{ 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffff7f, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffff5f, 0xffffffff, 0xffffff7f, 0xffffffff },
};

#if VERSION >= VERSION_NTSC_1_0
char *g_StringPointer = g_CheatMarqueeString;
char *g_StringPointer2 = &g_CheatMarqueeString[VERSION >= VERSION_PAL_FINAL ? 150 : 125];
#endif

s32 g_MpPlayerNum = 0;

#ifndef PLATFORM_N64
s32 g_MenuMouseControl = true;
s32 g_MenuUsingMouse = false;
s32 g_MenuKeyboardPlayer = -1;
s32 g_AllowMouseHeld = true;
s32 g_MouseDimmedMode = false;
s32 g_MouseEndDeferredSlider = false;
#endif

s32 menuAlt1Pressed(s32 playerNum)
{
	const s32 rshoulderKey = VK_JOY1_RSHOULDER + playerNum * INPUT_MAX_CONTROLLER_BUTTONS;

	if (playerNum == 0) {
		return inputKeyJustPressed(VK_LCTRL) || inputKeyJustPressed(rshoulderKey);
	}

	return inputKeyJustPressed(rshoulderKey);
}

s32 menuAlt2Pressed(s32 playerNum)
{
	const s32 lshoulderKey = VK_JOY1_LSHOULDER + playerNum * INPUT_MAX_CONTROLLER_BUTTONS;

	if (playerNum == 0) {
		return inputKeyJustPressed(VK_LALT) || inputKeyJustPressed(lshoulderKey);
	}

	return inputKeyJustPressed(lshoulderKey);
}

s32 menuAltAnyPressed(s32 playerNum)
{
	return menuAlt1Pressed(playerNum) || menuAlt2Pressed(playerNum);
}

void menuPlaySound(s32 menusound)
{
	s32 sound = -1;
	s32 setpitch = false;
	s32 setvol = false;
	f32 pitch = 1;

	switch (menusound) {
	case MENUSOUND_SWIPE:
		sound = SFX_MENU_SWIPE;
		break;
	case MENUSOUND_OPENDIALOG:
		sound = SFX_MENU_OPENDIALOG;
		break;
	case MENUSOUND_FOCUS:
		sound = SFX_MENU_FOCUS;
		break;
	case MENUSOUND_SELECT:
		sound = SFX_MENU_SELECT;
		break;
	case MENUSOUND_ERROR:
		pitch = 0.4f;
		sound = SFX_MENU_ERROR;
		setpitch = true;
		break;
	case MENUSOUND_EXPLOSION:
		sound = SFX_EXPLOSION_8098;
		break;
	case MENUSOUND_TOGGLEON:
		sound = SFX_MENU_SELECT;
		break;
	case MENUSOUND_TOGGLEOFF:
		sound = SFX_MENU_SUBFOCUS;
		break;
	case MENUSOUND_SUBFOCUS:
		sound = SFX_MENU_SUBFOCUS;
		break;
	case MENUSOUND_KEYBOARDFOCUS:
		sound = SFX_PICKUP_AMMO;
		setpitch = true;
		setvol = true;
		pitch = 3.5f;
		break;
	case MENUSOUND_KEYBOARDCANCEL:
		sound = SFX_MENU_CANCEL;
		setpitch = true;
		pitch = 0.41904801130295f;
		break;
	}

	if (sound != -1) {
		struct sndstate *handle;

#if VERSION >= VERSION_NTSC_1_0
		OSPri prevpri = osGetThreadPri(NULL);
		osSetThreadPri(0, osGetThreadPri(&g_AudioManager.thread) + 1);
#endif

		handle = sndStart(var80095200, sound, NULL, -1, -1, -1, -1, -1);

		if (handle && setpitch) {
			audioPostEvent(handle, AL_SNDP_PITCH_EVT, *(s32 *)&pitch);
		}

		if (handle && setvol) {
			audioPostEvent(handle, AL_SNDP_VOL_EVT, 0x4000);
		}

#if VERSION >= VERSION_NTSC_1_0
		osSetThreadPri(0, prevpri);
#endif
	}
}

bool menuIsSoloMissionOrMp(void)
{
	switch (g_MenuData.root) {
	case MENUROOT_MAINMENU:
	case MENUROOT_TRAINING:
		if (g_Vars.stagenum == STAGE_CITRAINING) {
			return false;
		}

		return true;
	case MENUROOT_MPPAUSE:
		return true;
	}

	return false;
}

bool currentPlayerIsMenuOpenInSoloOrMp(void)
{
	s32 mpindex = g_Vars.currentplayerstats->mpindex;

	if (menuIsSoloMissionOrMp()) {
		if (mpindex >= 4) {
			mpindex -= 4;
		}

		if (g_Menus[mpindex].curdialog) {
			return true;
		}
	}

	return false;
}

bool func0f0f0c68(void)
{
	if (g_MenuData.bg || g_MenuData.nextbg != 255) {
		return false;
	}

	return true;
}

void menuSetBanner(s32 bannernum, bool allplayers)
{
	if (allplayers) {
		g_MenuData.bannernum = bannernum;
		return;
	}

	g_Menus[g_MpPlayerNum].bannernum = bannernum;
}

#if VERSION >= VERSION_NTSC_1_0
Gfx *menuRenderBanner(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, bool big, s32 msgnum, s32 arg7, s32 arg8)
#else
Gfx *menuRenderBanner(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, bool big, s32 msgnum)
#endif
{
	s32 midx;
	s32 x;
	s32 y;
	s32 texttop;
	s32 textheight;
	s32 textwidth;
	s32 waitheight;
	s32 waitwidth;
	s32 bannertop;
	s32 bannerbottom;
	struct fontchar *chars;
	struct font *font;

	static u16 msgs[] = {
		L_MPMENU_484, // "Searching for Camera!"
		L_MPMENU_485, // "Calibrating Camera"
		L_MPMENU_486, // "Downloading Image"
		L_MPMENU_487, // "Loading Image"
		L_MPMENU_488, // "Saving Image"
		L_MPMENU_489, // "Transferring Image"
		L_MPMENU_490, // "Uploading Segment"
		L_MPMENU_491, // "Checking Controller Pak"
		L_MPMENU_492, // "Getting PerfectHead"
		L_MPMENU_493, // "Saving PerfectHead"
		L_MPMENU_494, // "Auto Camera Adjustment"
	};

	chars = g_CharsHandelGothicSm;
	font = g_FontHandelGothicSm;

	if (big) {
#if VERSION >= VERSION_PAL_BETA
		if (g_CharsHandelGothicMd) {
			chars = g_CharsHandelGothicMd;
			font = g_FontHandelGothicMd;
		}
#else
		chars = g_CharsHandelGothicMd;
		font = g_FontHandelGothicMd;
#endif
	}

#if VERSION >= VERSION_NTSC_1_0
	midx = (x1 + arg7 + x2 - arg8) / 2;
#else
	midx = (x1 + x2) / 2;
#endif

	y = (y1 + y2) / 2;

	textMeasure(&textheight, &textwidth, langGet(msgs[msgnum]), chars, font, 0);

	// "Please Wait..."
	textMeasure(&waitheight, &waitwidth, langGet(L_MPMENU_495), chars, font, 0);

#if VERSION >= VERSION_NTSC_1_0 && VERSION < VERSION_JPN_FINAL
	if (msgs[msgnum] == L_MPMENU_491) { // "Checking Controller Pak"
		// Add space for "TM"
		textwidth += 7;
	}
#endif

	y -= (textheight + waitheight) / 2;
	texttop = y;
	bannertop = y - 4;
	bannerbottom = y + textheight + waitheight + 7;

	// Black fill
	gdl = textSetPrimColour(gdl, 0x0000007f);
	gDPFillRectangleScaled(gdl++, x1, y1, x2, y2);
	gdl = text0f153838(gdl);

	// Dark blue fill
	gdl = textSetPrimColour(gdl, 0x00007f7f);
	gDPFillRectangleScaled(gdl++, x1, bannertop, x2, bannerbottom);
	gdl = text0f153838(gdl);

	// Top and bottom borders (light blue)
	gdl = textSetPrimColour(gdl, 0x7f7fff7f);
	gDPFillRectangleScaled(gdl++, x1, bannerbottom + 2, x2, bannerbottom + 4);
	gDPFillRectangleScaled(gdl++, x1, bannertop - 4, x2, bannertop - 2);
	gdl = text0f153838(gdl);

	gdl = text0f153628(gdl);

	// Render the selected message's shadow
	x = midx - textwidth / 2 + 2;
	y += 2;
	gdl = textRenderProjected(gdl, &x, &y, langGet(msgs[msgnum]),
			chars, font, 0x000000ff, viGetWidth(), viGetWidth(), 0, 0);

	// Render "Please Wait..." shadow
	x = midx - waitwidth / 2 + 2;
	y += 3;
	gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_495),
			chars, font, 0x000000ff, viGetWidth(), viGetWidth(), 0, 0);

	// Render the selected message proper
	x = midx - textwidth / 2;
	y = texttop;
	gdl = textRenderProjected(gdl, &x, &y, langGet(msgs[msgnum]),
			chars, font, 0xbfbfffff, viGetWidth(), viGetWidth(), 0, 0);

	// Render "Please Wait..." proper
	x = midx - waitwidth / 2;
	y += 3;
	gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_495),
			chars, font, 0xbfbfffff, viGetWidth(), viGetWidth(), 0, 0);

#if VERSION >= VERSION_NTSC_1_0 && VERSION < VERSION_JPN_FINAL
	if (msgs[msgnum] == L_MPMENU_491) { // "Checking Controller Pak"
		// Render "TM"
		y = texttop - 1;
		x = textwidth / 2 + midx - 7;
		gdl = textRenderProjected(gdl, &x, &y, "TM",
				g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xbfbfffff, viGetWidth(), viGetWidth(), 0, 0);
	}
#endif

	gdl = text0f153780(gdl);

	return gdl;
}

u32 var80071464 = 0;

struct menudfc *func0f0f1338(struct menuitem *item)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Menus[0].unkdfc); i++) {
		if (g_Menus[g_MpPlayerNum].unkdfc[i].item == item) {
			return &g_Menus[g_MpPlayerNum].unkdfc[i];
		}
	}

	return NULL;
}

void func0f0f139c(struct menuitem *item, f32 arg1)
{
	struct menudfc *thing = func0f0f1338(item);

	if (thing) {
		thing->unk04 = arg1;
		return;
	}

	thing = func0f0f1338(NULL);

	if (thing) {
		thing->item = item;
		thing->unk04 = arg1;
	}
}

void func0f0f13ec(struct menuitem *item)
{
	struct menudfc *thing = func0f0f1338(item);

	if (thing) {
		thing->item = NULL;
	}
}

void func0f0f1418(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Menus[0].unkdfc); i++) {
		if (g_Menus[g_MpPlayerNum].unkdfc[i].item) {
#if VERSION >= VERSION_PAL_BETA
			g_Menus[g_MpPlayerNum].unkdfc[i].unk04 += g_Vars.diffframe60freal / 60.0f;
#else
			g_Menus[g_MpPlayerNum].unkdfc[i].unk04 += g_Vars.diffframe60f / 60.0f;
#endif
		}
	}
}

void func0f0f1494(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Menus[0].unkdfc); i++) {
		g_Menus[g_MpPlayerNum].unkdfc[i].item = NULL;
	}
}

char *menuResolveText(uintptr_t thing, void *dialogoritem)
{
	char *(*handler)(void *dialogoritem) = (void *)thing;

	// Null/zero
	if (thing == 0) {
		return NULL;
	}

	// Text ID
	if (thing < 0x5a00) {
		return langGet((uintptr_t)thing);
	}

#ifdef PLATFORM_N64 // unreliable otherwise, the above check should be enough?
	if (thing > (uintptr_t)func0f1a78b0) {
#if VERSION < VERSION_NTSC_1_0
		CRASH();
#endif
		return NULL;
	}
#endif

	// Function pointer
	if (handler) {
		return handler(dialogoritem);
	}

	return "";
}

char *menuResolveParam2Text(struct menuitem *item)
{
#ifndef PLATFORM_N64
	if (item->flags & MENUITEMFLAG_LITERAL_TEXT) {
		return (const char *)item->param2;
	}
#endif
	return menuResolveText(item->param2, item);
}

char *menuResolveDialogTitle(struct menudialogdef *dialogdef)
{
#ifndef PLATFORM_N64
	if (dialogdef->flags & MENUDIALOGFLAG_LITERAL_TEXT) {
		return (const char *)dialogdef->title;
	}
#endif
	return menuResolveText(dialogdef->title, dialogdef);
}

void menuGetItemBlocksRequired(struct menuitem *item, s32 *numwords)
{
	switch (item->type) {
	case MENUITEMTYPE_SLIDER:
	case MENUITEMTYPE_CHECKBOX:
	case MENUITEMTYPE_RANKING:
	case MENUITEMTYPE_14:
	case MENUITEMTYPE_18:
		*numwords = 1;
		break;
#if VERSION < VERSION_PAL_BETA
	case MENUITEMTYPE_SCROLLABLE:
#endif
	case MENUITEMTYPE_MARQUEE:
	case MENUITEMTYPE_CONTROLLER:
		*numwords = 2;
		break;
	case MENUITEMTYPE_LIST:
#if VERSION >= VERSION_PAL_BETA
	case MENUITEMTYPE_SCROLLABLE:
#endif
		*numwords = 3;
		break;
	case MENUITEMTYPE_DROPDOWN:
		*numwords = 4;
		break;
	case MENUITEMTYPE_PLAYERSTATS:
		*numwords = 5;
		break;
	case MENUITEMTYPE_KEYBOARD:
	case MENUITEMTYPE_10:
	case MENUITEMTYPE_16:
		*numwords = 3;
		break;
	}
}

void menuCalculateItemSize(struct menuitem *item, s16 *width, s16 *height, struct menudialog *dialog)
{
	char *text;
	s32 textwidth;
	s32 textheight;
	struct fontchar *chars = g_CharsHandelGothicSm;
	struct font *font = g_FontHandelGothicSm;
	s32 i;
	union handlerdata handlerdata;
	union handlerdata handlerdata2;
	char *text2;
	s32 numobjectives;

	// Check if item's handler handles MENUOP_CHECKHIDDEN
	if (item->handler && (item->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0) {
		if (item->handler(MENUOP_CHECKHIDDEN, item, &handlerdata)) {
			*width = 0;
			*height = 0;
			return;
		}
	}

	switch (item->type) {
	case MENUITEMTYPE_CONTROLLER:
#if VERSION == VERSION_JPN_FINAL
		*height = 190;
		*width = 240;
#elif PAL
		*height = 156;
		*width = 230;
#else
		*height = 150;
		*width = 230;
#endif
		break;
	case MENUITEMTYPE_18:
		*height = item->param2 == 1 ? 170 : 126;
		*width = 210;
		break;
	case MENUITEMTYPE_14:
		*width = 90;
		*height = 54;
		break;
	case MENUITEMTYPE_METER:
		*width = 24;
		*height = 6;
		break;
	case MENUITEMTYPE_KEYBOARD:
		*width = 130;
#ifndef PLATFORM_N64
		*height = 84;
#else
		*height = 73;
#endif
		break;
	case MENUITEMTYPE_LIST:
		if (item->param2 > 0) {
			*width = item->param2;
		} else {
			*width = 80;

#if VERSION >= VERSION_NTSC_1_0
			if ((item->flags & MENUITEMFLAG_LIST_WIDE) != 0) {
				*width = 180;
			}
#else
			if ((item->flags && MENUITEMFLAG_LIST_WIDE) != 0) {
				*width = 180;
			}
#endif
		}

#if VERSION >= VERSION_JPN_FINAL
		*height = item->param3 > 0 ? item->param3 : 112;
#else
		*height = item->param3 > 0 ? item->param3 : 121;
#endif
		break;
	case MENUITEMTYPE_DROPDOWN:
		text = menuResolveParam2Text(item);

		if (text && strcmp(text, "") == 0) {
			*width = 0;
			*height = 0;
		} else {
			textwidth = 0;
			textheight = 0;

			if (text != NULL) {
				textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
			}

			*width = textwidth + 20;
			*height = VERSION == VERSION_JPN_FINAL ? 14 : 12;

			if (item->handler) {
				handlerdata2.dropdown.value = 0;
				item->handler(MENUOP_GETSELECTEDINDEX, item, &handlerdata2);
				handlerdata2.dropdown.unk04 = 0;
				text2 = (char *)item->handler(MENUOP_GETOPTIONTEXT, item, &handlerdata2);
				textMeasure(&textheight, &textwidth, text2, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

#if VERSION >= VERSION_PAL_FINAL
				if ((item->flags & MENUITEMFLAG_ADJUSTWIDTH) == 0) {
					*width += textwidth + 10;
				} else {
					*width += textwidth + 3;
				}
#else
				*width += textwidth + 10;
#endif
			}
		}
		break;
	case MENUITEMTYPE_13:
		*width = 70;
		*height = 50;
		break;
	case MENUITEMTYPE_SLIDER:
		if (dialog && dialog->unk6e) {
			*width = 120;
			*height = 22;
			return;
		}

		*width = 150;
		*height = VERSION == VERSION_JPN_FINAL ? 14 : 12;

		if (item->flags & MENUITEMFLAG_SLIDER_ALTSIZE) {
			*height = 22;
			*width = 120;
		} else if (item->flags & MENUITEMFLAG_SLIDER_WIDE) {
			*width = 200;
			*height = VERSION == VERSION_JPN_FINAL ? 14 : 12;
		}
		break;
	case MENUITEMTYPE_CHECKBOX:
		if (item->flags & MENUITEMFLAG_SMALLFONT) {
			chars = g_CharsHandelGothicXs;
			font = g_FontHandelGothicXs;
		}

		text = menuResolveParam2Text(item);

		if (text == NULL) {
			*width = 120;
		} else if (strcmp(text, "") == 0) {
			*width = 0;
			*height = 0;
		} else {
			textMeasure(&textheight, &textwidth, text, chars, font, 0);
			*width = (s16)textwidth + 34;
		}
		*height = VERSION == VERSION_JPN_FINAL ? 14 : 12;
		break;
	case MENUITEMTYPE_MODEL:
		*width = item->param2;
		*height = item->param3;
		break;
	case MENUITEMTYPE_SEPARATOR:
		*width = 1;
		if (item->param2) {
			*width = item->param2;
		}
		*height = VERSION == VERSION_JPN_FINAL ? 2 : 5;
		break;
	case MENUITEMTYPE_MARQUEE:
		*width = 1;
#if VERSION == VERSION_JPN_FINAL
		*height = LINEHEIGHT;
#else
		if (item->flags & MENUITEMFLAG_SMALLFONT) {
			*height = LINEHEIGHT;
		} else {
			*height = LINEHEIGHT + 2;
		}
#endif
		break;
	case MENUITEMTYPE_LABEL:
	case MENUITEMTYPE_SELECTABLE:
		text = menuResolveParam2Text(item);

		if (text == NULL) {
			*width = 0;
			*height = 0;
			return;
		}

		if (item->flags & MENUITEMFLAG_SMALLFONT) {
			chars = g_CharsHandelGothicXs;
			font = g_FontHandelGothicXs;
		}

		if (item->flags & MENUITEMFLAG_BIGFONT) {
			chars = g_CharsHandelGothicMd;
			font = g_FontHandelGothicMd;
		}

		if (strcmp(text, "") == 0) {
			*height = 0;
			*width = *height;
		} else {
			textMeasure(&textheight, &textwidth, text, chars, font, 0);
			*width = (s16)textwidth + 8;

			if ((item->flags & (MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_ADJUSTWIDTH)) == 0) {
				*width += 20;
			}

#if VERSION == VERSION_JPN_FINAL
			*height = textheight;
#else
			*height = textheight + 3;

			if (item->flags & MENUITEMFLAG_SMALLFONT) {
				*height -= 2;
			}
#endif

			if ((item->flags & (MENUITEMFLAG_LABEL_HASRIGHTTEXT | MENUITEMFLAG_BIGFONT)) == 0) {
#ifndef PLATFORM_N64
				if (item->flags & MENUITEMFLAG_LITERAL_TEXT) {
					text = (const char *)item->param3;
				} else
#endif
				text = menuResolveText(item->param3, item);

				// @bug: This is not how you check for an empty string
				if (text != NULL && text != "") {
					textMeasure(&textheight, &textwidth, text, chars, font, 0);
					*width += textwidth + 5;

					if (item->flags & MENUITEMFLAG_ADJUSTWIDTH) {
						*width -= 6;
					}
				}
			}
		}

		if (item->flags & MENUITEMFLAG_BIGFONT) {
			*height = 26;
			*width += 36;
		}

		if (item->flags & MENUITEMFLAG_LESSHEIGHT) {
			*height -= 1;
		}
		break;
	case MENUITEMTYPE_SCROLLABLE:
		*width = item->param2 > 0 ? item->param2 : 240;
		*height = item->param3 > 0 ? item->param3 : 150;
		break;
	case MENUITEMTYPE_OBJECTIVES:
		numobjectives = 0;
		*width = 240;

		for (i = 0; i < ARRAYCOUNT(g_Briefing.objectivenames); i++) {
			if (g_Briefing.objectivenames[i] && (g_Briefing.objectivedifficulties[i] & (1 << lvGetDifficulty()))) {
				numobjectives++;
			}
		}

#if VERSION >= VERSION_JPN_FINAL
		if (item->param == 0) {
			*height = 9 + numobjectives * 24;
		} else if (item->param == 1) {
			*height = 9 + numobjectives * 16;
		} else if (item->param == 2) {
			*height = 9 + numobjectives * 36;
			*height -= 5;
			*width = 120;
		}
#elif VERSION >= VERSION_PAL_FINAL
		if (item->param == 0) {
			*height = 9 + numobjectives * 18;
		} else if (item->param == 1) {
			*height = 9 + numobjectives * 14;
		} else if (item->param == 2) {
			*height = 9 + numobjectives * 34;
			*width = 120;
		}
#else
		if (item->param == 0) {
			*height = 9 + numobjectives * 18;
		} else if (item->param == 1) {
			*height = 9 + numobjectives * 14;
		} else if (item->param == 2) {
			*height = 9 + numobjectives * 30;
			*width = 120;
		}
#endif
		break;
	case MENUITEMTYPE_07:
		*width = 120;
		*height = 120;
		break;
	case MENUITEMTYPE_PLAYERSTATS:
		*width = 125;
		*height = 68;
		break;
	case MENUITEMTYPE_RANKING:
		*width = 125;
		*height = 58;
		break;
	case MENUITEMTYPE_10:
		*width = item->param2 ? item->param2 + 2 : 66;
		*height = *width;
		break;
	case MENUITEMTYPE_16:
		*width = 66;
		*height = *width;
		break;
	case MENUITEMTYPE_CAROUSEL:
		*width = 130;
		*height = item->param3;
		break;
	default:
		*width = 80;
		*height = 12;
		break;
	}
}

const char var7f1b2400[] = "(BNC:Menu) findItem Warning: Item not found\n";
const char var7f1b2430[] = "(BNC:Menu) menuFirstFocus - Menu %s does not have any focusable item!\n";
const char var7f1b2478[] = "(BNC:Menu) menuLastFocus - Menu %s does not have any focusable item!\n";
const char var7f1b24c0[] = "Init menu %x\n";
const char var7f1b24d0[] = "[]-[] Added Main menu to ctl slot %d\n";
const char var7f1b24f8[] = "[]-[] Added Slide menu to ctl slot %d\n";
const char var7f1b2520[] = "[]-[] StackStart Stats: slides %d, ctlPtr %d slidePtr %d\n";
const char var7f1b255c[] = "$-$-$avePtr: %d\n";
const char var7f1b2570[] = "[]-[] Menu End: now %d slides\n";
const char var7f1b2590[] = "Freeing challenge mem\n";
const char var7f1b25a8[] = "IG:) style %d gbHead:%d\n";
const char var7f1b25c4[] = "GRABBED GUN MEM!\n";
const char var7f1b25d8[] = "Freeing challenge mem\n";

void func0f0f1d6c(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu)
{
	s32 colindex = menu->colend - 1;
	s32 rowindex = menu->rowend;
	s32 itemindex = 0;
	s32 numblocksthisitem;
	struct menuitem *item = dialogdef->items;
	s16 blockindex = menu->blockend;

	dialog->numcols = 0;
	dialog->colstart = (u16) colindex + 1;
	dialog->blockstart = blockindex;

	if (item) {
		bool newcolumn = true;

		while (item->type != MENUITEMTYPE_END) {
			if (item->flags & MENUITEMFLAG_NEWCOLUMN) {
				newcolumn = true;
			}

			if (newcolumn) {
				dialog->numcols++;
				colindex++;

				menu->cols[colindex].width = 0;
				menu->cols[colindex].height = 0;
				menu->cols[colindex].unk04 = 0;
				menu->cols[colindex].numrows = 0;
				menu->cols[colindex].rowstart = rowindex;

				newcolumn = false;
			}

			numblocksthisitem = -1;
			menuGetItemBlocksRequired(item, &numblocksthisitem);

			if (numblocksthisitem != -1) {
				menu->rows[rowindex].blockindex = blockindex;
				blockindex += (s16)numblocksthisitem;
			} else {
				menu->rows[rowindex].blockindex = -1;
			}

			menu->rows[rowindex].itemindex = itemindex;
			menu->cols[colindex].numrows++;
			rowindex++;
			item++;
			itemindex++;
		}
	}

	menu->colend = colindex + 1;
	menu->rowend = rowindex;
	menu->blockend = blockindex;
}

void dialog0f0f1ef4(struct menudialog *dialog)
{
	s32 bodyheight = dialog->height - LINEHEIGHT - 1;
	s32 itemheight;
	s32 i;
	s32 j;

	if ((dialog->definition->flags & MENUDIALOGFLAG_SMOOTHSCROLLABLE) == 0
			&& g_MenuData.root != MENUROOT_TRAINING
			&& bodyheight < dialog->contentheight) {
		for (i = 0; i < dialog->numcols; i++) {
			s32 colindex = dialog->colstart + i;
			s32 remaining = g_Menus[g_MpPlayerNum].cols[colindex].height - bodyheight;

			if (remaining > 0) {
				for (j = 0; j < g_Menus[g_MpPlayerNum].cols[colindex].numrows; j++) {
					if (remaining > 0) {
						s32 itemheight = 0;
						s32 rowindex = g_Menus[g_MpPlayerNum].cols[colindex].rowstart + j;
						struct menuitem *item = &dialog->definition->items[g_Menus[g_MpPlayerNum].rows[rowindex].itemindex];

						switch (item->type) {
						case MENUITEMTYPE_LIST:
							if (item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
								itemheight = remaining;

								if (g_Menus[g_MpPlayerNum].rows[rowindex].height - itemheight < 30) {
									itemheight = g_Menus[g_MpPlayerNum].rows[rowindex].height - 30;
								}
							} else {
								itemheight = (remaining + 10) / 11 * 11;
							}
							break;
						case MENUITEMTYPE_SCROLLABLE:
						case MENUITEMTYPE_MODEL:
							itemheight = remaining;
							if (g_Menus[g_MpPlayerNum].rows[rowindex].height - remaining < 50) {
								itemheight = g_Menus[g_MpPlayerNum].rows[rowindex].height - 50;
							}
							break;
						}

						if (itemheight > 0) {
							g_Menus[g_MpPlayerNum].rows[rowindex].height -= itemheight;
							remaining -= itemheight;
						}
					}
				}
			}
		}
	}
}

void dialogCalculateContentSize(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu)
{
	s32 contentheight;
	s32 rowindex;
	s32 contentwidth;
	s32 titleextra;
	s32 i;
	s32 textwidth;
	s32 textheight;
	u32 stack;
	struct menuitem *item = dialogdef->items;
	s32 colindex = dialog->colstart - 1;

	// Iterate items and calculate their dimensions
	if (item != NULL) {
		bool newcolumn = true;
		s16 width;
		s16 height;

		while (item->type != MENUITEMTYPE_END) {
			if (item->flags & MENUITEMFLAG_NEWCOLUMN) {
				newcolumn = true;
			}

			if (newcolumn) {
				colindex++;
				menu->cols[colindex].width = 0;
				menu->cols[colindex].height = 0;

				newcolumn = false;
				rowindex = menu->cols[colindex].rowstart;
			}

			menuCalculateItemSize(item, &width, &height, dialog);

			if (width > menu->cols[colindex].width) {
				menu->cols[colindex].width = width;
			}

			menu->rows[rowindex].height = height;
			menu->cols[colindex].height += height;
			rowindex++;
			item++;
		}
	}

	contentheight = 0;
	contentwidth = 0;

	for (i = 0; i < dialog->numcols; i++) {
		s32 columnheight = menu->cols[dialog->colstart + i].height;

		contentwidth += menu->cols[dialog->colstart + i].width;

		if (columnheight > contentheight) {
			contentheight = columnheight;
		}
	}

#if VERSION == VERSION_JPN_FINAL
	contentheight += 15;

	if ((dialog->definition->flags & MENUDIALOGFLAG_1000) == 0) {
		contentheight += 2;
	}
#else
	contentheight += 12;
#endif

	// Calculate and consider the title width.
	// Some of the multiplayer dialogs have a player number
	// in the top right, so extra space is considered for those.
	textMeasure(&textheight, &textwidth, menuResolveDialogTitle(dialog->definition), g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

	titleextra = 8;

	switch (g_MenuData.root) {
	case MENUROOT_MPSETUP:
	case MENUROOT_MPPAUSE:
	case MENUROOT_MPENDSCREEN:
	case MENUROOT_4MBMAINMENU:
		titleextra = 17;
		break;
	}

	if (textwidth + titleextra > contentwidth) {
		contentwidth = textwidth + titleextra;
	}

	dialog->contentwidth = contentwidth;
	dialog->contentheight = contentheight;
}

/**
 * Find the given item in the given dialog, and write its column and row indices
 * to the given pointers. Return the y value of the item relative to the dialog.
 */
s32 dialogFindItem(struct menudialog *dialog, struct menuitem *item, s32 *rowindex, s32 *colindex)
{
	for (*colindex = dialog->colstart; *colindex < dialog->colstart + dialog->numcols; *colindex += 1) {
		s32 y = 0;
		*rowindex = g_Menus[g_MpPlayerNum].cols[*colindex].rowstart;

		while (*rowindex < g_Menus[g_MpPlayerNum].cols[*colindex].rowstart + g_Menus[g_MpPlayerNum].cols[*colindex].numrows) {
			struct menuitem *thisitem = &dialog->definition->items[g_Menus[g_MpPlayerNum].rows[*rowindex].itemindex];

			if (thisitem == item) {
				return y;
			}

			y = y + g_Menus[g_MpPlayerNum].rows[*rowindex].height;
			*rowindex += 1;
		}
	}

	*colindex = dialog->colstart;
	*rowindex = g_Menus[g_MpPlayerNum].cols[*colindex].rowstart;

	return 0;
}

/**
 * If this returns true, the scrollable is rendered with less padding and
 * scrolling is disabled.
 */
bool menuIsScrollableUnscrollable(struct menuitem *item)
{
	if (item->type == MENUITEMTYPE_SCROLLABLE) {
		if (item->param == DESCRIPTION_MPCONFIG
				|| item->param == DESCRIPTION_MPCHALLENGE
				|| item->param == DESCRIPTION_DEVICETRAINING
				|| item->param == DESCRIPTION_FRWEAPON
				|| item->param == DESCRIPTION_HOLOTRAINING) {
			return true;
		}
	}

	return false;
}

bool menuIsItemDisabled(struct menuitem *item, struct menudialog *dialog)
{
	union handlerdata sp30;
	s16 width;
	s16 height;
	u32 stack[2];

	if (item->flags & MENUITEMFLAG_ALWAYSDISABLED) {
		return true;
	}

	if (mpIsPlayerLockedOut(g_MpPlayerNum) && item->flags & MENUITEMFLAG_LOCKABLEMAJOR) {
		return true;
	}

	if (menuIsScrollableUnscrollable(item)) {
		return true;
	}

	if (item->handler
			&& (item->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0
			&& item->handler(MENUOP_CHECKDISABLED, item, &sp30)) {
		return true;
	}

	menuCalculateItemSize(item, &width, &height, dialog);

	if (height == 0) {
		return true;
	}

	return false;
}

bool menuIsItemFocusable(struct menuitem *item, struct menudialog *dialog, s32 arg2)
{
	s32 rowindex;
	s32 colindex;

	switch (item->type) {
	case MENUITEMTYPE_LABEL:
	case MENUITEMTYPE_OBJECTIVES:
	case MENUITEMTYPE_07:
	case MENUITEMTYPE_SEPARATOR:
	case MENUITEMTYPE_MODEL:
	case MENUITEMTYPE_13:
	case MENUITEMTYPE_METER:
	case MENUITEMTYPE_MARQUEE:
	case MENUITEMTYPE_CONTROLLER:
#ifndef PLATFORM_N64
	case MENUITEMTYPE_COLORBOX:
#endif
		return false;
	case MENUITEMTYPE_10:
	case MENUITEMTYPE_14:
	case MENUITEMTYPE_16:
	case MENUITEMTYPE_18:
		dialogFindItem(dialog, item, &rowindex, &colindex);
	}

	if (menuIsItemDisabled(item, dialog)) {
		return false;
	}

	return true;
}

struct menuitem *dialogFindItemAtColY(s32 targety, s32 colindex, struct menudialogdef *dialogdef, s32 *rowindexptr, struct menudialog *dialog)
{
	struct menuitem *result = NULL;
	bool done = false;
	s32 rowindex = g_Menus[g_MpPlayerNum].cols[colindex].rowstart;
	s32 y;
	s32 i;

	for (i = 0, y = 0; !done && i < g_Menus[g_MpPlayerNum].cols[colindex].numrows; rowindex++, i++) {
		struct menuitem *item = &dialogdef->items[g_Menus[g_MpPlayerNum].rows[rowindex].itemindex];

		if (menuIsItemFocusable(item, dialog, 1)) {
			result = item;

			if (y >= targety) {
				done = true;
			}

			*rowindexptr = rowindex;
		}

		y = y + g_Menus[g_MpPlayerNum].rows[rowindex].height;
	}

	return result;
}

struct menuitem *dialogFindFirstItem(struct menudialog *dialog)
{
	s32 i;
	s32 colindex = dialog->colstart;
	s32 rowindex;

	for (i = 0; i < dialog->numcols; i++) {
		struct menuitem *item = dialogFindItemAtColY(0, colindex, dialog->definition, &rowindex, dialog);

		if (item != NULL) {
			return item;
		}

		colindex++;
	}

	menuResolveDialogTitle(dialog->definition);

	return dialog->definition->items;
}

struct menuitem *dialogFindFirstItemRight(struct menudialog *dialog)
{
	s32 i;
	s32 colindex = dialog->colstart + dialog->numcols - 1;
	s32 rowindex;

	for (i = 0; i < dialog->numcols; i++) {
		struct menuitem *item = dialogFindItemAtColY(0, colindex, dialog->definition, &rowindex, dialog);

		if (item != NULL) {
			return item;
		}

		colindex--;
	}

	menuResolveDialogTitle(dialog->definition);

	return dialog->definition->items;
}

void dialogChangeItemFocusVertically(struct menudialog *dialog, s32 updown)
{
	s32 rowindex;
	s32 colindex;
	bool done = false;
	s32 startrowindex;
	struct menuitem *item;
	s32 start;
	s32 end;

	dialogFindItem(dialog, dialog->focuseditem, &rowindex, &colindex);

	startrowindex = rowindex;

	while (!done) {
		rowindex += updown;
		start = g_Menus[g_MpPlayerNum].cols[colindex].rowstart;
		end = g_Menus[g_MpPlayerNum].cols[colindex].numrows + start;

		if (rowindex >= end) {
			rowindex = start;
		}

		if (rowindex < start) {
			rowindex = end - 1;
		}

		item = &dialog->definition->items[g_Menus[g_MpPlayerNum].rows[rowindex].itemindex];

		if (menuIsItemFocusable(item, dialog, updown)) {
			done = true;
		}

		if (rowindex == startrowindex) {
			done = true;
		}
	}

	dialog->focuseditem = item;
}

s32 dialogChangeItemFocusHorizontally(struct menudialog *dialog, s32 leftright)
{
	s32 rowindex;
	s32 colindex;
	bool done = false;
	s32 swipedir = 0;
	s32 y = dialogFindItem(dialog, dialog->focuseditem, &rowindex, &colindex);
	s32 startcolindex = colindex;
	struct menuitem *item;

	while (!done) {
		colindex += leftright;

		if (colindex >= dialog->colstart + dialog->numcols) {
			swipedir = 1;
			colindex = dialog->colstart;
		}

		if (colindex < dialog->colstart) {
			swipedir = -1;
			colindex = dialog->colstart + dialog->numcols - 1;
		}

		item = dialogFindItemAtColY(y, colindex, dialog->definition, &rowindex, dialog);

		if (item) {
			done = true;
		}

		if (colindex == startcolindex) {
			done = true;
		}
	}

	if (item) {
		dialog->focuseditem = item;
	}

	return swipedir;
}

#ifndef PLATFORM_N64

bool dialogChangeItemFocusWithMouse(struct menudialog *dialog, s32 mx, s32 my)
{
	struct menu *menu = &g_Menus[g_MpPlayerNum];
	s32 col, row;
	s32 curx, cury;
	s32 colwidth = 0;

	// only allow mouse control of player 1 menus; ignore mouse if navigating with keyboard
	if (menu->playernum != 0 || !g_MenuUsingMouse) {
		return false;
	}

	curx = dialog->x;

	for (col = 0; col < dialog->numcols; col++, curx += colwidth) {
		s32 colindex = dialog->colstart + col;
		colwidth = menu->cols[colindex].width;
		if (mx <= curx || mx >= curx + colwidth) {
			continue;
		}

		cury = dialog->y + LINEHEIGHT + 1 + dialog->scroll;
		for (row = 0; row < menu->cols[colindex].numrows; row++) {
			s32 rowindex = menu->cols[colindex].rowstart + row;
			s32 rowheight = menu->rows[rowindex].height;
			struct menuitem *item = &dialog->definition->items[menu->rows[rowindex].itemindex];
			if (my > cury && my < cury + rowheight &&
					item != dialog->focuseditem && menuIsItemFocusable(item, dialog, 0)) {
				dialog->focuseditem = item;
				return true;
			}
			cury += rowheight;
		}
	}

	return false;
}

#endif

s32 dialogChangeItemFocus(struct menudialog *dialog, struct menuinputs *inputs)
{
	s32 swipedir = 0;

	if (inputs->leftright == 0 && inputs->updown == 0) {
#ifndef PLATFORM_N64
		if (!dialogChangeItemFocusWithMouse(dialog, inputs->mousex, inputs->mousey)) {
			return 0;
		}
#else
		return 0;
#endif
	}

	if (inputs->updown != 0) {
		dialogChangeItemFocusVertically(dialog, inputs->updown);
	}

	if (inputs->leftright != 0) {
		swipedir = dialogChangeItemFocusHorizontally(dialog, inputs->leftright);
	}

	if (dialog->focuseditem != 0) {
		if (dialog->focuseditem->handler != NULL) {
			if ((dialog->focuseditem->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0) {
				union handlerdata data;
				dialog->focuseditem->handler(MENUOP_FOCUS, dialog->focuseditem, &data);
			}
		}
	}

	return swipedir;
}

void menuOpenDialog(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu)
{
	union handlerdata data3;
	struct menuitem *item;
	union handlerdata data1;
	union handlerdata data2;

	dialog->definition = dialogdef;

	switch (g_MenuData.root) {
	case MENUROOT_MPSETUP:
	case MENUROOT_4MBMAINMENU:
		dialog->unk6e = 1;
		break;
	case MENUROOT_MAINMENU:
	case MENUROOT_TRAINING:
	default:
		dialog->unk6e = 0;
		break;
	}

	func0f0f1d6c(dialogdef, dialog, menu);
	dialogInitItems(dialog);

	dialog->type = dialogdef->type;
	dialog->transitionfrac = -1;
	dialog->redrawtimer = 0;
	dialog->unk4c = RANDOMFRAC() * M_TAU;

	g_Menus[g_MpPlayerNum].curdialog->state = MENUDIALOGSTATE_PREOPEN;
	g_Menus[g_MpPlayerNum].curdialog->statefrac = 0;

	dialog->unk54 = 0;
	dialog->unk58 = 0;
	dialog->unk5c = 0;

	dialog->focuseditem = dialogFindFirstItem(dialog);

	// Check if any items should be focused automatically
	item = dialog->definition->items;

	while (item->type != MENUITEMTYPE_END) {
		if (item->handler
				&& (item->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0
				&& item->handler(MENUOP_CHECKPREFOCUSED, item, &data1)) {
			dialog->focuseditem = item;
		}

		item++;
	}

	// Run focus handler
	if (dialog->focuseditem
			&& dialog->focuseditem->handler
			&& (dialog->focuseditem->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0) {
		dialog->focuseditem->handler(MENUOP_FOCUS, dialog->focuseditem, &data2);
	}

	dialog->dimmed = false;
	dialog->scroll = 0;
	dialog->dstscroll = 0;

	if (dialogdef->handler) {
		dialogdef->handler(MENUOP_OPEN, dialogdef, &data3);
	}

	dialogCalculateContentSize(dialogdef, dialog, menu);
	dialogCalculatePosition(dialog);

	dialog->x = dialog->dstx;
	dialog->y = dialog->dsty;
	dialog->width = dialog->dstwidth;
	dialog->height = dialog->dstheight;
}

void menuPushDialog(struct menudialogdef *dialogdef)
{
#ifndef PLATFORM_N64
	// Prevent the mouse from immediately changing a slider's value, as was
	// happening while entering the Audio menu.
	g_AllowMouseHeld = false;
#endif

	if (dialogdef) {
		menuUnsetModel(&g_Menus[g_MpPlayerNum].menumodel);

		if (g_Menus[g_MpPlayerNum].depth < 6 && g_Menus[g_MpPlayerNum].numdialogs < ARRAYCOUNT(g_Menus[0].dialogs)) {
			struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth];
			struct menudialogdef *sibling;
			struct menudialog *dialog;

			g_Menus[g_MpPlayerNum].depth++;

			layer->numsiblings = 1;
			layer->cursibling = 0;

			dialog = &g_Menus[g_MpPlayerNum].dialogs[g_Menus[g_MpPlayerNum].numdialogs];
			g_Menus[g_MpPlayerNum].numdialogs++;
			layer->siblings[0] = dialog;
			g_Menus[g_MpPlayerNum].curdialog = dialog;
			dialog->swipedir = 0;

			menuOpenDialog(dialogdef, dialog, &g_Menus[g_MpPlayerNum]);

			dialog->dstx = (viGetWidth() - dialog->width) / 2;
			dialog->dsty = (viGetHeight() - dialog->height) / 2;

			g_Menus[g_MpPlayerNum].fm.unke40_00 = true;
			sibling = dialogdef->nextsibling;

			while (sibling && layer->numsiblings < 5) {
				// @bug:
				// If this limit were to be reached, the game would soft lock
				// because sibling is incremented inside the if-statement block.
				if (g_Menus[g_MpPlayerNum].numdialogs < ARRAYCOUNT(g_Menus[0].dialogs)) {
					dialog = &g_Menus[g_MpPlayerNum].dialogs[g_Menus[g_MpPlayerNum].numdialogs];
					g_Menus[g_MpPlayerNum].numdialogs++;

					layer->siblings[layer->numsiblings] = dialog;
					layer->numsiblings++;

					dialog->swipedir = -1;

					menuOpenDialog(sibling, dialog, &g_Menus[g_MpPlayerNum]);

					dialog->dstx = dialog->x = -SCREEN_320;
					dialog->dsty = dialog->y = (viGetHeight() - dialog->height) / 2;
					dialog->type = 0;

					sibling = sibling->nextsibling;
				}
			}

			if (sibling);

			menuPlaySound(MENUSOUND_OPENDIALOG);

			if (dialogdef->type == MENUDIALOGTYPE_DANGER) {
				menuPlaySound(MENUSOUND_ERROR);
			}

			if (dialogdef->type == MENUDIALOGTYPE_SUCCESS) {
				menuPlaySound(MENUSOUND_SUCCESS);
			}
		}
	}
}

#if VERSION >= VERSION_NTSC_1_0
bool func0f0f3220(s32 arg0)
{
	bool save = true;
	s32 i;

	if (g_MenuData.unk669[arg0] == 4) {
		s32 prevplayernum = g_MpPlayerNum;

		for (i = ARRAYCOUNT(g_Menus) - 1; i >= 0; i--) {
			if (g_Menus[i].curdialog) {
				g_MpPlayerNum = i;
			}
		}

		if (g_Menus[g_MpPlayerNum].depth >= 2) {
			save = false;
		}

		if (g_Menus[g_MpPlayerNum].curdialog
				&& g_Menus[g_MpPlayerNum].curdialog->definition->type == MENUDIALOGTYPE_DANGER) {
			save = false;

			if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_MpEndscreenChallengeCheatedMenuDialog
					|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_MpEndscreenChallengeFailedMenuDialog) {
				save = true;
			}
		}

		if (save) {
			filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
		}

		g_MpPlayerNum = prevplayernum;
	} else if (g_MenuData.unk669[arg0] < 4) {
		s32 prevplayernum = g_MpPlayerNum;
		g_MpPlayerNum = g_MenuData.unk669[arg0];
		filemgrSaveOrLoad(&g_PlayerConfigsArray[g_MpPlayerNum].fileguid, FILEOP_SAVE_MPPLAYER, g_MpPlayerNum);
		save = true;
		g_MpPlayerNum = prevplayernum;
	}

	if (save) {
		g_MenuData.unk66e--;
	}

	return save;
}
#else
void func0f0f3220(s32 arg0)
{
	s32 i;

	if (g_MenuData.unk669[arg0] == 4) {
		s32 prevplayernum = g_MpPlayerNum;

		for (i = ARRAYCOUNT(g_Menus) - 1; i >= 0; i--) {
			if (g_Menus[i].curdialog) {
				g_MpPlayerNum = i;
			}
		}

		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);

		g_MpPlayerNum = prevplayernum;
	} else if (g_MenuData.unk669[arg0] < 4) {
		s32 prevplayernum = g_MpPlayerNum;
		g_MpPlayerNum = g_MenuData.unk669[arg0];
		filemgrSaveOrLoad(&g_PlayerConfigsArray[g_MpPlayerNum].fileguid, FILEOP_SAVE_MPPLAYER, g_MpPlayerNum);
		g_MpPlayerNum = prevplayernum;
	}

	g_MenuData.unk66e--;
}
#endif

void menuCloseDialog(void)
{
	if (g_Menus[g_MpPlayerNum].depth > 0) {
		union handlerdata data;
		struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1];
		u32 value_prevent = 1; // constant value, but required for match
		s32 i;

		for (i = 0; i < layer->numsiblings; i++) {
			data.dialog1.preventclose = false;

			if (layer->siblings[i]->definition->handler) {
				layer->siblings[i]->definition->handler(MENUOP_CLOSE, layer->siblings[i]->definition, &data);
			}

			if (value_prevent == data.dialog1.preventclose) {
				return;
			}
		}

		for (i = 0; i < layer->numsiblings; i++) {
			g_Menus[g_MpPlayerNum].numdialogs--;
		}

		g_Menus[g_MpPlayerNum].rowend = g_Menus[g_MpPlayerNum].cols[layer->siblings[0]->colstart].rowstart;
		g_Menus[g_MpPlayerNum].colend = layer->siblings[0]->colstart;
		g_Menus[g_MpPlayerNum].blockend = layer->siblings[0]->blockstart;
		g_Menus[g_MpPlayerNum].depth--;

		menuPlaySound(MENUSOUND_0B);
	}

#if VERSION >= VERSION_NTSC_1_0
	if (g_MenuData.unk66e > 0 && g_Menus[g_MpPlayerNum].depth == 0)
#else
	if (g_MenuData.unk66e > 0)
#endif
	{
		s32 value = g_MenuData.unk66e;

		while (value >= 0) {
			func0f0f3220(value);
			value--;
		}
	}

	if (g_Menus[g_MpPlayerNum].depth == 0) {
		g_Menus[g_MpPlayerNum].curdialog = NULL;
	} else {
		struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1];
		g_Menus[g_MpPlayerNum].curdialog = layer->siblings[layer->cursibling];
	}
}

void menuUpdateCurFrame(void)
{
	s32 depth = g_Menus[g_MpPlayerNum].depth;

	if (depth == 0) {
		// No more parent menus - return control to the player
		g_Vars.currentplayer->joybutinhibit = 0xffffffff;
		menuClose();
		g_Menus[g_MpPlayerNum].curdialog = NULL;
	} else {
		// Set up parent menu
		struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[depth - 1];
		g_Menus[g_MpPlayerNum].curdialog = layer->siblings[layer->cursibling];
	}
}

void menuPopDialog(void)
{
	menuCloseDialog();
	menuUpdateCurFrame();
}

void func0f0f3704(struct menudialogdef *dialogdef)
{
	menuCloseDialog();
	menuPushDialog(dialogdef);
}

void menuConfigureModel(struct menumodel *menumodel, f32 x, f32 y, f32 z, f32 rotx, f32 roty, f32 rotz, f32 scale, u8 flags)
{
	menumodel->configuring = true;

	if (flags & MENUMODELFLAG_HASPOSITION) {
		menumodel->newposx = x;
		menumodel->newposy = y;
		menumodel->newposz = z;
	}

	if (flags & MENUMODELFLAG_HASROTATION) {
		menumodel->newrotx = rotx;
		menumodel->newroty = roty;
		menumodel->newrotz = rotz;
	}

	if (flags & MENUMODELFLAG_HASSCALE) {
		menumodel->newscale = scale;
	}

	menumodel->flags = flags;
	menumodel->configurefrac = 0.0f;
}

void menuUnsetModel(struct menumodel *menumodel)
{
	if (menumodel->curparams == 0x4fac5ace) {
		challengeUnsetCurrent();
	}

	menumodel->loaddelay = 0;
	menumodel->newparams = 0;
	menumodel->curparams = 0;
	menumodel->bodymodeldef = NULL;
	menumodel->headmodeldef = NULL;
	menumodel->newanimnum = 0;
	menumodel->curanimnum = 0;
	menumodel->perfectheadnum = 0;
	menumodel->isperfecthead = false;
	menumodel->unk5b1_02 = false;
	menumodel->unk5b1_03 = false;
	menumodel->reverseanim = false;
	menumodel->configuring = false;
	menumodel->unk5b1_06 = false;
	menumodel->drawbehinddialog = false;
	menumodel->partvisibility = NULL;
	menumodel->unk560 = -1;
	menumodel->headnum = -1;
	menumodel->bodynum = -1;
	menumodel->newrotx = menumodel->newroty = menumodel->newrotz = 0.0f;
	menumodel->newposx = menumodel->newposy = menumodel->newposz = 0.0f;
	menumodel->displacex = menumodel->displacey = menumodel->displacez = 0.0f;
	menumodel->currotx = menumodel->curroty = menumodel->currotz = 0.0f;
	menumodel->curposx = menumodel->curposy = menumodel->curposz = 0.0f;
	menumodel->unk558 = 0.0f;
	menumodel->unk55c = 1.0f;
	menumodel->curscale = 1.0f;
	menumodel->newscale = 1.0f;
	menumodel->zoom = -1.0f;
}

Lights1 var80071468 = gdSPDefLights1(0x96, 0x96, 0x96, 0xff, 0xff, 0xff, 0xb2, 0x4d, 0x2e);

/**
 * Render the hudpiece as well as any models within dialogs.
 */
Gfx *menuRenderModel(Gfx *gdl, struct menumodel *menumodel, s32 modeltype)
{
	f32 rotx;
	f32 roty;
	f32 rotz;
	f32 posx;
	f32 posy;
	f32 posz;
	f32 scale;
	s32 totalfilelen;
	struct texpool texpool;
	s32 bodyfilelen2;
	u16 bodyfilenum;
	u16 headfilenum;
	s32 bodynum;
	s32 headnum;

	if (g_Vars.stagenum != STAGE_CITRAINING && g_Vars.stagenum != STAGE_CREDITS) {
		if (g_MenuData.unk5d5_01 && modeltype != MENUMODELTYPE_HUDPIECE && modeltype < MENUMODELTYPE_3) {
			return gdl;
		}

		if (menumodel->allocstart == NULL) {
			if (bgunChangeGunMem(GUNMEMOWNER_INVMENU)) {
				menumodel->allocstart = bgunGetGunMem();
				menumodel->alloclen = bgunCalculateGunMemCapacity();
			} else {
				return gdl;
			}
		}
	}

	if (menumodel->allocstart == NULL) {
		return gdl;
	}

	// If the caller has asked for a new model to be loaded, do it
	if (menumodel->newparams != 0) {
		if (menumodel->newparams == menumodel->curparams) {
			menumodel->newparams = 0;
			menumodel->loaddelay = 0;
		} else {
			if (menumodel->curparams == 0x4fac5ace) {
				challengeUnsetCurrent();
			}

			if (menumodel->loaddelay == 0) {
				menumodel->loaddelay = 1;
				return gdl;
			}

			menumodel->loaddelay--;

			if (menumodel->loaddelay == 0) {
				if (MENUMODELPARAMS_GET_FILENUM(menumodel->newparams) == 0xffff || MENUMODELPARAMS_HAS_MASTER_HEADBODY(menumodel->newparams)) {
					if (MENUMODELPARAMS_HAS_MASTER_HEADBODY(menumodel->newparams)) {
						headnum = MENUMODELPARAMS_GET_MASTER_HEADNUM(menumodel->newparams);
						bodynum = MENUMODELPARAMS_GET_MASTER_BODYNUM(menumodel->newparams);
					} else {
						s32 mpheadnum = MENUMODELPARAMS_GET_MP_HEADNUM(menumodel->newparams);
						s32 mpbodynum = MENUMODELPARAMS_GET_MP_BODYNUM(menumodel->newparams);
						bodynum = mpGetBodyId(mpbodynum);

						if (mpheadnum < mpGetNumHeads2()) {
							headnum = mpGetHeadId(mpheadnum);
						} else {
							headnum = func0f14a9f8(mpheadnum - mpGetNumHeads2());
							headnum = mpGetBeauHeadId(headnum);
							menumodel->perfectheadnum = (mpheadnum - mpGetNumHeads2()) & 0xff;
						}
					}

					bodyfilenum = g_HeadsAndBodies[bodynum].filenum;

					totalfilelen = fileGetInflatedSize(bodyfilenum, LOADTYPE_MODEL);
					totalfilelen = ALIGN64(totalfilelen);

					if (g_HeadsAndBodies[bodynum].unk00_01) {
						headnum = -1;
						headfilenum = 0xffff;
					} else {
						headfilenum = g_HeadsAndBodies[headnum].filenum;
						totalfilelen += ALIGN64(fileGetInflatedSize(headfilenum, LOADTYPE_MODEL));
					}

#ifdef PLATFORM_64BIT
					totalfilelen += 0x6000;
#else
					totalfilelen += 0x4000;
#endif

#ifndef PLATFORM_N64
					videoFreeCachedTextures(menumodel->allocstart, menumodel->allocstart + menumodel->alloclen);
#endif

					texInitPool(&texpool, menumodel->allocstart + totalfilelen, menumodel->alloclen - totalfilelen);

					menumodel->headnum = headnum;
					menumodel->bodynum = bodynum;
					menumodel->bodymodeldef = modeldefLoad(bodyfilenum, menumodel->allocstart, totalfilelen, &texpool);
					bodyfilelen2 = ALIGN64(fileGetLoadedSize(bodyfilenum));
					modelAllocateRwData(menumodel->bodymodeldef);

					if (headnum < 0) {
						menumodel->headmodeldef = NULL;
					} else {
						menumodel->headmodeldef = modeldefLoad(headfilenum, menumodel->allocstart + bodyfilelen2, totalfilelen - bodyfilelen2, &texpool);
						fileGetLoadedSize(headfilenum);
						bodyCalculateHeadOffset(menumodel->headmodeldef, headnum, bodynum);
						modelAllocateRwData(menumodel->headmodeldef);
					}

					modelInit(&menumodel->bodymodel, menumodel->bodymodeldef, menumodel->rwdata, true);
					animInit(&menumodel->bodyanim);

#ifdef PLATFORM_64BIT
					menumodel->bodymodel.rwdatalen = 256 + 128;
#else
					menumodel->bodymodel.rwdatalen = 256;
#endif
					menumodel->bodymodel.anim = &menumodel->bodyanim;

					body0f02ce8c(bodynum, headnum, menumodel->bodymodeldef, menumodel->headmodeldef, totalfilelen * 0, &menumodel->bodymodel, false, 1);
				} else {
					totalfilelen = ALIGN64(fileGetInflatedSize(menumodel->newparams, LOADTYPE_MODEL)) + 0x4000;
					if (1);

#ifndef PLATFORM_N64
					videoFreeCachedTextures(menumodel->allocstart, menumodel->allocstart + menumodel->alloclen);
#endif

					texInitPool(&texpool, &menumodel->allocstart[(u32)totalfilelen], menumodel->alloclen - totalfilelen);

					menumodel->headnum = -1;
					menumodel->bodynum = -1;
					menumodel->bodymodeldef = modeldefLoad(menumodel->newparams, menumodel->allocstart, totalfilelen, &texpool);

					fileGetLoadedSize(menumodel->newparams);
					modelAllocateRwData(menumodel->bodymodeldef);
					modelInit(&menumodel->bodymodel, menumodel->bodymodeldef, menumodel->rwdata, true);
					animInit(&menumodel->bodyanim);

#ifdef PLATFORM_64BIT
					menumodel->bodymodel.rwdatalen = 256+128;
#else
					menumodel->bodymodel.rwdatalen = 256;
#endif
					menumodel->bodymodel.anim = &menumodel->bodyanim;
				}

				menumodel->curparams = menumodel->newparams;
				menumodel->curanimnum = 0;
				menumodel->newparams = 0;
			} else {
				return gdl;
			}
		}
	}

	if (menumodel->bodymodeldef != NULL) {
		struct modelrenderdata renderdata = {NULL, true, 3};
		Mtxf *matrices;
		s32 i;
		u32 stack[3];
		struct coord tmpcoord;
		f32 screenpos[2];
		Mtxf rotmtx;
		Mtxf posmtx;
		f32 screenz[1];
		bool haszoom;
		struct coord zoompos;
		f32 zoomy;
		bool dodefaultzoom;

		// Most models use the z-buffer and a scissor.
		// Types 2 and 3 are unused. Type 4 is the credits scrolling logo.
		if (modeltype < MENUMODELTYPE_3 && g_MenuData.usezbuf) {
			gdl = viPrepareZbuf(gdl);
			gdl = vi0000b1d0(gdl);

			g_MenuData.usezbuf = false;

			if (modeltype != MENUMODELTYPE_2) {
				gdl = menuApplyScissor(gdl);
			}

			gSPSetGeometryMode(gdl++, G_ZBUFFER);
		}

		gSPDisplayList(gdl++, var80061380);
		gSPDisplayList(gdl++, var800613a0);

		haszoom = false;

		// Handle updating the camera zoom in/out (eg. character selection dialog)
		if (menumodel->zoom > 0.0f) {
			dodefaultzoom = true;

			if (menumodel->bodymodeldef->skel == &g_SkelChr) {
				struct modelnode *node = modelGetPart(menumodel->bodymodeldef, MODELPART_CHR_0006);

				if (node) {
					struct modelrodata_position *rodata = &node->rodata->position;
					f32 frac = menuGetLinearOscPauseFrac(menumodel->zoomtimer60 / TICKS(480.0f));

					zoompos.f[0] = 0.0f;
					zoompos.f[1] = 0.0f - (rodata->pos.f[1] / 7.6f * (1.0f - frac * frac));
					zoompos.f[2] = 0.0f;

					haszoom = true;

					menumodel->zoom = 100.0f + (1.0f - frac) * 270.0f;
					zoomy = menumodel->zoom / (rodata->pos.f[1] / 2.0f);

					dodefaultzoom = false;

					modelFindBboxRodata(&menumodel->bodymodel);
				}

				if (1);
			}

			if (dodefaultzoom) {
				struct modelrodata_bbox *bbox = modelFindBboxRodata(&menumodel->bodymodel);

				if (bbox) {
					zoompos.x = -(bbox->xmax - ((bbox->xmax - bbox->xmin) * 0.5f));
					zoompos.y = -(bbox->ymax - ((bbox->ymax - bbox->ymin) * 0.5f));
					zoompos.z = -(bbox->zmax - ((bbox->zmax - bbox->zmin) * 0.5f));
					haszoom = true;
					zoomy = menumodel->zoom / ((bbox->ymax - bbox->ymin) * 0.5f);
				}
			}
		}

		mtx4LoadIdentity(&rotmtx);

		// For the hudpiece, tween the position and scale to the new values and apply rotation.
		if (modeltype == MENUMODELTYPE_HUDPIECE) {
			if (IS8MB()) {
				s32 i;

				if (menumodel->curposx != menumodel->newposx) {
					for (i = 0; i < g_Vars.diffframe60; i++) {
						menumodel->curposx = (menumodel->newposx * PALUPF(0.002f)) + ((1.0f - PALUPF(0.002f)) * menumodel->curposx);
					}
				}

				if (menumodel->curposy != menumodel->newposy) {
					for (i = 0; i < g_Vars.diffframe60; i++) {
						menumodel->curposy = (menumodel->newposy * PALUPF(0.002f)) + ((1.0f - PALUPF(0.002f)) * menumodel->curposy);
					}
				}

				if (menumodel->curposz != menumodel->newposz) {
					for (i = 0; i < g_Vars.diffframe60; i++) {
						menumodel->curposz = (menumodel->newposz * PALUPF(0.002f)) + ((1.0f - PALUPF(0.002f)) * menumodel->curposz);
					}
				}

				if (menumodel->curscale != menumodel->newscale) {
					for (i = 0; i < g_Vars.diffframe60; i++) {
						menumodel->curscale = (menumodel->newscale * PALUPF(0.002f)) + ((1.0f - PALUPF(0.002f)) * menumodel->curscale);
					}
				}

				posx = menumodel->curposx;

#if !PAL
				if (g_ViRes == VIRES_HI) {
					posx *= 2.0f;
				}
#endif

				posy = menumodel->curposy;
				posz = menumodel->curposz;

				scale = menumodel->curscale;

				menumodel->currotx = rotx = menumodel->newrotx;
				menumodel->curroty = roty = menumodel->newroty;
				menumodel->currotz = rotz = menumodel->newrotz;

				tmpcoord.x = rotx;
				tmpcoord.y = roty;
				tmpcoord.z = rotz;

				mtx4LoadRotation(&tmpcoord, &rotmtx);
			}
		} else {
			// If the caller is reconfiguring the model's position, rotation or scale, tween towards the new values.
			if (menumodel->configuring) {
#if VERSION >= VERSION_PAL_BETA
				menumodel->configurefrac += g_Vars.diffframe60freal / 40.0f;
#else
				menumodel->configurefrac += g_Vars.diffframe60f / 40.0f;
#endif

				if (menumodel->configurefrac > 1.0f) {
					menumodel->configuring = false;
					menumodel->curposx = menumodel->newposx;
					menumodel->curposy = menumodel->newposy;
					menumodel->curposz = menumodel->newposz;
					menumodel->curscale = menumodel->newscale;
				} else {
					f32 fracnew = (-cosf(menumodel->configurefrac * M_PI) * 0.5f) + 0.5f;
					f32 fraccur = 1.0f - fracnew;

					if (menumodel->flags & MENUMODELFLAG_HASPOSITION) {
						posx = (menumodel->curposx * fraccur) + (fracnew * menumodel->newposx);
						posy = (menumodel->curposy * fraccur) + (fracnew * menumodel->newposy);
						posz = (menumodel->curposz * fraccur) + (fracnew * menumodel->newposz);
					} else {
						posx = menumodel->curposx = menumodel->newposx;
						posy = menumodel->curposy = menumodel->newposy;
						posz = menumodel->curposz = menumodel->newposz;
					}

					if (menumodel->flags & MENUMODELFLAG_HASSCALE) {
						scale = (menumodel->curscale * fraccur) + (fracnew * menumodel->newscale);
					} else {
						scale = menumodel->curscale = menumodel->newscale;
					}

					if (menumodel->flags & MENUMODELFLAG_HASROTATION) {
						f32 sp2bc[4];
						f32 sp2ac[4];
						f32 sp29c[4];
						struct coord tmprot;

						tmprot.x = menumodel->currotx;
						tmprot.y = menumodel->curroty;
						tmprot.z = menumodel->currotz;

						quaternion0f096ca0(&tmprot, sp2bc);

						tmprot.x = menumodel->newrotx;
						tmprot.y = menumodel->newroty;
						tmprot.z = menumodel->newrotz;

						quaternion0f096ca0(&tmprot, sp2ac);
						quaternionSlerp(sp2bc, sp2ac, fracnew, sp29c);
						quaternionToMtx(sp29c, &rotmtx);
					} else {
						menumodel->currotx = rotx = menumodel->newrotx;
						menumodel->curroty = roty = menumodel->newroty;
						menumodel->currotz = rotz = menumodel->newrotz;

						tmpcoord.x = rotx;
						tmpcoord.y = roty;
						tmpcoord.z = rotz;

						mtx4LoadRotation(&tmpcoord, &rotmtx);
					}
				}
			}

			if (!menumodel->configuring) {
				posx = menumodel->curposx = menumodel->newposx;
				posy = menumodel->curposy = menumodel->newposy;
				posz = menumodel->curposz = menumodel->newposz;

				scale = menumodel->curscale = menumodel->newscale;

				if (1);

				menumodel->currotx = rotx = menumodel->newrotx;
				menumodel->curroty = roty = menumodel->newroty;
				menumodel->currotz = rotz = menumodel->newrotz;

				tmpcoord.x = rotx;
				tmpcoord.y = roty;
				tmpcoord.z = rotz;

				mtx4LoadRotation(&tmpcoord, &rotmtx);
			}
		}

		screenz[0] = -100.0f + posz;

		if (modeltype == MENUMODELTYPE_HUDPIECE) {
			if (IS8MB()) {
				screenpos[0] = menumodel->curposx * g_ScaleX;
				screenpos[1] = menumodel->curposy;
			}
		} else {
			screenpos[0] = posx * g_ScaleX + viGetViewLeft() + viGetViewWidth() * 0.5f;
			screenpos[1] = posy + viGetViewTop() + viGetViewHeight() * 0.5f;
		}

		cam0f0b4c3c(screenpos, &tmpcoord, 1.0f);

		mtx4LoadIdentity(&posmtx);

		// Show or hide model parts according to the visibility list
		if (menumodel->partvisibility != NULL) {
			struct modelpartvisibility *ptr = menumodel->partvisibility;

			while (ptr->part != 255) {
				struct modelnode *node = modelGetPart(menumodel->bodymodeldef, ptr->part);

				if (node) {
					union modelrwdata *rwdata = modelGetNodeRwData(&menumodel->bodymodel, node);

					if (rwdata) {
						if (ptr->visible) {
							rwdata->toggle.visible = true;
						} else {
							rwdata->toggle.visible = false;
						}
					}
				}

				ptr++;
			}
		}

		if (modeltype == MENUMODELTYPE_3) {
			tmpcoord.x = menumodel->curposx;
			tmpcoord.y = menumodel->curposy;
			tmpcoord.z = menumodel->curposz;
		} else {
			// Reusing rotz
			rotz = screenz[0] / tmpcoord.z;
			tmpcoord.x = rotz * tmpcoord.x;
			tmpcoord.y = rotz * tmpcoord.y;
			tmpcoord.z = rotz * tmpcoord.z;
		}

#if VERSION < VERSION_NTSC_1_0
		if (MENUMODELPARAMS_HAS_MASTER_HEADBODY(menumodel->curparams)) {
			struct coord oldpos;
			struct coord newpos = {0, 0, 0};
			u32 stack[3];

			modelUpdateInfo(&menumodel->bodymodel);

			modelGetRootPosition(&menumodel->bodymodel, &oldpos);

			if (joyGetButtons(0, R_TRIG)) {
				modelSetRootPosition(&menumodel->bodymodel, &newpos);
			}
		}
#endif

		mtx4LoadTranslation(&tmpcoord, &posmtx);

		if (haszoom) {
			mtx00015f04(scale * zoomy, &posmtx);
		} else {
			mtx00015f04(scale, &posmtx);
		}

		{
			Mtxf sp244;
			Mtxf sp204;
			Mtxf sp1c4;
			Mtxf sp184;

			if (haszoom) {
				mtx4LoadTranslation(&zoompos, &sp204);
			} else {
				tmpcoord.x = menumodel->displacex;
				tmpcoord.y = menumodel->displacey;
				tmpcoord.z = menumodel->displacez;

				mtx4LoadTranslation(&tmpcoord, &sp204);
			}

			mtx4MultMtx4(&posmtx, &rotmtx, &sp244);

			if (modeltype == MENUMODELTYPE_3) {
				credits0f13ae04(&sp1c4);
				mtx4MultMtx4(&sp1c4, &sp244, &sp184);
				mtx4MultMtx4(&sp184, &sp204, &menumodel->mtx);
			} else {
				mtx4MultMtx4(&sp244, &sp204, &menumodel->mtx);
			}
		}

		gdl = menugfx0f0e2348(gdl);

		if (modeltype < MENUMODELTYPE_3) {
			if (modeltype != MENUMODELTYPE_DEFAULT) {
				gdl = func0f0d49c8(gdl);
				gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
			} else {
#ifdef PLATFORM_N64
				s32 x1 = g_MenuScissorX1;
				s32 x2 = g_MenuScissorX2;
#else
				s32 halfScreenWidth = SCREEN_WIDTH_LO >> 1;
				f32 scale = SCREEN_ASPECT / videoGetAspect();
				f32 width = (g_MenuScissorX2 - g_MenuScissorX1) * scale;
				f32 center = (g_MenuScissorX1 + g_MenuScissorX2) * 0.5f;
				center = ((center - halfScreenWidth) * scale) + halfScreenWidth;

				s32 x1 = (s32)(center - width * 0.5f);
				s32 x2 = (s32)(center + width * 0.5f);
#endif

				f32 aspect = (f32) (x2 - x1) / (f32) (g_MenuScissorY2 - g_MenuScissorY1);

				static u32 znear = 10;
				static u32 zfar = 300;

				mainOverrideVariable("mzn", &znear);
				mainOverrideVariable("mzf", &zfar);

				gdl = func0f0d49c8(gdl);

				viSetViewPosition(x1 * g_ScaleX, g_MenuScissorY1);
				viSetFovAspectAndSize(g_Vars.currentplayer->fovy, aspect, (x2 - x1) * g_ScaleX, g_MenuScissorY2 - g_MenuScissorY1);

				gdl = vi0000af00(gdl, var800a2048[g_MpPlayerNum]);
				gdl = vi0000aca4(gdl, znear, zfar);
			}
		}

		// Allocate model matrices
		matrices = gfxAllocate(menumodel->bodymodeldef->nummatrices * sizeof(Mtxf));

		for (i = 0; i < menumodel->bodymodeldef->nummatrices; i++) {
			mtx4LoadIdentity(&matrices[i]);
		}

		menumodel->bodymodel.matrices = matrices;

		// Set new animation if requested
		if (menumodel->newanimnum && menumodel->curanimnum != menumodel->newanimnum) {
			if (menumodel->reverseanim) {
				modelSetAnimation(&menumodel->bodymodel, menumodel->newanimnum, false, 0, PALUPF(-0.5f), 0.0f);
				modelSetAnimFrame(&menumodel->bodymodel, modelGetNumAnimFrames(&menumodel->bodymodel));
			} else {
				modelSetAnimation(&menumodel->bodymodel, menumodel->newanimnum, false, 0, PALUPF(0.5f), 0.0f);
			}

			menumodel->curanimnum = menumodel->newanimnum;
		}

		menumodel->newanimnum = 0;

		// Tick the animation, if any
		if (menumodel->curanimnum != 0) {
			f32 frame;
			u32 stack;

			modelTickAnimQuarterSpeed(&menumodel->bodymodel, g_Vars.diffframe240, true);

			if (menumodel->reverseanim) {
				frame = modelGetNumAnimFrames(&menumodel->bodymodel) - modelGetCurAnimFrame(&menumodel->bodymodel);
			} else {
				frame = modelGetCurAnimFrame(&menumodel->bodymodel);
			}

			if (frame >= modelGetNumAnimFrames(&menumodel->bodymodel) - 1) {
				menumodel->curanimnum = 0;
			}
		}

		mtx4Copy(&menumodel->mtx, matrices);

		renderdata.unk00 = &menumodel->mtx;
		renderdata.unk10 = menumodel->bodymodel.matrices;

		modelSetMatricesWithAnim(&renderdata, &menumodel->bodymodel);

		if (menumodel->bodymodeldef->skel == &g_SkelHudPiece) {
			// Update the hudpiece's liquid texture
			struct modelnode *node = modelGetPart(menumodel->bodymodeldef, MODELPART_HUDPIECE_0000);

			if (node) {
				struct modelrodata_gundl *rodata = &node->rodata->gundl;
				s32 i;
				s32 j;

				for (i = 0; i < rodata->numvertices; i++) {
#if VERSION >= VERSION_PAL_BETA
					rodata->vertices[i].s -= (s32) (100.0f * g_Vars.diffframe60freal);
#else
					rodata->vertices[i].s -= 100 * g_Vars.diffframe60;
#endif

					if (rodata->vertices[i].s < -0x6000) {
						for (j = 0; j < rodata->numvertices; j++) {
							rodata->vertices[j].s += 0x2000;
						}
					}
				}
			}

			// Rotate the hudpiece's rotor thing
			node = modelGetPart(menumodel->bodymodeldef, MODELPART_HUDPIECE_0002);

			if (node) {
				s32 mtxindex = modelFindNodeMtxIndex(node, 0);
				Mtxf sp120;
				Mtxf spe0;

				mtx4LoadIdentity(&sp120);
				mtx4LoadXRotation(menuGetCosOscFrac(4), &sp120);
				mtx4MultMtx4((Mtxf *)((uintptr_t)matrices + mtxindex * sizeof(Mtxf)), &sp120, &spe0);
				mtx4Copy(&spe0, (Mtxf *)((uintptr_t)matrices + mtxindex * sizeof(Mtxf)));
			}

			// Make the menu projection lines come from the hudpiece eye
			node = modelGetPart(menumodel->bodymodeldef, MODELPART_HUDPIECE_0001);

			if (node) {
				if (g_MenuData.root == MENUROOT_MAINMENU
						|| g_MenuData.root == MENUROOT_FILEMGR
						|| g_MenuData.root == MENUROOT_MPSETUP
						|| g_MenuData.root == MENUROOT_TRAINING) {
					s32 mtxindex = modelFindNodeMtxIndex(node, 0);
					struct coord pos;
					f32 screenpos[2];

					pos.x = matrices[mtxindex].m[3][0];
					pos.y = matrices[mtxindex].m[3][1];
					pos.z = matrices[mtxindex].m[3][2];

					cam0f0b4d04(&pos, screenpos);

					g_MenuProjectFromX = ((s32)screenpos[0] - viGetWidth() / 2) / g_ScaleX;
					g_MenuProjectFromY = (s32)screenpos[1] - viGetHeight() / 2;
				}
			}
		}

		gSPSetLights1(gdl++, var80071468);

#ifdef AVOID_UB
		// during the credits camGetLookAt() can return NULL
		if (camGetLookAt())
#endif
		gSPLookAt(gdl++, camGetLookAt());

		renderdata.unk30 = 1;
		renderdata.envcolour = 0xffffffff;
		renderdata.fogcolour = 0xffffffff;

		gSPSetGeometryMode(gdl++, G_ZBUFFER);

		renderdata.gdl = gdl;
		renderdata.zbufferenabled = true;

		modelRender(&renderdata, &menumodel->bodymodel);

		gdl = renderdata.gdl;

		mtx00016760();

		for (i = 0; i < menumodel->bodymodeldef->nummatrices; i++) {
			Mtxf sp70;
			mtx4Copy((Mtxf *)((uintptr_t)menumodel->bodymodel.matrices + i * sizeof(Mtxf)), &sp70);
			mtxF2L(&sp70, &menumodel->bodymodel.matrices[i]);
		}

		mtx00016784();

		if (modeltype < MENUMODELTYPE_3) {
			gdl = func0f0d479c(gdl);
		}

		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetCombineMode(gdl++, G_CC_MODULATEI, G_CC_MODULATEI);
		gSPClearGeometryMode(gdl++, G_CULL_BOTH);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);

		texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

		gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

		texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

		gSPDisplayList(gdl++, var800613a0);
	}

	return gdl;
}

void menuGetTeamTitlebarColours(u32 *top, u32 *middle, u32 *bottom)
{
	const u32 colours[][3] = {
		// top, middle, bottom
		{ 0xbf000000, 0x50000000, 0xff000000 },
		{ 0xbfbf0000, 0x50500000, 0xffff0000 },
		{ 0x0000bf00, 0x00005000, 0x0000ff00 },
		{ 0xbf00bf00, 0x50005000, 0xff00ff00 },
		{ 0x00bfbf00, 0x00505000, 0x00ffff00 },
		{ 0xff885500, 0x7f482000, 0xff885500 },
		{ 0xff888800, 0x7f484800, 0xff888800 },
		{ 0x88445500, 0x48242000, 0x88445500 },
	};

	*top = colours[g_PlayerConfigsArray[g_MpPlayerNum].base.team][0] | (*top & 0xff);
	*middle = colours[g_PlayerConfigsArray[g_MpPlayerNum].base.team][1] | (*middle & 0xff);
	*bottom = colours[g_PlayerConfigsArray[g_MpPlayerNum].base.team][2] | (*bottom & 0xff);
}

Gfx *menuApplyScissor(Gfx *gdl)
{
	gDPPipeSync(gdl++);

#if VERSION >= VERSION_NTSC_1_0
	g_ScissorX1 = g_MenuScissorX1 * g_ScaleX;
	g_ScissorX2 = g_MenuScissorX2 * g_ScaleX;
	g_ScissorY1 = g_MenuScissorY1;
	g_ScissorY2 = g_MenuScissorY2;

	if (g_ScissorX1 < 0) {
		g_ScissorX1 = 0;
	}

	if (g_ScissorX2 < 0) {
		g_ScissorX2 = 0;
	}

	if (g_ScissorY1 < 0) {
		g_ScissorY1 = 0;
	}

	if (g_ScissorY2 < 0) {
		g_ScissorY2 = 0;
	}

	if (g_ScissorX1 > viGetBufWidth()) {
		g_ScissorX1 = viGetBufWidth();
	}

	if (g_ScissorX2 > viGetBufWidth()) {
		g_ScissorX2 = viGetBufWidth();
	}

	if (g_ScissorY1 > viGetBufHeight()) {
		g_ScissorY1 = viGetBufHeight();
	}

	if (g_ScissorY2 > viGetBufHeight()) {
		g_ScissorY2 = viGetBufHeight();
	}

	if (g_ScissorX2 < g_ScissorX1) {
		g_ScissorX2 = g_ScissorX1;
	}

	if (g_ScissorY2 < g_ScissorY1) {
		g_ScissorY2 = g_ScissorY1;
	}

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE,
			g_MenuScissorX1 * g_ScaleX, g_MenuScissorY1,
			g_MenuScissorX2 * g_ScaleX, g_MenuScissorY2);
#endif

	return gdl;
}

/**
 * Render a single menu dialog.
 *
 * The lightweight argument is always false. If set to true, a lighter-weight
 * variant of the dialog is rendered which has no borders, less background,
 * no overlays and no models such as inventory weapons.
 */
Gfx *dialogRender(Gfx *gdl, struct menudialog *dialog, struct menu *menu, bool lightweight)
{
	s32 i;
	s32 dialogleft;
	s32 dialogtop;
	s32 dialogright;
	s32 dialogbottom;
	s32 x;
	s32 y;
	s16 dialogwidth;
	union menuitemdata *itemdata;
	s32 j;
	u32 colour1;
	u32 colour2;
	u32 colour3;
	struct menurendercontext context;
	s32 curx;
	s32 bgx1;
	s32 bgy1;
	s32 bgx2;
	s32 bgy2;
	s16 dialogheight;
	char *title;
	u32 colour4;
	u32 colour5;
	f32 sp170;

	bgx1 = dialog->x;
	bgy1 = dialog->y;
	bgx2 = dialog->x + dialog->width;
	bgy2 = dialog->y + dialog->height;

#if VERSION >= VERSION_NTSC_1_0
	if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
			&& menuGetRoot() == MENUROOT_MPENDSCREEN
			&& !var8009dfc0) {
		return gdl;
	}
#endif

	colour1 = MIXCOLOUR(dialog, item_focused_outer);

	text0f156030(colour1);

	var8007fb9c = false;

	if (g_Menus[g_MpPlayerNum].curdialog == dialog
			&& (dialog->definition->flags & MENUDIALOGFLAG_0002)
			&& !lightweight
			&& g_Menus[g_MpPlayerNum].menumodel.drawbehinddialog == true) {
		gSPSetGeometryMode(gdl++, G_ZBUFFER);
		gdl = menuRenderModel(gdl, &g_Menus[g_MpPlayerNum].menumodel, MENUMODELTYPE_2);
		gSPClearGeometryMode(gdl++, G_ZBUFFER);
	}

	dialogwidth = dialog->width;
	dialogheight = dialog->height;

	if (dialog->state == MENUDIALOGSTATE_PREOPEN) {
#if VERSION >= VERSION_NTSC_1_0
		if (dialog->definition == &g_MpReadyMenuDialog) {
			return gdl;
		}
#endif

		sp170 = 1.0f - g_MenuData.unk010;

#if VERSION >= VERSION_NTSC_1_0
		if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && menuGetRoot() == MENUROOT_MPENDSCREEN) {
			sp170 = 1.0f - dialog->statefrac;
		}
#endif

		sp170 = 1.0f - sp170 * sp170;
		dialogheight *= sp170;
		bgy2 = dialog->y + dialogheight;
	}

	dialogleft = dialog->x;
	dialogtop = dialog->y;
	dialogright = dialogleft + dialogwidth;
	dialogbottom = dialogtop + dialogheight;

	title = menuResolveDialogTitle(dialog->definition);

	colour1 = MIXCOLOUR(dialog, dialog_border1);
	colour2 = MIXCOLOUR(dialog, dialog_titlebg);
	colour3 = MIXCOLOUR(dialog, dialog_border2);

	gSPClearGeometryMode(gdl++, G_ZBUFFER);

	colour4 = colour1;
	colour5 = colour3;

	if ((colour4 & 0xff) > 0x3f) {
		colour4 = (colour4 & 0xffffff00) | 0x3f;
	}

	if ((colour5 & 0xff) > 0x3f) {
		colour5 = (colour5 & 0xffffff00) | 0x3f;
	}

	var8009de90 = -1000;
	var8009de94 = 1000;

	if (dialog->definition->flags & MENUDIALOGFLAG_DISABLETITLEBAR) {
		bgy1 += LINEHEIGHT;
	}

	// Render the walls/floor/ceiling coming from the projection source.
	// Each surface is rendered a second time with the colours swapped.
	// The order is top, right, bottom, left.
	if (g_MenuData.root != MENUROOT_MPSETUP && (g_MenuData.root != MENUROOT_MPPAUSE || g_Vars.normmplayerisrunning)) {
		var800a4634 = menugfxDrawPlane(var800a4634, bgx1, bgy1, bgx2, bgy1, colour4, colour5, MENUPLANE_00);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx2, bgy1, bgx2, bgy2, colour5, colour4, MENUPLANE_00);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx2, bgy2, bgx1, bgy2, colour4, colour5, MENUPLANE_00);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx1, bgy2, bgx1, bgy1, colour5, colour4, MENUPLANE_00);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx1, bgy1, bgx2, bgy1, colour5, colour4, MENUPLANE_01);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx2, bgy1, bgx2, bgy2, colour4, colour5, MENUPLANE_01);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx2, bgy2, bgx1, bgy2, colour5, colour4, MENUPLANE_01);
		var800a4634 = menugfxDrawPlane(var800a4634, bgx1, bgy2, bgx1, bgy1, colour4, colour5, MENUPLANE_01);
	}

	// Render the title bar
	if ((dialog->definition->flags & MENUDIALOGFLAG_DISABLETITLEBAR) == 0) {
		if (((g_MenuData.root == MENUROOT_MPSETUP) || (g_MenuData.root == MENUROOT_4MBMAINMENU))
				&& (g_MpSetup.options & MPOPTION_TEAMSENABLED)
				&& g_Vars.mpsetupmenu != MPSETUPMENU_GENERAL) {
			menuGetTeamTitlebarColours(&colour1, &colour2, &colour3);
		}

		gdl = menugfxRenderGradient(gdl, dialogleft - 2, dialogtop, dialogright + 2, dialogtop + LINEHEIGHT, colour1, colour2, colour3);
		gdl = menugfxDrawShimmer(gdl, dialogleft - 2, dialogtop, dialogright + 2, dialogtop + 1, (colour1 & 0xff) >> 1, 1, 40, 0);
		gdl = menugfxDrawShimmer(gdl, dialogleft - 2, dialogtop + 10, dialogright + 2, dialogtop + LINEHEIGHT, (colour1 & 0xff) >> 1, 0, 40, 1);

		x = dialogleft + 2;
		y = dialogtop + 2;

		gdl = text0f153628(gdl);

		context.unk18 = false;

		if (lightweight) {
			context.unk18 = true;
		}

		{
			char *sp154[] = {
				"1\n",
				"2\n",
				"3\n",
				"4\n",
			};

			colour1 = MIXCOLOUR(dialog, dialog_titlefg);

			textSetWaveColours(g_MenuWave2Colours[dialog->type].dialog_titlefg, g_MenuWave1Colours[dialog->type].dialog_titlefg);

			// Title shadow
			x = dialogleft + 3;
			y = dialogtop + 3;

			gdl = textRenderProjected(gdl, &x, &y, title, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour1 & 0xff, dialogwidth, viGetHeight(), 0, 0);

			// Title proper
			x = dialogleft + 2;
			y = dialogtop + 2;

			gdl = textRenderProjected(gdl, &x, &y, title, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour1, dialogwidth, viGetHeight(), 0, 0);

			// In MP dialogs, render the player number in the top right
			if (g_MenuData.root == MENUROOT_MPSETUP
					|| g_MenuData.root == MENUROOT_MPPAUSE
					|| g_MenuData.root == MENUROOT_MPENDSCREEN
					|| g_MenuData.root == MENUROOT_4MBMAINMENU) {
				x = dialogright - 9;
				y = dialogtop + 2;

				gdl = textRenderProjected(gdl, &x, &y, sp154[g_MpPlayerNum], g_CharsHandelGothicSm, g_FontHandelGothicSm, colour1, dialogwidth, viGetHeight(), 0, 0);
			}
		}

		gdl = text0f153780(gdl);
	}

	// Configure things for the redraw effect
	if (!(dialog->redrawtimer < 0.0f)) {
		if (g_MenuData.root != MENUROOT_MPPAUSE) {
			if (dialog->state >= MENUDIALOGSTATE_POPULATED) {
				textSetDiagonalBlend(dialog->x, dialog->y, dialog->redrawtimer, DIAGMODE_REDRAW);
			} else {
				textSetDiagonalBlend(dialog->x, dialog->y, dialog->redrawtimer, DIAGMODE_FADEIN);
			}

			var8007fb9c = true;
		}
	} else if (dialog->state == MENUDIALOGSTATE_POPULATED) {
		textSetMenuBlend(dialog->statefrac);
	}

	if (dialogbottom < dialogtop + LINEHEIGHT) {
		dialogbottom = dialogtop + LINEHEIGHT;
	}

	colour1 = MIXCOLOUR(dialog, dialog_bodybg);

	if (dialog->dimmed) {
		colour1 = (colourBlend(colour1, 0x00000000, 44) & 0xffffff00) | (colour1 & 0xff);
	}

	colour2 = MIXCOLOUR(dialog, unused14);

	// Draw the dialog's background and outer borders
	if (!lightweight) {
		if (dialog->state == MENUDIALOGSTATE_OPENING) {
			gdl = menugfxRenderDialogBackground(gdl, dialogleft + 1, dialogtop + LINEHEIGHT, dialogright - 1, dialogbottom, dialog, colour1, colour2, 1.0f);
		} else if (dialog->state == MENUDIALOGSTATE_POPULATING) {
			gdl = menugfxRenderDialogBackground(gdl, dialogleft + 1, dialogtop + LINEHEIGHT, dialogright - 1, dialogbottom, dialog, colour1, colour2, dialog->statefrac);
		} else {
			gdl = menugfxRenderDialogBackground(gdl, dialogleft + 1, dialogtop + LINEHEIGHT, dialogright - 1, dialogbottom, dialog, colour1, colour2, -1.0f);
		}

		// No dialog has this flag, so this branch is unused
		if (dialog->definition->flags & MENUDIALOGFLAG_DISABLETITLEBAR) {
			gdl = menugfxDrawDialogBorderLine(gdl, dialogleft + 1, dialogtop + LINEHEIGHT, dialogright - 1, dialogtop + LINEHEIGHT + 1, MIXCOLOUR(dialog, dialog_border1), MIXCOLOUR(dialog, dialog_border2));
		}
	}

	if (dialog->state == MENUDIALOGSTATE_PREOPEN) {
		return gdl;
	}

	{
		struct menulayer *layer;
		s32 viewleft = viGetViewLeft() / g_ScaleX;
		s32 viewtop = viGetViewTop();
		s32 viewright = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX;
		s32 viewbottom = viGetViewTop() + viGetViewHeight();

		g_MenuScissorX1 = dialogleft + 2;
		g_MenuScissorX2 = dialogright - 2;
		g_MenuScissorY1 = dialogtop + LINEHEIGHT;
		g_MenuScissorY2 = dialogbottom - 1;

		if (g_MenuScissorX2 < viewleft) {
			return gdl;
		}

		if (g_MenuScissorY2 < viewtop) {
			return gdl;
		}

		if (g_MenuScissorX1 > viewright) {
			return gdl;
		}

		if (g_MenuScissorY1 > viewbottom) {
			return gdl;
		}

		if (g_MenuScissorX2 > viewright) {
			g_MenuScissorX2 = viewright;
		}

		if (g_MenuScissorY2 > viewbottom) {
			g_MenuScissorY2 = viewbottom;
		}

		if (g_MenuScissorX1 < viewleft) {
			g_MenuScissorX1 = viewleft;
		}

		// @bug: This should be g_MenuScissorY1
		// but the condition can't pass anyway.
		if (g_MenuScissorX2 < viewtop) {
			g_MenuScissorX2 = viewtop;
		}

		var8009de90 = g_MenuScissorY1;
		var8009de94 = g_MenuScissorY2;

		gdl = menuApplyScissor(gdl);

		// Render models (inventory, chr/vehicle bios)
		if (g_Menus[g_MpPlayerNum].curdialog == dialog
				&& (dialog->definition->flags & MENUDIALOGFLAG_0002)
				&& !lightweight
				&& !g_Menus[g_MpPlayerNum].menumodel.drawbehinddialog) {
			gSPSetGeometryMode(gdl++, G_ZBUFFER);

			gdl = menuRenderModel(gdl, &g_Menus[g_MpPlayerNum].menumodel, MENUMODELTYPE_DEFAULT);

			gSPClearGeometryMode(gdl++, G_ZBUFFER);

			viSetViewPosition(g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop);
			viSetFovAspectAndSize(g_Vars.currentplayer->fovy, g_Vars.currentplayer->aspect,
					g_Vars.currentplayer->viewwidth, g_Vars.currentplayer->viewheight);
		}

		// Render menu items
		if (dialog->type != 0 || dialog->transitionfrac >= 0.0f) {
			u32 stack;
			s32 sumwidth;
			s32 cury;
			bool prevwaslist;
			s32 colwidth;
			u32 sp120;
			bool offscreen;
			struct menuitem *item;
			s32 focused;
			s32 colindex;
			s32 rowindex;

			sumwidth = 0;
			curx = dialogleft;

			for (i = 0; i < dialog->numcols; i++) {
				cury = dialogtop + LINEHEIGHT + 1 + dialog->scroll;
				prevwaslist = false;

				sp120 = MIXCOLOUR(dialog, item_unfocused);
				sp120 = (sp120 & 0xffffff00) | 0x3f;

				colindex = dialog->colstart + i;

				if (i != 0 && (dialog->definition->flags & MENUDIALOGFLAG_0400) == 0) {
					gdl = menugfxDrawFilledRect(gdl, curx - 1, dialogtop + LINEHEIGHT + 1, curx, dialogbottom, sp120, sp120);
				}

				colwidth = menu->cols[colindex].width;
				sumwidth += colwidth;

				if (i == dialog->numcols - 1) {
					s32 v0 = (dialogright - dialogleft) - 2;

					if (sumwidth < v0) {
						colwidth = (colwidth + v0) - sumwidth;
					}
				}

				for (j = 0; j < menu->cols[colindex].numrows; j++) {
					focused = 0;
					rowindex = menu->cols[colindex].rowstart + j;
					item = &dialog->definition->items[menu->rows[rowindex].itemindex];
					itemdata = NULL;
					offscreen = false;

					if (item == dialog->focuseditem) {
						focused = 1;

						if (dialog->dimmed) {
							focused = 3;
						}
					}

					if (menu->rows[rowindex].blockindex != -1) {
						itemdata = (union menuitemdata *)&menu->blocks[menu->rows[rowindex].blockindex];
					}

					context.x = curx;
					context.y = cury;
					context.width = colwidth;
					context.height = menu->rows[rowindex].height;

					if (context.y + context.height < dialogtop + LINEHEIGHT + 1) {
						offscreen = true;
					}

					if (context.y > dialogbottom) {
						offscreen = true;
					}

					if (context.height == 0) {
						offscreen = true;
					}

					if (!offscreen) {
						context.item = item;
						context.data = itemdata;
						context.focused = focused;
						context.dialog = dialog;

						if (prevwaslist) {
							gdl = menugfxDrawFilledRect(gdl, context.x, context.y - 1, context.x + context.width, context.y, sp120, sp120);
							prevwaslist = false;
						}

						if ((item->flags & MENUITEMFLAG_DARKERBG) && !lightweight) {
							// Render a darker background behind the item
							s32 x1 = context.x;
							s32 y1 = context.y;
							s32 x2 = x1 + context.width;
							s32 y2 = y1 + context.height;
							u32 colour;
							u32 colour2;

							colour2 = MIXCOLOUR(dialog, item_focused_outer);
							colour = colourBlend(colour2, colour2 & 0xffffff00, 127);

							gdl = textSetPrimColour(gdl, colour);
							gDPFillRectangleScaled(gdl++, x1, y1, x2, y2);
							gdl = text0f153838(gdl);
						}

						if (focused) {
							if (item->type == MENUITEMTYPE_03
									|| item->type == MENUITEMTYPE_SELECTABLE
									|| item->type == MENUITEMTYPE_CHECKBOX
									|| item->type == MENUITEMTYPE_0A
									|| item->type == MENUITEMTYPE_SLIDER
									|| item->type == MENUITEMTYPE_DROPDOWN) {
#if VERSION >= VERSION_NTSC_1_0
								if (!(dialog->transitionfrac >= 0.0f && dialog->type2 == 0)
										&& !(dialog->transitionfrac < 0.0f && dialog->type == 0)) {
									text0f156024(1);
								}
#else
								text0f156024(1);
#endif
							}

							// Render the horizontal line behind the focused item
							if (item->type == MENUITEMTYPE_SELECTABLE
									|| item->type == MENUITEMTYPE_CHECKBOX
									|| item->type == MENUITEMTYPE_0A
									|| item->type == MENUITEMTYPE_DROPDOWN) {
								s32 liney = context.y + context.height / 2 - 1;
								s32 x1 = context.x;
								s32 x3 = context.x + 8;
								s32 x4 = context.x + context.width / 3;
								u32 colour = (sp120 & 0xffffff00) | 0x2f;

								// Left side
								gdl = menugfx0f0e2498(gdl);
								gdl = menugfxDrawTri2(gdl, x1, liney - 1, x3 - 3, liney, sp120, sp120, 0);
								gdl = menugfxDrawTri2(gdl, x3 - 3, liney - 1, x3, liney, sp120, 0xffffffff, 0);
								gdl = menugfxDrawTri2(gdl, x1, liney + 1, x3 - 3, liney + 2, sp120, sp120, 0);
								gdl = menugfxDrawTri2(gdl, x3 - 3, liney + 1, x3, liney + 2, sp120, 0xffffffff, 0);
								gdl = menugfxDrawTri2(gdl, x3 - 2, liney, x4, liney + 1, colour, sp120 & 0xffffff00, 0);

								if (item->flags & MENUITEMFLAG_SELECTABLE_CENTRE) {
									// Right side
									x1 = context.x + context.width;
									x3 = context.x + context.width - 8;
									x4 = context.x + context.width - context.width / 3;

									gdl = menugfxDrawTri2(gdl, x1 - 5, liney - 1, x1, liney, sp120, sp120, 0);
									gdl = menugfxDrawTri2(gdl, x3, liney - 1, x3 + 3, liney, -1, sp120, 0);
									gdl = menugfxDrawTri2(gdl, x3 + 3, liney + 1, x1, liney + 2, sp120, sp120, 0);
									gdl = menugfxDrawTri2(gdl, x3, liney + 1, x3 + 3, liney + 2, -1, sp120, 0);
									gdl = menugfxDrawTri2(gdl, x4, liney, x3 + 2, liney + 1, sp120 & 0xffffff00, colour, 0);
								}
							}
						}

						gdl = menuitemRender(gdl, &context);

						if (item->type == MENUITEMTYPE_LIST) {
							prevwaslist = true;
						}

						if (focused) {
							text0f156024(0);
						}
					}

					cury += menu->rows[rowindex].height;
				}

				curx += menu->cols[colindex].width;
			}

			// Render overlays, such as dropdown menus
			if (!lightweight) {
				gdl = textSetPrimColour(gdl, 0x00000000);

				curx = dialogleft;

				for (i = 0; i < dialog->numcols; i++) {
#if VERSION >= VERSION_NTSC_1_0
					s32 cury = dialogtop + LINEHEIGHT + 1 + dialog->scroll;
#else
					s32 cury = dialogtop + LINEHEIGHT + 1;
#endif
					colindex = dialog->colstart + i;

					for (j = 0; j < menu->cols[colindex].numrows; j++) {
						union menuitemdata *itemdata;
						struct menuitem *item;

						rowindex = menu->cols[colindex].rowstart + j;
						itemdata = NULL;
						item = &dialog->definition->items[menu->rows[rowindex].itemindex];

						if (menu->rows[rowindex].blockindex != -1) {
							itemdata = (union menuitemdata *)&menu->blocks[menu->rows[rowindex].blockindex];
						}

						gdl = menuitemOverlay(gdl, curx, cury, menu->cols[colindex].width, menu->rows[rowindex].height, item, dialog, itemdata);

						cury += menu->rows[rowindex].height;
					}

					curx += menu->cols[colindex].width;
				}

				gdl = text0f153838(gdl);
			}

			gDPSetScissor(gdl++, G_SC_NON_INTERLACE, viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());
		} else {
			gDPSetScissor(gdl++, G_SC_NON_INTERLACE, viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());
		}

		// Render left/right chevrons and sibling dialog titles
		layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1];

		if ((dialog->type != 0 || dialog->transitionfrac >= 0.0f)
				&& layer->siblings[layer->cursibling] == dialog
				&& layer->numsiblings >= 2) {
			// Draw chevrons
			u32 colour1;
			u32 colour;
			u32 weight = menuGetSinOscFrac(10) * 255.0f;

			colour1 = MIXCOLOUR(dialog, dialog_border1);
			colour = colourBlend(0xffffffff, colour1, weight);

			gdl = menugfxDrawDialogChevron(gdl, dialogleft - 5, (dialogtop + dialogbottom) / 2, 9, 1, colour, colour, menuGetSinOscFrac(20));
			gdl = menugfxDrawDialogChevron(gdl, dialogright + 5, (dialogtop + dialogbottom) / 2, 9, 3, colour, colour, menuGetSinOscFrac(20));

			if (g_MenuData.root == MENUROOT_MAINMENU
					|| g_MenuData.root == MENUROOT_4MBFILEMGR
					|| g_MenuData.root == MENUROOT_TRAINING
					|| g_MenuData.root == MENUROOT_FILEMGR) {
				char *title;
				s32 textheight;
				s32 textwidth;
				s32 previndex;
				s32 nextindex;

				textResetBlends();
				textSetRotation90(true);

				gdl = text0f153628(gdl);

				// Left/previous title
				previndex = layer->cursibling - 1;

				if (previndex < 0) {
					previndex = layer->numsiblings - 1;
				}

				title = menuResolveDialogTitle(layer->siblings[previndex]->definition);

				textMeasure(&textheight, &textwidth, title, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

				x = dialogleft - 2;
				y = (dialogtop + dialogbottom) / 2 - textwidth - 3;

				if (y < dialogtop) {
					y = (dialogtop + dialogbottom - textwidth) / 2;
					x -= 3;
				}

				gdl = textRenderProjected(gdl, &y, &x, title, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffffff, dialogwidth, viGetHeight(), 0, 0);

				// Right/next title
				nextindex = layer->cursibling + 1;

				if (nextindex >= layer->numsiblings) {
					nextindex = 0;
				}

				title = menuResolveDialogTitle(layer->siblings[nextindex]->definition);

				textMeasure(&textheight, &textwidth, title, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

#if VERSION == VERSION_JPN_FINAL
				x = dialogright + 13;
#else
				x = dialogright + 7;
#endif
				y = (dialogtop + dialogbottom) / 2 + 3;

				if (y + textwidth > dialogbottom) {
					y = (dialogtop + dialogbottom - textwidth) / 2;
					x += 3;
				}

				gdl = textRenderProjected(gdl, &y, &x, title, g_CharsHandelGothicXs, g_FontHandelGothicXs, -1, dialogwidth, viGetHeight(), 0, 0);
				gdl = text0f153780(gdl);

				textSetRotation90(false);
			}
		}
	}

	return gdl;
}

const char var7f1b2668[] = "[]-[] Terminate Complete\n";
const char var7f1b2684[] = "Enabling control %d\n";
const char var7f1b269c[] = "NOT IN MODE MULTIINGAME!\n";
const char var7f1b26b8[] = "Numactive now:%d\n";
const char var7f1b26cc[] = "[]-[] SwitchMenuMode called, context %d\n";

void menuGetContPads(s8 *contpadnum1, s8 *contpadnum2)
{
	switch (g_MenuData.root) {
	case MENUROOT_MPSETUP:
	case MENUROOT_FILEMGR:
	case MENUROOT_BOOTPAKMGR:
	case MENUROOT_4MBFILEMGR:
	case MENUROOT_4MBMAINMENU:
		*contpadnum1 = g_MpPlayerNum;
		*contpadnum2 = -1;
		break;
	default:
		*contpadnum1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
		*contpadnum2 = -1;

		if (!g_Vars.normmplayerisrunning) {
			s32 mode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);

			if (mode == CONTROLMODE_23 || mode == CONTROLMODE_24 || mode == CONTROLMODE_22 || mode == CONTROLMODE_21) {
				*contpadnum2 = optionsGetContpadNum2(g_Vars.currentplayerstats->mpindex);
			}
		}
		break;
	}
}

u32 g_MpNumJoined = 1;

/**
 * Choose which direction a new dialog should swipe from in the combat simulator
 * menus.
 */
void func0f0f7594(s32 arg0, s32 *vdir, s32 *hdir)
{
	if (g_MenuData.root == MENUROOT_MPSETUP) {
		s32 playernum = g_Menus[g_MpPlayerNum].playernum;

		*vdir = 0;
		*hdir = 0;

		switch (g_MpNumJoined) {
		case 1:
			*hdir = arg0;
			break;
		case 2:
			if (playernum == 0) {
				if (arg0 < 0) {
					*hdir = -1;
				} else {
					*vdir = -1;
				}
			} else {
				if (arg0 > 0) {
					*hdir = 1;
				} else {
					*vdir = 1;
				}
			}
			break;
		case 3:
			if (playernum == 2) {
				*hdir = arg0;
			} else if (playernum == 0) {
				if (arg0 < 0) {
					*hdir = -1;
				} else {
					*vdir = -1;
				}
			} else {
				if (arg0 > 0) {
					*hdir = 1;
				} else {
					*vdir = -1;
				}
			}
			break;
		case 4:
			if (playernum == 0 || playernum == 2) {
				if (arg0 < 0) {
					*hdir = -1;
				} else {
					*vdir = playernum == 0 ? -1 : 1;
				}
			} else {
				if (arg0 > 0) {
					*hdir = 1;
				} else {
					*vdir = playernum == 1 ? -1 : 1;
				}
			}
			break;
		}
	} else {
		*vdir = 0;
		*hdir = arg0;
	}
}

/**
 * Find the maximum possible dimensions for a dialog.
 *
 * This is pretty much the player's viewport with some adjustments made for
 * padding. There are some trickier calculations for MP setup where the players
 * are sharing a viewport.
 */
#if VERSION >= VERSION_JPN_FINAL
void menuFindAvailableSize(s32 *leftptr, s32 *topptr, s32 *rightptr, s32 *bottomptr, struct menudialog *dialog)
#else
void menuFindAvailableSize(s32 *leftptr, s32 *topptr, s32 *rightptr, s32 *bottomptr)
#endif
{
	s32 left = viGetViewLeft() / g_ScaleX + 20;
	s32 top = viGetViewTop() + 4;
	s32 right = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX - 20;
	s32 bottom = viGetViewTop() + viGetViewHeight() - 4;
	s32 playernum;
	u32 stack1;
#if VERSION >= VERSION_JPN_FINAL
	bool v1 = false;
#endif
	u32 stack2;

#if VERSION >= VERSION_JPN_FINAL
	if (currentPlayerIsMenuOpenInSoloOrMp()) {
		v1 = true;
	}

	if (dialog) {
		if (dialog->definition == &g_CiControlStylePlayer2MenuDialog
				|| dialog->definition == &g_CiControlStyleMenuDialog
				|| dialog->definition == &g_SoloMissionControlStyleMenuDialog) {
			v1 = false;
		}
	}

	if (v1) {
		// Make room for health bar
		top += 22;

		if (optionsGetEffectiveScreenSize() == SCREENSIZE_CINEMA) {
			top -= 8;
			bottom += 4;
		}
	}
#else
	if (currentPlayerIsMenuOpenInSoloOrMp()) {
		// Make room for health bar
		top += 22;

		if (optionsGetEffectiveScreenSize() == SCREENSIZE_CINEMA) {
			top -= 8;
			bottom += 4;
		}
	}
#endif

	switch (g_MenuData.root) {
	case MENUROOT_MPSETUP:
	case MENUROOT_4MBMAINMENU:
		playernum = g_Menus[g_MpPlayerNum].playernum;

		// Make room for the "Press START" labels
		if (g_MenuData.playerjoinalpha[0] > 0 || g_MenuData.playerjoinalpha[1] > 0) {
			top += 10;
		}

		if (g_MenuData.playerjoinalpha[2] > 0 || g_MenuData.playerjoinalpha[3] > 0) {
			bottom -= 10;
		}

		switch (g_MpNumJoined) {
		case 1:
			// 1 player - the full screen is available
			*leftptr = left;
			*rightptr = right;
			*topptr = top;
			*bottomptr = bottom;
			return;
		case 2:
			// 2 players - left/right halves each
			*topptr = top;
			*bottomptr = bottom;

			if (playernum == 0) {
				*leftptr = left;
				*rightptr = (left + right) / 2;
			} else {
				*leftptr = (left + right) / 2;
				*rightptr = right;
			}
			return;
		case 3:
			// 3 players - top two quarters and the third takes the full bottom
			if (playernum == 0 || playernum == 1) {
				*topptr = top;
				*bottomptr = (top + bottom) / 2;

				if (playernum == 0) {
					*leftptr = left;
					*rightptr = (left + right) / 2;
				} else {
					*leftptr = (left + right) / 2;
					*rightptr = right;
				}
			} else {
				*topptr = (top + bottom) / 2;
				*bottomptr = bottom;
				*leftptr = left;
				*rightptr = right;
			}
			break;
		case 4:
			// 4 players - quarters
			if (playernum == 0 || playernum == 2) {
				*leftptr = left;
				*rightptr = (left + right) / 2;
			} else {
				*leftptr = (left + right) / 2;
				*rightptr = right;
			}
			if (playernum == 0 || playernum == 1) {
				*topptr = top;
				*bottomptr = (top + bottom) / 2;
			} else {
				*topptr = (top + bottom) / 2;
				*bottomptr = bottom;
			}
			break;
		}
		break;
	case MENUROOT_MPPAUSE:
	case MENUROOT_MPENDSCREEN:
	case MENUROOT_PICKTARGET:
	case MENUROOT_4MBFILEMGR:
		*leftptr = g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewleft / g_ScaleX;
		*topptr = g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewtop;
		*rightptr = (g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewleft + g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewwidth) / g_ScaleX;
		*bottomptr = g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewtop + g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->viewheight;

		if (PLAYERCOUNT() > 2) {
			if (g_Menus[g_MpPlayerNum].playernum == 0 || g_Menus[g_MpPlayerNum].playernum == 2) {
				*leftptr += 22;
			} else {
				*rightptr -= 22;
			}
		}

#if VERSION >= VERSION_NTSC_1_0
		if (PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || IS4MB()))
#else
		if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL)
#endif
		{
			if (g_Menus[g_MpPlayerNum].playernum == 0) {
				*leftptr += 22;
			} else {
				*rightptr -= 22;
			}
		}
		break;
	default:
		*leftptr = left;
		*topptr = top;
		*rightptr = right;
		*bottomptr = bottom;
		break;
	}
}

void dialogCalculatePosition(struct menudialog *dialog)
{
	s32 xmin;
	s32 xmax;
	s32 ymin;
	s32 ymax;
	s32 width;
	s32 height;
	s32 vdir;
	s32 hdir;

#if VERSION >= VERSION_JPN_FINAL
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax, dialog);
#else
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax);
#endif

	height = ymax - ymin - 6;
	width = xmax - xmin - 6;

	if (width > dialog->contentwidth) {
		width = dialog->contentwidth;
	}

	if (height > dialog->contentheight) {
		height = dialog->contentheight;
	}

	dialog->dstx = (xmax + xmin - width) / 2;
	dialog->dsty = (ymin + ymax - height) / 2;
	dialog->dstwidth = width;
	dialog->dstheight = height;

	if (dialog->swipedir != 0) {
		func0f0f7594(dialog->swipedir, &vdir, &hdir);

		if (hdir < 0) {
			dialog->dstx = -4 - dialog->dstwidth;
		}

		if (hdir > 0) {
			dialog->dstx = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX + 4;
		}

		if (vdir < 0) {
			dialog->dsty = -4 - dialog->dstheight;
		}

		if (vdir > 0) {
			dialog->dsty = viGetViewTop() + viGetViewHeight() + 4;
		}
	}
}

void menuClose(void)
{
	g_Menus[g_MpPlayerNum].depth = 0;
	g_Menus[g_MpPlayerNum].numdialogs = 0;
	g_Menus[g_MpPlayerNum].rowend = 0;
	g_Menus[g_MpPlayerNum].colend = 0;
	g_Menus[g_MpPlayerNum].blockend = 0;
	g_Menus[g_MpPlayerNum].curdialog = NULL;
	g_Menus[g_MpPlayerNum].openinhibit = 10;

#ifndef PLATFORM_N64
	inputAutoLockMouse(true);
#endif

	if (g_MenuData.root == MENUROOT_MPPAUSE) {
		g_PlayersWithControl[g_Menus[g_MpPlayerNum].playernum] = true;
	}

	g_MenuData.count--;

	if (g_MenuData.root == MENUROOT_MPPAUSE && g_Vars.currentplayer->activemenumode == AMMODE_EDIT) {
		g_Vars.currentplayer->activemenumode = AMMODE_VIEW;
	}

	if (g_MenuData.root == MENUROOT_BOOTPAKMGR) {
		mainChangeToStage(STAGE_TITLE);
	}
}

void func0f0f8120(void)
{
#ifdef AVOID_UB
	u32 mpindex = g_MpPlayerNum % MAX_PLAYERS;
	struct menudialog *prev = g_Menus[mpindex].curdialog;
	s32 i;
#else
	struct menudialog *prev = g_Menus[g_MpPlayerNum].curdialog;
	s32 i;
#endif

	if (g_MenuData.unk66e > 0) {
		for (i = g_MenuData.unk66e; i >= 0; i--) {
			func0f0f3220(i);
		}
	}

#ifdef AVOID_UB
	mpindex = g_MpPlayerNum;
	if (mpindex >= MAX_PLAYERS)
		mpindex -= MAX_PLAYERS;
	if (g_Menus[mpindex].curdialog == prev) {
		while (g_Menus[mpindex].depth > 0) {
			menuPopDialog();
		}
	}
#else
	if (g_Menus[g_MpPlayerNum].curdialog == prev) {
		while (g_Menus[g_MpPlayerNum].depth > 0) {
			menuPopDialog();
		}
	}
#endif
}

void func0f0f820c(struct menudialogdef *dialogdef, s32 root)
{
	s32 i;
	s32 prevplayernum = g_MpPlayerNum;

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		if (g_Menus[i].curdialog) {
			g_MpPlayerNum = i;
			func0f0f8120();
		}
	}

	g_MpPlayerNum = prevplayernum;

	g_MenuData.prevmenuroot = root;
	g_MenuData.prevmenudialog = dialogdef;
}

void menuSetBackground(s32 bg)
{
	// Can only screenshot if there is no background already,
	// because we want a clean screenshot
	bool screenshot = g_MenuData.bg == 0;

	// If there's already a screenshottable background queued,
	// there's no need to create another
	if (g_MenuData.nextbg == MENUBG_BLUR || g_MenuData.nextbg == MENUBG_CONEALPHA) {
		screenshot = false;
	}

	if (g_MenuData.bg != bg) {
		g_MenuData.nextbg = bg;
	}

	if (screenshot && g_MenuData.bg == 0) {
		g_MenuData.screenshottimer = 1;
	}
}

void func0f0f8300(void)
{
	s32 i;

	if (g_MenuData.count == 0) {
		for (i = 0; i < ARRAYCOUNT(g_MenuData.playerjoinalpha); i++) {
			g_MenuData.playerjoinalpha[i] = 0;
		}
	}
}

void menuPushRootDialog(struct menudialogdef *dialogdef, s32 root)
{
	g_Menus[g_MpPlayerNum].numdialogs = 0;
	g_Menus[g_MpPlayerNum].depth = 0;

	g_MenuData.unk5d5_01 = false;
	g_MenuData.unk5d5_04 = false;

	g_PlayersWithControl[g_Menus[g_MpPlayerNum].playernum] = false;

#ifndef PLATFORM_N64
	inputAutoLockMouse(false);
#endif

	func0f0f1494();

	g_MenuData.count++;

	if (root == MENUROOT_PICKTARGET) {
		g_Menus[g_MpPlayerNum].mppause.unke40_00 = true;
		root = MENUROOT_MPPAUSE;
	}

	switch (root) {
	case MENUROOT_ENDSCREEN:
	case MENUROOT_MAINMENU:
	case MENUROOT_FILEMGR:
	case MENUROOT_BOOTPAKMGR:
	case MENUROOT_PICKTARGET:
	case MENUROOT_COOPCONTINUE:
	case MENUROOT_4MBFILEMGR:
	case MENUROOT_TRAINING:
		g_MenuData.count = 1;
		break;
	}

	g_Menus[g_MpPlayerNum].unk820 = 1;

	g_MenuData.root = root;
	g_MenuData.prevmenuroot = -1;
	g_MenuData.unk5d5_02 = false;

	if (root == MENUROOT_MAINMENU
			|| root == MENUROOT_MPSETUP
			|| root == MENUROOT_TRAINING
			|| root == MENUROOT_FILEMGR) {
		if (IS8MB() && (g_MenuData.unk5d4 == 0 || g_MenuData.hudpiece.reverseanim)) {
			if (!g_MenuData.unk5d5_04) {
				g_MenuData.unk5d5_05 = true;
			}
		}
	}

	menuPushDialog(dialogdef);

	switch (root) {
	case MENUROOT_MPSETUP:
		menuSetBackground(MENUBG_CONEALPHA);
		break;
	case MENUROOT_4MBFILEMGR:
		musicStartMenu();
		g_MenuData.bg = MENUBG_CONEOPAQUE;
		break;
	case MENUROOT_4MBMAINMENU:
		g_MenuData.bg = MENUBG_CONEOPAQUE;
		break;
	case MENUROOT_ENDSCREEN:
		if (dialogdef->type == MENUDIALOGTYPE_DANGER) {
			g_MenuData.unk5d4 = 0;
			g_MenuData.nextbg = MENUBG_FAILURE;
			break;
		}
		// fall-through
	case MENUROOT_MAINMENU:
	case MENUROOT_MPENDSCREEN:
	case MENUROOT_FILEMGR:
	case MENUROOT_COOPCONTINUE:
	case MENUROOT_TRAINING:
		menuSetBackground(MENUBG_BLUR);
		break;
	case MENUROOT_BOOTPAKMGR:
		musicStartMenu();
		g_MenuData.bg = MENUBG_GRADIENT;
		break;
	}

	if (menuIsSoloMissionOrMp()) {
		playerDisplayHealth();
	}
}

void func0f0f85e0(struct menudialogdef *dialogdef, s32 root)
{
	if (dialogdef == &g_CiMenuViaPcMenuDialog) {
		musicStartMenu();
	}

	menuPushRootDialog(dialogdef, root);
	lvSetPaused(true);
	g_Vars.currentplayer->pausemode = PAUSEMODE_PAUSED;
}

u32 g_MenuCThresh = 120;

Gfx *menuRenderDialog(Gfx *gdl, struct menudialog *dialog, struct menu *menu, bool lightweight)
{
	mainOverrideVariable("cthresh", &g_MenuCThresh);

	textSetWaveBlend(dialog->unk54, dialog->unk58, g_MenuCThresh);

	gdl = dialogRender(gdl, dialog, menu, lightweight);

	textResetBlends();

	return gdl;
}

const char var7f1b2700[] = "[]-[] slide from %d";
const char var7f1b2714[] = " to %d\n";
const char var7f1b271c[] = "UNPAUSE: enabling control 0\n";
const char var7f1b273c[] = "file: type %d guid %x-%x data %x err %d\n";
const char var7f1b2768[] = "StartSelects\n";

/**
 * Render all dialogs for the current player.
 *
 * There's usually only one on screen at a time, but there can be multiple when
 * transitioning between dialogs. This happens when swiping left or right beteen
 * dialogs on the same layer, or when opening or closing dialogs.
 */
Gfx *menuRenderDialogs(Gfx *gdl)
{
	if (g_Menus[g_MpPlayerNum].curdialog) {
		if (g_MenuData.root == MENUROOT_MPPAUSE
				|| g_MenuData.root == MENUROOT_PICKTARGET
				|| g_MenuData.root == MENUROOT_MPENDSCREEN) {
			g_MenuProjectFromX = g_Menus[g_MpPlayerNum].curdialog->x + g_Menus[g_MpPlayerNum].curdialog->width / 2 - viGetWidth() / (g_ScaleX * 2);
			g_MenuProjectFromY = g_Menus[g_MpPlayerNum].curdialog->y + g_Menus[g_MpPlayerNum].curdialog->height / 2 - viGetHeight() / 2;

			gdl = menuRenderDialog(gdl, g_Menus[g_MpPlayerNum].curdialog, &g_Menus[g_MpPlayerNum], 0);
		} else {
			s32 i;
			s32 j;

#if VERSION >= VERSION_NTSC_1_0
			// NTSC 1.0 and newer renders one "other" dialog plus the current
			// dialog. The other dialog is only rendered if on-screen, and it
			// ensures the current dialog is drawn last.
			struct menudialog *dialogs[] = { NULL, NULL };

			for (i = 0; i < g_Menus[g_MpPlayerNum].depth; i++) {
				struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[i];

				for (j = 0; j < layer->numsiblings; j++) {
					if (layer->siblings[j] != g_Menus[g_MpPlayerNum].curdialog) {
						if (layer->siblings[j]->type != 0 || layer->siblings[j]->transitionfrac >= 0) {
							dialogs[0] = layer->siblings[j];
						}
					}
				}
			}

			// Render the other dialog if any
			if (dialogs[0]) {
				gdl = menuRenderDialog(gdl, dialogs[0], &g_Menus[g_MpPlayerNum], 0);
			}

			// Render the current dialog
			if (g_Menus[g_MpPlayerNum].curdialog) {
				gdl = menuRenderDialog(gdl, g_Menus[g_MpPlayerNum].curdialog, &g_Menus[g_MpPlayerNum], 0);
			}
#else
			// NTSC beta renders all dialogs all the time, and in their natural order
			for (i = 0; i < g_Menus[g_MpPlayerNum].depth; i++) {
				struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[i];

				for (j = 0; j < layer->numsiblings; j++) {
					gdl = menuRenderDialog(gdl, layer->siblings[j], &g_Menus[g_MpPlayerNum], 0);
				}
			}
#endif
		}

		// Render banner messages
		if (g_Menus[g_MpPlayerNum].bannernum != -1
				&& (g_Menus[g_MpPlayerNum].curdialog->definition->flags & MENUDIALOGFLAG_DISABLEBANNER) == 0) {
			if (g_MenuData.count >= 2) {
				s32 xmin;
				s32 ymin;
				s32 xmax;
				s32 ymax;

#if VERSION >= VERSION_JPN_FINAL
				menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax, NULL);
#else
				menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax);
#endif

#if VERSION >= VERSION_NTSC_1_0
				gdl = menuRenderBanner(gdl, xmin, ymin, xmax, ymax, false, g_Menus[g_MpPlayerNum].bannernum, 0, 0);
#else
				gdl = menuRenderBanner(gdl, xmin, ymin, xmax, ymax, false, g_Menus[g_MpPlayerNum].bannernum);
#endif
			} else {
				s32 xmin = viGetViewLeft() / g_ScaleX;
				s32 ymin = viGetViewTop();
				s32 xmax = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX;
				s32 ymax = viGetViewTop() + viGetViewHeight();

#if VERSION >= VERSION_NTSC_1_0
				gdl = menuRenderBanner(gdl, xmin, ymin, xmax, ymax, true, g_Menus[g_MpPlayerNum].bannernum, 0, 0);
#else
				gdl = menuRenderBanner(gdl, xmin, ymin, xmax, ymax, true, g_Menus[g_MpPlayerNum].bannernum);
#endif
			}
		}
	}

	return gdl;
}

u32 var800714e8 = 0;

void menuResetModel(struct menumodel *menumodel, u32 allocationlen, bool allocate)
{
	menumodel->alloclen = allocationlen;
	menumodel->allocstart = allocate ? mempAlloc(allocationlen, MEMPOOL_STAGE) : NULL;
	menumodel->loaddelay = 0;
	menumodel->newparams = MENUMODELPARAMS_SET_FILENUM(0xffff);
	menumodel->bodymodeldef = NULL;
	menumodel->curparams = 0;

	menumodel->curposx = menumodel->newposx = 0.0f;
	menumodel->curposy = menumodel->newposy = 0.0f;
	menumodel->curposz = menumodel->newposz = 0.0f;

	menumodel->curscale = menumodel->newscale = 0.0f;

	menumodel->currotx = menumodel->newrotx = 0.0f;
	menumodel->curroty = menumodel->newroty = 0.0f;
	menumodel->currotz = menumodel->newrotz = 0.0f;

	menumodel->displacex = menumodel->displacey = menumodel->displacez = 0.0f;

	menumodel->unk56c = 0;
	menumodel->unk570 = 0;
	menumodel->partvisibility = NULL;
	menumodel->isperfecthead = false;
	menumodel->unk5b1_02 = false;
	menumodel->reverseanim = false;
	menumodel->unk5b1_06 = false;
	menumodel->headnum = -1;
	menumodel->bodynum = -1;
}

void menuReset(void)
{
	s32 i;

	func0f110bf0();

	var8009dfc0 = 0;

	if (IS8MB()) {
		g_BlurBuffer = mempAlloc(0x4b00, MEMPOOL_STAGE);
	}

	g_MenuData.unk5d5_01 = false;

	texLoadFromConfig(&g_TexGeneralConfigs[1]);
	texLoadFromConfig(&g_TexGeneralConfigs[6]);
	texLoadFromConfig(&g_TexGeneralConfigs[51]);
	texLoadFromConfig(&g_TexGeneralConfigs[52]);
	texLoadFromConfig(&g_TexGeneralConfigs[53]);
	texLoadFromConfig(&g_TexGeneralConfigs[54]);
	texLoadFromConfig(&g_TexGeneralConfigs[55]);

	if (g_Vars.stagenum == STAGE_CITRAINING) {
		for (i = 12; i < 34; i++) {
			texLoadFromConfig(&g_TexGeneralConfigs[i]);
		}

		texLoadFromConfig(&g_TexGeneralConfigs[34]);
		texLoadFromConfig(&g_TexGeneralConfigs[36]);
		texLoadFromConfig(&g_TexGeneralConfigs[35]);
	}

	if (g_Vars.mplayerisrunning) {
		texLoadFromConfig(&g_TexGeneralConfigs[35]);
	}

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		g_Menus[i].menumodel.allocstart = NULL;
	}

	g_MenuData.hudpiece.allocstart = NULL;

	if (g_Vars.stagenum == STAGE_CITRAINING) {
		g_MissionConfig.iscoop = false;
		g_MissionConfig.isanti = false;
		g_MissionConfig.pdmode = false;
	}

	if (!g_Vars.mplayerisrunning) {
		s32 max = 0;

		if (g_Vars.stagenum == STAGE_CITRAINING) {
			max = 4;
		}

		if (g_Vars.stagenum == STAGE_4MBMENU) {
			max = 4;
		}

		for (i = 0; i < max; i++) {
#ifdef PLATFORM_64BIT
			menuResetModel(&g_Menus[i].menumodel, IS4MB() ? 0xb400 : 0x38400, true); // 50% more
#else
			menuResetModel(&g_Menus[i].menumodel, IS4MB() ? 0xb400 : 0x25800, true);
#endif
		}

		if (IS8MB()) {
#ifdef PLATFORM_64BIT
			menuResetModel(&g_MenuData.hudpiece, 0x12c00, true); // 50% more
#else
			menuResetModel(&g_MenuData.hudpiece, 0xc800, true);
#endif
		}

		g_MenuData.hudpiece.newparams = MENUMODELPARAMS_SET_FILENUM(FILE_GHUDPIECE);
		g_MenuData.hudpiece.curroty = g_MenuData.hudpiece.newroty = -M_PI;
		g_MenuData.hudpiece.currotx = g_MenuData.hudpiece.newrotx = 0;
		g_MenuData.hudpiece.currotz = g_MenuData.hudpiece.newrotz = 0;
		g_MenuData.hudpiece.curposx = g_MenuData.hudpiece.newposx = -205.5f;
		g_MenuData.hudpiece.curposy = g_MenuData.hudpiece.newposy = 244.7f;
		g_MenuData.hudpiece.curposz = g_MenuData.hudpiece.newposz = 68.3f;
		g_MenuData.hudpiece.curscale = g_MenuData.hudpiece.newscale = 0.12209f;
		g_MenuData.hudpiece.isperfecthead = false;
		g_MenuData.hudpiece.zoomtimer60 = 0;
		g_MenuData.hudpiece.removingpiece = false;
	}

	g_MenuData.unk5d4 = 0;
	g_MenuData.unk5d5_05 = false;

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		g_Menus[i].curdialog = NULL;
		g_Menus[i].depth = 0;
		g_Menus[i].numdialogs = 0;
		g_Menus[i].unk820 = 0;
		g_Menus[i].rowend = 0;
		g_Menus[i].blockend = 0;
		g_Menus[i].colend = 0;
		g_Menus[i].bannernum = -1;
		g_Menus[i].fm.unke41 = 0;
		g_Menus[i].fm.unke64 = 0;
		g_Menus[i].fm.headtextures = NULL;
	}

	g_MenuData.unk668 = -1;
	g_MenuData.prevmenudialog = 0;
	g_MenuData.prevmenuroot = -1;
	g_MenuData.count = 0;
	g_MenuData.root = 0;
	g_MenuData.unk010 = 0;
	g_MenuData.bg = 0;
	g_MenuData.isdialogopen = false;
	g_MenuData.nextbg = 255;
	g_MenuData.bannernum = -1;

	for (i = 0; i < ARRAYCOUNT(g_MenuData.unk669); i++) {
		g_MenuData.unk669[i] = 0xff;
	}

	g_MenuData.unk66e = 0;
	g_MenuData.unk66f = 0;
}

void menuSwipe(s32 direction)
{
	struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1];
	struct menuitem *item;
	union handlerdata sp50;
	union handlerdata sp40;

	if (layer->numsiblings >= 2) {
		g_Menus[g_MpPlayerNum].curdialog->swipedir = -direction;

		layer->cursibling += (s8)direction;

		if (layer->cursibling < 0) {
			layer->cursibling = layer->numsiblings - 1;
		}

		if (layer->cursibling >= layer->numsiblings) {
			layer->cursibling = 0;
		}

		g_Menus[g_MpPlayerNum].curdialog = layer->siblings[layer->cursibling];

		if (direction == 1) {
			g_Menus[g_MpPlayerNum].curdialog->focuseditem = dialogFindFirstItem(g_Menus[g_MpPlayerNum].curdialog);
		} else {
			g_Menus[g_MpPlayerNum].curdialog->focuseditem = dialogFindFirstItemRight(g_Menus[g_MpPlayerNum].curdialog);
		}

		item = g_Menus[g_MpPlayerNum].curdialog->definition->items;

		while (item->type != MENUITEMTYPE_END) {
			if (item->handler
					&& (item->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0
					&& item->handler(MENUOP_CHECKPREFOCUSED, item, &sp50)) {
				g_Menus[g_MpPlayerNum].curdialog->focuseditem = item;
			}

			item++;
		}

		if (g_Menus[g_MpPlayerNum].curdialog->focuseditem != 0
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->handler
				&& ((g_Menus[g_MpPlayerNum].curdialog->focuseditem->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) == 0)) {
			g_Menus[g_MpPlayerNum].curdialog->focuseditem->handler(MENUOP_FOCUS, g_Menus[g_MpPlayerNum].curdialog->focuseditem, &sp40);
		}

		g_Menus[g_MpPlayerNum].curdialog->swipedir = direction;

		dialogCalculatePosition(g_Menus[g_MpPlayerNum].curdialog);

		g_Menus[g_MpPlayerNum].curdialog->x = g_Menus[g_MpPlayerNum].curdialog->dstx;
		g_Menus[g_MpPlayerNum].curdialog->y = g_Menus[g_MpPlayerNum].curdialog->dsty;
		g_Menus[g_MpPlayerNum].curdialog->swipedir = 0;
		g_Menus[g_MpPlayerNum].curdialog->state = MENUDIALOGSTATE_PREOPEN;
		g_Menus[g_MpPlayerNum].curdialog->statefrac = 0.0f;

		menuUnsetModel(&g_Menus[g_MpPlayerNum].menumodel);

		menuPlaySound(MENUSOUND_SWIPE);
	}
}

extern struct menudialogdef g_MpDropOut4MbMenuDialog;

void dialogTick(struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags)
{
	bool usedefaultbehaviour;
	struct menudialogdef *definition;
	struct menu *menu;
	u8 transitiontotype;
	s32 oldfracint;
	s32 oldx;
	s32 col;
	f32 newx;
	union handlerdata data;
	struct menuinputs spd8;
	s32 oldy;
	f32 newy;
	s32 oldwidth;
	f32 newwidth;
	s32 oldheight;
	f32 newheight;
	s32 i;
	u32 stack;

	usedefaultbehaviour = false;
	definition = dialog->definition;
	menu = &g_Menus[g_MpPlayerNum];

	if (g_Menus[g_MpPlayerNum].fm.unke40_00 || g_MainIsDebugMenuOpen) {
		inputs->leftright = inputs->updown = inputs->select = inputs->back = inputs->xaxis = inputs->yaxis = inputs->shoulder = inputs->back2 = inputs->unk14 = 0;
		g_Menus[g_MpPlayerNum].fm.unke40_00 = false;
	}

	g_Menus[g_MpPlayerNum].fm.unke40_00 = false;

	spd8.select = 0;
	spd8.back = inputs->back;
	spd8.leftright = inputs->leftright;
	spd8.updown = inputs->updown;
	spd8.xaxis = inputs->xaxis;
	spd8.yaxis = inputs->yaxis;
	spd8.leftrightheld = inputs->leftrightheld;
	spd8.updownheld = inputs->updownheld;
	spd8.start = 0;
	spd8.unk0c = inputs->unk0c;
	spd8.unk10 = inputs->unk10;

	dialog->unk54++;
	dialog->unk5c += g_Vars.diffframe60;
	dialog->unk54 += dialog->unk5c / 9;
	dialog->unk5c %= 9;
	dialog->unk54 %= g_MenuCThresh;

	// For endscreens, handle transitioning of background and dialog type
	if (dialog->transitionfrac < 0.0f) {
		// Transition not yet started
		if (dialog == g_Menus[g_MpPlayerNum].curdialog) {
			transitiontotype = definition->type;

			if (mpIsPlayerLockedOut(g_MpPlayerNum) && (dialog->definition->flags & MENUDIALOGFLAG_MPLOCKABLE)) {
				transitiontotype = MENUDIALOGTYPE_DANGER;
			}

			if (g_MenuData.root == MENUROOT_ENDSCREEN) {
				if (g_MenuData.bg == MENUBG_8 || g_MenuData.bg == MENUBG_SUCCESS) {
					transitiontotype = MENUDIALOGTYPE_DEFAULT;
				}

				if (g_StageIndex == STAGEINDEX_DEFENSE
						&& g_MenuData.bg != MENUBG_FAILURE
						&& g_MenuData.nextbg != MENUBG_FAILURE) {
					transitiontotype = MENUDIALOGTYPE_WHITE;
					dialog->type = MENUDIALOGTYPE_WHITE;
				}
			}

			if (dialog->type != transitiontotype) {
				dialog->type2 = transitiontotype;
				dialog->colourweight = 0;
				dialog->transitionfrac = 0;
			}
		} else if (dialog->type != 0) {
			dialog->type2 = 0;
			dialog->colourweight = 0;
			dialog->transitionfrac = 0;
		}
	} else {
		// Mid-transition
		if (g_MenuData.root == MENUROOT_ENDSCREEN
				&& g_StageIndex == STAGEINDEX_DEFENSE
				&& g_MenuData.bg != MENUBG_FAILURE
				&& g_MenuData.nextbg != MENUBG_FAILURE
				&& dialog->type2 != 0) {
			dialog->type2 = MENUDIALOGTYPE_WHITE;
		}

		// Success endscreens transition to the background slower than failures
		if (g_MenuData.root == MENUROOT_ENDSCREEN
				&& dialog->type == MENUDIALOGTYPE_SUCCESS
				&& (g_MenuData.bg == MENUBG_8 || g_MenuData.bg == MENUBG_SUCCESS)) {
#if VERSION >= VERSION_PAL_BETA
			dialog->transitionfrac += g_Vars.diffframe60freal * 0.01f;
#else
			dialog->transitionfrac += g_Vars.diffframe60f * 0.01f;
#endif
		} else {
#if VERSION >= VERSION_PAL_BETA
			dialog->transitionfrac += g_Vars.diffframe60freal * 0.042f;
#else
			dialog->transitionfrac += g_Vars.diffframe60f * 0.042f;
#endif
		}

		if (dialog->transitionfrac > 1.0f) {
			dialog->transitionfrac = -1.0f;
			dialog->type = dialog->type2;
		}

		dialog->colourweight = dialog->transitionfrac * 255.0f;
	}

	// When fully open and ready, the dialog shows the text fully opaque for
	// 2 seconds (redrawtimer < 0.0f) then starts the redraw timer and waits for
	// it to finish before repeating the process. While the redraw timer is
	// active, the text begins to fade before being redrawn.
	if (dialog->state == MENUDIALOGSTATE_POPULATED) {
		if (g_MenuData.nextbg != MENUBG_CONEALPHA) {
			if (dialog->redrawtimer < 0.0f) {
#if VERSION >= VERSION_PAL_BETA
				dialog->statefrac += g_Vars.diffframe60freal / 120.0f;
#else
				dialog->statefrac += g_Vars.diffframe60f / 120.0f;
#endif

				if (dialog->statefrac > 1.0f) {
					dialog->redrawtimer = 0.0f;
				}
			} else {
				dialog->statefrac = 0.0f;
			}
		}
	}

	// When populating the text for the first time, wait for both the redraw
	// timer to finish and a minimum duration.
	if (dialog->state == MENUDIALOGSTATE_POPULATING) {
#if VERSION >= VERSION_PAL_BETA
		dialog->statefrac -= 0.05f * g_Vars.diffframe60freal;
#else
		dialog->statefrac -= 0.05f * g_Vars.diffframe60f;
#endif

		if (dialog->statefrac < 0.0f) {
			dialog->statefrac = 0.0f;

			if (dialog->redrawtimer < 0.0f) {
				dialog->state = MENUDIALOGSTATE_POPULATED;
			}
		}
	}

	// When opening the dialog, the height increases gradually over several
	// frames. This part doesn't actually change the height - it updates the
	// statefrac based on the current height.
	if (dialog->state == MENUDIALOGSTATE_OPENING) {
		oldfracint = dialog->statefrac;

		if (dialog->statefrac != dialog->height) {
			for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
				dialog->statefrac = dialog->height * 0.235f + 0.765f * dialog->statefrac;
#else
				dialog->statefrac = dialog->height * 0.2f + 0.8f * dialog->statefrac;
#endif
			}
		}

		if ((s32)dialog->statefrac == oldfracint) {
			dialog->statefrac = oldfracint + 1.0f;
		}

		if (dialog->statefrac > dialog->height - 1.0f && dialog->statefrac < dialog->height + 1.0f) {
			dialog->state = MENUDIALOGSTATE_POPULATING;
			dialog->statefrac = 1.0f;
		}
	}

#if VERSION >= VERSION_NTSC_1_0
	if (dialog->state == MENUDIALOGSTATE_PREOPEN) {
		if (dialog->definition == &g_MpReadyMenuDialog) {
			if (dialog->statefrac < 0.1f) {
				dialog->statefrac += 0.04f;
			} else {
				dialog->state = MENUDIALOGSTATE_OPENING;
				dialog->redrawtimer = 0.0f;
				dialog->statefrac = 0.5f;
			}
		} else if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && menuGetRoot() == MENUROOT_MPENDSCREEN) {
			if (var8009dfc0) {
#if VERSION >= VERSION_PAL_BETA
				dialog->statefrac += g_Vars.diffframe240freal / 60.0f;
#else
				dialog->statefrac += g_Vars.diffframe240 / 60.0f;
#endif

				if (dialog->statefrac > 1.0f) {
					dialog->state = MENUDIALOGSTATE_OPENING;
					dialog->redrawtimer = 0.0f;
					dialog->statefrac = 0.5f;
				}
			}
		} else {
			if (g_MenuData.nextbg == 255 || g_MenuData.bg != 0) {
				dialog->state = MENUDIALOGSTATE_OPENING;
				dialog->redrawtimer = 0.0f;
				dialog->statefrac = 0.5f;
			}
		}
	}
#else
	if (dialog->state == MENUDIALOGSTATE_PREOPEN) {
		if (g_MenuData.nextbg == 255 || g_MenuData.bg != 0) {
			dialog->state = MENUDIALOGSTATE_OPENING;
			dialog->redrawtimer = 0.0f;
			dialog->statefrac = 0.5f;
		}
	}
#endif

	// Increment the redraw timer, which makes the contents fade and then get
	// redrawn. The initial draw is done faster than subsequent draws.
	if (dialog->redrawtimer < 0.0f) {
		// Redraw not active
	} else {
#if VERSION >= VERSION_PAL_BETA
		if (dialog->state == MENUDIALOGSTATE_POPULATED) {
			dialog->redrawtimer += g_Vars.diffframe60freal + g_Vars.diffframe60freal;
		} else {
			dialog->redrawtimer += 5 * g_Vars.diffframe60freal;
		}
#else
		if (dialog->state == MENUDIALOGSTATE_POPULATED) {
			dialog->redrawtimer += 2 * g_Vars.diffframe60;
		} else {
			dialog->redrawtimer += 5 * g_Vars.diffframe60;
		}
#endif

		if (dialog->redrawtimer > 600.0f) {
			dialog->redrawtimer = -1.0f;
		}
	}

	if ((dialog->definition->flags & MENUDIALOGFLAG_DISABLERESIZE) == 0) {
		dialogCalculateContentSize(dialog->definition, dialog, menu);
	}

	dialogCalculatePosition(dialog);
	dialog0f0f1ef4(dialog);

	// Update slide
	if (g_MenuData.root == MENUROOT_MPPAUSE
			|| g_MenuData.root == MENUROOT_PICKTARGET
			|| g_MenuData.root == MENUROOT_MPENDSCREEN) {
		// Don't slide
		dialog->x = dialog->dstx;
		dialog->y = dialog->dsty;
	} else {
		// Slide X
		if (dialog->x != dialog->dstx) {
			oldx = dialog->x;
			newx = dialog->x;

			for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
				newx = dialog->dstx * 0.348f + 0.652f * newx;
#else
				newx = dialog->dstx * 0.3f + 0.7f * newx;
#endif
			}

			dialog->x = newx;

			if (dialog->x != dialog->dstx && dialog->x == oldx) {
				if (dialog->x < dialog->dstx) {
					dialog->x++;
				} else {
					dialog->x--;
				}
			}
		}

		// Slide Y
		if (dialog->y != dialog->dsty) {
			oldy = dialog->y;
			newy = dialog->y;

			for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
				newy = dialog->dsty * 0.348f + 0.652f * newy;
#else
				newy = dialog->dsty * 0.3f + 0.7f * newy;
#endif
			}

			dialog->y = newy;

			if (dialog->y != dialog->dsty && dialog->y == oldy) {
				if (dialog->y < dialog->dsty) {
					dialog->y++;
				} else {
					dialog->y--;
				}
			}
		}
	}

	// Tween the width
	if (dialog->width != dialog->dstwidth) {
		oldwidth = dialog->width;
		newwidth = dialog->width;

		for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
			newwidth = dialog->dstwidth * 0.348f + 0.652f * newwidth;
#else
			newwidth = dialog->dstwidth * 0.3f + 0.7f * newwidth;
#endif
		}

		dialog->width = newwidth;

		if (dialog->width != dialog->dstwidth && dialog->width == oldwidth) {
			if (dialog->width < dialog->dstwidth) {
				dialog->width++;
			} else {
				dialog->width--;
			}
		}
	}

	// Tween the height
	if (dialog->height != dialog->dstheight) {
		s32 i;
		oldheight = dialog->height;
		newheight = dialog->height;

		for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
			newheight = dialog->dstheight * 0.348f + 0.652f * newheight;
#else
			newheight = dialog->dstheight * 0.3f + 0.7f * newheight;
#endif
		}

		dialog->height = newheight;

		if (dialog->height != dialog->dstheight && dialog->height == oldheight) {
			if (dialog->height < dialog->dstheight) {
				dialog->height++;
			} else {
				dialog->height--;
			}
		}
	}

	// Call the dialog's tick handler, if any
	data.dialog2.inputs = inputs;

	if (definition->handler != NULL) {
		definition->handler(MENUOP_TICK, definition, &data);
	}

	if (dialog->dimmed) {
		dialog->unk10 += g_Vars.diffframe60;
	} else {
		dialog->unk10 = 0;
	}

	// Tick each item in the dialog
	{
		s32 j;

		for (col = 0; col < dialog->numcols; col++) {
			s32 colindex = dialog->colstart + col;

			for (j = 0; j < menu->cols[colindex].numrows; j++) {
				s32 rowindex = menu->cols[colindex].rowstart + j;
				struct menuitem *item = &dialog->definition->items[menu->rows[rowindex].itemindex];
				union menuitemdata *handlerdata = NULL;
				struct menuinputs *inputsptr = inputs;

				if (mpIsPlayerLockedOut(g_MpPlayerNum) && (item->flags & MENUITEMFLAG_LOCKABLEMINOR)) {
					inputsptr = &spd8;
				} else if ((item->flags & MENUITEMFLAG_MPWEAPONSLOT) && mpGetWeaponSet() != func0f189088()) {
					inputsptr = &spd8;
				} else if (g_MenuData.root == MENUROOT_12) {
					inputsptr = &spd8;
				}

				if (inputsptr);

				if (menu->rows[rowindex].blockindex != -1) {
					handlerdata = (union menuitemdata *)&menu->blocks[menu->rows[rowindex].blockindex];
				}

				if (g_Menus[g_MpPlayerNum].fm.unke40_00 == 0 && !g_MainIsDebugMenuOpen) {
					if ((tickflags & MENUTICKFLAG_DIALOGISCURRENT) && item == dialog->focuseditem) {
						u32 itemtickflags = tickflags | MENUTICKFLAG_ITEMISFOCUSED;

						if (dialog->dimmed) {
							usedefaultbehaviour = menuitemTick(item, dialog, inputsptr, itemtickflags | MENUTICKFLAG_DIALOGISDIMMED, handlerdata);
						} else {
							usedefaultbehaviour = menuitemTick(item, dialog, inputsptr, itemtickflags, handlerdata);
						}
					} else {
						menuitemTick(item, dialog, inputsptr, tickflags, handlerdata);
					}
				}
			}
		}
	}

	// If the focused item is disabled somehow, automatically jump to the next
	if (dialog->focuseditem
			&& menuIsItemDisabled(dialog->focuseditem, dialog)
			&& (tickflags & MENUTICKFLAG_DIALOGISCURRENT)) {
		usedefaultbehaviour = true;
		inputs->updown = 1;
		dialog->dimmed = false;
	}

	// Apply default navigational behaviour if requested
#ifdef AVOID_UB
	if (usedefaultbehaviour && (tickflags & MENUTICKFLAG_DIALOGISCURRENT) && !dialog->dimmed && g_Menus[g_MpPlayerNum].depth >= 1) {
#else
	if (usedefaultbehaviour && (tickflags & MENUTICKFLAG_DIALOGISCURRENT) && !dialog->dimmed) {
#endif
		struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1];

		if (layer->numsiblings <= 1) {
			struct menuitem *prevfocuseditem = dialog->focuseditem;

			dialogChangeItemFocus(dialog, inputs);

			if (dialog->focuseditem != prevfocuseditem) {
				menuPlaySound(MENUSOUND_FOCUS);
			}
		} else {
			struct menuitem *prevfocuseditem = dialog->focuseditem;
			s32 swipedir = dialogChangeItemFocus(dialog, inputs);

			if (swipedir != 0) {
				menuSwipe(swipedir);
			} else if (prevfocuseditem != dialog->focuseditem) {
				menuPlaySound(MENUSOUND_FOCUS);
			}
		}

		if (inputs->back) {
			if ((dialog->definition->flags & MENUDIALOGFLAG_DROPOUTONCLOSE) && g_Vars.unk000498) {
				if (IS4MB()) {
					menuPushDialog(&g_MpDropOut4MbMenuDialog);
				} else {
					menuPushDialog(&g_MpDropOutMenuDialog);
				}
			} else if ((dialog->definition->flags & MENUDIALOGFLAG_IGNOREBACK) == 0) {
				menuPopDialog();
			}
		} else if (dialog->definition->flags & MENUDIALOGFLAG_CLOSEONSELECT) {
			if (dialog->state > MENUDIALOGSTATE_PREOPEN) {
				if ((inputs->select & 1) == 1 || (inputs->back & 1) == 1) {
					menuPopDialog();
				}
			}
		}
	}

	// Scrolling related (when the dialog is too big vertically)
	if (dialog->focuseditem && (dialog->definition->flags & MENUDIALOGFLAG_DISABLEITEMSCROLL) == 0) {
#if VERSION >= VERSION_NTSC_1_0
		s32 dstscroll;
		s32 itemy;
		s32 rowindex;
		s32 colindex;
#else
		s32 dstscroll;
		s32 rowindex;
		s32 colindex;
		s32 itemy;
#endif

		s32 y = dialogFindItem(dialog, dialog->focuseditem, &rowindex, &colindex);

		if ((dialog->focuseditem->flags & MENUITEMFLAG_00010000) == 0) {
#ifndef PLATFORM_N64
			if (g_MenuUsingMouse && !dialog->dimmed) {
				dstscroll = dialog->dstscroll - inputs->mousescroll * LINEHEIGHT;
			} else
#endif
			{
				itemy = y + menu->rows[rowindex].height / 2;
				dstscroll = (dialog->height - LINEHEIGHT - 1) / 2 - itemy;
			}

			if (dstscroll > 0) {
				dstscroll = 0;
			}

			if (dstscroll < dialog->height - dialog->contentheight) {
				dstscroll = dialog->height - dialog->contentheight;
			}

			dialog->dstscroll = dstscroll;
		} else {
			dialog->dstscroll = 0;
		}
	} else if (dialog->definition->flags & MENUDIALOGFLAG_SMOOTHSCROLLABLE) {
#if VERSION >= VERSION_PAL_BETA
		s32 adjustment = (s32)(inputs->yaxis * g_Vars.diffframe60freal / 20) - (s32)(inputs->updownheld * g_Vars.diffframe60freal);
#else
		s32 adjustment = inputs->yaxis * g_Vars.diffframe60 / 20 - inputs->updownheld * g_Vars.diffframe60;
#endif
#ifndef PLATFORM_N64
		adjustment -= inputs->mousescroll * (LINEHEIGHT + 1);
#endif
		dialog->dstscroll += adjustment;

		if (dialog->dstscroll > 0) {
			dialog->dstscroll = 0;
		}

		if (dialog->dstscroll < dialog->height - dialog->contentheight) {
			dialog->dstscroll = dialog->height - dialog->contentheight;
		}

		dialog->scroll = dialog->dstscroll;
	}

	{
		if (dialog->scroll != dialog->dstscroll) {
			s32 oldscroll = dialog->scroll;
			f32 newscroll = dialog->scroll;
			s32 i;

			for (i = 0; i < g_Vars.diffframe60; i++) {
#if PAL
				newscroll = (dialog->dstscroll * 0.235f) + (0.765f * newscroll);
#else
				newscroll = (dialog->dstscroll * 0.2f) + (0.8f * newscroll);
#endif
			}

			dialog->scroll = newscroll;

			if (dialog->scroll != dialog->dstscroll && dialog->scroll == oldscroll) {
				if (dialog->scroll < dialog->dstscroll) {
					dialog->scroll++;
				} else {
					dialog->scroll--;
				}
			}
		}
	}

	if (g_Menus[g_MpPlayerNum].fm.unke40_00 || g_MainIsDebugMenuOpen) {
		inputs->leftright = inputs->updown = inputs->select = inputs->back = inputs->xaxis = inputs->yaxis = inputs->shoulder = inputs->back2 = inputs->unk14 = 0;
		g_Menus[g_MpPlayerNum].fm.unke40_00 = false;
	}
}

void dialogInitItems(struct menudialog *dialog)
{
	struct menu *menu = &g_Menus[g_MpPlayerNum];
	s32 i;
	s32 j;

	for (i = 0; i < dialog->numcols; i++) {
		s32 colindex = dialog->colstart + i;

		for (j = 0; j < menu->cols[colindex].numrows; j++) {
			s32 rowindex = menu->cols[colindex].rowstart + j;
			struct menuitem *item = &dialog->definition->items[menu->rows[rowindex].itemindex];
			union menuitemdata *data = NULL;

			if (menu->rows[rowindex].blockindex != -1) {
				data = (union menuitemdata *)&menu->blocks[menu->rows[rowindex].blockindex];
			}

			menuitemInit(item, data);
		}
	}
}

void func0f0fa6ac(void)
{
	switch (g_MenuData.root) {
	case MENUROOT_MAINMENU:
	case MENUROOT_MPSETUP:
	case MENUROOT_FILEMGR:
	case MENUROOT_4MBMAINMENU:
	case MENUROOT_TRAINING:
		playerUnpause();
		g_PlayersWithControl[0] = true;
	}
}

void menuProcessInput(void)
{
	s32 yhelddir;
	s32 xhelddir;
	s32 ytapdir;
	s32 xtapdir;
	bool starttoselect;
	struct menuinputs inputs;
	struct menudialog *dialog;
	struct menu *menu;
	bool starttap;
	s32 stickx;
	s32 sticky;
	s32 numcontpads;
	s32 i;
	s32 contpadnums[4];
	s8 contpadnum1;
	s8 contpadnum2;
	struct fileguid guid;
	s32 xdeadzone;
	s32 ydeadzone;
	s32 digitalrepeatinterval;
	s32 xstickintervalmult;
	s32 stickintervalbase;
	s32 ystickintervalmult;
	s32 allowdiagonal;

	yhelddir = 0;
	xhelddir = 0;
	ytapdir = 0;
	xtapdir = 0;
	starttoselect = false;

	menu = &g_Menus[g_MpPlayerNum];
	dialog = g_Menus[g_MpPlayerNum].curdialog;

	if (g_MenuData.root == MENUROOT_MPPAUSE) {
		g_AmIndex = g_Vars.currentplayernum;
	}

	func0f0f1418();

	inputs.select = 0;
	inputs.back = 0;
	inputs.shoulder = 0;
	inputs.back2 = 0;

#ifndef PLATFORM_N64
	inputs.mouseheld = false;
	inputs.mousemoved = false;
	inputs.mousescroll = 0;
	inputs.mousex = 0;
	inputs.mousey = 0;
	// only allow mouse controls for player 1 menus
	if (menu->playernum == 0) {
		// ESC always acts as back
		inputs.back = inputKeyJustPressed(VK_ESCAPE);
		if (inputMouseIsEnabled() && !inputMouseIsLocked() && g_MenuMouseControl) {
			inputs.mouseheld = inputKeyPressed(VK_MOUSE_LEFT);
			if (!inputs.mouseheld) {
				g_AllowMouseHeld = true;
				if (g_MouseDimmedMode) {
					g_MouseDimmedMode = false;
					g_MouseEndDeferredSlider = true;
				}
			}
			inputs.mousescroll = inputKeyPressed(VK_MOUSE_WHEEL_DN) - inputKeyPressed(VK_MOUSE_WHEEL_UP);
			inputs.mousemoved = inputMouseGetPosition(&inputs.mousex, &inputs.mousey) || inputs.mousescroll;
			// aspect correct the X
			const f32 cx = ((f32)inputs.mousex - (f32)(SCREEN_WIDTH_LO / 2)) * (videoGetAspect() / SCREEN_ASPECT);
			inputs.mousex = (f32)(SCREEN_WIDTH_LO / 2) + cx;
		}
		if (dialog && inputs.mousemoved) {
			g_MenuUsingMouse = true;
		}
		if (inputs.back && g_MenuKeyboardPlayer != -1) {
			// eat the back input so that other menus don't quit when we hit ESC
			inputs.back = false;
			if (g_MenuKeyboardPlayer == menu->playernum) {
				g_MenuKeyboardPlayer = -1;
				inputStopTextInput();
			}
		}
	}
#endif

	if (g_Menus[g_MpPlayerNum].curdialog) {
		stickx = 0;
		sticky = 0;
		starttap = false;
		numcontpads = 0;

		// Decide which controller pads will control this player's menu
		if (g_MenuData.root == MENUROOT_BOOTPAKMGR) {
			contpadnums[0] = 0;
			contpadnums[1] = 1;
			contpadnums[2] = 2;
			contpadnums[3] = 3;
			numcontpads = 4;
		} else {
			menuGetContPads(&contpadnum1, &contpadnum2);

			if (contpadnum1 >= 0) {
				contpadnums[numcontpads] = contpadnum1;
				numcontpads++;
			}

			if (contpadnum2 >= 0) {
				contpadnums[numcontpads] = contpadnum2;
				numcontpads++;
			}
		}

		// Handle some kind of file save/load timer
		if (g_Menus[g_MpPlayerNum].fm.unke41 > 0) {
			g_Menus[g_MpPlayerNum].fm.unke41--;

			if (g_Menus[g_MpPlayerNum].fm.unke41 == 0) {
				guid.fileid = g_Menus[g_MpPlayerNum].fm.fileid;
				guid.deviceserial = g_Menus[g_MpPlayerNum].fm.deviceserial;

				filemgrSaveOrLoad(&guid, -1, 0);
			} else {
				g_Menus[g_MpPlayerNum].fm.unke40_00 = true;
			}
		}

		// Iterate controllers and figure out which buttons are being pressed.
		// For the control stick input, take whichever stick is pressed the most.
		for (i = 0; i < numcontpads; i++) {
			s8 thisstickx = joyGetStickX(contpadnums[i]);
			s8 thissticky = joyGetStickY(contpadnums[i]);
			s8 thisrstickx = joyGetRStickX(contpadnums[i]);
			s8 thisrsticky = joyGetRStickY(contpadnums[i]);
			u32 buttons = joyGetButtons(contpadnums[i], 0xffffffff);
			u32 buttonsnow = joyGetButtonsPressedThisFrame(contpadnums[i], 0xffffffff);

			if (buttonsnow & A_BUTTON) {
				inputs.select = 1;
			}

#ifndef PLATFORM_N64
			// separate buttons for UI accept/cancel
			// cancel/confirm layout can be swapped either controller type or user preference
            u32 acceptBtn = inputConfirmCancelButtonSwap(contpadnums[i], BUTTON_UI_ACCEPT);
            u32 cancelBtn = inputConfirmCancelButtonSwap(contpadnums[i], BUTTON_UI_CANCEL);

            if (buttonsnow & acceptBtn) {
                inputs.select = 1;
            }
            if (buttonsnow & cancelBtn) {
                inputs.back = 1;
            }
#endif

			if (buttonsnow & B_BUTTON) {
				inputs.back = 1;
			}

			if (buttonsnow & Z_TRIG) {
				inputs.select = 1;
			}

			if (buttonsnow & START_BUTTON) {
				starttap = true;
			}

			if (buttons & R_TRIG) {
				inputs.shoulder = 1;
			}

			if ((stickx < 0 ? -stickx : stickx) < (thisstickx < 0 ? -thisstickx : thisstickx)) {
				stickx = thisstickx;
			}

#ifndef PLATFORM_N64
			if ((stickx < 0 ? -stickx : stickx) < (thisrstickx < 0 ? -thisrstickx : thisrstickx)) {
				stickx = thisrstickx;
			}
#endif

			if ((sticky < 0 ? -sticky : sticky) < (thissticky < 0 ? -thissticky : thissticky)) {
				sticky = thissticky;
			}

#ifndef PLATFORM_N64
			if ((sticky < 0 ? -sticky : sticky) < (thisrsticky < 0 ? -thisrsticky : thisrsticky)) {
				sticky = thisrsticky;
			}
#endif

			if (buttons & U_CBUTTONS) {
				yhelddir = -1;
			}

			if (buttonsnow & U_CBUTTONS) {
				ytapdir = -1;
			}

			if (buttons & D_CBUTTONS) {
				yhelddir = 1;
			}

			if (buttonsnow & D_CBUTTONS) {
				ytapdir = 1;
			}

			if (buttons & L_CBUTTONS) {
				xhelddir = -1;
			}

			if (buttonsnow & L_CBUTTONS) {
				xtapdir = -1;
			}

			if (buttons & R_CBUTTONS) {
				xhelddir = 1;
			}

			if (buttonsnow & R_CBUTTONS) {
				xtapdir = 1;
			}

		}

		// Prevent select and going back on the same frame
		if (inputs.select) {
			inputs.back = 0;
		}

		if (ytapdir != 0) {
			yhelddir = ytapdir;
		}

		if (xtapdir != 0) {
			xhelddir = xtapdir;
		}

		// Choose repeat rate settings
		digitalrepeatinterval = TICKS(10);
		xdeadzone = 30;
		ydeadzone = 20;
		stickintervalbase = 60;
		xstickintervalmult = 33;
		ystickintervalmult = 44;
		allowdiagonal = false;

		if (g_Menus[g_MpPlayerNum].curdialog) {
			struct menuitem *item = g_Menus[g_MpPlayerNum].curdialog->focuseditem;

			if (item) {
				if (item->type == MENUITEMTYPE_SLIDER || item->type == MENUITEMTYPE_10) {
					if (g_Menus[g_MpPlayerNum].curdialog->dimmed) {
						digitalrepeatinterval = TICKS(5);
						xdeadzone = 20;
						stickintervalbase = 30;
						xstickintervalmult = 10;
					}
				}

				if (item->type == MENUITEMTYPE_KEYBOARD) {
					allowdiagonal = true;
					digitalrepeatinterval = TICKS(5);
					xdeadzone = 20;
					xstickintervalmult = 10;
					ystickintervalmult = 10;
				}
			}
		}

		// Handle left/right repeat
		{
			s32 absstickx;
			s32 abssticky;
			s32 oldslot;
			s32 newslot;
			s32 interval;
			bool apply = false;

			// This check doesn't exist in the up/down code later on...
			// It's likely unnecessary
			if (xhelddir == 0) {
				menu->xrepeatmode = MENUREPEATMODE_RELEASED;
			}

			if (xtapdir != 0) {
				// Direction was pressed this frame - reset the repeat properties
				menu->xrepeatmode = MENUREPEATMODE_SLOW;
				menu->xrepeattimer60 = 0;
				menu->xrepeatdir = xtapdir;
				apply = true;
			} else if (xhelddir != 0) {
				xhelddir = menu->xrepeatdir;
			}

			// If held for 1 second, repeat faster
			if (menu->xrepeattimer60 > TICKS(60)) {
				menu->xrepeatmode = MENUREPEATMODE_FAST;
			}

			// Calculate the old and new repeat slots.
			// If these are different, the repeat will be applied on this tick.
			oldslot = menu->xrepeattimer60 / digitalrepeatinterval;
			newslot = (menu->xrepeattimer60 + g_Vars.diffframe60) / digitalrepeatinterval;

			if (menu->xrepeatmode == MENUREPEATMODE_SLOW) {
				oldslot /= 2;
				newslot /= 2;
			}

			inputs.leftrightheld = xhelddir;

			// Check if the stick is being pushed left or right
			absstickx = stickx < 0 ? -stickx : stickx;
			abssticky = sticky < 0 ? -sticky : sticky;

			if (absstickx >= xdeadzone && (absstickx > abssticky || allowdiagonal)) {
				// Reset the repeat if it's a different direction
				if (stickx < 0 && menu->xrepeatcount > 0) {
					menu->xrepeatcount = 0;
				}

				if (stickx > 0 && menu->xrepeatcount < 0) {
					menu->xrepeatcount = 0;
				}

				if (menu->xrepeatcount == 0) {
					menu->xrepeattimer60 = 0;
				}

				// Calculate the repeat interval based on the stick pressure
				if (absstickx > 70) {
					absstickx = 70;
				}

				absstickx -= xdeadzone;
				interval = stickintervalbase - xstickintervalmult * absstickx / (70 - xdeadzone);

				// After 3 repeats, halve the interval (ie. make faster)
				if (menu->xrepeatcount >= 3 || menu->xrepeatcount <= -3) {
					interval /= 2;
				}

				if (interval > 0) {
#if VERSION >= VERSION_PAL_BETA
					if (interval > 3) {
						interval = TICKS(interval);
					}
#endif

					oldslot = menu->xrepeattimer60 / interval;
					newslot = (menu->xrepeattimer60 + g_Vars.diffframe60) / interval;

					xhelddir = stickx < 0 ? -1 : 1;

					if (oldslot != newslot) {
						apply = true;
					}

					if (menu->xrepeatcount == 0) {
						apply = true;
					}

					if (apply) {
						menu->xrepeatcount += xhelddir;
					}
				}
			} else {
				menu->xrepeatcount = 0;
			}

			if (oldslot != newslot) {
				apply = true;
			}

			if (!apply) {
				xhelddir = 0;
			}
		}

		// Handle up/down repeat
		{
			s32 absstickx;
			s32 abssticky;
			s32 oldslot;
			s32 newslot;
			s32 interval;
			bool apply = false;

			if (ytapdir != 0) {
				// Direction was pressed this frame - reset the repeat properties
				apply = true;
				menu->yrepeatmode = MENUREPEATMODE_SLOW;
				menu->yrepeattimer60 = 0;
				menu->yrepeatdir = ytapdir;
			} else if (yhelddir != 0) {
				yhelddir = menu->yrepeatdir;
			}

			// If held for 1 second, repeat faster
			if (menu->yrepeattimer60 > TICKS(60)) {
				menu->yrepeatmode = MENUREPEATMODE_FAST;
			}

			// Calculate the old and new repeat slots.
			// If these are different, the repeat will be applied on this tick.
			oldslot = menu->yrepeattimer60 / digitalrepeatinterval;
			newslot = (menu->yrepeattimer60 + g_Vars.diffframe60) / digitalrepeatinterval;

			if (menu->yrepeatmode == MENUREPEATMODE_SLOW) {
				oldslot /= 2;
				newslot /= 2;
			}

			inputs.updownheld = yhelddir;

			// Check if the stick is being pushed up or down
			abssticky = sticky < 0 ? -sticky : sticky;
			absstickx = stickx < 0 ? -stickx : stickx;

			if (abssticky >= ydeadzone && (abssticky > absstickx || allowdiagonal)) {
				// Reset the repeat if it's a different direction
				if (sticky < 0 && menu->yrepeatcount < 0) {
					menu->yrepeatcount = 0;
				}

				if (sticky > 0 && menu->yrepeatcount > 0) {
					menu->yrepeatcount = 0;
				}

				if (menu->yrepeatcount == 0) {
					menu->yrepeattimer60 = 0;
				}

				// Calculate the repeat interval based on the stick pressure
				if (abssticky > 70) {
					abssticky = 70;
				}

				abssticky -= ydeadzone;
				interval = stickintervalbase - ystickintervalmult * abssticky / 50;

				// After 3 repeats, third the interval (ie. make faster)
				if (menu->yrepeatcount >= 3 || menu->yrepeatcount <= -3) {
					interval /= 3;
				}

				if (interval > 0) {
#if VERSION >= VERSION_PAL_BETA
					if (interval > 3) {
						interval = TICKS(interval);
					}
#endif

					oldslot = menu->yrepeattimer60 / interval;
					newslot = (menu->yrepeattimer60 + g_Vars.diffframe60) / interval;

					yhelddir = sticky > 0 ? -1 : 1;

					if (oldslot != newslot) {
						apply = true;
					}

					if (menu->yrepeatcount == 0) {
						apply = true;
					}

					if (apply) {
						menu->yrepeatcount += yhelddir;
					}
				}
			} else {
				menu->yrepeatcount = 0;
			}

			if (oldslot != newslot) {
				apply = true;
			}

			if (!apply) {
				yhelddir = 0;
			}
		}

		menu->xrepeattimer60 += g_Vars.diffframe60;
		menu->yrepeattimer60 += g_Vars.diffframe60;

		inputs.leftright = xhelddir;
		inputs.updown = yhelddir;
		inputs.xaxis = stickx;
		inputs.yaxis = sticky;
		inputs.unk14 = 0;
		inputs.start = starttap ? true : false;

#ifndef PLATFORM_N64
		// if we haven't been using the mouse but we have been keyboard scrolling disable the mouse
		if (!inputs.mousemoved && (inputs.leftright || inputs.updown || inputs.leftrightheld || inputs.updownheld)) {
			g_MenuUsingMouse = false;
		}
		// otherwise rotate left/right by clicking on the side of the menu
		if (g_MenuUsingMouse && inputs.select && !inputs.leftright && !inputs.leftrightheld) {
			if (inputs.mousey > dialog->y && inputs.mousey < dialog->y + dialog->height) {
				if (inputs.mousex < dialog->x) {
					inputs.leftright = -1;
					inputs.select = 0;
				} else if (inputs.mousex > dialog->x + dialog->width) {
					inputs.leftright = 1;
					inputs.select = 0;
				}
			}
		}
#endif

		// Handle dialogs that allow pressing start to select,
		// and handle pressing start on a list item.
		if (g_Menus[g_MpPlayerNum].curdialog && starttap) {
			struct menuitem *item;

			if (g_Menus[g_MpPlayerNum].curdialog->definition->flags & MENUDIALOGFLAG_STARTSELECTS) {
				inputs.select = true;
				starttoselect = true;
			}

			item = g_Menus[g_MpPlayerNum].curdialog->focuseditem;

			if (item && item->type == MENUITEMTYPE_LIST) {
				inputs.select = true;
			}
		}

		// Iterate all dialogs and give them the input for processing
		{
			bool foundcurrent = false;
			s32 i;
			s32 j;

			for (i = 0; i < g_Menus[g_MpPlayerNum].depth; i++) {
				struct menulayer *layer = &g_Menus[g_MpPlayerNum].layers[i];

				for (j = 0; j < layer->numsiblings; j++) {
					u32 tickflags = 0;

					if (i == g_Menus[g_MpPlayerNum].depth - 1 && j == layer->cursibling && !foundcurrent) {
						tickflags |= MENUTICKFLAG_DIALOGISCURRENT;
						foundcurrent = true;
					}

					dialogTick(layer->siblings[j], &inputs, tickflags);
				}
			}
		}

		switch (g_MenuData.root) {
		case MENUROOT_MPSETUP:
		case MENUROOT_4MBMAINMENU:
			// Allow pressing start on most MP setup dialogs to jump straight to
			// the Ready dialog, or apply the quick start setup.
			if (inputs.start && !starttoselect && g_Menus[g_MpPlayerNum].curdialog && !dialog->dimmed) {
				if (g_Menus[g_MpPlayerNum].curdialog->definition);

				if (g_Vars.mpsetupmenu != MPSETUPMENU_GENERAL
						&& g_Menus[g_MpPlayerNum].curdialog->definition != &g_MpReadyMenuDialog) {
					menuPushDialog(&g_MpReadyMenuDialog);
				} else if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_MpQuickTeamGameSetupMenuDialog) {
					func0f17f428();
				}
			}
			break;
		case MENUROOT_MPPAUSE:
			if (g_InCutscene) {
				func0f0f8120();
			}
			g_Menus[g_MpPlayerNum].openinhibit = 10;
			// fall-through
		case MENUROOT_ENDSCREEN:
		case MENUROOT_MAINMENU:
		case MENUROOT_MPENDSCREEN:
		case MENUROOT_TRAINING:
			if (inputs.start && !starttoselect && g_Menus[g_MpPlayerNum].curdialog
					&& (dialog->definition->flags & MENUDIALOGFLAG_IGNOREBACK) == 0) {
				func0f0f8120();
			}
			break;
		}
	}
}

Gfx *menugfxRenderBgFailureAlt(Gfx *gdl);

/**
 * Render layer 1 of the menu background.
 *
 * Layer 1 is drawn before the hud piece. Almost everything is in layer 1.
 *
 * frac is used when transitioning between two backgrounds.
 * A value of 1 means draw this background with full alpha.
 */
Gfx *menuRenderBackgroundLayer1(Gfx *gdl, u8 bg, f32 frac)
{
	static u32 bblur = 1;

	switch (bg) {
	case MENUBG_BLUR:
		{
			u32 alpha = 255 * frac;

			// Render the blurred background texture with full alpha
			gdl = menugfxRenderBgBlur(gdl, 0xffffff00 | alpha, 0, 0);

			// Render it twice more with half alpha and offset
			gdl = menugfxRenderBgBlur(gdl, 0xffffff00 | alpha >> 1, -BLUR_OFS, -BLUR_OFS);
			gdl = menugfxRenderBgBlur(gdl, 0xffffff00 | alpha >> 1, BLUR_OFS, BLUR_OFS);
		}
		break;
	case MENUBG_BLACK:
	case MENUBG_8:
		{
			u32 colour = 255 * frac;
			gSPDisplayList(gdl++, var800613a0);
			gdl = textSetPrimColour(gdl, colour);
			gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());
			gdl = text0f153838(gdl);
		}
		break;
	case MENUBG_SUCCESS:
		{
			// Fill with black
			gSPDisplayList(gdl++, var800613a0);
			gdl = textSetPrimColour(gdl, 0x000000ff);
			gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());
			gdl = text0f153838(gdl);

			// Render the success BG
			gdl = menugfxRenderBgSuccess(gdl);

			// Render alpha black if fading in
			{
				u32 alpha = (1.0f - frac) * 255;

				if (alpha) {
					gSPDisplayList(gdl++, var800613a0);
					gdl = textSetPrimColour(gdl, alpha);
					gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());
					gdl = text0f153838(gdl);
				}
			}
		}
		break;
	case MENUBG_FAILURE:
		{
			// Fill with white -> black while fading in
			u32 stack;
			u32 channel = (1.0f - frac) * 255;
			gSPDisplayList(gdl++, var800613a0);
			gdl = textSetPrimColour(gdl, channel << 24 | channel << 16 | channel << 8 | 0xff);
			gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());
			gdl = text0f153838(gdl);

			// Render the failure BG
			gdl = menugfxRenderBgFailure(gdl);
		}
		break;
	case MENUBG_CONEALPHA:
		mainOverrideVariable("bblur", &bblur);

		if (g_MenuData.screenshottimer) {
			return gdl;
		}

		if (bblur) {
			// Render the blurred background
			gdl = menugfxRenderBgBlur(gdl, 0xffffffff, 0, 0);

			// While fading, render red
			if (frac < 1.0f) {
				u32 alpha;
				u32 stack;

				gSPDisplayList(gdl++, var800613a0);
				alpha = (1.0f - frac) * 255;
				gdl = textSetPrimColour(gdl, 0xff000000 | alpha);
				gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());
				gdl = text0f153838(gdl);
			}
		}
		break;
	case MENUBG_GRADIENT:
		// Blue to red
		gdl = menugfxRenderGradient(gdl, 0, 0, viGetWidth(), viGetHeight(), 0x00007f7f, 0x000000ff, 0x8f0000ff);
		break;
	case MENUBG_CONEOPAQUE:
		// Yellow to yellow (ie. not a gradient)
		gdl = menugfxRenderGradient(gdl, 0, 0, viGetWidth(), viGetHeight(), 0x3f3f00ff, 0x7f0000ff, 0x3f3f00ff);
		break;
	}

	return gdl;
}

u32 var800714f0 = 1;

Gfx *menuRenderBackgroundLayer2(Gfx *gdl, u8 bg, f32 frac)
{
	if (bg == MENUBG_CONEALPHA || bg == MENUBG_CONEOPAQUE) {
		mainOverrideVariable("cone", &var800714f0);

		if (var800714f0
				&& (g_MenuData.nextbg == MENUBG_CONEALPHA || g_MenuData.nextbg == 0 || g_MenuData.nextbg == 255)) {
			gdl = menugfxRenderBgCone(gdl);
		}
	}

	return gdl;
}

Gfx *menuRender(Gfx *gdl)
{
	static u32 usepiece = 1;

	g_MpPlayerNum = 0;

#if PAL
	g_ScaleX = 1;
#else
	g_ScaleX = g_ViRes == VIRES_HI ? 2 : 1;
#endif

	gdl = func0f0d479c(gdl);

	gSPDisplayList(gdl++, var800613a0);

	// Render the background
	if (g_MenuData.nextbg != 255) {
		if (g_MenuData.nextbg == 0) {
			gdl = menuRenderBackgroundLayer1(gdl, g_MenuData.bg, 1.0f - g_MenuData.unk010);
		} else {
			gdl = menuRenderBackgroundLayer1(gdl, g_MenuData.bg, 1.0f);
			gdl = menuRenderBackgroundLayer1(gdl, g_MenuData.nextbg, g_MenuData.unk010);
		}
	} else {
		gdl = menuRenderBackgroundLayer1(gdl, g_MenuData.bg, 1.0f);
	}

#ifndef PLATFORM_N64
	gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
#endif

	// Calculate hudpiece things then render it
	if (g_MenuData.unk5d5_05) {
		g_MenuData.hudpiece.curanimnum = 0;
		g_MenuData.hudpiece.newanimnum = ANIM_040D;
		g_MenuData.hudpiece.removingpiece = false;
		g_MenuData.hudpiece.reverseanim = false;
		g_MenuData.unk5d4 = 1;
		g_MenuData.unk5d5_05 = false;
	}

	if (IS8MB() && g_MenuData.unk5d4) {
		bool removepiece = false;

		gSPSetGeometryMode(gdl++, G_ZBUFFER);

		// Everyone 1 in 100 frames on average, calculate a new X/Y for the hudpiece
		// Note: unintentional 64-bit float comparison done here
		if (RANDOMFRAC() < 0.01) {
			g_MenuData.hudpiece.newposx = RANDOMFRAC() * 80.0f + -205.5f - 40.0f;
			g_MenuData.hudpiece.newposy = RANDOMFRAC() * 80.0f + 244.7f - 40.0f;
		}

		g_MenuProjectFromX = g_MenuProjectFromY = 0;

		if (g_MenuData.root == MENUROOT_MPSETUP) {
			if (g_MenuData.count <= 0) {
				removepiece = true;
			}
		}

		if (g_MenuData.root != MENUROOT_MAINMENU
				&& g_MenuData.root != MENUROOT_MPSETUP
				&& g_MenuData.root != MENUROOT_FILEMGR
				&& g_MenuData.root != MENUROOT_TRAINING) {
			removepiece = true;
		}

		if (g_Menus[g_MpPlayerNum].curdialog == NULL) {
			if (g_MenuData.root != MENUROOT_MPSETUP) {
				removepiece = true;
			}
		}

		if (g_Vars.currentplayer->eyespy
				&& (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)
				&& g_Vars.currentplayer->eyespy->active) {
			removepiece = true;
		}

		if (removepiece) {
			if (!g_MenuData.hudpiece.removingpiece) {
				g_MenuData.hudpiece.reverseanim = true;
				g_MenuData.hudpiece.curanimnum = 0;
				g_MenuData.hudpiece.newanimnum = ANIM_040D;
				g_MenuData.hudpiece.removingpiece = true;
			} else if (g_MenuData.hudpiece.curanimnum == 0) {
				g_MenuData.hudpiece.removingpiece = false;
				g_MenuData.unk5d4 = 0;
			}
		}

		mainOverrideVariable("usePiece", &usepiece);

		if (usepiece) {
			g_MenuData.usezbuf = false;

			gdl = menuRenderModel(gdl, &g_MenuData.hudpiece, MENUMODELTYPE_HUDPIECE);
			gSPClearGeometryMode(gdl++, G_ZBUFFER);

			g_MenuData.usezbuf = true;
		}
	} else {
		g_MenuProjectFromX = g_MenuProjectFromY = 0;
	}

	if (g_MenuData.unk5d5_04) {
		g_MenuProjectFromX = g_MenuData.unk670;
		g_MenuProjectFromY = g_MenuData.unk674;
	}

	// Render the second layer of the background (for the combat simulator cone,
	// which draws over the top of the hud piece)
	if (g_MenuData.nextbg != 255) {
		if (g_MenuData.nextbg == 0) {
			gdl = menuRenderBackgroundLayer2(gdl, g_MenuData.bg, 1.0f - g_MenuData.unk010);
		} else {
			gdl = menuRenderBackgroundLayer2(gdl, g_MenuData.bg, 1.0f);
			gdl = menuRenderBackgroundLayer2(gdl, g_MenuData.nextbg, g_MenuData.unk010);
		}
	} else {
		gdl = menuRenderBackgroundLayer2(gdl, g_MenuData.bg, 1.0f);
	}

	// Render the health bar (playerRenderHealthBar may choose not to render)
	if ((g_MenuData.bg || g_MenuData.nextbg != 255)
			&& (!g_Vars.currentplayer->eyespy || !g_Vars.currentplayer->eyespy->active)) {
		gdl = func0f0d49c8(gdl);
#ifndef PLATFORM_N64
		gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
#endif
		gdl = playerRenderHealthBar(gdl);
#ifndef PLATFORM_N64
		gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
#endif
		gdl = func0f0d479c(gdl);
	}

	if (g_MenuData.count > 0) {
		// Render dialogs
		gdl = text0f153ab0(gdl);

		if (g_MenuData.root == MENUROOT_MPPAUSE || g_MenuData.root == MENUROOT_MPENDSCREEN) {
			g_MpPlayerNum = g_Vars.currentplayerstats->mpindex;
			gdl = menuRenderDialogs(gdl);
		} else {
			s32 i;

			for (i = 0; i < MAX_PLAYERS; i++) {
				g_MpPlayerNum = i;
				gdl = menuRenderDialogs(gdl);
			}
		}

		g_MpPlayerNum = 0;

		gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
		gSPDisplayList(gdl++, var800613a0);

		text0f153b40();

		// Render corner texts in combat simulator
		if (g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU) {
			s32 i;
			s32 j;
			s32 viewleft = viGetViewLeft() / g_ScaleX + 20;
			s32 viewtop = viGetViewTop() + 4;
			s32 viewright = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX - 20;
			s32 viewbottom = viGetViewTop() + viGetViewHeight() - 4;
			s32 textheight;
			s32 textwidth;
			bool renderit;
			char text[32];
			s32 tmp1;
			s32 tmp2;
			s32 x;
			s32 y;
			s32 colour;

			gdl = text0f153628(gdl);

			for (i = 0; i < MAX_PLAYERS; i++) {
				// Figure out what text will be displayed. The text calculated
				// here is for measuring purposes only and isn't rendered.
				// Amusingly, there's a %d placeholder in the text which isn't
				// replaced prior to measuring, so the width is slightly wrong.
				if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL && g_Vars.waitingtojoin[i]) {
					// Player has pressed start but they can't open the player-specific
					// dialog yet because they're still on the Combat Simulator dialog
					// or similar. Show "Ready" in their corner.
					renderit = true;
					// "Player %d: " and "Ready!"
					sprintf(text, "%s%s", langGet(L_MPMENU_482), langGet(L_MISC_461));
				} else {
					if (g_MenuData.root == MENUROOT_4MBMAINMENU) {
						if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL) {
							renderit = true;

							for (j = 0; j < ARRAYCOUNT(g_Vars.waitingtojoin); j++) {
								if (g_Vars.waitingtojoin[j]) {
									renderit = false;
								}
							}
						} else {
							renderit = g_MpNumJoined < 2;
						}
					} else {
						renderit = true;
					}

					// "Player %d: " and "Press START!"
					sprintf(text, "%s%s", langGet(L_MPMENU_482), langGet(L_MPMENU_483));
				}

				if (renderit) {
					textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

					// Check which controllers are connected
					// and update the alpha of the label
					if (((g_MpSetup.chrslots | ~joyGetConnectedControllers()) & (1 << i)) == 0) {
#if VERSION >= VERSION_PAL_BETA
						tmp1 = g_Vars.diffframe60freal * 3;
#else
						tmp1 = g_Vars.diffframe60 * 3;
#endif

						if (g_MenuData.playerjoinalpha[i] < 255) {
							if (255 - g_MenuData.playerjoinalpha[i] > tmp1) {
								g_MenuData.playerjoinalpha[i] += tmp1;
							} else {
								g_MenuData.playerjoinalpha[i] = 255;
							}
						}
					} else {
#if VERSION >= VERSION_PAL_BETA
						tmp2 = g_Vars.diffframe60freal * 9;
#else
						tmp2 = g_Vars.diffframe60 * 9;
#endif

						if (g_MenuData.playerjoinalpha[i] > 0) {
							if (g_MenuData.playerjoinalpha[i] > tmp2) {
								g_MenuData.playerjoinalpha[i] -= tmp2;
							} else {
								g_MenuData.playerjoinalpha[i] = 0;
							}
						}
					}

					if (g_MenuData.playerjoinalpha[i] > 0) {
						u32 weight = menuGetSinOscFrac(20) * 255.0f;

						// "Player %d: "
						sprintf(text, langGet(L_MPMENU_482), i + 1);

						if (i < 2) {
							y = viewtop + 2;
						} else {
							y = viewbottom - 9;
						}

						if (i == 1 || i == 3) {
							x = viewright - textwidth - 2;
						} else {
							x = viewleft + 2;
						}

						gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, g_MenuData.playerjoinalpha[i] | 0x5070ff00, viGetWidth(), viGetHeight(), 0, 0);

						if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL && g_Vars.waitingtojoin[i]) {
							// "Ready!"
#if VERSION >= VERSION_JPN_FINAL
							colour = L_MISC_461;
							strcpy(text, langGet(colour));
							colour = 0xffffffff;
#else
							strcpy(text, langGet(L_MISC_461));
							colour = g_MenuData.playerjoinalpha[i] | 0xd00020ff;
#endif
						} else {
							// "Press START!"
							strcpy(text, langGet(L_MPMENU_483));
							colour = colourBlend(0x00ffff00, 0xffffff00, weight) | g_MenuData.playerjoinalpha[i];
						}

						gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, viGetWidth(), viGetHeight(), 0, 0);
					}
				}
			}

			gdl = text0f153780(gdl);
		}

		gSPSetGeometryMode(gdl++, G_ZBUFFER);
	}

	// Render banner messages, such as "Please Wait...",
	// "Checking Controller Pak" and some unused game boy camera texts.
	if (g_MenuData.bannernum != -1) {
		s32 x1 = viGetViewLeft() / g_ScaleX;
		s32 y1 = viGetViewTop();
		s32 x2 = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX;
		s32 y2 = viGetViewTop() + viGetViewHeight();

#if VERSION >= VERSION_NTSC_1_0
		s32 left = 0;
		s32 right = 0;

		if (PLAYERCOUNT() >= 3) {
			if (g_Vars.currentplayernum == 1 || g_Vars.currentplayernum == 3) {
				right = 15;
			} else {
				left = 15;
			}
		}

		if (PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || IS4MB())) {
			if (g_Vars.currentplayernum == 1) {
				right = 15;
			} else {
				left = 15;
			}
		}

		gdl = menuRenderBanner(gdl, x1, y1, x2, y2, PLAYERCOUNT() < 2, g_MenuData.bannernum, left, right);
#else
		if (PLAYERCOUNT() >= 3) {
			if (g_Vars.currentplayernum == 1 || g_Vars.currentplayernum == 3) {
				x2 -= 10;
			} else {
				x1 += 10;
			}
		}

		if (PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || IS4MB())) {
			if (g_Vars.currentplayernum == 1) {
				x2 -= 10;
			} else {
				x1 += 10;
			}
		}

		gdl = menuRenderBanner(gdl, x1, y1, x2, y2, PLAYERCOUNT() < 2, g_MenuData.bannernum);
#endif
	}

#ifndef PLATFORM_N64
	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);
#endif

	gdl = func0f0d49c8(gdl);

	g_ScaleX = 1;

	return gdl;
}

const char var7f1b27a4[] = "Tune Selector - mode %d\n";

u32 menuChooseMusic(void)
{
	s32 missionsuccess = MUSIC_MISSION_SUCCESS;

	if (g_StageIndex == STAGEINDEX_DEFENSE) {
		missionsuccess = MUSIC_MISSION_UNKNOWN;
	}

	if (g_MenuData.root == MENUROOT_ENDSCREEN) {
		if (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()) {
			return MUSIC_MISSION_FAILED;
		}

		return missionsuccess;
	}

	if (g_MenuData.root == MENUROOT_MPENDSCREEN) {
		if (g_Vars.coopplayernum >= 0) {
			if ((g_Vars.bond->isdead && g_Vars.coop->isdead)
					|| g_Vars.bond->aborted
					|| g_Vars.coop->aborted
					|| !objectiveIsAllComplete()) {
				return MUSIC_MISSION_FAILED;
			}

			return missionsuccess;
		}

		if (g_Vars.antiplayernum >= 0) {
			if (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()) {
				return MUSIC_MISSION_FAILED;
			}

			return missionsuccess;
		}
	}

	if (g_MenuData.root == MENUROOT_FILEMGR) {
		return MUSIC_MAINMENU;
	}

	if (g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU) {
		return MUSIC_COMBATSIM_MENU;
	}

	if (g_MenuData.root == MENUROOT_MPPAUSE) {
		return MUSIC_COMBATSIM_COMPLETE;
	}

	if (g_MenuData.root == MENUROOT_BOOTPAKMGR) {
		return MUSIC_MISSION_UNKNOWN;
	}

	if (g_Vars.mplayerisrunning) {
		return MUSIC_COMBATSIM_COMPLETE;
	}

	if (g_Vars.stagenum == STAGE_CITRAINING || g_Vars.stagenum == STAGE_4MBMENU) {
		return MUSIC_MAINMENU;
	}

	return MUSIC_PAUSEMENU;
}

bool func0f0fcbcc(void)
{
	if (g_FileState == FILESTATE_UNSELECTED && g_Vars.stagenum == STAGE_CITRAINING) {
		return true;
	}

	return false;
}

bool func0f0fcc04(void)
{
	return false;
}

u32 menuGetRoot(void)
{
	if (g_MenuData.count == 0) {
		return 0;
	}

	return g_MenuData.root;
}

#if VERSION >= VERSION_NTSC_1_0
struct menudialogdef g_PakAttemptRepairMenuDialog;

MenuItemHandlerResult menuhandler000fcc34(s32 operation, struct menuitem *item, union handlerdata *data)
{
	bool done = false;

	if (operation == MENUOP_SET) {
		while (!done) {
			done = true;

			if (g_Menus[g_MpPlayerNum].curdialog) {
				if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakRepairSuccessMenuDialog
						|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakRemovedMenuDialog
						|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakRepairFailedMenuDialog
						|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakAttemptRepairMenuDialog
						|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakDamagedMenuDialog
						|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_PakFullMenuDialog) {
					done = false;
					menuPopDialog();
				}
			}
		}
	}

	return 0;
}
#endif

MenuDialogHandlerResult menudialog000fcd48(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog
				&& g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
				&& joyGetPakState(g_Menus[g_MpPlayerNum].fm.device3) == PAKSTATE_NOPAK) {
			func0f0f3704(&g_PakRemovedMenuDialog);
		}
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult func0f0fcdd0(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f3704(&g_PakDamagedMenuDialog);
	}

	return 0;
}
#endif

MenuItemHandlerResult menuhandlerRepairPak(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (pakRepair(g_Menus[g_MpPlayerNum].fm.device3)) {
			func0f0f3704(&g_PakRepairSuccessMenuDialog);
		} else {
			func0f0f3704(&g_PakRepairFailedMenuDialog);
		}
	}

	return 0;
}

void menuPushPakDialogForPlayer(struct menudialogdef *dialogdef, s32 playernum, s32 paknum)
{
	s32 prevplayernum = g_MpPlayerNum;

	g_MpPlayerNum = playernum;
	g_Menus[g_MpPlayerNum].fm.device3 = paknum;

	if (g_Menus[g_MpPlayerNum].curdialog == NULL) {
		if (PLAYERCOUNT() == 1) {
			menuPushRootDialog(dialogdef, MENUROOT_MAINMENU);
			lvSetPaused(true);
			g_Vars.currentplayer->pausemode = PAUSEMODE_PAUSED;
		} else {
			menuPushRootDialog(dialogdef, MENUROOT_MPPAUSE);
		}
	} else {
		menuPushDialog(dialogdef);
	}

	g_MpPlayerNum = prevplayernum;
}

struct menuitem g_PakRemovedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_174, // "The Controller Pak has been removed."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
#if VERSION >= VERSION_NTSC_1_0
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		menuhandler000fcc34,
#else
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		NULL,
#endif
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakRemovedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_173, // "Error"
	g_PakRemovedMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_PakRepairSuccessMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_181, // "The Controller Pak has been repaired."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
#if VERSION >= VERSION_NTSC_1_0
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		menuhandler000fcc34,
#else
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		NULL,
#endif
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakRepairSuccessMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPWEAPONS_180, // "Repair Successful"
	g_PakRepairSuccessMenuItems,
	menudialog000fcd48,
	0,
	NULL,
};

struct menuitem g_PakRepairFailedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_183, // "The Controller Pak  cannot be repaired. You will not be able to load from or save to this Controller Pak."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakRepairFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_182, // "Repair Failed"
	g_PakRepairFailedMenuItems,
	menudialog000fcd48,
	0,
	NULL,
};

struct menuitem g_PakAttemptRepairMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_176, // "Are you sure you want to attempt repair of this Controller Pak?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_177, // "Data may be lost!"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_178, // "Cancel"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_179, // "Repair"
		0,
		menuhandlerRepairPak,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakAttemptRepairMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_175, // "Attempt Repair"
	g_PakAttemptRepairMenuItems,
	menudialog000fcd48,
	0,
	NULL,
};

char *menuTextSaveDeviceName(struct menuitem *item)
{
	u16 devices[] = {
		L_OPTIONS_112, // "Controller Pak 1"
		L_OPTIONS_113, // "Controller Pak 2"
		L_OPTIONS_114, // "Controller Pak 3"
		L_OPTIONS_115, // "Controller Pak 4"
#if VERSION >= VERSION_NTSC_1_0
		L_OPTIONS_111, // "Game Pak"
#endif
	};

	if ((u8)g_Menus[g_MpPlayerNum].fm.device3 < ARRAYCOUNT(devices)) {
		return langGet(devices[(u8)g_Menus[g_MpPlayerNum].fm.device3]);
	}

	return NULL;
}

MenuItemHandlerResult menuhandlerRetrySavePak(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();

#if VERSION >= VERSION_NTSC_1_0
		g_Vars.pakstocheck &= 0xfff0;
		g_Vars.pakstocheck |= 0x0008;
		g_Vars.pakstocheck |= 1 << ((u8)g_Menus[g_MpPlayerNum].fm.device3 + 8);
#else
		pak0f1169c8(g_Menus[g_MpPlayerNum].fm.device3, false);
#endif
	}

	return 0;
}

MenuItemHandlerResult menuhandlerWarnRepairPak(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
#if VERSION >= VERSION_NTSC_1_0
		menuPushDialog(&g_PakAttemptRepairMenuDialog);
#else
		func0f0f3704(&g_PakAttemptRepairMenuDialog);
#endif
	}

	return 0;
}

s32 menuPakNumToPlayerNum(s32 paknum)
{
	u32 result = 0;

	if (g_Vars.normmplayerisrunning) {
		if (g_MpSetup.chrslots & (1 << paknum)) {
			result = paknum;
		}
	} else {
		if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
				&& PLAYERCOUNT() >= 2
				&& paknum == 1) {
			result = 1;
		}
	}

	return result;
}

bool menuIsReadyForPakError(s32 paknum, s32 pakerrordialog)
{
	s32 playernum = menuPakNumToPlayerNum(paknum);
	bool result = true;

	if (g_Vars.lvframenum < 20) {
		result = false;
	}

	if (g_FileState == FILESTATE_UNSELECTED) {
		result = false;
	}

	if (g_Vars.stagenum == STAGE_BOOTPAKMENU) {
		result = true;
	}

	if (g_Menus[playernum].curdialog) {
		if (g_Menus[playernum].curdialog->definition == &g_PakDamagedMenuDialog
#if VERSION >= VERSION_NTSC_1_0
				|| g_Menus[playernum].curdialog->definition == &g_PakCannotReadGameBoyMenuDialog
				|| g_Menus[playernum].curdialog->definition == &g_PakDataLostMenuDialog
#endif
				|| g_Menus[playernum].curdialog->definition == &g_PakFullMenuDialog
				|| g_Menus[playernum].curdialog->definition == &g_PakAttemptRepairMenuDialog
				|| g_Menus[playernum].curdialog->definition == &g_PakRemovedMenuDialog
				|| g_Menus[playernum].curdialog->definition == &g_PakRepairSuccessMenuDialog
				|| g_Menus[playernum].curdialog->definition == &g_PakRepairFailedMenuDialog) {
			result = false;
		}
	} else if (g_MenuData.nextbg != 255 || g_MenuData.bg || g_MenuData.unk5d4) {
		result = false;
	}

	return result;
}

void menuPushPakErrorDialog(s32 paknum, s32 pakerrordialog)
{
	s32 prevplayernum = g_MpPlayerNum;
	s32 playernum = menuPakNumToPlayerNum(paknum);
	bool found;
	s32 i;

	g_MpPlayerNum = playernum;

	switch (pakerrordialog) {
	case PAKERRORDIALOG_CORRUPT:
	case PAKERRORDIALOG_DEVICEERROR:
		menuPushPakDialogForPlayer(&g_PakDamagedMenuDialog, playernum, paknum);
		break;
	case PAKERRORDIALOG_FULL:
		found = false;

		for (i = 0; i < g_Menus[g_MpPlayerNum].depth; i++) {
			if (g_Menus[g_MpPlayerNum].layers[i].siblings[0]
					&& g_Menus[g_MpPlayerNum].layers[i].siblings[0]->definition == &g_PakChoosePakMenuDialog) {
				found = true;
			}
		}

		if (!found) {
			menuPushPakDialogForPlayer(&g_PakFullMenuDialog, playernum, paknum);
		}
		break;
#if VERSION >= VERSION_NTSC_1_0
	case PAKERRORDIALOG_GB_UNREADABLE:
		menuPushPakDialogForPlayer(&g_PakCannotReadGameBoyMenuDialog, playernum, paknum);
		break;
	case PAKERRORDIALOG_DATALOST:
		menuPushPakDialogForPlayer(&g_PakDataLostMenuDialog, playernum, paknum);
		break;
#endif
	}

	g_MpPlayerNum = prevplayernum;
}

void func0f0fd494(struct coord *pos)
{
	f32 xy[2];
	struct coord coord;
	Mtxf *matrix;

	g_MenuData.unk5d5_04 = true;

	matrix = camGetWorldToScreenMtxf();

	mtx4TransformVec(matrix, pos, &coord);
	cam0f0b4d04(&coord, xy);

	g_MenuData.unk670 = (s32)xy[0] - viGetWidth() / 2;
	g_MenuData.unk674 = (s32)xy[1] - viGetHeight() / 2;

	g_MenuData.unk5d5_05 = false;
}

void func0f0fd548(s32 arg0)
{
	g_MenuData.unk669[g_MenuData.unk66e++] = arg0;
	g_MenuData.unk66f = 0;
}

struct menudialog *menuIsDialogOpen(struct menudialogdef *dialogdef)
{
	s32 i;
	s32 j;

	if (g_Menus[g_MpPlayerNum].curdialog) {
		for (i = 0; i < g_Menus[g_MpPlayerNum].depth; i++) {
			for (j = 0; j < g_Menus[g_MpPlayerNum].layers[i].numsiblings; j++) {
				if (g_Menus[g_MpPlayerNum].layers[i].siblings[j]
						&& g_Menus[g_MpPlayerNum].layers[i].siblings[j]->definition == dialogdef) {
					return g_Menus[g_MpPlayerNum].layers[i].siblings[j];
				}
			}
		}
	}

	return false;
}

struct menuitem g_PakDamagedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&menuTextSaveDeviceName,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_065, // "is damaged or"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_066, // "inserted incorrectly."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_067, // "Attempt Repair"
		0,
		menuhandlerWarnRepairPak,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_068, // "Retry"
		0,
		menuhandlerRetrySavePak,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPWEAPONS_069, // "Continue without using the Controller Pak"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakDamagedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_064, // "Damaged Controller Pak"
	g_PakDamagedMenuItems,
	menudialog000fcd48,
#if VERSION >= VERSION_NTSC_1_0
	0x00000020,
#else
	0,
#endif
	NULL,
};

struct menuitem g_PakFullMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&menuTextSaveDeviceName,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_071, // "is too full to save note - 1 note and 28 pages required to save."
		0,
		NULL,
	},
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_LESSHEIGHT,
		L_OPTIONS_003, // ""
		0,
		NULL,
	},
#endif
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_072, // "Enter the Controller Pak Menu to free some space (hold START while powering up.)"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_073, // "OK"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakFullMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_070, // "Full Controller Pak"
	g_PakFullMenuItems,
	menudialog000fcd48,
#if VERSION >= VERSION_NTSC_1_0
	0x00000020,
#else
	0,
#endif
	NULL,
};

#if VERSION >= VERSION_NTSC_1_0
struct menuitem g_PakCannotReadGameBoyMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_254, // "Cannot read Game Boy Game Pak. Check connections and make sure correct Game Boy Game Pak is being used."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPWEAPONS_255, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakCannotReadGameBoyMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_253, // "Error"
	g_PakCannotReadGameBoyMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

struct menuitem g_PakDataLostMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&menuTextSaveDeviceName,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_257, // "The saved data has"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_258, // "been erased due to"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		L_MPWEAPONS_259, // "corruption or damage."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPWEAPONS_260, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakDataLostMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPWEAPONS_256, // "Error"
	g_PakDataLostMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};
#endif
