#include <ultra64.h>
#include "constants.h"
#include "game/bossfile.h"
#include "game/cheats.h"
#include "game/game_006900.h"
#include "game/title.h"
#include "game/pdmode.h"
#include "game/objectives.h"
#include "game/bondgun.h"
#include "game/debug.h"
#include "game/game_0b0fd0.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/filemgr.h"
#include "game/endscreen.h"
#include "game/stagetable.h"
#include "game/lv.h"
#include "game/mplayer/ingame.h"
#include "game/challenge.h"
#include "game/gamefile.h"
#include "game/lang.h"
#include "game/options.h"
#include "game/mpstats.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/str.h"
#include "data.h"
#include "types.h"
#include "savestate.h"

MenuItemHandlerResult endscreenHandleDeclineMission(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPopDialog();
	}

	return 0;
}

MenuDialogHandlerResult endscreenHandleRetryMission(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_TICK:
		{
			/**
			 * NTSC Final adds this check to make sure the given dialog is
			 * either the one being displayed or its sibling. This most likely
			 * fixes a bug - perhaps there is some way that this handler is run
			 * when the dialog is not on screen?
			 */
#if VERSION >= VERSION_NTSC_FINAL
			if (g_Menus[g_MpPlayerNum].curdialog) {
				if (dialogdef == g_Menus[g_MpPlayerNum].curdialog->definition
						|| (dialogdef->nextsibling && dialogdef->nextsibling == g_Menus[g_MpPlayerNum].curdialog->definition)) {
#endif
					struct menuinputs *inputs = data->dialog2.inputs;
					bool accept = false;

					if (inputs->back) {
						menuPopDialog();
						menuPopDialog();
					}

					inputs->back = false;

					if (inputs->start) {
						accept = true;
					}

					inputs->start = false;

					if (inputs->select
							&& g_Menus[g_MpPlayerNum].curdialog
							&& dialogdef->nextsibling
							&& dialogdef->nextsibling == g_Menus[g_MpPlayerNum].curdialog->definition) {
						accept = true;
						inputs->select = false;
					}

					if (accept) {
						union handlerdata data2;
						menuhandlerAcceptMission(MENUOP_SET, &dialogdef->items[1], &data2);
					}
#if VERSION >= VERSION_NTSC_FINAL
				}
			}
#endif
		}
	}

	return menudialog00103608(operation, dialogdef, data);
}

char *endscreenMenuTitleRetryMission(struct menudialogdef *dialogdef)
{
	char *name;
	char *prefix;

	if (g_Menus[g_MpPlayerNum].curdialog->definition != dialogdef) {
		return langGet(L_OPTIONS_300); // "Objectives"
	}

	prefix = langGet(L_OPTIONS_296); // "Retry"
	name = langGet(g_SoloStages[g_MissionConfig.stageindex].name3);

	sprintf(g_StringPointer, "%s: %s\n", prefix, name);

	return g_StringPointer;
}

char *endscreenMenuTitleNextMission(struct menudialogdef *dialogdef)
{
	char *name;
	char *prefix;

	if (g_Menus[g_MpPlayerNum].curdialog->definition != dialogdef) {
		return langGet(L_OPTIONS_300); // "Objectives"
	}

	prefix = langGet(L_OPTIONS_297); // "Next Mission"
	name = langGet(g_SoloStages[g_MissionConfig.stageindex].name3);

	sprintf(g_StringPointer, "%s: %s\n", prefix, name);

	return g_StringPointer;
}

MenuItemHandlerResult endscreenHandleReplayPreviousMission(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_MissionConfig.stageindex--;
		g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;
	}

	return menuhandlerAcceptMission(operation, NULL, data);
}

struct menuitem g_RetryMissionMenuItems[] = {
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
		L_OPTIONS_298, // "Accept"
		0,
		menuhandlerAcceptMission,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_299, // "Decline"
		0,
		endscreenHandleDeclineMission,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_RetryMissionMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&endscreenMenuTitleRetryMission,
	g_RetryMissionMenuItems,
	endscreenHandleRetryMission,
	MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_DISABLEITEMSCROLL,
	&g_PreAndPostMissionBriefingMenuDialog,
};

struct menuitem g_NextMissionMenuItems[] = {
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
		L_OPTIONS_298, // "Accept"
		0,
		menuhandlerAcceptMission,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_299, // "Decline"
		0,
		endscreenHandleDeclineMission,
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
		L_MISC_470, // "Replay Previous Mission"
		0,
		endscreenHandleReplayPreviousMission,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_NextMissionMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&endscreenMenuTitleNextMission,
	g_NextMissionMenuItems,
	endscreenHandleRetryMission,
	MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_DISABLEITEMSCROLL,
	&g_PreAndPostMissionBriefingMenuDialog,
};

char *endscreenMenuTextNumKills(struct menuitem *item)
{
	sprintf(g_StringPointer, "%d", mpstatsGetPlayerKillCount());
	return g_StringPointer;
}

char *endscreenMenuTextNumShots(struct menuitem *item)
{
	sprintf(g_StringPointer, "%d", mpstatsGetPlayerShotCountByRegion(SHOTREGION_TOTAL));
	return g_StringPointer;
}

char *endscreenMenuTextNumHeadShots(struct menuitem *item)
{
	sprintf(g_StringPointer, "%d", mpstatsGetPlayerShotCountByRegion(SHOTREGION_HEAD));
	return g_StringPointer;
}

char *endscreenMenuTextNumBodyShots(struct menuitem *item)
{
	sprintf(g_StringPointer, "%d", mpstatsGetPlayerShotCountByRegion(SHOTREGION_BODY));
	return g_StringPointer;
}

char *endscreenMenuTextNumLimbShots(struct menuitem *item)
{
	sprintf(g_StringPointer, "%d", mpstatsGetPlayerShotCountByRegion(SHOTREGION_LIMB));
	return g_StringPointer;
}

