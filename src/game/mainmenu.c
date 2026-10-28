#include <ultra64.h>
#include <stdlib.h>
#include "constants.h"
#include "game/bondgun.h"
#include "game/bossfile.h"
#include "game/challenge.h"
#include "game/cheats.h"
#include "game/debug.h"
#include "game/filemgr.h"
#include "game/game_0b0fd0.h"
#include "game/game_1531a0.h"
#include "game/gamefile.h"
#include "game/inv.h"
#include "game/lang.h"
#include "game/lv.h"
#include "game/mainmenu.h"
#include "game/menu.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/mplayer.h"
#include "game/objectives.h"
#include "game/options.h"
#include "game/pdmode.h"
#include "game/player.h"
#include "game/setup.h"
#include "game/tex.h"
#include "game/title.h"
#include "game/training.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/str.h"
#include "data.h"
#include "types.h"

u8 g_InventoryWeapon;

struct menudialogdef g_2PMissionControlStyleMenuDialog;
struct menudialogdef g_CiControlPlayer2MenuDialog;
struct menudialogdef g_CinemaMenuDialog;
#ifndef PLATFORM_N64
extern struct menudialogdef g_ExtendedMenuDialog;
#endif

char *menuTextCurrentStageName(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s\n", langGet(g_SoloStages[g_MissionConfig.stageindex].name3));
	return g_StringPointer;
}

char *soloMenuTextDifficulty(struct menuitem *item)
{
#if VERSION >= VERSION_NTSC_1_0
	if (g_MissionConfig.pdmode) {
		return langGet(L_MPWEAPONS_221);
	}
#endif

	switch (g_MissionConfig.difficulty) {
	case DIFF_SA:
		return langGet(L_OPTIONS_252);
	case DIFF_PA:
		return langGet(L_OPTIONS_253);
	case DIFF_A:
	default:
		return langGet(L_OPTIONS_251);
	}
}

u16 g_ControlStyleOptions[] = {
	L_OPTIONS_239, // "1.1"
	L_OPTIONS_240, // "1.2"
	L_OPTIONS_241, // "1.3"
	L_OPTIONS_242, // "1.4"
	L_OPTIONS_243, // "2.1"
	L_OPTIONS_244, // "2.2"
	L_OPTIONS_245, // "2.3"
	L_OPTIONS_246, // "2.4"
};