char *endscreenMenuTextNumOtherShots(struct menuitem *item)
{
	u32 total = mpstatsGetPlayerShotCountByRegion(SHOTREGION_GUN) + mpstatsGetPlayerShotCountByRegion(SHOTREGION_HAT);
	sprintf(g_StringPointer, "%d", total);
	return g_StringPointer;
}

char *endscreenMenuTextAccuracy(struct menuitem *item)
{
	s32 total = mpstatsGetPlayerShotCountByRegion(SHOTREGION_TOTAL);
	s32 numhead = mpstatsGetPlayerShotCountByRegion(SHOTREGION_HEAD);
	s32 numbody = mpstatsGetPlayerShotCountByRegion(SHOTREGION_BODY);
	s32 numlimb = mpstatsGetPlayerShotCountByRegion(SHOTREGION_LIMB);
	s32 numgun = mpstatsGetPlayerShotCountByRegion(SHOTREGION_GUN);
	s32 numhat = mpstatsGetPlayerShotCountByRegion(SHOTREGION_HAT);
	s32 numobject = mpstatsGetPlayerShotCountByRegion(SHOTREGION_OBJECT);
	f32 accuracy;

	if (total > 0) {
		s32 hits = numhead + numbody + numlimb + numgun + numhat + numobject;
		accuracy = hits * 100.0f / total;
	} else {
		accuracy = 0;
	}

	if (accuracy > 100.0f) {
		accuracy = 100.0f;
	}

	sprintf(g_StringPointer, "%s%s%.1f%%", "", "", accuracy);
	return g_StringPointer;
}

char *endscreenMenuTextMissionStatus(struct menuitem *item)
{
	if (g_CheatsActiveBank0 || g_CheatsActiveBank1) {
		return langGet(L_MPWEAPONS_135); // "Cheated"
	}

	if (g_Vars.coopplayernum >= 0) {
		if (g_Vars.bond->aborted || g_Vars.coop->aborted) {
			return langGet(L_OPTIONS_295); // "Aborted"
		}

		if (g_Vars.bond->isdead && g_Vars.coop->isdead) {
			return langGet(L_OPTIONS_293); // "Failed"
		}
	} else if (g_Vars.antiplayernum >= 0) {
		if (g_Vars.currentplayer == g_Vars.bond) {
			if (g_Vars.bond->aborted) {
				return langGet(L_OPTIONS_295); // "Aborted"
			}

			if (g_Vars.anti->aborted) {
				return langGet(L_OPTIONS_295); // "Aborted"
			}

			if (g_Vars.bond->isdead) {
				return langGet(L_OPTIONS_293); // "Failed"
			}
		} else {
			if (g_Vars.anti->aborted) {
				return langGet(L_OPTIONS_295); // "Aborted"
			}

			if (!g_Vars.bond->aborted && !g_Vars.bond->isdead) {
				return langGet(L_OPTIONS_293); // "Failed"
			}
		}
	} else {
		if (g_Vars.bond->aborted) {
			return langGet(L_OPTIONS_295); // "Aborted"
		}

		if (g_Vars.bond->isdead) {
			return langGet(L_OPTIONS_293); // "Failed"
		}
	}

	if (objectiveIsAllComplete() == false) {
		return langGet(L_OPTIONS_293); // "Failed"
	}

	if (g_StageIndex == STAGEINDEX_DEFENSE) {
		return langGet(L_MPWEAPONS_062); // "Unknown"
	}

	return langGet(L_OPTIONS_294); // "Completed"
}

char *endscreenMenuTextAgentStatus(struct menuitem *item)
{
	if (g_CheatsActiveBank0 || g_CheatsActiveBank1) {
		return langGet(L_MPWEAPONS_134); // "Dishonored"
	}

	if (g_Vars.currentplayer->aborted) {
		return langGet(L_OPTIONS_292); // "Disavowed"
	}

	if (g_Vars.currentplayer->isdead) {
		return langGet(L_OPTIONS_290); // "Deceased"
	}

	if (g_StageIndex == STAGEINDEX_DEFENSE) {
		return langGet(L_MPWEAPONS_063); // "Missing"
	}

	return langGet(L_OPTIONS_291); // "Active"
}

char *endscreenMenuTitleStageCompleted(struct menuitem *item)
{
#if VERSION >= VERSION_NTSC_1_0
	sprintf(g_StringPointer, "%s: %s\n",
			langGet(g_SoloStages[g_Menus[g_MpPlayerNum].endscreen.stageindex].name3),
			langGet(L_OPTIONS_276)); // "Completed"
#else
	sprintf(g_StringPointer, "%s: %s\n",
			langGet(g_SoloStages[g_MissionConfig.stageindex].name3),
			langGet(L_OPTIONS_276)); // "Completed"
#endif

	return g_StringPointer;
}

#if VERSION >= VERSION_NTSC_1_0
char *endscreenMenuTextCurrentStageName3(struct menuitem *item)
{
	char *name = langGet(g_SoloStages[g_MissionConfig.stageindex].name3);
	sprintf(g_StringPointer, "%s\n", name);

	return g_StringPointer;
}
#endif

char *endscreenMenuTitleStageFailed(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s: %s\n",
			langGet(g_SoloStages[g_MissionConfig.stageindex].name3),
			langGet(L_OPTIONS_277)); // "Failed"

	return g_StringPointer;
}

char *endscreenMenuTextMissionTime(struct menuitem *item)
{
	formatTime(g_StringPointer, playerGetMissionTime(), TIMEPRECISION_SECONDS);
	strcat(g_StringPointer, "\n");

	return g_StringPointer;
}

struct menudialogdef *endscreenAdvance(void)
{
#if VERSION < VERSION_NTSC_1_0
	if (g_MissionConfig.stagenum == STAGE_SKEDARRUINS) {
		g_MissionConfig.stagenum = STAGE_CREDITS;
		titleSetNextStage(g_MissionConfig.stagenum);
		lvSetDifficulty(g_MissionConfig.difficulty);
		titleSetNextMode(TITLEMODE_SKIP);
		mainChangeToStage(g_MissionConfig.stagenum);

		return NULL;
	}
#endif

	g_MissionConfig.stageindex++;
	g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;

	return &g_NextMissionMenuDialog;
}

void endscreenResetModels(void)
{
	menuResetModel(&g_Menus[0].menumodel, bgunCalculateGunMemCapacity() - menugfxGetParticleArraySize(), false);
	g_Menus[0].menumodel.allocstart = bgunGetGunMem() + menugfxGetParticleArraySize();

	menuResetModel(&g_Menus[1].menumodel, bgunCalculateGunMemCapacity() - menugfxGetParticleArraySize(), false);
	g_Menus[1].menumodel.allocstart = bgunGetGunMem() + menugfxGetParticleArraySize();

	menuResetModel(&g_Menus[2].menumodel, bgunCalculateGunMemCapacity() - menugfxGetParticleArraySize(), false);
	g_Menus[2].menumodel.allocstart = bgunGetGunMem() + menugfxGetParticleArraySize();

	menuResetModel(&g_Menus[3].menumodel, bgunCalculateGunMemCapacity() - menugfxGetParticleArraySize(), false);
	g_Menus[3].menumodel.allocstart = bgunGetGunMem() + menugfxGetParticleArraySize();
}

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult endscreenHandleReplayLastLevel(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;
		return menuhandlerAcceptMission(operation, NULL, data);
	}

	return 0;
}
#endif

struct menuitem g_2PMissionEndscreenObjectivesVMenuItems[] = {
	{
		MENUITEMTYPE_OBJECTIVES,
		2,
		0,
		0,
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
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_301, // "Press START"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_SoloEndscreenObjectivesMenuItems[] = {
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
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Load Checkpoint\n",
		0,
		menuhandlerLoadCheckpoint,
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
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_301, // "Press START"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_SoloEndscreenObjectivesFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_300, // "Objectives"
	g_SoloEndscreenObjectivesMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	NULL,
};

struct menudialogdef g_SoloEndscreenObjectivesCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_OPTIONS_300, // "Objectives"
	g_SoloEndscreenObjectivesMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	NULL,
};

struct menudialogdef g_2PMissionEndscreenObjectivesFailedVMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_300, // "Objectives"
	g_2PMissionEndscreenObjectivesVMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	NULL,
};

struct menudialogdef g_2PMissionEndscreenObjectivesCompletedVMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_OPTIONS_300, // "Objectives"
	g_2PMissionEndscreenObjectivesVMenuItems,
	soloMenuDialogPauseStatus,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	NULL,
};

#if VERSION >= VERSION_NTSC_1_0
/**
 * Displayed after Defense and Skedar Ruins completion screens.
 */
MenuItemHandlerResult endscreenHandleContinueMission(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		endscreenContinue(2);
	}

	return 0;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
struct menuitem g_MissionContinueOrReplyMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_244, // "Continue"
		0,
		endscreenHandleContinueMission,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_245, // "Replay Last Level"
		0,
		endscreenHandleReplayLastLevel,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MissionContinueOrReplyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&endscreenMenuTextCurrentStageName3,
	g_MissionContinueOrReplyMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};
#endif

#if VERSION >= VERSION_NTSC_1_0
/**
 * Context is:
 *
 * 0 when closing a completed endscreen
 * 1 unsure - is invoked directly by menuTick
 * 2 when pressing continue
 */
void endscreenContinue(s32 context)
{
	if (g_Vars.antiplayernum >= 0) {
		menuPopDialog();
	} else if (g_Vars.coopplayernum >= 0 && PLAYERCOUNT() >= 2 && context == 0) {
		menuPopDialog();
	} else {
		if (g_Vars.stagenum == STAGE_DEEPSEA || g_Vars.stagenum == STAGE_SKEDARRUINS) {
			if (context == 2 || g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion) {
				// Pressed continue
				if (g_Vars.stagenum == STAGE_DEEPSEA) {
					if (!isStageDifficultyUnlocked(g_MissionConfig.stageindex + 1, g_MissionConfig.difficulty)) {
						menuPopDialog();
						menuPopDialog();
					} else {
						// Commit to starting next stage
						g_MissionConfig.stageindex++;
						g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;

						titleSetNextStage(g_MissionConfig.stagenum);

						if (g_MissionConfig.iscoop) {
							if (g_Vars.numaibuddies == 0) {
								g_Vars.bondplayernum = 0;
								g_Vars.coopplayernum = 1;
								g_Vars.antiplayernum = -1;
								setNumPlayers(2);
							} else {
								g_Vars.bondplayernum = 0;
								g_Vars.coopplayernum = -1;
								g_Vars.antiplayernum = -1;
								setNumPlayers(1);
							}
						} else {
							g_Vars.bondplayernum = 0;
							g_Vars.coopplayernum = -1;
							g_Vars.antiplayernum = -1;
							setNumPlayers(1);
						}

						lvSetDifficulty(g_MissionConfig.difficulty);
						titleSetNextMode(TITLEMODE_SKIP);
						mainChangeToStage(g_MissionConfig.stagenum);
						viBlack(true);
					}
				} else if (g_Vars.stagenum == STAGE_SKEDARRUINS) {
					// Commit to starting credits
					g_MissionConfig.stagenum = STAGE_CREDITS;
					titleSetNextStage(g_MissionConfig.stagenum);
					lvSetDifficulty(g_MissionConfig.difficulty);
					titleSetNextMode(TITLEMODE_SKIP);
					mainChangeToStage(g_MissionConfig.stagenum);
					viBlack(true);
				}
			} else {
				if (context == 1) {
					menuPushRootDialog(&g_MissionContinueOrReplyMenuDialog, MENUROOT_COOPCONTINUE);
				} else {
					menuPushDialog(&g_MissionContinueOrReplyMenuDialog);
				}
			}
		} else {
			if (context == 2) {
				menuPopDialog();
			}

			if (isStageDifficultyUnlocked(g_MissionConfig.stageindex + 1, g_MissionConfig.difficulty) == 0) {
				if (context == 2) {
					menuPopDialog();
					menuPopDialog();
				} else {
					menuPushDialog(&g_MissionContinueOrReplyMenuDialog);
				}
			} else if (stageGetIndex(g_MissionConfig.stagenum) < 0
						|| g_Vars.stagenum == STAGE_CITRAINING
						|| g_MissionConfig.stageindex >= SOLOSTAGEINDEX_MBR) {
				if (context == 2) {
					menuPopDialog();
					menuPopDialog();
				} else {
					menuPushDialog(&g_MissionContinueOrReplyMenuDialog);
				}
			} else {
				endscreenResetModels();

				if (context == 1) {
					menuPushRootDialog(endscreenAdvance(), MENUROOT_COOPCONTINUE);
				} else {
					menuPushDialog(endscreenAdvance());
				}
			}
		}
	}
}
#endif