MenuItemHandlerResult menuhandlerControlStyleImpl(s32 operation, struct menuitem *item, union handlerdata *data, s32 mpindex)
{
	u16 categories[] = {
		L_OPTIONS_237, // "Single"
		L_OPTIONS_238, // "Double"
	};

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpindex = g_Vars.currentplayerstats->mpindex;
	}

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = 9;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 3;
		break;
	case MENUOP_GETOPTIONTEXT:
		if (data->list.value > 7) {
			return (uintptr_t) "Ext";
		} else {
			return (uintptr_t) langGet(g_ControlStyleOptions[data->list.value]);
		}
	case MENUOP_GETOPTGROUPTEXT:
		if (data->list.value > 1) {
			return (uintptr_t) "Port";
		} else {
			return (uintptr_t) langGet(categories[data->list.value]);
		}
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = data->list.value * 4;
		break;
	case MENUOP_SET:
		optionsSetControlMode(mpindex, data->list.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
#ifndef PLATFORM_N64
		g_PlayerExtCfg[mpindex & 3].extcontrols = (data->list.value == CONTROLMODE_PC);
#endif
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = optionsGetControlMode(mpindex);
		g_Menus[g_MpPlayerNum].main.mpindex = mpindex;
		break;
	case MENUOP_LISTITEMFOCUS:
		if (g_MenuData.root == MENUROOT_MAINMENU) {
			g_Menus[g_MpPlayerNum].main.controlmode = data->list.value;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandler001024dc(s32 operation, struct menuitem *item, union handlerdata *data)
{
	return menuhandlerControlStyleImpl(operation, item, data, 4);
}

MenuItemHandlerResult menuhandler001024fc(s32 operation, struct menuitem *item, union handlerdata *data)
{
	return menuhandlerControlStyleImpl(operation, item, data, 5);
}

MenuItemHandlerResult menuhandlerReversePitch(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return !optionsGetForwardPitch(mpchrnum);
	case MENUOP_SET:
		optionsSetForwardPitch(mpchrnum, data->checkbox.value == 0);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAimControl(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 playernum = (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
		? g_Vars.currentplayerstats->mpindex : item->param3;

#if VERSION >= VERSION_PAL_FINAL
	s32 index = 0;

	u16 options[2][2] = {
		{ L_OPTIONS_201,   L_OPTIONS_202   }, // "Hold", "Toggle"
		{ L_MPWEAPONS_276, L_MPWEAPONS_277 }, // "Hold", "Toggle"
	};

	if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL && PLAYERCOUNT() >= 2) {
		index = 1;
	}
#else
	u16 options[] = {
		L_OPTIONS_201, // "Hold"
		L_OPTIONS_202, // "Toggle"
	};
#endif

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
#if VERSION >= VERSION_PAL_FINAL
		return (uintptr_t) langGet(options[index][data->dropdown.value]);
#else
		return (uintptr_t) langGet(options[data->dropdown.value]);
#endif
	case MENUOP_SET:
		optionsSetAimControl(playernum, data->dropdown.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetAimControl(playernum);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerSoundMode(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 options[] = {
		L_OPTIONS_232, // "Mono"
		L_OPTIONS_233, // "Stereo"
		L_OPTIONS_234, // "Headphone"
		L_OPTIONS_235, // "Surround"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 4;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(options[data->dropdown.value]);
	case MENUOP_SET:
		sndSetSoundMode(data->dropdown.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_SoundMode;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerScreenSize(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 options[] = {
		L_OPTIONS_220, // "Full"
		L_OPTIONS_221, // "Wide"
		L_OPTIONS_222, // "Cinema"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 3;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(options[data->dropdown.value]);
	case MENUOP_SET:
		optionsSetScreenSize(data->dropdown.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetEffectiveScreenSize();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerScreenRatio(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 options[] = {
		L_OPTIONS_223, // "Normal"
		L_OPTIONS_224, // "16:9"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(options[data->dropdown.value]);
	case MENUOP_SET:
		optionsSetScreenRatio(data->dropdown.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetScreenRatio();
	}

	return 0;
}

#if PAL
MenuItemHandlerResult menuhandlerLanguage(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 labels[] = {
		L_MPWEAPONS_262, // English
		L_MPWEAPONS_263, // French
		L_MPWEAPONS_264, // German
		L_MPWEAPONS_265, // Italian
		L_MPWEAPONS_266, // Spanish
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 5;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t)langGet(labels[data->dropdown.value]);
	case MENUOP_SET:
		g_Vars.language = data->dropdown.value;
		langSetEuropean(g_Vars.language);
		g_Vars.modifiedfiles |= MODFILE_GAME | MODFILE_BOSS;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_Vars.language;

		if (data->dropdown.value > LANGUAGE_PAL_ES) {
			data->dropdown.value = LANGUAGE_PAL_EN;
		}
		break;
	}

	return 0;
}
#endif

MenuItemHandlerResult menuhandlerScreenSplit(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 options[] = {
		L_OPTIONS_225, // "Horizontal"
		L_OPTIONS_226, // "Vertical"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(options[data->dropdown.value]);
	case MENUOP_SET:
		if (data->dropdown.value != (u32)optionsGetScreenSplit()) {
			optionsSetScreenSplit(data->dropdown.value);

			g_Vars.modifiedfiles |= MODFILE_GAME;

			if (PLAYERCOUNT() > 1) {
				u32 prevplayernum = g_MpPlayerNum;
				g_MpPlayerNum = 0;
				func0f0f8120();
				g_MpPlayerNum = 1;
				func0f0f8120();
				g_MpPlayerNum = prevplayernum;
			}
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetScreenSplit();
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerLookAhead(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetLookAhead(mpchrnum);
	case MENUOP_SET:
		optionsSetLookAhead(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerHeadRoll(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetHeadRoll(mpchrnum);
	case MENUOP_SET:
		optionsSetHeadRoll(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerInGameSubtitles(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return optionsGetInGameSubtitles();
	case MENUOP_SET:
		optionsSetInGameSubtitles(data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerCutsceneSubtitles(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return optionsGetCutsceneSubtitles();
	case MENUOP_SET:
		optionsSetCutsceneSubtitles(data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAlternativeTitle(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		if (g_Vars.stagenum != STAGE_CITRAINING || (u8)g_AltTitleUnlocked == false) {
			return true;
		}
		break;
	case MENUOP_GET:
		return g_AltTitleEnabled;
	case MENUOP_SET:
		g_AltTitleEnabled = data->checkbox.value;
		g_Vars.modifiedfiles |= MODFILE_BOSS;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerHiRes(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		if (IS4MB()) {
			return true;
		}
		if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
			if (PLAYERCOUNT() >= 2) {
				return true;
			}
		}
		break;
	case MENUOP_GET:
		return g_HiResEnabled == true;
	case MENUOP_SET:
		playerSetHiResEnabled(data->checkbox.value ? 1 : 0);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAmmoOnScreen(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetAmmoOnScreen(mpchrnum);
	case MENUOP_SET:
		optionsSetAmmoOnScreen(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerShowGunFunction(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_CHECKDISABLED:
		if (optionsGetAmmoOnScreen(mpchrnum) == 0) {
			return true;
		}
		break;
	case MENUOP_GET:
		return optionsGetShowGunFunction(mpchrnum);
	case MENUOP_SET:
		optionsSetShowGunFunction(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerShowMissionTime(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetShowMissionTime(mpchrnum);
	case MENUOP_SET:
		optionsSetShowMissionTime(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAlwaysShowTarget(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_CHECKDISABLED:
		if (optionsGetSightOnScreen(mpchrnum) == 0) {
			return true;
		}
		break;
	case MENUOP_GET:
		return optionsGetAlwaysShowTarget(mpchrnum);
	case MENUOP_SET:
		optionsSetAlwaysShowTarget(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerShowZoomRange(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_CHECKDISABLED:
		if (optionsGetSightOnScreen(mpchrnum) == 0) {
			return true;
		}
		break;
	case MENUOP_GET:
		return optionsGetShowZoomRange(mpchrnum);
	case MENUOP_SET:
		optionsSetShowZoomRange(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerPaintball(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetPaintball(mpchrnum);
	case MENUOP_SET:
		optionsSetPaintball(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerSightOnScreen(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetSightOnScreen(mpchrnum);
	case MENUOP_SET:
		optionsSetSightOnScreen(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAutoAim(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 mpchrnum;

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mpchrnum = g_Vars.currentplayerstats->mpindex;
	} else {
		mpchrnum = item->param3;
	}

	switch (operation) {
	case MENUOP_GET:
		return optionsGetAutoAim(mpchrnum);
	case MENUOP_SET:
		optionsSetAutoAim(mpchrnum, data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMusicVolume(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = optionsGetMusicVolume();
		break;
	case MENUOP_SET:
		optionsSetMusicVolume(data->slider.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerSfxVolume(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = VOLUME(g_SfxVolume);
		break;
	case MENUOP_SET:
		sndSetSfxVolume(data->slider.value);
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuDialogHandlerResult menudialogBriefing(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog
				&& g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			struct menuinputs *inputs = data->dialog2.inputs;

			if (inputs->start) {
				menuhandlerAcceptMission(MENUOP_SET, NULL, data);
			}

			inputs->start = false;
		}
	}

	return 0;
}

struct menuitem g_PreAndPostMissionBriefingMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_BRIEFING,
		0,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PreAndPostMissionBriefingMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_247, // "Briefing"
	g_PreAndPostMissionBriefingMenuItems,
	menudialogBriefing,
	MENUDIALOGFLAG_DISABLEITEMSCROLL,
	NULL,
};

MenuItemHandlerResult menuhandlerAcceptMission(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuStop();

		if (g_Vars.stagenum == g_MissionConfig.stagenum) {
			g_Vars.restartlevel = true;
		}

		titleSetNextStage(g_MissionConfig.stagenum);

		if (g_MissionConfig.iscoop) {
			if (g_Vars.numaibuddies == 0) {
				// Coop with human buddy
				g_Vars.bondplayernum = 0;
				g_Vars.coopplayernum = 1;
				g_Vars.antiplayernum = -1;
				setNumPlayers(2);
			} else {
				// Coop with AI buddies
				g_Vars.bondplayernum = 0;
				g_Vars.coopplayernum = -1;
				g_Vars.antiplayernum = -1;
				setNumPlayers(1);
			}
		} else if (g_MissionConfig.isanti) {
			if (g_Vars.pendingantiplayernum == 1) {
				g_Vars.bondplayernum = 0;
				g_Vars.antiplayernum = 1;
			} else {
				g_Vars.bondplayernum = 1;
				g_Vars.antiplayernum = 0;
			}

			g_Vars.coopplayernum = -1;
			setNumPlayers(2);
		} else {
			// Solo
			g_Vars.bondplayernum = 0;
			g_Vars.coopplayernum = -1;
			g_Vars.antiplayernum = -1;
			setNumPlayers(1);
		}

		lvSetDifficulty(g_MissionConfig.difficulty);
		titleSetNextMode(TITLEMODE_SKIP);
		mainChangeToStage(g_MissionConfig.stagenum);

#if VERSION >= VERSION_NTSC_1_0
		viBlack(true);
#endif
	}

	return 0;
}

char *soloMenuTitleStageOverview(struct menudialogdef *dialogdef)
{
	if (dialogdef != g_Menus[g_MpPlayerNum].curdialog->definition) {
		return langGet(L_OPTIONS_273); // "Overview"
	}

	sprintf(g_StringPointer, "%s: %s\n",
			langGet(g_SoloStages[g_MissionConfig.stageindex].name3),
			langGet(L_OPTIONS_273));

	return g_StringPointer;
}

MenuDialogHandlerResult menudialog00103608(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_Menus[g_MpPlayerNum].menumodel.curparams = 0;

		setupLoadBriefing(g_MissionConfig.stagenum,
				g_Menus[g_MpPlayerNum].menumodel.allocstart,
				g_Menus[g_MpPlayerNum].menumodel.alloclen, &g_Briefing);
		break;
	case MENUOP_CLOSE:
		langClearBank(g_Briefing.langbank);
		break;
	}

	return 0;
}

struct menuitem g_AcceptMissionMenuItems[] = {
	{
		MENUITEMTYPE_OBJECTIVES,
		1,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_274, // "Accept"
		0,
		menuhandlerAcceptMission,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_275, // "Decline"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AcceptMissionMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&soloMenuTitleStageOverview,
	g_AcceptMissionMenuItems,
	menudialog00103608,
	MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_DISABLEITEMSCROLL,
	&g_PreAndPostMissionBriefingMenuDialog,
};

f32 func0f1036ac(u8 value, s32 prop)
{
	if (prop == PDMODEPROP_REACTION) {
		return value / 255.0f;
	}

	return mpHandicapToDamageScale(value);
}

MenuItemHandlerResult menuhandlerPdModeSetting(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u8 *property;
	f32 fvalue;

	switch (item->param) {
	case PDMODEPROP_REACTION: property = &g_MissionConfig.pdmodereaction; break;
	case PDMODEPROP_HEALTH:   property = &g_MissionConfig.pdmodehealth;   break;
	case PDMODEPROP_DAMAGE:   property = &g_MissionConfig.pdmodedamage;   break;
	case PDMODEPROP_ACCURACY: property = &g_MissionConfig.pdmodeaccuracy; break;
	default: return 0;
	}

	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = *property;
		break;
	case MENUOP_SET:
		*property = (u16)data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		fvalue = func0f1036ac(*property, item->param);
		if (item->param == 0) {
			fvalue = fvalue * 4 + 1.0f;
		}
		sprintf(data->slider.label, "%s%s%.00f%%\n", "", "", fvalue * 100.0f);
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAcceptPdModeSettings(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_MissionConfig.pdmode = true;
		g_MissionConfig.pdmodehealthf = func0f1036ac(g_MissionConfig.pdmodehealth, PDMODEPROP_HEALTH);
		g_MissionConfig.pdmodedamagef = func0f1036ac(g_MissionConfig.pdmodedamage, PDMODEPROP_DAMAGE);
		g_MissionConfig.pdmodeaccuracyf = func0f1036ac(g_MissionConfig.pdmodeaccuracy, PDMODEPROP_ACCURACY);
		g_MissionConfig.difficulty = DIFF_PA;
		lvSetDifficulty(g_MissionConfig.difficulty);
		menuPopDialog();
		menuPopDialog();
		menuPushDialog(&g_AcceptMissionMenuDialog);
	}

	return 0;
}

struct menuitem g_PdModeSettingsMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_222, // "Choose Settings:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SLIDER,
		1,
		MENUITEMFLAG_SLIDER_ALTSIZE,
		L_MPWEAPONS_224, // "Enemy Health:"
		0x000000ff,
		menuhandlerPdModeSetting,
	},
	{
		MENUITEMTYPE_SLIDER,
		2,
		MENUITEMFLAG_SLIDER_ALTSIZE,
		L_MPWEAPONS_225, // "Enemy Damage:"
		0x000000ff,
		menuhandlerPdModeSetting,
	},
	{
		MENUITEMTYPE_SLIDER,
		3,
		MENUITEMFLAG_SLIDER_ALTSIZE,
		L_MPWEAPONS_226, // "Enemy Accuracy:"
		0x000000ff,
		menuhandlerPdModeSetting,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x000000b4,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_227, // "OK"
		0,
		menuhandlerAcceptPdModeSettings,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPWEAPONS_228, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PdModeSettingsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPWEAPONS_221, // "Perfect Dark"
	g_PdModeSettingsMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

/**
 * This function assumes that the stage that's given to it is already unlocked.
 * It is checking the difficulty for that stage only.
 *
 * This function does not test for PD mode being unlocked.
 */
bool isStageDifficultyUnlocked(s32 stageindex, s32 difficulty)
{
	s32 s;
	s32 d;

	// Handle special missions
	if (stageindex > SOLOSTAGEINDEX_SKEDARRUINS) {
#if VERSION >= VERSION_NTSC_1_0
		// If the player has completed Skedar Ruins on the same difficulty as
		// the one that's being queried, then they have access to this
		// difficulty for all special missions. Agent is gifted here, so if the
		// bonus mission is available at all then Agent is also available.
		s32 maxcompleteddiff = DIFF_A;

		for (d = DIFF_A; d <= DIFF_PA; d++) {
			if (g_GameFile.besttimes[SOLOSTAGEINDEX_SKEDARRUINS][d] != 0) {
				maxcompleteddiff = d;
			}
		}

		if (difficulty <= maxcompleteddiff) {
			return true;
		}
#endif

		// Otherwise, grant them the difficulty if they've completed all prior
		// difficulties on this stage.
		for (d = DIFF_A; d < difficulty; d++) {
			if (g_GameFile.besttimes[stageindex][d] == 0) {
				return false;
			}
		}

		return true;
	}

	// Handle normal missions
	if (stageindex <= SOLOSTAGEINDEX_SKEDARRUINS && difficulty <= DIFF_PA) {
		// Defection is always unlocked on all difficulties
		if (g_SoloStages[stageindex].stagenum == STAGE_DEFECTION) {
			return true;
		}

		// If the stage has already been completed on the queried difficulty
		// or higher then the queried difficulty is made available.
		// For coop and anti, coop completions are also checked.
		for (d = difficulty; d <= DIFF_PA; d++) {
			if (g_GameFile.besttimes[stageindex][d] != 0) {
				return true;
			}

			if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
					&& (g_GameFile.coopcompletions[d] & (1 << stageindex))) {
				return true;
			}
		}

		if (stageindex > 0) {
			if (g_SoloStages[stageindex].stagenum != STAGE_SKEDARRUINS) {
				// For normal stages prior to Skedar Ruins, test if the
				// prior stage is complete on the same difficulty or higher.
				for (d = difficulty; d <= DIFF_PA; d++) {
					if (g_GameFile.besttimes[stageindex - 1][d] != 0) {
						return true;
					}

					if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
							&& (g_GameFile.coopcompletions[d] & (1 << (stageindex - 1)))) {
						return true;
					}
				}
			} else {
				// For Skedar Ruins, check that all prior stages are complete
				// on the queried difficulty or higher.
				for (s = 0; s < stageindex; s++) {
					for (d = difficulty; d <= DIFF_PA; d++) {
						if (g_GameFile.besttimes[s][d] != 0) {
							break;
						}

						if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
								&& (g_GameFile.coopcompletions[d] & (1 << s)) != 0) {
							break;
						}
					}

					if (d > DIFF_PA) {
						// A stage was not complete
						break;
					}
				}

				if (s >= stageindex) {
					return true;
				}
			}
		}

		// If all normal stages are complete on any difficulty, and we're
		// querying SA or higher, grant the difficulty if the stage is complete
		// on the prior difficulty or higher.
		if (difficulty >= DIFF_SA) {
			if (g_SoloStages[stageindex].stagenum != STAGE_SKEDARRUINS) {
				// Check if all normal stages are complete on any difficulty
				for (s = 0; s <= SOLOSTAGEINDEX_SKEDARRUINS; s++) {
					for (d = DIFF_A; d <= DIFF_PA; d++) {
						if (g_GameFile.besttimes[s][d] != 0) {
							break;
						}

						if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
								&& (g_GameFile.coopcompletions[d] & (1 << s)) != 0) {
							break;
						}
					}

					if (d > DIFF_PA) {
						// A stage was not complete
						break;
					}
				}

				if (s >= SOLOSTAGEINDEX_MBR) {
					for (d = difficulty - 1; d <= DIFF_PA; d++) {
						if (g_GameFile.besttimes[stageindex][d] != 0) {
							return true;
						}

						if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
								&& (g_GameFile.coopcompletions[d] & (1 << stageindex)) != 0) {
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

MenuItemHandlerResult menuhandlerSoloDifficulty(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKPREFOCUSED:
#if VERSION >= VERSION_NTSC_1_0
		if (isStageDifficultyUnlocked(g_MissionConfig.stageindex, item->param)) {
			if (item->param3 == 0) {
				return true;
			}
			if (item->param <= (u32)g_GameFile.autodifficulty) {
				return true;
			}
		}
#else
		if (item->param3 == 0) {
			return true;
		}
		if (item->param <= (u32)g_GameFile.autodifficulty) {
			return true;
		}
#endif
		break;
	case MENUOP_SET:
		g_MissionConfig.pdmode = false;
		g_MissionConfig.difficulty = item->param;
		lvSetDifficulty(g_MissionConfig.difficulty);
		menuPopDialog();
		menuPushDialog(&g_AcceptMissionMenuDialog);
		break;
	case MENUOP_CHECKDISABLED:
		if (!isStageDifficultyUnlocked(g_MissionConfig.stageindex, item->param)) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerPdMode(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		menuPushDialog(&g_PdModeSettingsMenuDialog);
		break;
	case MENUOP_CHECKHIDDEN:
		if (g_GameFile.besttimes[SOLOSTAGEINDEX_SKEDARRUINS][DIFF_PA] == 0) {
			return true;
		}
	}

	return 0;
}

char *soloMenuTextBestTime(struct menuitem *item)
{
	u16 time = g_GameFile.besttimes[g_MissionConfig.stageindex][item->param];
	s32 hours = time / 3600;

	if (time == 0) {
		return "--:--\n";
	}

	if (time >= 0xfff) { // 1 hour, 8 minutes, 15 seconds
		return "==:==\n";
	}

	if (hours == 0) {
		s32 mins = time / 60;
		sprintf(g_StringPointer, "%dm:%02ds", mins % 60, time % 60);
	} else {
		s32 mins = time / 60;
		sprintf(g_StringPointer, "%dh:%02dm:%02ds", hours, mins % 60, time % 60);
	}

	return g_StringPointer;
}

struct menuitem g_SoloMissionDifficultyMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_OPTIONS_249, // "Difficulty"
		L_OPTIONS_250, // "Best Time"
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_251, // "Agent"
		(uintptr_t)&soloMenuTextBestTime,
		menuhandlerSoloDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		L_OPTIONS_252, // "Special Agent"
		(uintptr_t)&soloMenuTextBestTime,
		menuhandlerSoloDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		L_OPTIONS_253, // "Perfect Agent"
		(uintptr_t)&soloMenuTextBestTime,
		menuhandlerSoloDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_221, // "Perfect Dark"
		0,
		menuhandlerPdMode,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_254, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionDifficultyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_248, // "Select Difficulty"
	g_SoloMissionDifficultyMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult menuhandlerBuddyOptionsContinue(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPushDialog(&g_AcceptMissionMenuDialog);
	}

	if (operation == MENUOP_CHECKPREFOCUSED) {
		return true;
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
s32 getMaxAiBuddies(void)
{
	u32 stack;
	s32 extra = 0;
	s32 max = 1 - g_MissionConfig.difficulty;
	s32 d;

	for (d = 0; d != 3; d++) {
		if ((g_GameFile.coopcompletions[d] | 0xfffe0000) == 0xffffffff) {
			extra = d + 1;
		}
	}

	max += extra;

	if (max > 4) {
		max = 4;
	}

	if (max < 1) {
		max = 1;
	}

#if VERSION == VERSION_PAL_BETA
#ifdef DEBUG
	if (debugIsAllBuddiesEnabled()) {
		max = 4;
	}
#endif
#endif

	return max;
}
#endif

MenuDialogHandlerResult menudialogCoopAntiOptions(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
#if VERSION >= VERSION_NTSC_1_0
	if (operation == MENUOP_OPEN) {
		s32 max = getMaxAiBuddies();

		if (g_Vars.numaibuddies > max) {
			g_Vars.numaibuddies = max;
		}
	}
#endif

	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog && g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			struct menuinputs *inputs = data->dialog2.inputs;

			if (inputs->start) {
				menuhandlerBuddyOptionsContinue(MENUOP_SET, NULL, NULL);
			}

			inputs->start = false;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerCoopRadar(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return g_Vars.coopradaron;
	case MENUOP_SET:
		g_Vars.coopradaron = data->checkbox.value ? true : false;
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerCoopFriendlyFire(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return g_Vars.coopfriendlyfire;
	case MENUOP_SET:
		g_Vars.coopfriendlyfire = data->checkbox.value ? true : false;
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerCoopBuddy(s32 operation, struct menuitem *item, union handlerdata *data)
{
	const u16 labels[] = {
		L_OPTIONS_261, // "Human"
		L_OPTIONS_262, // "1 Simulant"
		L_OPTIONS_263, // "2 Simulants"
		L_OPTIONS_264, // "3 Simulants"
		L_OPTIONS_265, // "4 Simulants"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
#if VERSION >= VERSION_NTSC_1_0
		{
			s32 maxaibuddies = getMaxAiBuddies();
			s32 human = 0;

			if (joyGetConnectedControllers() & 2) {
				human = 1;
			}

			data->dropdown.value = human + maxaibuddies;
		}
#else
		{
			s32 extrabuddies = 0;
			s32 i;
			s32 maxbuddies = 1 - g_MissionConfig.difficulty;
			s32 human = 0;

			if (joyGetConnectedControllers() & 2) {
				human = 1;
			}

			for (i = 0; i < 3; i++) {
				if ((g_GameFile.coopcompletions[i] | 0xfffe0000) == 0xffffffff) {
					extrabuddies = i + 1;
				}
			}

			maxbuddies += extrabuddies;

			if (maxbuddies > 4) {
				maxbuddies = 4;
			}

			if (maxbuddies < 1) {
				maxbuddies = 1;
			}

#ifdef DEBUG
			if (debugIsAllBuddiesEnabled()) {
				maxbuddies = 4;
			}
#endif

			data->dropdown.value = human + maxbuddies;
		}
#endif
		break;
	case MENUOP_GETOPTIONTEXT:
		{
			s32 extra = 1;

			if (joyGetConnectedControllers() & 2) {
				extra = 0;
			}

			return (uintptr_t)langGet(labels[data->dropdown.value + extra]);
		}
	case MENUOP_SET:
		{
			s32 extra = 1;

			if (joyGetConnectedControllers() & 2) {
				extra = 0;
			}

			g_Vars.numaibuddies = data->dropdown.value + extra;
			g_Vars.modifiedfiles |= MODFILE_GAME;
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		{
			s32 extra = 1;

			if (joyGetConnectedControllers() & 2) {
				extra = 0;
			}

			if (extra == 1 && g_Vars.numaibuddies == 0) {
				g_Vars.numaibuddies = 1;
			}

			data->dropdown.value = g_Vars.numaibuddies - extra;
		}
		break;
	}

	return 0;
}

struct menuitem g_CoopOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_256, // "Radar On"
		0,
		menuhandlerCoopRadar,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_257, // "Friendly Fire"
		0,
		menuhandlerCoopFriendlyFire,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_258, // "Perfect Buddy"
		0,
		menuhandlerCoopBuddy,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	}, // ""
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_259, // "Continue"
		0,
		menuhandlerBuddyOptionsContinue,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_260, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END }, // ""
};

struct menudialogdef g_CoopOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_255, // "Co-Operative Options"
	g_CoopOptionsMenuItems,
	menudialogCoopAntiOptions,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult menuhandlerAntiRadar(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return g_Vars.antiradaron;
	case MENUOP_SET:
		g_Vars.antiradaron = data->checkbox.value ? 1 : 0;
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAntiPlayer(s32 operation, struct menuitem *item, union handlerdata *data)
{
	const u16 labels[] = {L_OPTIONS_271, L_OPTIONS_272};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(labels[data->dropdown.value]);
	case MENUOP_SET:
		g_Vars.pendingantiplayernum = data->dropdown.value;
		g_Vars.modifiedfiles |= MODFILE_GAME;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_Vars.pendingantiplayernum;
		break;
	}

	return 0;
}

struct menuitem g_AntiOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_267, // "Radar On"
		0,
		menuhandlerAntiRadar,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_268, // "Counter-Operative"
		0,
		menuhandlerAntiPlayer,
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
		0,
		L_OPTIONS_269, // "Continue"
		0,
		menuhandlerBuddyOptionsContinue,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_270, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AntiOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_266, // "Counter-Operative Options"
	g_AntiOptionsMenuItems,
	menudialogCoopAntiOptions,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult menuhandlerCoopDifficulty(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		g_MissionConfig.pdmode = false;
		g_MissionConfig.difficulty = item->param;
		lvSetDifficulty(g_MissionConfig.difficulty);
		menuPopDialog();
		menuPushDialog(&g_CoopOptionsMenuDialog);
		break;
	case MENUOP_CHECKDISABLED:
		if (!isStageDifficultyUnlocked(g_MissionConfig.stageindex, item->param)) {
			return true;
		}
	}

	return 0;
}

struct menuitem g_CoopMissionDifficultyMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_251, // "Agent"
		0,
		menuhandlerCoopDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		L_OPTIONS_252, // "Special Agent"
		0,
		menuhandlerCoopDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		L_OPTIONS_253, // "Perfect Agent"
		0,
		menuhandlerCoopDifficulty,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_254, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CoopMissionDifficultyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_248, // "Select Difficulty"
	g_CoopMissionDifficultyMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult menuhandlerAntiDifficulty(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		g_MissionConfig.pdmode = false;
		g_MissionConfig.difficulty = item->param;
		lvSetDifficulty(g_MissionConfig.difficulty);
		menuPopDialog();
		menuPushDialog(&g_AntiOptionsMenuDialog);
	}

	return 0;
}

struct menuitem g_AntiMissionDifficultyMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_251, // "Agent"
		0,
		menuhandlerAntiDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		L_OPTIONS_252, // "Special Agent"
		0,
		menuhandlerAntiDifficulty,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		L_OPTIONS_253, // "Perfect Agent"
		0,
		menuhandlerAntiDifficulty,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_254, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AntiMissionDifficultyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_248, // "Select Difficulty"
	g_AntiMissionDifficultyMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

struct solostage g_SoloStages[NUM_SOLOSTAGES] = {
	// stage,             unk04,
	{ STAGE_DEFECTION,     0x0c, L_OPTIONS_133, L_OPTIONS_134, L_MPWEAPONS_124 },
	{ STAGE_INVESTIGATION, 0x0d, L_OPTIONS_135, L_OPTIONS_136, L_MPWEAPONS_172 },
	{ STAGE_EXTRACTION,    0x0e, L_OPTIONS_137, L_OPTIONS_138, L_MPWEAPONS_125 },
	{ STAGE_VILLA,         0x0f, L_OPTIONS_139, L_OPTIONS_140, L_OPTIONS_139   },
	{ STAGE_CHICAGO,       0x10, L_OPTIONS_141, L_OPTIONS_142, L_OPTIONS_141   },
	{ STAGE_G5BUILDING,    0x11, L_OPTIONS_143, L_OPTIONS_144, L_OPTIONS_143   },
	{ STAGE_INFILTRATION,  0x12, L_OPTIONS_145, L_OPTIONS_146, L_MPWEAPONS_126 },
	{ STAGE_RESCUE,        0x13, L_OPTIONS_147, L_OPTIONS_148, L_MPWEAPONS_127 },
	{ STAGE_ESCAPE,        0x14, L_OPTIONS_149, L_OPTIONS_150, L_MPWEAPONS_128 },
	{ STAGE_AIRBASE,       0x15, L_OPTIONS_151, L_OPTIONS_152, L_OPTIONS_151   },
	{ STAGE_AIRFORCEONE,   0x16, L_OPTIONS_153, L_OPTIONS_154, L_OPTIONS_153   },
	{ STAGE_CRASHSITE,     0x17, L_OPTIONS_155, L_OPTIONS_156, L_OPTIONS_155   },
	{ STAGE_PELAGIC,       0x18, L_OPTIONS_157, L_OPTIONS_158, L_OPTIONS_157   },
	{ STAGE_DEEPSEA,       0x19, L_OPTIONS_159, L_OPTIONS_160, L_OPTIONS_159   },
	{ STAGE_DEFENSE,       0x1a, L_OPTIONS_161, L_OPTIONS_162, L_OPTIONS_161   },
	{ STAGE_ATTACKSHIP,    0x1b, L_OPTIONS_163, L_OPTIONS_164, L_OPTIONS_163   },
	{ STAGE_SKEDARRUINS,   0x1c, L_OPTIONS_165, L_OPTIONS_166, L_OPTIONS_165   },
	{ STAGE_MBR,           0x1c, L_OPTIONS_167, L_OPTIONS_003, L_OPTIONS_167   },
	{ STAGE_MAIANSOS,      0x1c, L_OPTIONS_168, L_OPTIONS_003, L_OPTIONS_168   },
	{ STAGE_WAR,           0x1c, L_OPTIONS_170, L_OPTIONS_003, L_OPTIONS_170   },
	{ STAGE_DUEL,          0x1c, L_OPTIONS_171, L_OPTIONS_003, L_OPTIONS_171   },
};

s32 getNumUnlockedSpecialStages(void)
{
	s32 count = 0;
	s32 offsetforduel = 1;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_GameFile.besttimes[0]); i++) {
		if (g_GameFile.besttimes[SOLOSTAGEINDEX_SKEDARRUINS][i]) {
			count = i + 1;
		}
	}

	if (g_MissionConfig.iscoop || g_MissionConfig.isanti) {
		offsetforduel = 0;
	} else {
		for (i = 0; i < (VERSION >= VERSION_NTSC_1_0 ? 32 : 33); i++) {
			if (ciGetFiringRangeScore(i) <= 0) {
				offsetforduel = 0;
			}
		}
	}

	return count + offsetforduel;
}

s32 func0f104720(s32 value)
{
	s32 next = 0;
	s32 d;

	for (d = 0; d < ARRAYCOUNT(g_GameFile.besttimes[0]); d++) {
		if (g_GameFile.besttimes[SOLOSTAGEINDEX_SKEDARRUINS][d]) {
			next = d + 1;
		}
	}

	if (next > value) {
		return 17 + value;
	}

	return 20;
}

MenuItemHandlerResult menuhandlerMissionList(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct optiongroup groups[] = {
		{  0, L_OPTIONS_123 }, // "Mission 1"
		{  3, L_OPTIONS_124 }, // "Mission 2"
		{  4, L_OPTIONS_125 }, // "Mission 3"
		{  6, L_OPTIONS_126 }, // "Mission 4"
		{  9, L_OPTIONS_127 }, // "Mission 5"
		{ 12, L_OPTIONS_128 }, // "Mission 6"
		{ 14, L_OPTIONS_129 }, // "Mission 7"
		{ 15, L_OPTIONS_130 }, // "Mission 8"
		{ 16, L_OPTIONS_131 }, // "Mission 9"
		{ 99, L_OPTIONS_132 }, // "Special Assignments"
	};

	s32 i;
	s32 j;
	bool stageiscomplete;
	union handlerdata sp18c;
	u32 sp188;
	union handlerdata sp178;
	union handlerdata sp168;
	s32 sp164;
	s32 sp160;
	union handlerdata sp150;
	s32 k;
	union handlerdata sp13c;
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	s32 x;
	s32 y;
	s32 stack;
	s32 incompleteindex;
	char text[50];
	s32 stageindex;
	union handlerdata spdc;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = 0;

		for (i = 0; i <= SOLOSTAGEINDEX_SKEDARRUINS; i++) {
			stageiscomplete = false;

			for (j = 0; j < ARRAYCOUNT(g_GameFile.besttimes[i]); j++) {
				if (g_GameFile.besttimes[i][j] != 0) {
					stageiscomplete = true;
				}

				if ((g_MissionConfig.iscoop || g_MissionConfig.isanti)
						&& (g_GameFile.coopcompletions[j] & (1 << i))) {
					stageiscomplete = true;
				}
			}

			data->list.value++;

			if (!stageiscomplete) {
				break;
			}
		}

		data->list.value += getNumUnlockedSpecialStages();
		break;
	case MENUOP_GETOPTIONTEXT:
		if (data->list.unk04u32 == 0) {
			menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &sp18c);
			data->list.unk04u32 = sp18c.list.value - getNumUnlockedSpecialStages();
		}

		if (data->list.value < data->list.unk04u32) {
			// Regular stage such as "dataDyne Central - Defection"
			// Return the name before the dash, such as "dataDyne Central"
			return (uintptr_t) langGet(g_SoloStages[data->list.value].name1);
		}

		// Special stages have no dash and suffix, so just return the name
		return (uintptr_t) langGet(g_SoloStages[func0f104720(data->list.value - data->list.unk04u32)].name1);
	case MENUOP_SET:
		sp188 = data->list.value;
		menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &sp178);
		sp178.list.value -= getNumUnlockedSpecialStages();

		if (data->list.value >= sp178.list.value) {
			sp188 = func0f104720(data->list.value - sp178.list.value);
		}

		g_Vars.mplayerisrunning = false;
		g_Vars.normmplayerisrunning = false;
		g_MissionConfig.stagenum = g_SoloStages[sp188].stagenum;
		g_MissionConfig.stageindex = sp188;

		if (g_MissionConfig.iscoop) {
			menuPushDialog(&g_CoopMissionDifficultyMenuDialog);
		} else if (g_MissionConfig.isanti) {
			menuPushDialog(&g_AntiMissionDifficultyMenuDialog);
		} else {
			menuPushDialog(&g_SoloMissionDifficultyMenuDialog);
		}

		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_25:
		if (data->list.unk04 == 0 && !g_MissionConfig.iscoop && !g_MissionConfig.isanti) {
			data->list.value = g_GameFile.autostageindex;

			menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &sp168);
			sp168.list.value -= getNumUnlockedSpecialStages();

			if (data->list.value >= sp168.list.value) {
				sp164 = getNumUnlockedSpecialStages();

				data->list.value = sp168.list.value - 1;

				for (sp160 = 0; sp160 < sp164; sp160++) {
					if (func0f104720(sp160) == g_GameFile.autostageindex) {
						data->list.value = sp168.list.values32 + sp160;
					}
				}
			}
		}
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &sp150);
		sp150.list.value -= getNumUnlockedSpecialStages();

		data->list.unk0c = 0;

		for (i = 0; i < ARRAYCOUNT(groups); i++) {
			if (groups[i].offset < sp150.list.value) {
				data->list.unk0c++;
			}
		}

		data->list.value = data->list.unk0c + 1;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		if (data->list.unk0c == data->list.value) {
			return (uintptr_t) langGet(groups[9].name); // "Special Assignments"
		}
		return (uintptr_t) langGet(groups[data->list.value].name);
	case MENUOP_GETGROUPSTARTINDEX:
		if (data->list.unk0c == data->list.value) {
			menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &sp13c);
			data->list.groupstartindex = sp13c.list.value - getNumUnlockedSpecialStages();
		} else {
			data->list.groupstartindex = groups[data->list.value].offset;
		}
		break;
	case MENUOP_RENDER:
		gdl = data->type19.gdl;
		renderdata = data->type19.renderdata2;
		incompleteindex = 0;
		stageindex = data->type19.unk04u32;

		if (data->type19.unk0c == 0) {
			menuhandlerMissionList(MENUOP_GETOPTIONCOUNT, item, &spdc);
			data->type19.unk0c = spdc.list.value - getNumUnlockedSpecialStages();
		}

		if (data->type19.unk04u32 >= data->type19.unk0c) {
			stageindex = func0f104720(data->type19.unk04u32 - data->type19.unk0c);
		}

		// Draw the thumbnail
		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);

#if VERSION >= VERSION_NTSC_1_0
		texSelect(&gdl, g_TexGeneralConfigs + 13 + stageindex, 2, 0, 2, true, NULL);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetCombineMode(gdl++, G_CC_CUSTOM_00, G_CC_CUSTOM_00);
		gDPSetTextureFilter(gdl++, G_TF_POINT);
		gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 255 / 256));
#else
		texSelect(&gdl, g_TexGeneralConfigs + 13 + stageindex, 1, 0, 2, true, NULL);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
		gDPSetTextureFilter(gdl++, G_TF_POINT);
#endif

		gSPTextureRectangle(gdl++,
				((renderdata->x + 4) << 2) * g_ScaleX, (renderdata->y + 3) << 2,
				((renderdata->x + 60) << 2) * g_ScaleX, (renderdata->y + 39) << 2,
				G_TX_RENDERTILE, 0, 0x0480, 1024 / g_ScaleX, -1024);

		if (g_MissionConfig.isanti) {
			// No stars
		} else if (g_MissionConfig.iscoop) {
			texSelect(&gdl, &g_TexGeneralConfigs[36], 2, 0, 2, true, NULL);

			gDPSetCycleType(gdl++, G_CYC_1CYCLE);
			gDPSetTextureFilter(gdl++, G_TF_POINT);

			for (k = 0; k < 3; k++) {
				s32 relx = 63 + k * 17;

				if ((g_GameFile.coopcompletions[k] & (1 << stageindex)) == 0) {
#if VERSION >= VERSION_NTSC_1_0
					gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 63 / 256));
#else
					gDPSetEnvColorViaWord(gdl++, 0xffffff3f);
#endif
					gDPSetCombineLERP(gdl++,
							TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
							TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
				} else {
#if VERSION >= VERSION_NTSC_1_0
					gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 207 / 256));
#else
					gDPSetEnvColorViaWord(gdl++, 0xffffffcf);
#endif
					gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
				}

				gSPTextureRectangle(gdl++,
						((renderdata->x + relx) << 2) * g_ScaleX, (renderdata->y + 25) << 2,
						((renderdata->x + relx + 14) << 2) * g_ScaleX, (renderdata->y + 39) << 2,
						G_TX_RENDERTILE, 0x0010, 0x01c0, 1024 / g_ScaleX, -1024);
			}
		} else {
			texSelect(&gdl, &g_TexGeneralConfigs[34], 2, 0, 2, true, NULL);

			gDPSetCycleType(gdl++, G_CYC_1CYCLE);
			gDPSetTextureFilter(gdl++, G_TF_POINT);
			gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);

#if VERSION >= VERSION_NTSC_1_0
			gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 175 / 256));
#else
			gDPSetEnvColorViaWord(gdl++, 0xffffffaf);
#endif

			for (k = 0; k < 3; k++) {
				if (g_GameFile.besttimes[stageindex][k] != 0) {
					incompleteindex = k + 1;
				}
			}

			for (k = 0; k < 3; k++) {
				s32 relx = 63 + k * 17;

				if (k == incompleteindex) {
					// Set transparency
#if VERSION >= VERSION_NTSC_1_0
					gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 63 / 256));
#else
					gDPSetEnvColorViaWord(gdl++, 0xffffff3f);
#endif
					gDPSetCombineLERP(gdl++,
							TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
							TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
				}

				gSPTextureRectangle(gdl++,
						((renderdata->x + relx) << 2) * g_ScaleX, (renderdata->y + 25) << 2,
						((renderdata->x + relx + 14) << 2) * g_ScaleX, (renderdata->y + 39) << 2,
						G_TX_RENDERTILE, 0x0010, 0x01c0, 1024 / g_ScaleX, -1024);
			}
		}

		x = renderdata->x + 62;
		y = renderdata->y + 3;

		gdl = text0f153628(gdl);

		// Draw first part of name
		strcpy(text, langGet(g_SoloStages[stageindex].name1));
		strcat(text, "\n");

		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicMd, g_FontHandelGothicMd,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

		// Draw last part of name
		strcpy(text, langGet(g_SoloStages[stageindex].name2));

		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

		gdl = text0f153780(gdl);

		return (uintptr_t) gdl;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = 42;
		break;
	}

	return 0;
}

MenuDialogHandlerResult menudialog0010559c(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		break;
	case MENUOP_CLOSE:
		if ((g_Vars.modifiedfiles & MODFILE_GAME) && g_Vars.coopplayernum < 0 && g_Vars.antiplayernum < 0) {
			if (filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_001, 0) == 0) {
				data->dialog1.preventclose = true;
			}

			g_Vars.modifiedfiles &= ~MODFILE_GAME;
		}

		if (g_Vars.modifiedfiles & MODFILE_BOSS) {
			bossfileSave();
			g_Vars.modifiedfiles &= ~MODFILE_BOSS;
		}
		break;
	}

	return 0;
}

struct menuitem g_MissionBriefingMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_BRIEFING,
		0,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_2PMissionBreifingVMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_BRIEFING,
		0,
		0x00000078,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionBriefingMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_247, // "Briefing"
	g_MissionBriefingMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEITEMSCROLL,
	NULL,
};

struct menudialogdef g_2PMissionBriefingHMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_247, // "Briefing"
	g_MissionBriefingMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEITEMSCROLL,
	NULL,
};

struct menudialogdef g_2PMissionBriefingVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_247, // "Briefing"
	g_2PMissionBreifingVMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEITEMSCROLL,
	NULL,
};

char *func0f105664(struct menuitem *item)
{
	union handlerdata data;

	menuhandler001024dc(MENUOP_GETSELECTEDINDEX, item, &data);

	return (char *)menuhandler001024dc(MENUOP_GETOPTIONTEXT, item, &data);
}

char *func0f1056a0(struct menuitem *item)
{
	union handlerdata data;

	menuhandler001024fc(MENUOP_GETSELECTEDINDEX, item, &data);

	return (char *)menuhandler001024fc(MENUOP_GETOPTIONTEXT, item, &data);
}

MenuItemHandlerResult menuhandlerLangFilter(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return g_Vars.langfilteron;
	case MENUOP_SET:
		g_Vars.langfilteron = data->checkbox.value;
		g_Vars.modifiedfiles |= MODFILE_GAME;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerControlStyle(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (PLAYERCOUNT() >= 2) {
			menuPushDialog(&g_2PMissionControlStyleMenuDialog);
		} else {
			menuPushDialog(&g_SoloMissionControlStyleMenuDialog);
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandler001057ec(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_002, 0);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerChangeAgent(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f820c(NULL, -7);
	}

	return 0;
}

#ifndef PLATFORM_N64
MenuItemHandlerResult menuhandlerExitGame(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		exit(0);
	}

	return 0;
}
#endif

struct menuitem g_2PMissionControlStyleMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_AUTOWIDTH,
		0x00000050,
		0,
		menuhandler001024dc,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_2PMissionControlStyleMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_236, // "Control"
	g_2PMissionControlStyleMenuItems,
	NULL,
	MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_SoloMissionControlStyleMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_AUTOWIDTH,
		0x00000028,
#if VERSION == VERSION_JPN_FINAL
		0xbe,
#elif PAL
		0x9c,
#else
		0x96,
#endif
		menuhandler001024dc,
	},
	{
		MENUITEMTYPE_CONTROLLER,
		0,
		MENUITEMFLAG_NEWCOLUMN,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionControlStyleMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_236, // "Control"
	g_SoloMissionControlStyleMenuItems,
	NULL,
	MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_CiControlStyleMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_AUTOWIDTH,
		0x00000028,
#if VERSION == VERSION_JPN_FINAL
		0xbe,
#elif PAL
		0x9c,
#else
		0x96,
#endif
		menuhandler001024dc,
	},
	{
		MENUITEMTYPE_CONTROLLER,
		0,
		MENUITEMFLAG_NEWCOLUMN,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiControlStyleMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_236, // "Control"
	g_CiControlStyleMenuItems,
	NULL,
	MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_CiControlStylePlayer2MenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_AUTOWIDTH,
		0x00000028,
#if VERSION == VERSION_JPN_FINAL
		0xbe,
#elif PAL
		0x9c,
#else
		0x96,
#endif
		menuhandler001024fc,
	},
	{
		MENUITEMTYPE_CONTROLLER,
		0,
		MENUITEMFLAG_NEWCOLUMN,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiControlStylePlayer2MenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_236, // "Control"
	g_CiControlStylePlayer2MenuItems,
	NULL,
	MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_AudioOptionsMenuItems[] = {
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		L_OPTIONS_228, // "Sound"
#if VERSION >= VERSION_NTSC_1_0
		L_MPMENU_000, // ""
#else
		0x7fff,
#endif
		menuhandlerSfxVolume,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		L_OPTIONS_229, // "Music"
#if VERSION >= VERSION_NTSC_1_0
		L_MPMENU_000, // ""
#else
		0x7fff,
#endif
		menuhandlerMusicVolume,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_230, // "Sound Mode"
		0,
		menuhandlerSoundMode,
	},
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_218, // "Language Filter"
		0,
		menuhandlerLangFilter,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		(VERSION >= VERSION_PAL_FINAL ? 200 : 0),
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_231, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AudioOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_227, // "Audio Options"
	g_AudioOptionsMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_2PMissionAudioOptionsVMenuItems[] = {
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE | MENUITEMFLAG_SLIDER_ALTSIZE,
		L_OPTIONS_228, // "Sound"
#if VERSION >= VERSION_NTSC_1_0
		L_MPMENU_000, // ""
#else
		0x7fff,
#endif
		menuhandlerSfxVolume,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE | MENUITEMFLAG_SLIDER_ALTSIZE,
		L_OPTIONS_229, // "Music"
#if VERSION >= VERSION_NTSC_1_0
		L_MPMENU_000, // ""
#else
		0x7fff,
#endif
		menuhandlerMusicVolume,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_153, // "Mode"
		0,
		menuhandlerSoundMode,
	},
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_218, // "Language Filter"
		0,
		menuhandlerLangFilter,
	},
#endif
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_231, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_2PMissionAudioOptionsVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_227, // "Audio Options"
	g_2PMissionAudioOptionsVMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_VideoOptionsMenuItems[] = {
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_215, // "Screen Size"
		0,
		menuhandlerScreenSize,
	},
#ifdef PLATFORM_N64
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_216, // "Ratio"
		0,
		menuhandlerScreenRatio,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_217, // "Hi-Res"
		0,
		menuhandlerHiRes,
	},
#endif
#if PAL
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_269, // ""
		0,
		menuhandlerLanguage,
	},
#endif
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_218, // "2-Player Screen Split"
		0,
		menuhandlerScreenSplit,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_170, // "Alternative Title Screen"
		0,
		menuhandlerAlternativeTitle,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x000000c8,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_219, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_2PMissionVideoOptionsMenuItems[] = {
#ifdef PLATFORM_N64
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_216, // "Ratio"
		0,
		menuhandlerScreenRatio,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_217, // "Hi-Res"
		0,
		menuhandlerHiRes,
	},
#endif
#if PAL
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_269, // ""
		0,
		menuhandlerLanguage,
	},
#endif
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_154, // "Split"
		0,
		menuhandlerScreenSplit,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_219, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_VideoOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_214, // "Video Options"
	g_VideoOptionsMenuItems,
	NULL,
	0,
	NULL,
};

struct menudialogdef g_2PMissionVideoOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_214, // "Video Options"
	g_2PMissionVideoOptionsMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MissionDisplayOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_205, // "Sight on Screen"
		0x00000004,
		menuhandlerSightOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_206, // "Always Show Target"
		0x00000004,
		menuhandlerAlwaysShowTarget,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_207, // "Show Zoom Range"
		0x00000004,
		menuhandlerShowZoomRange,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_208, // "Ammo on Screen"
		0x00000004,
		menuhandlerAmmoOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_209, // "Show Gun Function"
		0x00000004,
		menuhandlerShowGunFunction,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_210, // "Paintball"
		0x00000004,
		menuhandlerPaintball,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_169, // "In-Game Subtitles"
		0x00000004,
		menuhandlerInGameSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_168, // "Cutscene Subtitles"
		0x00000004,
		menuhandlerCutsceneSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_212, // "Show Mission Time"
		0x00000004,
		menuhandlerShowMissionTime,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_213, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MissionDisplayOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_203, // "Display Options"
	g_MissionDisplayOptionsMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_2PMissionDisplayOptionsVMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_145, // "Sight on Screen"
		0x00000004,
		menuhandlerSightOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_146, // "Target"
		0x00000004,
		menuhandlerAlwaysShowTarget,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_147, // "Zoom Range"
		0x00000004,
		menuhandlerShowZoomRange,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_148, // "Show Ammo"
		0x00000004,
		menuhandlerAmmoOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_149, // "Gun Function"
		0x00000004,
		menuhandlerShowGunFunction,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_150, // "Paintball"
		0x00000004,
		menuhandlerPaintball,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
#if VERSION >= VERSION_PAL_FINAL
		L_MPWEAPONS_279, // "In-Game Subtitles"
#else
		L_MPWEAPONS_169, // "In-Game Subtitles"
#endif
		0x00000004,
		menuhandlerInGameSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
#if VERSION >= VERSION_PAL_FINAL
		L_MPWEAPONS_278, // "Cutscene Subtitles"
#else
		L_MPWEAPONS_168, // "Cutscene Subtitles"
#endif
		0x00000004,
		menuhandlerCutsceneSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_152, // "Mission Time"
		0x00000004,
		menuhandlerShowMissionTime,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_213, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_2PMissionDisplayOptionsVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_203, // "Display Options"
	g_2PMissionDisplayOptionsVMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_CiDisplayMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_205, // "Sight on Screen"
		0x00000004,
		menuhandlerSightOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_206, // "Always Show Target"
		0x00000004,
		menuhandlerAlwaysShowTarget,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_207, // "Show Zoom Range"
		0x00000004,
		menuhandlerShowZoomRange,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_208, // "Ammo on Screen"
		0x00000004,
		menuhandlerAmmoOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_209, // "Show Gun Function"
		0x00000004,
		menuhandlerShowGunFunction,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_210, // "Paintball"
		0x00000004,
		menuhandlerPaintball,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_169, // "In-Game Subtitles"
		0x00000004,
		menuhandlerInGameSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_168, // "Cutscene Subtitles"
		0x00000004,
		menuhandlerCutsceneSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_212, // "Show Mission Time"
		0x00000004,
		menuhandlerShowMissionTime,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_213, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiDisplayPlayer2MenuDialog;

struct menudialogdef g_CiDisplayMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_203, // "Display Options"
	g_CiDisplayMenuItems,
	NULL,
	0,
	&g_CiDisplayPlayer2MenuDialog,
};

struct menuitem g_CiDisplayPlayer2MenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_205, // "Sight on Screen"
		0x00000005,
		menuhandlerSightOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_206, // "Always Show Target"
		0x00000005,
		menuhandlerAlwaysShowTarget,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_207, // "Show Zoom Range"
		0x00000005,
		menuhandlerShowZoomRange,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_208, // "Ammo on Screen"
		0x00000005,
		menuhandlerAmmoOnScreen,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_209, // "Show Gun Function"
		0x00000005,
		menuhandlerShowGunFunction,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_210, // "Paintball"
		0x00000005,
		menuhandlerPaintball,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_169, // "In-Game Subtitles"
		0x00000005,
		menuhandlerInGameSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_168, // "Cutscene Subtitles"
		0x00000005,
		menuhandlerCutsceneSubtitles,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_212, // "Show Mission Time"
		0x00000005,
		menuhandlerShowMissionTime,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_213, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiDisplayPlayer2MenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_204, // "Display Player 2"
	g_CiDisplayPlayer2MenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MissionControlOptionsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_194, // "Control Style"
		(uintptr_t)&func0f105664,
		menuhandlerControlStyle,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_195, // "Reverse Pitch"
		0x00000004,
		menuhandlerReversePitch,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_196, // "Look Ahead"
		0x00000004,
		menuhandlerLookAhead,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_197, // "Head Roll"
		0x00000004,
		menuhandlerHeadRoll,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_198, // "Auto-Aim"
		0x00000004,
		menuhandlerAutoAim,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_199, // "Aim Control"
		0x00000004,
		menuhandlerAimControl,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_200, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MissionControlOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_192, // "Control Options"
	g_MissionControlOptionsMenuItems,
	NULL,
	0,
	NULL,
};

#if VERSION >= VERSION_PAL_FINAL
struct menuitem g_CiControlOptionsMenuItems2[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_270, // ""
		(uintptr_t)&func0f105664,
		menuhandlerControlStyle,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_271, // ""
		0x00000004,
		menuhandlerReversePitch,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_272, // ""
		0x00000004,
		menuhandlerLookAhead,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_273, // ""
		0x00000004,
		menuhandlerHeadRoll,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_MPWEAPONS_274, // ""
		0x00000004,
		menuhandlerAutoAim,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_275, // ""
		0x00000004,
		menuhandlerAimControl,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_200, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiControlOptionsMenuDialog2 = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_192, // "Control Options"
	g_CiControlOptionsMenuItems2,
	NULL,
	0,
	NULL,
};
#endif

struct menuitem g_CiControlOptionsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_194, // "Control Style"
		(uintptr_t)&func0f105664,
		(void *)&g_CiControlStyleMenuDialog,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_195, // "Reverse Pitch"
		0x00000004,
		menuhandlerReversePitch,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_196, // "Look Ahead"
		0x00000004,
		menuhandlerLookAhead,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_197, // "Head Roll"
		0x00000004,
		menuhandlerHeadRoll,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_198, // "Auto-Aim"
		0x00000004,
		menuhandlerAutoAim,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_199, // "Aim Control"
		0x00000004,
		menuhandlerAimControl,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_200, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiControlOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_192, // "Control Options"
	g_CiControlOptionsMenuItems,
	NULL,
	0,
	&g_CiControlPlayer2MenuDialog,
};

struct menuitem g_CiControlPlayer2MenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_194, // "Control Style"
		(uintptr_t)&func0f1056a0,
		(void *)&g_CiControlStylePlayer2MenuDialog,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_195, // "Reverse Pitch"
		0x00000005,
		menuhandlerReversePitch,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_196, // "Look Ahead"
		0x00000005,
		menuhandlerLookAhead,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_197, // "Head Roll"
		0x00000005,
		menuhandlerHeadRoll,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		L_OPTIONS_198, // "Auto-Aim"
		0x00000005,
		menuhandlerAutoAim,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_OPTIONS_199, // "Aim Control"
		0x00000005,
		menuhandlerAimControl,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_200, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiControlPlayer2MenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_193, // "Control Player 2"
	g_CiControlPlayer2MenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_ChangeAgentMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_OPTIONS_189, // "Do you want to load another agent?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_190, // "Yes"
		0,
		menuhandlerChangeAgent,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_191, // "No"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_ChangeAgentMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_188, // "Warning"
	g_ChangeAgentMenuItems,
	NULL,
	0,
	NULL,
};

#ifndef PLATFORM_N64

struct menuitem g_ExitGameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Are you sure you want to exit?\n",
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
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_190, // "Yes"
		0,
		menuhandlerExitGame,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_191, // "No"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_ExitGameMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_188, // "Warning"
	g_ExitGameMenuItems,
	NULL,
	0,
	NULL,
};

#endif

struct menuitem g_SoloMissionOptionsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_181, // "Audio"
		0,
		(void *)&g_AudioOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_182, // "Video"
		0,
		(void *)&g_VideoOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_183, // "Control"
		0,
		(void *)&g_MissionControlOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_184, // "Display"
		0,
		(void *)&g_MissionDisplayOptionsMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Extended\n",
		0,
		(void *)&g_ExtendedMenuDialog,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menuitem g_2PMissionOptionsHMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_181, // "Audio"
		0,
		(void *)&g_AudioOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_182, // "Video"
		0,
		(void *)&g_2PMissionVideoOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_183, // "Control"
		0,
		(void *)&g_MissionControlOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_184, // "Display"
		0,
		(void *)&g_MissionDisplayOptionsMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Extended\n",
		0,
		(void *)&g_ExtendedMenuDialog,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000064,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		(uintptr_t)&menutextPauseOrUnpause,
		0,
		menuhandlerMpPause,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_2PMissionOptionsVMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_181, // "Audio"
		0,
		(void *)&g_2PMissionAudioOptionsVMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_182, // "Video"
		0,
		(void *)&g_2PMissionVideoOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_183, // "Control"
		0,
#if VERSION >= VERSION_PAL_FINAL
		(void *)&g_CiControlOptionsMenuDialog2,
#else
		(void *)&g_MissionControlOptionsMenuDialog,
#endif
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_184, // "Display"
		0,
		(void *)&g_2PMissionDisplayOptionsVMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Extended\n",
		0,
		(void *)&g_ExtendedMenuDialog,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000064,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		(uintptr_t)&menutextPauseOrUnpause,
		0,
		menuhandlerMpPause,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_CiOptionsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_181, // "Audio"
		1,
		(void *)&g_AudioOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_182, // "Video"
		2,
		(void *)&g_VideoOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_183, // "Control"
		3,
		(void *)&g_CiControlOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_184, // "Display"
		4,
		(void *)&g_CiDisplayMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_185, // "Cheats"
		5,
		(void *)&g_CheatsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_121, // "Cinema"
		6,
		(void *)&g_CinemaMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Extended\n",
		7,
		(void *)&g_ExtendedMenuDialog,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_180, // "Options"
	g_SoloMissionOptionsMenuItems,
	menudialog0010559c,
	0,
	&g_SoloMissionBriefingMenuDialog,
};

struct menudialogdef g_CiOptionsViaPcMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_180, // "Options"
	g_CiOptionsMenuItems,
	menudialog0010559c,
	0,
	NULL,
};

struct menudialogdef g_CiOptionsViaPauseMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_180, // "Options"
	g_CiOptionsMenuItems,
	menudialog0010559c,
	0,
	NULL,
};

struct menudialogdef g_2PMissionOptionsHMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_180, // "Options"
	g_2PMissionOptionsHMenuItems,
	menudialog0010559c,
	0,
	&g_2PMissionBriefingHMenuDialog,
};

struct menudialogdef g_2PMissionOptionsVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_180, // "Options"
	g_2PMissionOptionsVMenuItems,
	menudialog0010559c,
	0,
	&g_2PMissionBriefingVMenuDialog,
};

u8 var80072d88 = 255;

char *invMenuTextPrimaryFunction(struct menuitem *item)
{
	struct weaponfunc *primaryfunc = weaponGetFunctionById(g_InventoryWeapon, 0);
	struct weaponfunc *secondaryfunc = weaponGetFunctionById(g_InventoryWeapon, 1);

	if (primaryfunc && secondaryfunc) {
		return langGet(primaryfunc->name);
	}

	return langGet(L_OPTIONS_003); // "\n"
}

char *invMenuTextSecondaryFunction(struct menuitem *item)
{
	struct weaponfunc *primaryfunc = weaponGetFunctionById(g_InventoryWeapon, 0);
	struct weaponfunc *secondaryfunc = weaponGetFunctionById(g_InventoryWeapon, 1);

	if (secondaryfunc) {
		return langGet(secondaryfunc->name);
	}

	if (primaryfunc) {
		return langGet(primaryfunc->name);
	}

	return langGet(L_OPTIONS_003); // "\n"
}

void func0f105948(s32 weaponnum)
{
	f32 gunconfig[][5] = {
		{ 23.299999237061f,   -16.799999237061f,  -153.39999389648f,  6.4140100479126f, 0.48769000172615f },
		{ 22.299999237061f,   -13.5f,             -216.60000610352f,  6.443009853363f,  0.34057000279427f },
		{ 19.5f,              -31.89999961853f,   -154.89999389648f,  6.3730101585388f, 0.41813001036644f },
		{ -2.5f,              14.300000190735f,   16.200000762939f,   6.4340100288391f, 0.34057000279427f },
		{ -2.4000000953674f,  21.0f,              -98.900001525879f,  5.7630100250244f, 0.32354000210762f },
		{ -4.0999999046326f,  -30.5f,             -29.39999961853f,   6.3770098686218f, 0.37735998630524f },
		{ 0.69999998807907f,  13.89999961853f,    23.10000038147f,    6.4730100631714f, 0.37735998630524f },
		{ 0.69999998807907f,  13.89999961853f,    23.10000038147f,    6.4730100631714f, 0.37735998630524f },
		{ -5.1999998092651f,  36.5f,              -370.39999389648f,  6.5040102005005f, 0.37735998630524f },
		{ -5.5f,              -79.5f,             -661.0f,            6.3190097808838f, 0.214640006423f   },
		{ -2.9000000953674f,  -57.200000762939f,  -110.09999847412f,  6.3170099258423f, 0.27739998698235f },
		{ -6.1999998092651f,  -33.900001525879f,  101.40000152588f,   6.3320097923279f, 0.27739998698235f },
		{ -23.5f,             -4.0999999046326f,  -209.60000610352f,  6.1110100746155f, 0.214640006423f   },
		{ -3.9000000953674f,  -63.099998474121f,  -872.0f,            6.3720102310181f, 0.214640006423f   },
		{ 218.19999694824f,   -56.299999237061f,  -210.89999389648f,  6.3500099182129f, 0.22594000399113f },
		{ 0.5f,               -84.599998474121f,  -377.20001220703f,  6.1880102157593f, 0.18402999639511f },
		{ -1.6000000238419f,  -68.400001525879f,  -874.5f,            6.3720102310181f, 0.214640006423f   },
		{ -3.7999999523163f,  -145.5f,            52.5f,              6.3170099258423f, 0.32354000210762f },
		{ 117.19999694824f,   -13.800000190735f,  -177.60000610352f,  6.1730098724365f, 0.23782999813557f },
		{ -69.699996948242f,  -135.10000610352f,  -146.10000610352f,  6.18901014328f,   0.16608999669552f },
		{ 0.20000000298023f,  -176.60000610352f,  -276.29998779297f,  6.2660098075867f, 0.16608999669552f },
		{ -0.80000001192093f, -21.200000762939f,  3.5999999046326f,   6.3030200004578f, 0.26352998614311f },
		{ -94.800003051758f,  -13.300000190735f,  -307.70001220703f,  6.2500200271606f, 0.25034999847412f },
		{ -2.2000000476837f,  -45.599998474121f,  -131.89999389648f,  6.3580098152161f, 0.19371999800205f },
		{ -148.69999694824f,  26.10000038147f,    -251.69999694824f,  42.328819274902f, 0.32354000210762f },
		{ -4.0f,              -3.0f,              -157.60000610352f,  43.489791870117f, 0.48769000172615f },
		{ -4.8000001907349f,  14.0f,              -89.0f,             43.927791595459f, 0.5688099861145f  },
		{ -0.40000000596046f, -29.89999961853f,   -8.8000001907349f,  43.981800079346f, 0.73510998487473f },
		{ -23.700000762939f,  -35.799999237061f,  -237.89999389648f,  43.153789520264f, 0.6983500123024f  },
		{ -23.700000762939f,  -35.799999237061f,  -237.89999389648f,  43.153789520264f, 0.6983500123024f  },
		{ 63.700000762939f,   53.0f,              -171.60000610352f,  43.153789520264f, 0.9025200009346f  },
		{ 63.700000762939f,   53.0f,              -171.60000610352f,  43.153789520264f, 0.9025200009346f  },
		{ 63.700000762939f,   53.0f,              -171.60000610352f,  43.153789520264f, 0.9025200009346f  },
		{ 0.20000000298023f,  -1.5f,              1.0f,               43.288791656494f, 6.6717000007629f  },
		{ -68.400001525879f,  14.699999809265f,   -92.5f,             44.255790710449f, 0.59876000881195f },
		{ -2.9000000953674f,  33.5f,              61.400001525879f,   44.254791259766f, 0.48769000172615f },
		{ -1.5f,              41.599998474121f,   -49.900001525879f,  44.198810577393f, 0.41813001036644f },
		{ -2.5999999046326f,  -0.20000000298023f, -237.10000610352f,  44.029800415039f, 0.21465000510216f },
		{ -1.2999999523163f,  13.39999961853f,    -43.700000762939f,  44.2587890625f,   0.34057000279427f },
		{ 0.10000000149012f,  32.099998474121f,   -161.69999694824f,  44.111789703369f, 0.39722999930382f },
		{ -1.0f,              -31.89999961853f,   -300.0f,            44.034790039062f, 0.18402999639511f },
		{ 0.30000001192093f,  -44.900001525879f,  45.099998474121f,   44.078788757324f, 0.27739998698235f },
		{ -4.8000001907349f,  14.0f,              -89.0f,             43.927791595459f, 0.5688099861145f  },
		{ -0.69999998807907f, -1.7000000476837f,  -9.3000001907349f,  44.255809783936f, 3.6051800251007f  },
		{ 16.0f,              -56.099998474121f,  7.5f,               44.468811035156f, 0.77380001544952f },
		{ -0.69999998807907f, -1.7000000476837f,  -9.3000001907349f,  44.255809783936f, 3.6051800251007f  },
		{ -1.3999999761581f,  -41.5f,             -120.30000305176f,  44.265800476074f, 0.3585000038147f  },
		{ 1.6000000238419f,   3.5f,               -0.20000000298023f, 44.75479888916f,  0.48769000172615f },
		{ -5.0999999046326f,  -9.5f,              2.0f,               43.715789794922f, 0.44014000892639f },
		{ -1.3999999761581f,  -41.5f,             -120.30000305176f,  44.265800476074f, 0.3585000038147f  },
		{ -1.3999999761581f,  -41.5f,             -120.30000305176f,  44.265800476074f, 0.3585000038147f  },
		{ -50.099998474121f,  20.0f,              -139.5f,            43.179790496826f, 0.69836002588272f },
		{ 60.700000762939f,   27.60000038147f,    -146.30000305176f,  43.265789031982f, 0.81453001499176f },
		{ 0.60000002384186f,  -1.6000000238419f,  -0.5f,              38.538738250732f, 0.90254002809525f },
		{ 0.60000002384186f,  -1.6000000238419f,  -0.5f,              38.538738250732f, 0.90254002809525f },
		{ 0.40000000596046f,  0.5f,               -0.60000002384186f, 38.68675994873f,  0.66345000267029f },
		{ -22.700000762939f,  -1.7999999523163f,  -12.300000190735f,  5.8997898101807f, 0.25036001205444f },
		{ 4.1999998092651f,   -13.199999809265f,  4.0999999046326f,   43.32479095459f,  0.21465000510216f },
		{ -8.5f,              -8.1000003814697f,  10.199999809265f,   42.137790679932f, 0.16608999669552f },
		{ -8.5f,              -8.1000003814697f,  10.199999809265f,   43.388809204102f, 0.54038000106812f },
		{ -8.5f,              -8.1000003814697f,  10.199999809265f,   43.388809204102f, 0.54038000106812f },
		{ -8.5f,              -8.1000003814697f,  10.199999809265f,   43.388809204102f, 0.54038000106812f },
		{ -0.89999997615814f, -14.10000038147f,   1.7000000476837f,   0.0f,             1.0f              },
		{ -0.89999997615814f, -14.10000038147f,   1.7000000476837f,   0.0f,             1.0f              },
		{ -2.7000000476837f,  9.1000003814697f,   -2.9000000953674f,  43.391819000244f, 0.54038000106812f },
		{ -6.0999999046326f,  -0.69999998807907f, -2.0f,              43.391819000244f, 0.69836002588272f },
		{ 0.40000000596046f,  -7.0f,              1.7999999523163f,   43.211811065674f, 1.6702300310135f  },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
		{ 281.89999389648f,   0.89999997615814f,  8.3999996185303f,   5.0027899742126f, 0.18402999639511f },
		{ -1.8999999761581f,  0.89999997615814f,  -55.0f,             43.142780303955f, 0.14989000558853f },
#if VERSION >= VERSION_NTSC_1_0
		{ -3.7999999523163f,  6.1999998092651f,   1.0f,               5.6747899055481f, 0.29199999570847f },
#endif
		{ -3.7999999523163f,  6.1999998092651f,   1.0f,               5.8997898101807f, 2.0506100654602f  },
	};

	s32 useindex;
	struct weapon *weapon;
	u32 stack;
	s32 wantindex;

	useindex = weaponnum - 2;
	wantindex = useindex;

	if ((u32)wantindex < 0 || wantindex >= ARRAYCOUNT(gunconfig)) {
		useindex = 0;
	}

	if (weaponHasFlag(weaponnum, WEAPONFLAG_HIDEMENUMODEL) == false && (u32)wantindex >= 0 && useindex >= 0) {
		weapon = weaponFindById(weaponnum);

		g_Menus[g_MpPlayerNum].menumodel.loaddelay = 8;
		g_Menus[g_MpPlayerNum].menumodel.curparams = 0;
		g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(weaponGetFileNum(weaponnum));

		g_Menus[g_MpPlayerNum].menumodel.curposx = g_Menus[g_MpPlayerNum].menumodel.newposx = 0;
		g_Menus[g_MpPlayerNum].menumodel.curposy = g_Menus[g_MpPlayerNum].menumodel.newposy = 0;
		g_Menus[g_MpPlayerNum].menumodel.curposz = g_Menus[g_MpPlayerNum].menumodel.newposz = 0;

		g_Menus[g_MpPlayerNum].menumodel.currotz = g_Menus[g_MpPlayerNum].menumodel.newrotz = 0;

		g_Menus[g_MpPlayerNum].menumodel.displacex = gunconfig[useindex][0];
		g_Menus[g_MpPlayerNum].menumodel.displacey = gunconfig[useindex][1];
		g_Menus[g_MpPlayerNum].menumodel.displacez = gunconfig[useindex][2];

		g_Menus[g_MpPlayerNum].menumodel.newrotx = gunconfig[useindex][3];
		g_Menus[g_MpPlayerNum].menumodel.currotx = gunconfig[useindex][3];

		menuConfigureModel(&g_Menus[g_MpPlayerNum].menumodel, 0, 0, 0, 0, 0, 0, gunconfig[useindex][4], MENUMODELFLAG_HASSCALE);

		g_Menus[g_MpPlayerNum].menumodel.curscale = 0;
		g_Menus[g_MpPlayerNum].menumodel.partvisibility = weapon->partvisibility;
		g_Menus[g_MpPlayerNum].menumodel.zoom = -1;

		// These indexes correspond to WEAPON_DISGUISE40 and WEAPON_DISGUISE41
		if (wantindex == 0x3e || wantindex == 0x3f) {
			if ((u32)wantindex == 0x3e) {
				g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_MP_HEADBODY(MPHEAD_DARK_FROCK, MPBODY_DARKLAB);
			} else {
				g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_MP_HEADBODY(MPHEAD_DARK_COMBAT, MPBODY_DARK_AF1);
			}

			g_Menus[g_MpPlayerNum].menumodel.partvisibility = NULL;
			g_Menus[g_MpPlayerNum].menumodel.removingpiece = false;

			menuConfigureModel(&g_Menus[g_MpPlayerNum].menumodel, 0, 0, 0, 0, 0, 0, 1, MENUMODELFLAG_HASSCALE);

			g_Menus[g_MpPlayerNum].menumodel.rottimer60 = TICKS(60);
			g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = TICKS(120);
			g_Menus[g_MpPlayerNum].menumodel.curroty = g_Menus[g_MpPlayerNum].menumodel.newroty = -0.2f;
		}
	} else {
		g_Menus[g_MpPlayerNum].menumodel.bodymodeldef = NULL;
		g_Menus[g_MpPlayerNum].menumodel.curparams = 0;
		g_Menus[g_MpPlayerNum].menumodel.newparams = 0;
	}
}

MenuDialogHandlerResult inventoryMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog && g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 -= g_Vars.diffframe60;
			g_Menus[g_MpPlayerNum].menumodel.newroty = 18.849555969238f * g_20SecIntervalFrac;
			g_Menus[g_MpPlayerNum].menumodel.curroty = 18.849555969238f * g_20SecIntervalFrac;
			g_Menus[g_MpPlayerNum].menumodel.currotz = 0;
			g_Menus[g_MpPlayerNum].menumodel.newrotz = 0;

			if (var80072d88 != g_InventoryWeapon) {
				func0f105948(g_InventoryWeapon);
				var80072d88 = g_InventoryWeapon;
			}

			if (g_InventoryWeapon == WEAPON_DISGUISE40 || g_InventoryWeapon == WEAPON_DISGUISE41) {
				g_Menus[g_MpPlayerNum].menumodel.newanimnum = ANIM_006A;
				g_Menus[g_MpPlayerNum].menumodel.rottimer60 = TICKS(60);
				g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = TICKS(120);
			}
		} else {
			var80072d88 = 255;
		}
	}

	return 0;
}