MenuDialogHandlerResult endscreenHandle2PCompleted(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Menus[g_MpPlayerNum].endscreen.unke1c = 0;
	}

	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog) {
			if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
					|| (dialogdef->nextsibling && dialogdef->nextsibling == g_Menus[g_MpPlayerNum].curdialog->definition)) {
				struct menuinputs *inputs = data->dialog2.inputs;

				if (inputs->select || inputs->back || inputs->start) {
					g_Menus[g_MpPlayerNum].endscreen.unke1c = VERSION >= VERSION_NTSC_1_0 ? 6 : 3;
				}

				if (g_Menus[g_MpPlayerNum].endscreen.unke1c) {
					if (var8009dfc0) {
						g_Menus[g_MpPlayerNum].endscreen.unke1c--;
					}

					if (g_Menus[g_MpPlayerNum].endscreen.unke1c == 0) {
#if VERSION >= VERSION_NTSC_1_0
						endscreenContinue(0);
#else
						if (g_Vars.stagenum == STAGE_DEEPSEA) {
							if (g_Vars.antiplayernum >= 0 || (g_Vars.coopplayernum >= 0 && PLAYERCOUNT() >= 2)) {
								menuPopDialog();
							} else {
								g_MissionConfig.stageindex++;
								g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;

								titleSetNextStage(g_MissionConfig.stagenum);
								lvSetDifficulty(g_MissionConfig.difficulty);
								titleSetNextMode(TITLEMODE_SKIP);
								mainChangeToStage(g_MissionConfig.stagenum);
							}
						} else if (g_Vars.antiplayernum >= 0
								|| (g_Vars.coopplayernum >= 0 && PLAYERCOUNT() >= 2)
								|| (stageGetIndex(g_MissionConfig.stagenum) < 0
									|| g_Vars.stagenum == STAGE_CITRAINING
									|| g_MissionConfig.stageindex > SOLOSTAGEINDEX_SKEDARRUINS
									|| ((g_CheatsActiveBank0 || g_CheatsActiveBank1)
										&& !isStageDifficultyUnlocked(g_MissionConfig.stageindex + 1, g_MissionConfig.difficulty)))) {
							menuPopDialog();
						} else {
							endscreenResetModels();
							menuPushDialog(endscreenAdvance());
						}
#endif
					}
				}

				inputs->select = inputs->back = inputs->start = false;
			}
		}
	}

	return 0;
}

MenuDialogHandlerResult endscreenHandle2PFailed(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Menus[g_MpPlayerNum].endscreen.unke1c = 0;
	}

	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog) {
			if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
					|| (dialogdef->nextsibling && dialogdef->nextsibling == g_Menus[g_MpPlayerNum].curdialog->definition)) {
				struct menuinputs *inputs = data->dialog2.inputs;

				if (inputs->select || inputs->back || inputs->start) {
					g_Menus[g_MpPlayerNum].endscreen.unke1c = VERSION >= VERSION_NTSC_1_0 ? 6 : 3;
				}

				if (g_Menus[g_MpPlayerNum].endscreen.unke1c) {
					if (var8009dfc0) {
						g_Menus[g_MpPlayerNum].endscreen.unke1c--;
					}

					if (g_Menus[g_MpPlayerNum].endscreen.unke1c == 0) {
						if (g_Vars.antiplayernum >= 0
								|| (g_Vars.coopplayernum >= 0 && PLAYERCOUNT() >= 2)
								|| stageGetIndex(g_MissionConfig.stagenum) < 0
								|| g_Vars.stagenum == STAGE_CITRAINING) {
							menuPopDialog();
						} else {
							endscreenResetModels();
							menuPushDialog(&g_RetryMissionMenuDialog);
						}
					}
				}

				inputs->select = inputs->back = inputs->start = false;
			}
		}
	}

	return 0;
}

struct menuitem g_2PMissionEndscreenVMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_278, // "Mission Status:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&endscreenMenuTextMissionStatus,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_279, // "Agent Status:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&endscreenMenuTextAgentStatus,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_280, // "Mission Time:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&endscreenMenuTextMissionTime,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_MPWEAPONS_129, // "Difficulty:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)soloMenuTextDifficulty,
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
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_281, // "Weapon of Choice:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&mpMenuTextWeaponOfChoiceName,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_282, // "Kills:"
		(uintptr_t)&endscreenMenuTextNumKills,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_283, // "Accuracy:"
		(uintptr_t)&endscreenMenuTextAccuracy,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_284, // "Shot Total:"
		(uintptr_t)&endscreenMenuTextNumShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_285, // "Head Shots:"
		(uintptr_t)&endscreenMenuTextNumHeadShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_286, // "Body Shots:"
		(uintptr_t)&endscreenMenuTextNumBodyShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_287, // "Limb Shots:"
		(uintptr_t)&endscreenMenuTextNumLimbShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		L_OPTIONS_288, // "Others:"
		(uintptr_t)&endscreenMenuTextNumOtherShots,
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
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_289, // "Press START"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

#if VERSION >= VERSION_NTSC_1_0
/**
 * This function is re-used for several values on the endscreen.
 * item->param is used to determine which value it is. Values are:
 *
 * 0 = mission time
 * 1 = target time
 * 2 = separator and new cheat available
 * 3 = completion cheat name
 * 4 = others (shots)
 * 5 = timed cheat name
 * 6 = limb shots
 */