/**
 * Return name, but if there is no manufacturer then return a blank value
 * because the name is being shown in the manufacturer slot.
 */
char *invMenuTextWeaponName(struct menuitem *item)
{
	struct weapon *weapon = weaponFindById(g_InventoryWeapon);

	if (weapon) {
		if (weapon->manufacturer == L_GUN_000) { // "\n"
			return langGet(L_OPTIONS_003); // "\n"
		}

		return langGet(weapon->name);
	}

	return langGet(L_OPTIONS_003); // "\n"
}

/**
 * Return manufacturer, with fallback to weapon name if manufacturer is blank.
 */
char *invMenuTextWeaponManufacturer(struct menuitem *item)
{
	struct weapon *weapon = weaponFindById(g_InventoryWeapon);
	u32 textid = L_GUN_000; // "\n"

	if (weapon) {
		textid = weapon->manufacturer;
	}

	if (textid != L_GUN_000) {
		return langGet(textid);
	}

	weapon = weaponFindById(g_InventoryWeapon);

	if (weapon) {
		return langGet(weapon->name);
	}

	return langGet(L_OPTIONS_003); // "\n"
}

char *invMenuTextWeaponDescription(struct menuitem *item)
{
	struct weapon *weapon = weaponFindById(g_InventoryWeapon);

	if (weapon) {
		if (g_InventoryWeapon == WEAPON_EYESPY && g_Vars.currentplayer->eyespy) {
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				return langGet(L_GUN_237); // Drugspy description
			}

			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_BOMBSPY) {
				return langGet(L_GUN_236); // Bombspy description
			}
		}

		if (g_InventoryWeapon == WEAPON_NECKLACE
				&& g_Vars.stagenum == (VERSION >= VERSION_NTSC_1_0 ? STAGE_ATTACKSHIP : STAGE_SKEDARRUINS)
				&& lvGetDifficulty() >= DIFF_PA) {
#if VERSION >= VERSION_NTSC_1_0
			// Phrases included here to assist people searching the code for them:
			// CDV780322
			// I8MOZYM8NDI85

			u8 username[] = {
				'C' + 9 * 1,
				'D' + 9 * 2,
				'V' + 9 * 3,
				'7' + 9 * 4,
				'8' + 9 * 5,
				'0' + 9 * 6,
				'3' + 9 * 7,
				'2' + 9 * 8,
				'2' + 9 * 9,
				'\0' + 9 * 10,
			};

			u8 password[] = {
				'I' + 4 * 1,
				'8' + 4 * 2,
				'M' + 4 * 3,
				'O' + 4 * 4,
				'Z' + 4 * 5,
				'Y' + 4 * 6,
				'M' + 4 * 7,
				'8' + 4 * 8,
				'N' + 4 * 9,
				'D' + 4 * 10,
				'I' + 4 * 11,
				'8' + 4 * 12,
				'5' + 4 * 13,
				'\0' + 4 * 14,
			};

			s32 i;

			for (i = 0; i < ARRAYCOUNT(username); i++) {
				username[i] -= i * 9 + 9;
			}

			for (i = 0; i < ARRAYCOUNT(password); i++) {
				password[i] -= i * 4 + 4;
			}

			// "Cassandra De Vries' replacement necklace.  Username: %s  Password: %s"
			sprintf(g_StringPointer, langGet(L_GUN_239), &username, &password);
			return g_StringPointer;
#else
			// ntsc-beta stores the whole thing as a single plain text string
			return langGet(L_GUN_239);
#endif
		}

		return langGet(weapon->description);
	}

	return langGet(L_OPTIONS_003); // "\n"
}

struct menuitem g_SoloMissionInventoryMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		0x0000006e,
		(VERSION >= VERSION_JPN_FINAL ? 0x54 : 0x63),
		menuhandlerInventoryList,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_NEWCOLUMN | MENUITEMFLAG_00000002 | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextWeaponManufacturer,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LABEL_ALTCOLOUR | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextWeaponName,
		NULL,
	},
	{
		MENUITEMTYPE_MODEL,
		0,
		0,
		0x0000008c,
		(VERSION >= VERSION_JPN_FINAL ? 0x14 : 0x37),
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextPrimaryFunction,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextSecondaryFunction,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		(uintptr_t)&invMenuTextWeaponDescription,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_FrWeaponsAvailableMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		0x0000006e,
		0x00000063,
		menuhandlerFrInventoryList,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_NEWCOLUMN | MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextWeaponManufacturer,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LABEL_ALTCOLOUR | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextWeaponName,
		NULL,
	},
	{
		MENUITEMTYPE_MODEL,
		0,
		0,
		0x0000008c,
		0x00000037,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextPrimaryFunction,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_003, // ""
		(uintptr_t)&invMenuTextSecondaryFunction,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		(uintptr_t)&invMenuTextWeaponDescription,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionInventoryMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_178, // "Inventory"
	g_SoloMissionInventoryMenuItems,
	inventoryMenuDialog,
#if VERSION >= VERSION_JPN_FINAL
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE | MENUDIALOGFLAG_0400 | MENUDIALOGFLAG_1000,
#else
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE | MENUDIALOGFLAG_0400,
#endif
	&g_SoloMissionOptionsMenuDialog,
};