MenuItemHandlerResult endscreenHandleCheatInfo(s32 operation, struct menuitem *item, union handlerdata *data)
{
	static u32 cheatcolour = 0xff7f7fff;

	if (operation == MENUOP_GETCOLOUR
			&& ((g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x200) || item->param == 5)) {
		// Timed cheat just got unlocked, and this item is the timed cheat name
		u32 weight = menuGetSinOscFrac(40) * 255;

		mainOverrideVariable("ctcol", &cheatcolour);

		if (item->param == 0
				&& cheatGetTime(g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xff) == 0) {
			return 0;
		}

		data->label.colour2 = colourBlend(data->label.colour2, cheatcolour, weight);

		if (item->param == 3) { // completion cheat name
			data->label.colour1 = colourBlend(data->label.colour1, cheatcolour, weight);
		}

		if (item->param == 5) { // timed cheat name
			data->label.colour1 = colourBlend(data->label.colour1, cheatcolour, weight);
		}
	}

	if (operation == MENUOP_CHECKHIDDEN) {
		if (item->param == 1) { // target time
			u32 info = g_Menus[g_MpPlayerNum].endscreen.cheatinfo;

			if (info & 0x800) { // completion cheat just got unlocked
				return true;
			}

			// (has timed cheat)
			// and (timed cheat just got unlocked or timed cheat already unlocked) == 0
			// and cheat has a target time configured
			if ((info & 0x100) && (info & 0x600) == 0 && cheatGetTime(info & 0xff) > 0) {
				return false;
			}

			return true;
		} else if (item->param == 2 && (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xa00) == 0) {
			// new cheat available
			return true;
		} else if (item->param == 3 && (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x200) == 0) {
			// completion cheat name
			return true;
		} else if (item->param == 4 && (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xa00)) {
			// others (shots)
			return true;
		} else if (item->param == 6 && (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xa00) == 0xa00) {
			// limb shots
			return true;
		} else if (item->param == 5 && (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x800) == 0) {
			// timed cheat name
			return true;
		}
	}

	return false;
}
#endif

struct menuitem g_MissionEndscreenMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_278, // "Mission Status:"
		(uintptr_t)&endscreenMenuTextMissionStatus,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_279, // "Agent Status:"
		(uintptr_t)&endscreenMenuTextAgentStatus,
		NULL,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		L_OPTIONS_280, // "Mission Time:"
		(uintptr_t)&endscreenMenuTextMissionTime,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		1,
		MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		L_MPWEAPONS_242, // "Target Time:"
		(uintptr_t)&endscreenMenuTextTargetTime,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_MPWEAPONS_129, // "Difficulty:"
		(uintptr_t)&soloMenuTextDifficulty,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		2,
		0,
		0,
		0,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		2,
		0,
		L_MPWEAPONS_243, // "New Cheat Available!:"
		0,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		3,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		(uintptr_t)&endscreenMenuTextTimedCheatName,
		0,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		5,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		(uintptr_t)&endscreenMenuTextCompletionCheatName,
		0,
		endscreenHandleCheatInfo,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_281, // "Weapon of Choice:"
		(uintptr_t)&mpMenuTextWeaponOfChoiceName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_282, // "Kills:"
		(uintptr_t)&endscreenMenuTextNumKills,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_283, // "Accuracy:"
		(uintptr_t)&endscreenMenuTextAccuracy,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_284, // "Shot Total:"
		(uintptr_t)&endscreenMenuTextNumShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_285, // "Head Shots:"
		(uintptr_t)&endscreenMenuTextNumHeadShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_286, // "Body Shots:"
		(uintptr_t)&endscreenMenuTextNumBodyShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		6,
		0,
		L_OPTIONS_287, // "Limb Shots:"
		(uintptr_t)&endscreenMenuTextNumLimbShots,
		endscreenHandleCheatInfo,
	},
	{
		MENUITEMTYPE_LABEL,
		4,
		0,
		L_OPTIONS_288, // "Others:"
		(uintptr_t)&endscreenMenuTextNumOtherShots,
		endscreenHandleCheatInfo,
	},
#else
	// NTSC beta's endscreen dialog lacks cheat information
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_280, // "Mission Time:"
		(uintptr_t)&endscreenMenuTextMissionTime,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_MPWEAPONS_129, // "Difficulty:"
		(uintptr_t)&soloMenuTextDifficulty,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_281, // "Weapon of Choice:"
		(uintptr_t)&mpMenuTextWeaponOfChoiceName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_282, // "Kills:"
		(uintptr_t)&endscreenMenuTextNumKills,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_283, // "Accuracy:"
		(uintptr_t)&endscreenMenuTextAccuracy,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_284, // "Shot Total:"
		(uintptr_t)&endscreenMenuTextNumShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_285, // "Head Shots:"
		(uintptr_t)&endscreenMenuTextNumHeadShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_286, // "Body Shots:"
		(uintptr_t)&endscreenMenuTextNumBodyShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_287, // "Limb Shots:"
		(uintptr_t)&endscreenMenuTextNumLimbShots,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_OPTIONS_288, // "Others:"
		(uintptr_t)&endscreenMenuTextNumOtherShots,
		NULL,
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
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_289, // "Press START"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

#if VERSION >= VERSION_NTSC_1_0
char *endscreenMenuTextTimedCheatName(struct menuitem *item)
{
	if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x00000300) {
		return cheatGetName(g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xff);
	}

	return NULL;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *endscreenMenuTextCompletionCheatName(struct menuitem *item)
{
	if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x00000800) {
		return cheatGetName((g_Menus[g_MpPlayerNum].endscreen.cheatinfo >> 16) & 0xff);
	}

	return NULL;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *endscreenMenuTextTargetTime(struct menuitem *item)
{
	s32 time;
	s32 time2;

	if ((g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x00000100) == 0) {
		return NULL;
	}

	time = g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xff;
	time = cheatGetTime(time);

	if (!time) {
		return NULL;
	}

	formatTime(g_StringPointer, time * 60, TIMEPRECISION_SECONDS);
	strcat(g_StringPointer, "\n");
	return g_StringPointer;
}
#endif