struct menudialogdef g_FrWeaponsAvailableMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_179, // "Weapons Available"
	g_FrWeaponsAvailableMenuItems,
	inventoryMenuDialog,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE | MENUDIALOGFLAG_0400,
	NULL,
};

MenuItemHandlerResult menuhandlerFrInventoryList(s32 operation, struct menuitem *item, union handlerdata *data)
{
	static u8 g_FrFocusedSlotIndex = 0;

	switch (operation) {
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 0;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return 0;
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = 0;
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = frGetNumWeaponsAvailable();
		break;
	case MENUOP_GETOPTIONTEXT:
		g_FrFocusedSlotIndex = data->list.value;
		return (uintptr_t)bgunGetName(frGetWeaponBySlot(data->list.value));
	case MENUOP_SET:
		g_FrFocusedSlotIndex = data->list.value;
		return 0;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = g_FrFocusedSlotIndex;
		break;
	case MENUOP_LISTITEMFOCUS:
		g_InventoryWeapon = frGetWeaponBySlot(data->list.value);
		g_Menus[g_MpPlayerNum].training.weaponnum = g_InventoryWeapon;
		g_FrFocusedSlotIndex = data->list.value;

		// These items are labels
		func0f0f139c(&g_SoloMissionInventoryMenuItems[1], -1.0f); // manufacturer
		func0f0f139c(&g_SoloMissionInventoryMenuItems[2], -1.0f); // weapon name
		func0f0f139c(&g_SoloMissionInventoryMenuItems[4], -1.0f); // primary function
		func0f0f139c(&g_SoloMissionInventoryMenuItems[5], -1.0f); // secondary function
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerInventoryList(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = invGetCount();
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t)invGetNameByIndex(data->list.value);
	case MENUOP_SET:
		{
			s32 weaponnum = invGetWeaponNumByIndex(data->list.value);
			bool equippable = true;

			if (weaponnum != WEAPON_NONE) {
				s32 state = currentPlayerGetDeviceState(weaponnum);

				if (state != DEVICESTATE_UNEQUIPPED) {
					equippable = false;

					if (data->list.unk04 == 0) {
						if (state == DEVICESTATE_INACTIVE) {
							currentPlayerSetDeviceActive(weaponnum, true);
						} else {
							currentPlayerSetDeviceActive(weaponnum, false);
						}
					}
				}
			}

			if (equippable) {
				invSetCurrentIndex(data->list.value);

				if (invHasDoubleWeaponIncAllGuns(weaponnum, weaponnum)) {
					bgunEquipWeapon2(HAND_RIGHT, weaponnum);
					bgunEquipWeapon2(HAND_LEFT, weaponnum);
				} else {
					bgunEquipWeapon2(HAND_RIGHT, weaponnum);
					// don't unequip detonator
					// if we already have it equipped
					if (weaponnum == WEAPON_REMOTEMINE) {
						bgunEquipWeapon2(HAND_LEFT, weaponnum);
					} else {
						bgunEquipWeapon2(HAND_LEFT, WEAPON_NONE);
					}
				}
			}

			var800711f0 = data->list.value;
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = invGetCurrentIndex();
		break;
	case MENUOP_GETLISTITEMCHECKBOX:
		{
			s32 weaponnum = invGetWeaponNumByIndex(data->list.value);

			if (weaponnum != WEAPON_NONE) {
				s32 state = currentPlayerGetDeviceState(weaponnum);

				if (state != DEVICESTATE_UNEQUIPPED) {
					data->list.unk04 = state;
				}
			}
		}
		break;
	case MENUOP_LISTITEMFOCUS:
		g_InventoryWeapon = invGetWeaponNumByIndex(data->list.value);
		g_Menus[g_MpPlayerNum].training.weaponnum = g_InventoryWeapon;

		func0f0f139c(&g_SoloMissionInventoryMenuItems[1], -1);
		func0f0f139c(&g_SoloMissionInventoryMenuItems[2], -1);
		func0f0f139c(&g_SoloMissionInventoryMenuItems[4], -1);
		func0f0f139c(&g_SoloMissionInventoryMenuItems[5], -1);
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerAbortMission(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Vars.currentplayer->aborted = true;
		mainEndStage();
	}

	return 0;
}

MenuDialogHandlerResult menudialogAbortMission(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		// empty
	}

	return 0;
}

struct menuitem g_MissionAbortMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_OPTIONS_175, // "Do you want to abort the mission?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_176, // "Cancel"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_177, // "Abort"
		0,
		menuhandlerAbortMission,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MissionAbortMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_174, // "Warning"
	g_MissionAbortMenuItems,
	menudialogAbortMission,
	0,
	NULL,
};

struct menuitem g_2PMissionAbortVMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_MPWEAPONS_155, // "Do you want to abort the mission?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_176, // "Cancel"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_177, // "Abort"
		0,
		menuhandlerAbortMission,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_2PMissionAbortVMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_174, // "Warning"
	g_2PMissionAbortVMenuItems,
	menudialogAbortMission,
	0,
	NULL,
};

MenuDialogHandlerResult soloMenuDialogPauseStatus(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		struct briefingobj *briefing = g_BriefingObjs;
		struct objective *objective;
		s32 wanttype = BRIEFINGTYPE_TEXT_PA;
		s32 i;

		if (lvGetDifficulty() == DIFF_A) {
			wanttype = BRIEFINGTYPE_TEXT_A;
		}

		if (lvGetDifficulty() == DIFF_SA) {
			wanttype = BRIEFINGTYPE_TEXT_SA;
		}

		g_Briefing.briefingtextnum = L_MISC_042; // "No briefing for this mission"

		while (briefing) {
			if (briefing->type == BRIEFINGTYPE_TEXT_PA) {
				g_Briefing.briefingtextnum = briefing->text;
			}

			if (briefing->type == wanttype) {
				g_Briefing.briefingtextnum = briefing->text;
				break;
			}

			briefing = briefing->next;
		}

		for (i = 0; i < objectiveGetCount(); i++) {
			if (g_Objectives[i]) {
				g_Briefing.objectivenames[i] = g_Objectives[i]->text;
				g_Briefing.objectivedifficulties[i] = objectiveGetDifficultyBits(i);
			}
		}
	}

	return 0;
}