void endscreenSetCoopCompleted(void)
{
	if (g_CheatsActiveBank0 == 0 && g_CheatsActiveBank1 == 0) {
#if VERSION >= VERSION_NTSC_1_0
		if (g_GameFile.coopcompletions[g_MissionConfig.difficulty] & (1 << g_MissionConfig.stageindex)) {
			g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion = true;
		}
#endif

		g_GameFile.coopcompletions[g_MissionConfig.difficulty] |= (1 << g_MissionConfig.stageindex);
	}
}

struct menudialogdef g_SoloMissionEndscreenCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	(uintptr_t)&endscreenMenuTitleStageCompleted,
	g_MissionEndscreenMenuItems,
	endscreenHandle2PCompleted,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_SoloEndscreenObjectivesCompletedMenuDialog,
};

struct menudialogdef g_SoloMissionEndscreenFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	(uintptr_t)&endscreenMenuTitleStageFailed,
	g_MissionEndscreenMenuItems,
	endscreenHandle2PFailed,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_SoloEndscreenObjectivesFailedMenuDialog,
};

// Offered instead of the Mission Failed screen when the player dies and a checkpoint exists
// (port/src/savestate.c). Quit carries on to the normal Mission Failed screen.
MenuItemHandlerResult endscreenHandleDeathLoadCheckpoint(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		savestateRequestLoad();
	}

	return 0;
}

MenuItemHandlerResult endscreenHandleDeathQuit(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f3704(&g_SoloMissionEndscreenFailedMenuDialog);
	}

	return 0;
}

struct menuitem g_CheckpointDeathMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t)"Agent down. Load the last checkpoint?\n",
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
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Load Last Checkpoint\n",
		0,
		endscreenHandleDeathLoadCheckpoint,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Quit\n",
		0,
		endscreenHandleDeathQuit,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheckpointDeathMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	(uintptr_t)"Mission Failed",
	g_CheckpointDeathMenuItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT | MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

/**
 * Prepare the endscreen by calculating unlocked cheats, setting the new best
 * time, choosing the default stage and difficulty and saving the game file.
 *
 * NTSC beta doesn't have cheats implemented, and has a different autostageindex
 * and thumbnail calculation.
 */