char *soloMenuTitlePauseStatus(struct menudialogdef *dialogdef)
{
	if (dialogdef != g_Menus[g_MpPlayerNum].curdialog->definition) {
		return langGet(L_OPTIONS_172); // "Status"
	}

	sprintf(g_StringPointer, "%s: %s\n",
			langGet(g_SoloStages[g_MissionConfig.stageindex].name3),
			langGet(L_OPTIONS_172));

	return g_StringPointer;
}

struct menuitem g_2PMissionPauseVMenuItems[] = {
	{
		MENUITEMTYPE_OBJECTIVES,
		2,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_173, // "Abort!"
		0,
		(void *)&g_2PMissionAbortVMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_MissionPauseMenuItems[] = {
	{
		MENUITEMTYPE_OBJECTIVES,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_OPTIONS_173, // "Abort!"
		0,
		(void *)&g_MissionAbortMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloMissionPauseMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&soloMenuTitlePauseStatus,
	g_MissionPauseMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_SoloMissionInventoryMenuDialog,
};

struct menudialogdef g_2PMissionPauseHMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&soloMenuTitlePauseStatus,
	g_MissionPauseMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_2PMissionInventoryHMenuDialog,
};

struct menudialogdef g_2PMissionPauseVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_172, // "Status"
	g_2PMissionPauseVMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_2PMissionInventoryVMenuDialog,
};

struct cutscene g_Cutscenes[] = {
	// stage ID, mission, scene, name
	{ /* 0*/ STAGE_DEFECTION,      0, 0, L_OPTIONS_450 },
	{ /* 1*/ STAGE_DEFECTION,      0, 1, L_OPTIONS_451 },
	{ /* 2*/ STAGE_INVESTIGATION,  1, 0, L_OPTIONS_452 },
	{ /* 3*/ STAGE_INVESTIGATION,  1, 1, L_OPTIONS_453 },
	{ /* 4*/ STAGE_EXTRACTION,     2, 0, L_OPTIONS_454 },
	{ /* 5*/ STAGE_EXTRACTION,     2, 1, L_OPTIONS_455 },
	{ /* 6*/ STAGE_VILLA,          3, 0, L_OPTIONS_456 },
#if VERSION < VERSION_NTSC_1_0
	{ /* 7*/ STAGE_VILLA,          3, 1, L_OPTIONS_457 },
#endif
	{ /* 7*/ STAGE_VILLA,          3, 2, L_OPTIONS_458 },
	{ /* 8*/ STAGE_CHICAGO,        4, 0, L_OPTIONS_459 },
	{ /* 9*/ STAGE_CHICAGO,        4, 1, L_OPTIONS_460 },
	{ /*10*/ STAGE_G5BUILDING,     5, 0, L_OPTIONS_461 },
	{ /*11*/ STAGE_G5BUILDING,     5, 1, L_OPTIONS_462 },
	{ /*12*/ STAGE_G5BUILDING,     5, 2, L_OPTIONS_463 },
	{ /*13*/ STAGE_INFILTRATION,   6, 0, L_OPTIONS_464 },
	{ /*14*/ STAGE_INFILTRATION,   6, 1, L_OPTIONS_465 },
	{ /*15*/ STAGE_RESCUE,         7, 0, L_OPTIONS_466 },
	{ /*16*/ STAGE_RESCUE,         7, 1, L_OPTIONS_467 },
	{ /*17*/ STAGE_ESCAPE,         8, 0, L_OPTIONS_468 },
	{ /*18*/ STAGE_ESCAPE,         8, 1, L_OPTIONS_469 },
	{ /*19*/ STAGE_ESCAPE,         8, 2, L_OPTIONS_470 },
	{ /*20*/ STAGE_AIRBASE,        9, 0, L_OPTIONS_471 },
	{ /*21*/ STAGE_AIRBASE,        9, 1, L_OPTIONS_472 },
	{ /*22*/ STAGE_AIRFORCEONE,   10, 0, L_OPTIONS_473 },
	{ /*23*/ STAGE_AIRFORCEONE,   10, 1, L_OPTIONS_474 },
	{ /*24*/ STAGE_AIRFORCEONE,   10, 2, L_OPTIONS_475 },
	{ /*25*/ STAGE_CRASHSITE,     11, 0, L_OPTIONS_476 },
	{ /*26*/ STAGE_CRASHSITE,     11, 1, L_OPTIONS_477 },
	{ /*27*/ STAGE_PELAGIC,       12, 0, L_OPTIONS_478 },
	{ /*28*/ STAGE_PELAGIC,       12, 1, L_OPTIONS_479 },
	{ /*29*/ STAGE_DEEPSEA,       13, 0, L_OPTIONS_480 },
	{ /*30*/ STAGE_DEEPSEA,       13, 1, L_OPTIONS_481 },
	{ /*31*/ STAGE_DEEPSEA,       13, 2, L_OPTIONS_482 },
	{ /*32*/ STAGE_DEFENSE,       14, 0, L_OPTIONS_483 },
	{ /*33*/ STAGE_DEFENSE,       14, 1, L_OPTIONS_484 },
	{ /*34*/ STAGE_ATTACKSHIP,    15, 0, L_OPTIONS_485 },
	{ /*35*/ STAGE_ATTACKSHIP,    15, 1, L_OPTIONS_486 },
	{ /*36*/ STAGE_SKEDARRUINS,   16, 0, L_OPTIONS_487 },
	{ /*37*/ STAGE_SKEDARRUINS,   16, 1, L_OPTIONS_488 },
};

u32 g_CutsceneCountsByMission[] = {
	/* 0*/ 1,  // 0 missions completed => 1 cutscene available (Def intro)
	/* 1*/ 3,  // 1 mission completed => 3 cutscenes available (Def intro, outro, Invest intro)
	/* 2*/ 5,
	/* 3*/ 7,
#if VERSION >= VERSION_NTSC_1_0
	// NTSC beta has an extra Villa cutscene
	// so the numbers are bumped forward in that version
	/* 4*/ 9,
	/* 5*/ 11,
	/* 6*/ 14,
	/* 7*/ 16,
	/* 8*/ 18,
	/* 9*/ 21,
	/*10*/ 23,
	/*11*/ 26,
	/*12*/ 28,
	/*13*/ 30,
	/*14*/ 33,
	/*15*/ 35,
	/*16*/ 37,
	/*17*/ 38,
#else
	/* 4*/ 10,
	/* 5*/ 12,
	/* 6*/ 15,
	/* 7*/ 17,
	/* 8*/ 19,
	/* 9*/ 22,
	/*10*/ 24,
	/*11*/ 27,
	/*12*/ 29,
	/*13*/ 31,
	/*14*/ 34,
	/*15*/ 36,
	/*16*/ 38,
	/*17*/ 39,
#endif
};

s32 getNumCompletedMissions(void)
{
	s32 s;
	s32 d;
	s32 count = 0;

	for (s = 0; s != 17; s++) {
		bool done = false;

		for (d = 0; d != 3; d++) {
			if (g_GameFile.besttimes[s][d] || (g_GameFile.coopcompletions[d] & (1 << s))) {
				count++;
				done = true;
				break;
			}
		}

		if (!done) {
			break;
		}
	}

	return count;
}

struct cutscenegroup {
	u32 first_cutscene_index;
	u16 name;
};

MenuItemHandlerResult menuhandlerCinema(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct cutscenegroup groups[] = {
		{ /* 0*/  0, L_OPTIONS_436 }, // "Special"
		{ /* 1*/  1, L_OPTIONS_438 }, // "Mission 1 - dataDyne Central"
		{ /* 2*/  7, L_OPTIONS_439 },
#if VERSION >= VERSION_NTSC_1_0
		{ /* 3*/  9, L_OPTIONS_440 },
		{ /* 4*/ 14, L_OPTIONS_441 },
		{ /* 5*/ 21, L_OPTIONS_442 },
		{ /* 6*/ 28, L_OPTIONS_443 },
		{ /* 7*/ 33, L_OPTIONS_444 },
		{ /* 8*/ 35, L_OPTIONS_445 },
		{ /* 9*/ 37, L_OPTIONS_446 }, // "Mission 9 - Skedar Ruins"
		{ /*10*/ 39, L_OPTIONS_447 }, // "Finale"
#else
		{ /* 3*/ 10, L_OPTIONS_440 },
		{ /* 4*/ 15, L_OPTIONS_441 },
		{ /* 5*/ 22, L_OPTIONS_442 },
		{ /* 6*/ 29, L_OPTIONS_443 },
		{ /* 7*/ 34, L_OPTIONS_444 },
		{ /* 8*/ 36, L_OPTIONS_445 },
		{ /* 9*/ 38, L_OPTIONS_446 }, // "Mission 9 - Skedar Ruins"
		{ /*10*/ 40, L_OPTIONS_447 }, // "Finale"
#endif
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		// Add one for Play All option
		data->list.value = g_CutsceneCountsByMission[getNumCompletedMissions()] + 1;
		break;
	case MENUOP_GETOPTIONTEXT:
		if (data->list.value == 0) {
			sprintf(g_StringPointer, langGet(L_OPTIONS_448)); // "Play All"
			return (uintptr_t) g_StringPointer;
		}
		return (uintptr_t) langGet(g_Cutscenes[data->list.value - 1].name);
	case MENUOP_SET:
		if (data->list.value == 0) {
			// Play all
			s32 index = getNumCompletedMissions();
			g_Vars.autocutgroupcur = 0;
			g_Vars.autocutgroupleft = g_CutsceneCountsByMission[index];
			menuPopDialog();
			menuStop();
		} else {
			// Play specific cutscene
			g_Vars.autocutgroupcur = data->list.value - 1;
			g_Vars.autocutgroupleft = 1;
			menuPopDialog();
			menuStop();
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = ARRAYCOUNT(groups);
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return (uintptr_t) langGet(groups[data->list.value].name);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = groups[data->list.value].first_cutscene_index;
		break;
	}

	return 0;
}

struct menuitem g_CinemaMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		0x000000eb,
		0,
		menuhandlerCinema,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CinemaMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_490, // "Cinema"
	g_CinemaMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

struct menuitem g_SelectMissionMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x000000eb,
		0,
		menuhandlerMissionList,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SelectMissionMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_122, // "Mission Select"
	g_SelectMissionMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult menuhandlerMainMenuSoloMissions(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_MissionConfig.iscoop = false;
		g_MissionConfig.isanti = false;
		menuPushDialog(&g_SelectMissionMenuDialog);
	}

	if (operation == MENUOP_CHECKPREFOCUSED) {
		if (isStageDifficultyUnlocked(SOLOSTAGEINDEX_INVESTIGATION, DIFF_A)) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMainMenuCombatSimulator(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Vars.bondplayernum = 0;
		g_Vars.coopplayernum = -1;
		g_Vars.antiplayernum = -1;
		challengeDetermineUnlockedFeatures();
		g_Vars.mpsetupmenu = MPSETUPMENU_GENERAL;
		func0f0f820c(&g_CombatSimulatorMenuDialog, MENUROOT_MPSETUP);
		func0f0f8300();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMainMenuCooperative(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_MissionConfig.iscoop = true;
		g_MissionConfig.isanti = false;
		menuPushDialog(&g_SelectMissionMenuDialog);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMainMenuCounterOperative(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		if ((joyGetConnectedControllers() & 2) == 0) {
			return true;
		}
	}

	if (operation == MENUOP_SET) {
		g_MissionConfig.iscoop = false;
		g_MissionConfig.isanti = true;
		menuPushDialog(&g_SelectMissionMenuDialog);
	}

	return 0;
}

MenuDialogHandlerResult menudialogMainMenu(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_Menus[g_MpPlayerNum].main.unke2c = 0;
		break;
	case MENUOP_TICK:
#ifndef PLATFORM_N64
		if (g_SkipToCombatSimulator) {
			menuhandlerMainMenuCombatSimulator(MENUOP_SET, NULL, NULL);
			break;
		}
#endif
		if (g_Menus[g_MpPlayerNum].curdialog &&
				g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			g_MissionConfig.iscoop = false;
			g_MissionConfig.isanti = false;
		}
		break;
	}

	return false;
}

char *mainMenuTextLabel(struct menuitem *item)
{
	u16 nocheats[] = {
		L_OPTIONS_117, // "Solo Missions"
		L_OPTIONS_118, // "Combat Simulator"
		L_OPTIONS_119, // "Co-Operative"
		L_OPTIONS_120, // "Counter-Operative"
	};

	u16 withcheats[] = {
		L_MPWEAPONS_130, // "Cheat Solo Missions"
		L_MPWEAPONS_131, // "Cheat Combat Simulator"
		L_MPWEAPONS_132, // "Cheat Co-Operative"
		L_MPWEAPONS_133, // "Cheat Counter-Operative"
	};

	if (g_CheatsEnabledBank0 || g_CheatsEnabledBank1) {
		return langGet(withcheats[item->param]);
	}

	return langGet(nocheats[item->param]);
}

struct menuitem g_MainMenuMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_BIGFONT,
		L_MISC_446, // "Carrington Institute"
		0x00000001,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_BIGFONT,
		(uintptr_t)&mainMenuTextLabel,
		0x00000002,
		menuhandlerMainMenuSoloMissions,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		MENUITEMFLAG_BIGFONT,
		(uintptr_t)&mainMenuTextLabel,
		0x00000003,
		menuhandlerMainMenuCombatSimulator,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		MENUITEMFLAG_BIGFONT,
		(uintptr_t)&mainMenuTextLabel,
		0x00000004,
		menuhandlerMainMenuCooperative,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		3,
		MENUITEMFLAG_BIGFONT,
		(uintptr_t)&mainMenuTextLabel,
		0x00000005,
		menuhandlerMainMenuCounterOperative,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_187, // "Change Agent..."
		0x00000006,
		(void *)&g_ChangeAgentMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Exit Game",
		0x00000007,
		(void *)&g_ExitGameMenuDialog,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CiMenuViaPcMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_116, // "Perfect Menu"
	g_MainMenuMenuItems,
	menudialogMainMenu,
	MENUDIALOGFLAG_STARTSELECTS,
	&g_CiOptionsViaPcMenuDialog,
};

struct menudialogdef g_CiMenuViaPauseMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_116, // "Perfect Menu"
	g_MainMenuMenuItems,
	menudialogMainMenu,
	MENUDIALOGFLAG_STARTSELECTS,
	&g_CiOptionsViaPauseMenuDialog,
};

bool soloChoosePauseDialog(void)
{
	if (g_Menus[g_MpPlayerNum].openinhibit == 0) {
		g_Menus[g_MpPlayerNum].playernum = 0;

		if (g_Vars.stagenum == STAGE_CITRAINING) {
			bool handled = false;

			if (ciIsTourDone()) {
				struct trainingdata *dtdata = dtGetData();
				s32 room = g_Vars.currentplayer->prop->rooms[0];

				if (room >= ROOM_DISH_HOLO1 && room <= ROOM_DISH_HOLO4) {
					struct trainingdata *htdata = getHoloTrainingData();

					if (htdata->intraining) {
						menuPushRootDialog(&g_HtDetailsMenuDialog, MENUROOT_TRAINING);
					} else if (htdata->finished) {
						htPushEndscreen();
					} else {
						menuPushRootDialog(&g_HtListMenuDialog, MENUROOT_TRAINING);
					}

					handled = true;
				} else if (room == ROOM_DISH_DEVICELAB) {
					if (dtdata->intraining) {
						menuPushRootDialog(&g_DtDetailsMenuDialog, MENUROOT_TRAINING);
					} else if (dtdata->finished) {
						dtPushEndscreen();
					} else {
						menuPushRootDialog(&g_DtListMenuDialog, MENUROOT_TRAINING);
					}

					handled = true;
				} else if (dtdata->intraining) {
					menuPushRootDialog(&g_DtDetailsMenuDialog, MENUROOT_TRAINING);
					handled = true;
				} else if (dtdata->finished) {
					dtPushEndscreen();
					handled = true;
				} else if (room == ROOM_DISH_FIRINGRANGE) {
					if (frIsInTraining()) {
						menuPushRootDialog(&g_FrTrainingInfoInGameMenuDialog, MENUROOT_TRAINING);
					} else {
						menuPushRootDialog(&g_FrWeaponListMenuDialog, MENUROOT_TRAINING);
					}

					handled = true;
				}
			}

			if (!handled) {
				menuPushRootDialog(&g_CiMenuViaPauseMenuDialog, MENUROOT_MAINMENU);
				return true;
			}
		} else {
			menuPushRootDialog(&g_SoloMissionPauseMenuDialog, MENUROOT_MAINMENU);
		}

		return true;
	}

	return false;
}