void endscreenPrepare(void)
{
	s32 timedcheatid;
	s32 complcheatid;
	s32 d;
	s32 s;
	u32 secs;
	s32 timedalreadyunlocked;
	s32 complalreadyunlocked;
	u16 prevbest;
	bool nowunlocked;

#if VERSION >= VERSION_NTSC_1_0
	g_Menus[g_MpPlayerNum].endscreen.stageindex = g_MissionConfig.stageindex;
#endif

	if (g_MenuData.root != MENUROOT_ENDSCREEN && g_Vars.mplayerisrunning == false) {
#if VERSION >= VERSION_NTSC_1_0
		g_Menus[g_MpPlayerNum].endscreen.cheatinfo = 0;
		g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion = false;
		g_Menus[g_MpPlayerNum].playernum = 0;

		// Set cheat info
		if (g_MissionConfig.iscoop == false
				&& g_MissionConfig.isanti == false
				&& g_MissionConfig.pdmode == false) {
			timedcheatid = cheatGetByTimedStageIndex(g_MissionConfig.stageindex, g_MissionConfig.difficulty);
			complcheatid = cheatGetByCompletedStageIndex(g_MissionConfig.stageindex);

			if (timedcheatid >= 0) {
				g_Menus[g_MpPlayerNum].endscreen.cheatinfo = 0x0100 | timedcheatid;
			}

			if (complcheatid >= 0) {
				g_Menus[g_MpPlayerNum].endscreen.cheatinfo |= 0x1000 | (complcheatid << 16);
			}
		}
#else
		g_Menus[g_MpPlayerNum].playernum = 0;
#endif

		// Push the endscreen
#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
		if ((g_Vars.currentplayer->isdead || g_Vars.currentplayer->aborted || !objectiveIsAllComplete()) && !debugIsSetCompleteEnabled())
#else
		if (g_Vars.currentplayer->isdead || g_Vars.currentplayer->aborted || !objectiveIsAllComplete())
#endif
		{
			if (g_Vars.currentplayer->isdead && !g_Vars.currentplayer->aborted && savestateHasCheckpoint()) {
				menuPushRootDialog(&g_CheckpointDeathMenuDialog, MENUROOT_ENDSCREEN);
			} else {
				menuPushRootDialog(&g_SoloMissionEndscreenFailedMenuDialog, MENUROOT_ENDSCREEN);
			}
		} else {
			menuPushRootDialog(&g_SoloMissionEndscreenCompletedMenuDialog, MENUROOT_ENDSCREEN);

			if (g_MissionConfig.iscoop) {
				endscreenSetCoopCompleted();
			}
		}

		if (g_MissionConfig.iscoop == false && g_MissionConfig.isanti == false) {
#if VERSION >= VERSION_NTSC_1_0
			timedalreadyunlocked = false;
			complalreadyunlocked = false;

			// If there's a timed cheat for this stage + difficulty
			if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x100) {
				timedalreadyunlocked = cheatIsUnlocked(g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xff);

				if (timedalreadyunlocked) {
					g_Menus[g_MpPlayerNum].endscreen.cheatinfo |= 0x400;
				}
			}

			// If there's a completion cheat for this stage (ie. not a special stage)
			if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x1000) {
				complalreadyunlocked = cheatIsUnlocked((g_Menus[g_MpPlayerNum].endscreen.cheatinfo >> 16) & 0xff);
			}
#else
			playerGetMissionTime();
#endif

			// Update total mission time
			secs = playerGetMissionTime() / 60;

			if (secs != 0) {
				if (secs >= S32_MAX || S32_MAX - secs <= g_GameFile.totaltime) {
					g_GameFile.totaltime = S32_MAX;
				} else {
					g_GameFile.totaltime += secs;
				}
			}

			g_GameFile.autostageindex = g_MissionConfig.stageindex;
			g_GameFile.autodifficulty = g_MissionConfig.difficulty;

#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
			if (g_CheatsActiveBank0 == 0
					&& g_CheatsActiveBank1 == 0
					&& g_MissionConfig.pdmode == false
					&& ((g_Vars.currentplayer->isdead == false
							&& g_Vars.currentplayer->aborted == false
							&& objectiveIsAllComplete())
						|| debugIsSetCompleteEnabled()))
#elif VERSION >= VERSION_NTSC_1_0
			if (g_CheatsActiveBank0 == 0
					&& g_CheatsActiveBank1 == 0
					&& g_MissionConfig.pdmode == false
					&& g_Vars.currentplayer->isdead == false
					&& g_Vars.currentplayer->aborted == false
					&& objectiveIsAllComplete())
#else
			if (g_Vars.currentplayer->isdead == false
					&& g_Vars.currentplayer->aborted == false
					&& objectiveIsAllComplete()
					&& g_CheatsActiveBank0 == 0
					&& g_CheatsActiveBank1 == 0)
#endif
			{
				secs = playerGetMissionTime() / 60;

				// The save file allows 12 bits per time, which is up to
				// 1h 8m 16s. If the timer is higher than this, reduce it.
				if (secs > 0xfff) {
					secs = 0xfff;
				}

#if VERSION >= VERSION_NTSC_1_0
				// Zero is used as an indicator that the stage is not completed,
				// so if the player managed to legitly complete a stage in 0:00
				// adjust it to 0:01.
				if (secs == 0) {
					secs = 1;
				}

				// Set best time
				prevbest = g_GameFile.besttimes[g_MissionConfig.stageindex][g_MissionConfig.difficulty];

				if (prevbest == 0) {
					g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion = true;
				}

				if (secs < prevbest || prevbest == 0) {
					g_GameFile.besttimes[g_MissionConfig.stageindex][g_MissionConfig.difficulty] = secs;
				}
#else
				prevbest = g_GameFile.besttimes[g_MissionConfig.stageindex][g_MissionConfig.difficulty];

				if (secs < prevbest || prevbest == 0) {
					g_GameFile.besttimes[g_MissionConfig.stageindex][g_MissionConfig.difficulty] = secs;
				}
#endif

#if VERSION >= VERSION_NTSC_1_0
				// Recalculate thumbnail for file select screen
				if (g_MissionConfig.stageindex <= SOLOSTAGEINDEX_SKEDARRUINS) {
					g_GameFile.autostageindex = g_MissionConfig.stageindex + 1;

					if (g_GameFile.autostageindex > SOLOSTAGEINDEX_SKEDARRUINS) {
						g_GameFile.autostageindex = SOLOSTAGEINDEX_SKEDARRUINS;
					}

					for (d = 0; d != 3; d++) {
						for (s = 0; s <= SOLOSTAGEINDEX_SKEDARRUINS; s++) {
							if (g_GameFile.besttimes[s][d]) {
								g_GameFile.thumbnail = s + 1;
							}
						}
					}
				}

				if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x100) {
					nowunlocked = cheatIsUnlocked(g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0xff);

					if (!timedalreadyunlocked && nowunlocked) {
						g_Menus[g_MpPlayerNum].endscreen.cheatinfo |= 0x0200;
					}
				}

				if (g_Menus[g_MpPlayerNum].endscreen.cheatinfo & 0x1000) {
					nowunlocked = cheatIsUnlocked((g_Menus[g_MpPlayerNum].endscreen.cheatinfo >> 16) & 0xff);

					if (!complalreadyunlocked && nowunlocked) {
						g_Menus[g_MpPlayerNum].endscreen.cheatinfo |= 0x0800;
					}
				}
#else
				// 154
				if (g_MissionConfig.stageindex <= SOLOSTAGEINDEX_SKEDARRUINS) {
					g_GameFile.autostageindex = g_MissionConfig.stageindex + 1;

					if (g_GameFile.autostageindex > SOLOSTAGEINDEX_SKEDARRUINS) {
						g_GameFile.autostageindex = SOLOSTAGEINDEX_SKEDARRUINS;
					}

					g_GameFile.thumbnail = g_MissionConfig.stageindex + 1;
				}
#endif

				challengeDetermineUnlockedFeatures();

				if (g_MissionConfig.stagenum == STAGE_SKEDARRUINS && g_AltTitleUnlocked == false) {
					g_AltTitleUnlocked = true;
#if VERSION >= VERSION_NTSC_1_0
					*(s8 *)&g_AltTitleEnabled = true;
#else
					g_AltTitleEnabled = true;
#endif
					bossfileSave();
				}
			}
		}

		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
	}

	if (g_MenuData.root == MENUROOT_ENDSCREEN) {
		lvSetPaused(true);
		g_Vars.currentplayer->pausemode = PAUSEMODE_PAUSED;
	}
}

struct menudialogdef g_2PMissionEndscreenCompletedHMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	(uintptr_t)&endscreenMenuTitleStageCompleted,
	g_MissionEndscreenMenuItems,
	endscreenHandle2PCompleted,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_SoloEndscreenObjectivesCompletedMenuDialog,
};

struct menudialogdef g_2PMissionEndscreenFailedHMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	(uintptr_t)&endscreenMenuTitleStageFailed,
	g_MissionEndscreenMenuItems,
	endscreenHandle2PFailed,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_SoloEndscreenObjectivesFailedMenuDialog,
};

struct menudialogdef g_2PMissionEndscreenCompletedVMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_OPTIONS_276, // "Completed"
	g_2PMissionEndscreenVMenuItems,
	endscreenHandle2PCompleted,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_2PMissionEndscreenObjectivesCompletedVMenuDialog,
};

struct menudialogdef g_2PMissionEndscreenFailedVMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_OPTIONS_277, // "Failed"
	g_2PMissionEndscreenVMenuItems,
	endscreenHandle2PFailed,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_2PMissionEndscreenObjectivesFailedVMenuDialog,
};

void endscreenPushCoop(void)
{
	u32 prevplayernum = g_MpPlayerNum;

	lvSetPaused(true);

	g_MpPlayerNum = g_Vars.currentplayerstats->mpindex;

#if VERSION >= VERSION_NTSC_1_0
	g_Menus[g_MpPlayerNum].endscreen.cheatinfo = 0;
	g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion = false;
	g_Menus[g_MpPlayerNum].endscreen.stageindex = g_MissionConfig.stageindex;
#endif

	g_Menus[g_MpPlayerNum].playernum = g_Vars.currentplayernum;

#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
	if (((g_Vars.bond->isdead && g_Vars.coop->isdead)
			|| g_Vars.bond->aborted
			|| g_Vars.coop->aborted
			|| !objectiveIsAllComplete()) && !debugIsSetCompleteEnabled())
#else
	if ((g_Vars.bond->isdead && g_Vars.coop->isdead)
			|| g_Vars.bond->aborted
			|| g_Vars.coop->aborted
			|| !objectiveIsAllComplete())
#endif
	{
		// Failed or aborted
		if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			menuPushRootDialog(&g_2PMissionEndscreenFailedVMenuDialog, MENUROOT_MPENDSCREEN);
		} else {
			menuPushRootDialog(&g_2PMissionEndscreenFailedHMenuDialog, MENUROOT_MPENDSCREEN);
		}
	} else {
		// Completed
		if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			menuPushRootDialog(&g_2PMissionEndscreenCompletedVMenuDialog, MENUROOT_MPENDSCREEN);
		} else {
			menuPushRootDialog(&g_2PMissionEndscreenCompletedHMenuDialog, MENUROOT_MPENDSCREEN);
		}

		endscreenSetCoopCompleted();
	}

	if (g_Vars.currentplayer == g_Vars.bond) {
		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
	}

	g_MpPlayerNum = prevplayernum;
}

/**
 * This function is misnamed. It pushes the menu after the endscreen
 * (ie. retry, next mission or continue), and it looks like it might be for coop
 * only but I'm not 100% sure.
 *
 * This function is only called from menuTick, which is a bit weird...
 */
void endscreenPushSolo(void)
{
	u32 prevplayernum = g_MpPlayerNum;

	g_MpPlayerNum = 0;
	g_Menus[g_MpPlayerNum].playernum = 0;

#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
	if (((g_Vars.bond->isdead && g_Vars.coop->isdead)
			|| g_Vars.bond->aborted
			|| g_Vars.coop->aborted
			|| !objectiveIsAllComplete()) && !debugIsSetCompleteEnabled())
#else
	if ((g_Vars.bond->isdead && g_Vars.coop->isdead)
			|| g_Vars.bond->aborted
			|| g_Vars.coop->aborted
			|| !objectiveIsAllComplete())
#endif
	{
		// Failed or aborted
		endscreenResetModels();
		menuPushRootDialog(&g_RetryMissionMenuDialog, MENUROOT_COOPCONTINUE);
	} else {
		// Completed
#if VERSION >= VERSION_NTSC_1_0
		endscreenContinue(1);
#else
		struct menudialogdef *definition = endscreenAdvance();

		if (definition) {
			endscreenResetModels();
			menuPushRootDialog(definition, MENUROOT_COOPCONTINUE);
		}
#endif
	}

	g_MpPlayerNum = prevplayernum;
}

void endscreenPushAnti(void)
{
	u32 prevplayernum = g_MpPlayerNum;

	lvSetPaused(true);

	g_MpPlayerNum = g_Vars.currentplayerstats->mpindex;

#if VERSION >= VERSION_NTSC_1_0
	g_Menus[g_MpPlayerNum].endscreen.cheatinfo = 0;
	g_Menus[g_MpPlayerNum].endscreen.isfirstcompletion = false;
	g_Menus[g_MpPlayerNum].endscreen.stageindex = g_MissionConfig.stageindex;
#endif

	g_Menus[g_MpPlayerNum].playernum = g_Vars.currentplayernum;

	if (g_Vars.currentplayer == g_Vars.bond) {
#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
		if (!g_Vars.anti->aborted && (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()) && !debugIsSetCompleteEnabled())
#else
		if (!g_Vars.anti->aborted && (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()))
#endif
		{
			// Bond - failed or aborted
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
				menuPushRootDialog(&g_2PMissionEndscreenFailedVMenuDialog, MENUROOT_MPENDSCREEN);
			} else {
				menuPushRootDialog(&g_2PMissionEndscreenFailedHMenuDialog, MENUROOT_MPENDSCREEN);
			}
		} else {
			// Bond - completed
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
				menuPushRootDialog(&g_2PMissionEndscreenCompletedVMenuDialog, MENUROOT_MPENDSCREEN);
			} else {
				menuPushRootDialog(&g_2PMissionEndscreenCompletedHMenuDialog, MENUROOT_MPENDSCREEN);
			}
		}

		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
	} else {
#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
		if (!g_Vars.anti->aborted && (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()) && !debugIsSetCompleteEnabled())
#else
		if (!g_Vars.anti->aborted && (g_Vars.bond->isdead || g_Vars.bond->aborted || !objectiveIsAllComplete()))
#endif
		{
			// Anti - completed
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
				menuPushRootDialog(&g_2PMissionEndscreenCompletedVMenuDialog, MENUROOT_MPENDSCREEN);
			} else {
				menuPushRootDialog(&g_2PMissionEndscreenCompletedHMenuDialog, MENUROOT_MPENDSCREEN);
			}
		} else {
			// Anti - failed or aborted
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
				menuPushRootDialog(&g_2PMissionEndscreenFailedVMenuDialog, MENUROOT_MPENDSCREEN);
			} else {
				menuPushRootDialog(&g_2PMissionEndscreenFailedHMenuDialog, MENUROOT_MPENDSCREEN);
			}
		}
	}

	g_MpPlayerNum = prevplayernum;
}
