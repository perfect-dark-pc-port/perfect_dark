#include <ultra64.h>
#include "constants.h"
#include "game/bossfile.h"
#include "game/game_006900.h"
#include "game/title.h"
#include "game/pdmode.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/filelist.h"
#include "game/mainmenu.h"
#include "game/endscreen.h"
#include "game/playermgr.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/setup.h"
#include "game/challenge.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/mplayer/mplayer.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "data.h"
#include "types.h"

u8 g_FileState = 0;
u8 var80062944 = 0;
u8 var80062948 = 0;
u8 var8006294c = 0;

const char var7f1a85b0[] = "lvup: %d\n";
const char var7f1a85bc[] = "file id %x-%x";
const char var7f1a85cc[] = " ticking: ";
const char var7f1a85d8[] = "1";
const char var7f1a85dc[] = "0";
const char var7f1a85e0[] = "Live: %d\n";
const char var7f1a85ec[] = "current:";
const char var7f1a85f8[] = " numactive %d ";

void menuCountDialogs(void)
{
	s32 i;
	g_MenuData.count = 0;

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		if (g_Menus[i].curdialog) {
			g_MenuData.count++;
		}
	}
}

void menuTick(void)
{
	s32 i;
	s32 j;
	s32 k;
	s32 isdialogopen;
	s32 sp340 = true;
	s32 anyopen = false;

#if PAL
	g_ScaleX = 1;
#else
	g_ScaleX = g_ViRes == VIRES_HI ? 2 : 1;
#endif

	menuTickTimers();

	if (g_MenuData.count) {
		// empty
	}

	menuCountDialogs();

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		if (i);

		if (g_Menus[i].openinhibit > 0) {
			g_Menus[i].openinhibit--;
		}

		if (g_Menus[i].curdialog) {
			anyopen = true;
		}
	}

	if (!anyopen && g_MenuData.bg != 0 && g_MenuData.nextbg == 255) {
		g_MenuData.nextbg = 0;
	}

	if (anyopen && g_MenuData.unk66e > 0 && var8009dfc0) {
		s32 bVar12 = 50;
		s32 bVar11 = false;

		for (j = 0; j < ARRAYCOUNT(g_Menus); j++) {
			if (g_Menus[j].curdialog) {
				if (g_Menus[j].curdialog->state == MENUDIALOGSTATE_OPENING
						|| g_Menus[j].curdialog->state == MENUDIALOGSTATE_POPULATING
						|| g_Menus[j].curdialog->state == MENUDIALOGSTATE_PREOPEN) {
					bVar11 = true;
				}
			}
		}

		if (g_Vars.normmplayerisrunning) {
			bVar12 = 40;
		}

		if (g_MenuData.unk66f > bVar12 || !bVar11) {
			func0f0f3220(g_MenuData.unk66e - 1);
		} else {
			g_MenuData.unk66f++;
		}
	}

	if (g_MenuData.nextbg != 255) {
		if (g_MenuData.nextbg == g_MenuData.bg) {
			g_MenuData.nextbg = 255;
		} else {
			f32 mult = 0.02f;

			if (g_MenuData.bg == 0) {
				mult = mult + mult;
			}

			if (g_MenuData.nextbg == 0) {
				mult = mult + mult;
			}

			if (g_MenuData.nextbg == MENUBG_8) {
				mult = mult / 5.0f;
			}

			if (g_MenuData.nextbg == MENUBG_SUCCESS) {
				mult = mult / 3.0f;
			}

			if (g_MenuData.nextbg == MENUBG_6) {
				mult = mult / 10.0f;
			}

			if (g_MenuData.nextbg == 0) {
				var8009dfc0 = false;

				if (g_Vars.currentplayer->gunctrl.gunmemowner != GUNMEMOWNER_BONDGUN) {
					g_Vars.currentplayer->gunctrl.loadall = true;
				}
			}

			if (g_MenuData.screenshottimer == 0 || g_MenuData.bg != 0) {
#if VERSION >= VERSION_PAL_BETA
				f32 diffframe = g_Vars.diffframe60freal;
#else
				f32 diffframe = g_Vars.diffframe60f;
#endif

				if (diffframe > 4) {
					diffframe = 4;
				}

				g_MenuData.unk010 += mult * diffframe;
			}

			if (g_MenuData.unk010 > 1) {
				if (g_MenuData.nextbg) {
					var8009dfc0 = true;
				}

				g_MenuData.unk010 = 0;
				g_MenuData.bg = g_MenuData.nextbg;
				g_MenuData.nextbg = 255;

				if (g_MenuData.root == MENUROOT_ENDSCREEN) {
					if (g_MenuData.bg == MENUBG_BLUR) {
						g_MenuData.nextbg = MENUBG_6;
					}

					if (g_MenuData.bg == MENUBG_6) {
						menugfxFreeParticles();
						g_MenuData.bg = MENUBG_BLUR;
						g_MenuData.nextbg = MENUBG_8;
					}

					if (g_MenuData.bg == MENUBG_8) {
						g_MenuData.nextbg = MENUBG_SUCCESS;
					}
				}

				if (g_MenuData.bg == 0) {
					func0f0fa6ac();
				}
			}

			if (g_MenuData.nextbg == MENUBG_FAILURE) {
				var8009dfc0 = true;
			}

			if (var8009dfc0 && g_Vars.currentplayer->gunmem2) {
				playerRemoveChrBody();

				if (g_Vars.currentplayer->gunmem2);
			}
		}
	} else {
		g_MenuData.unk010 = 0;
		var8009dfc0 = g_MenuData.bg == 0 ? false : true;
	}

	// Check if returning from a multiplayer match
	if (var80087260 > 0) {
		if (g_Vars.lvframenum >= 4) {
			if (g_Vars.stagenum == STAGE_CITRAINING || g_Vars.stagenum == STAGE_4MBMENU) {
				viBlack(false);
				g_MpNumJoined = 0;

				if (g_Vars.usingadvsetup) {
					g_Vars.mpsetupmenu = MPSETUPMENU_ADVSETUP;
				} else {
					g_Vars.mpsetupmenu = MPSETUPMENU_GENERAL;
				}

				for (i = 0; i < MAX_PLAYERS; i++) {
					g_Vars.waitingtojoin[i] = false;

					if (g_MpSetup.chrslots & (1 << i)) {
						g_MpPlayerNum = i;

						if (g_Vars.mpsetupmenu == MPSETUPMENU_ADVSETUP) {
							g_MpNumJoined++;
							func0f17fcb0(true);
						} else if (g_MpNumJoined == 0) {
							g_MpNumJoined++;

							if (IS4MB()) {
								menuPushRootDialog(&g_MainMenu4MbMenuDialog, MENUROOT_4MBMAINMENU);
							} else {
								menuPushRootDialog(&g_CombatSimulatorMenuDialog, MENUROOT_MPSETUP);
							}
						} else {
							g_Vars.waitingtojoin[i] = true;
						}
					}
				}

				g_MpPlayerNum = 0;

				if (g_MpSetup.chrslots & 0xf) {
					sndStart(var80095200, SFX_EXPLOSION_8098, 0, -1, -1, -1, -1, -1);

					playerPause(IS4MB() ? MENUROOT_4MBMAINMENU : MENUROOT_MPSETUP);
				}
			}

			var80087260 = 0;
		} else {
			viBlack(true);
			g_PlayersWithControl[0] = false;
		}
	}

	// If a game file hasn't been selected (ie. just powered on),
	// force the file select menu open
	if (g_FileState == FILESTATE_UNSELECTED && g_Vars.stagenum == STAGE_CITRAINING) {
		g_PlayersWithControl[0] = false;

#ifndef PLATFORM_N64
		if ((g_Vars.lvframenum > 30 && g_Vars.tickmode != TICKMODE_CUTSCENE) ||
			(g_SkipJoOnPc && g_Vars.lvframenum >= 7)) {
			g_SkipJoOnPc = false;
#else
		if (g_Vars.lvframenum > 30 && g_Vars.tickmode != TICKMODE_CUTSCENE) {
#endif
			g_Menus[0].openinhibit = 0;
			g_Menus[1].openinhibit = 0;
			g_Menus[2].openinhibit = 0;
			g_Menus[3].openinhibit = 0;
			g_Vars.currentplayer->pausemode = PAUSEMODE_UNPAUSED;
			playerPause(MENUROOT_FILEMGR);
			g_FileState = FILESTATE_SELECTED;
		}
	}

	g_Vars.unk000498 = 0;

	if (g_MenuData.count > 0) {
		var8006294c = 1;

		if (g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU) {
			if (g_MenuData.prevmenuroot == -1) {
#ifndef PLATFORM_N64
				if (g_MpSetup.storedbotbits) {
					g_MpSetup.chrslots |= g_MpSetup.storedbotbits;
					g_MpSetup.storedbotbits = 0;
				}
#endif
				g_MpSetup.chrslots &= 0xfff0;
			}

			g_MpNumJoined = 0;

			for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
				if (g_Menus[i].curdialog) {
					g_Menus[i].playernum = g_MpNumJoined++;

					if (g_MenuData.prevmenuroot == -1) {
						g_MpSetup.chrslots |= (1 << i);
					}
				}
			}

			mpCalculateLockIfLastWinnerOrLoser();
			challengePerformSanityChecks();
		}

		for (i = 0; i < MAX_PLAYERS; i++) {
			g_MpPlayerNum = i;

			if (g_Menus[g_MpPlayerNum].curdialog) {
				if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_MpReadyMenuDialog) {
					g_Vars.unk000498 = 1;
				} else {
					sp340 = false;
				}
			}
		}

		for (i = 0; i < MAX_PLAYERS; i++) {
			g_MpPlayerNum = i;

			if (g_Menus[g_MpPlayerNum].curdialog) {
				// Player has a dialog open - tick it
				s32 prevplayernum = g_Vars.currentplayernum;

				if (g_Menus[g_MpPlayerNum].playernum < PLAYERCOUNT()) {
					setCurrentPlayerNum(g_Menus[g_MpPlayerNum].playernum);
				}

				menuProcessInput();
				setCurrentPlayerNum(prevplayernum);
			} else {
				if (g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU) {
					// Check if player is joining the game
					bool canjoin;
					u32 buttons = joyGetButtonsPressedThisFrame(i, 0xffffffff);

					if (g_MenuData.root == MENUROOT_4MBMAINMENU) {
						if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL) {
							// Limit to 2 players? But in a roundabout kind of way
							canjoin = true;

							for (j = 0; j < MAX_PLAYERS; j++) {
								if (g_Vars.waitingtojoin[j]) {
									canjoin = false;
								}
							}
						} else {
							// Quick go or advanced setup - limit to 2 players
							canjoin = g_MpNumJoined < 2;
						}
					} else {
						// 8MB - no restrictions on joining
						canjoin = true;
					}

					if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
						g_PlayerConfigsArray[i].base.team = 0;
					}

					if (canjoin && (buttons & START_BUTTON)) {
						g_PlayerConfigsArray[i].handicap = 128;

						if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL) {
							// Joining from a general area such as the Combat
							// Simulator menu. We can't open dialogs for other
							// players here, so they are waiting to join.
#if VERSION >= VERSION_NTSC_1_0
							if (!g_Vars.waitingtojoin[i]) {
								sndStart(var80095200, SFX_EXPLOSION_809A, 0, -1, -1, -1, -1, -1);
							}
							g_Vars.waitingtojoin[i] = true;
#else
							g_Vars.waitingtojoin[i] = true;
							sndStart(var80095200, SFX_EXPLOSION_809A, 0, -1, -1, -1, -1, -1);
#endif

						} else if (g_Vars.mpsetupmenu == MPSETUPMENU_QUICKGO) {
							// Joining from quick go - open Quick Go dialog
							g_MpNumJoined++;

							if (IS4MB()) {
								menuPushRootDialog(&g_MpQuickGo4MbMenuDialog, MENUROOT_4MBMAINMENU);
							} else {
								menuPushRootDialog(&g_MpQuickGoMenuDialog, MENUROOT_MPSETUP);
							}
						} else {
							// Joining from advanced setup
							g_MpNumJoined++;
							func0f17fcb0(false);
						}
					}

					if ((buttons & START_BUTTON) == 0) {
						if (buttons & B_BUTTON) {
							// No dialog open and pressing B -> no longer
							// waiting to join
							if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL) {
								g_Vars.waitingtojoin[i] = false;
							}
						} else if (g_Vars.waitingtojoin[i]) {
							if (g_Vars.mpsetupmenu == MPSETUPMENU_QUICKGO) {
								// Player was waiting to join and we have just
								// reached the quick go layer - open the dialog
								g_Vars.waitingtojoin[i] = false;
								g_MpNumJoined++;

								if (IS4MB()) {
									menuPushRootDialog(&g_MpQuickGo4MbMenuDialog, MENUROOT_4MBMAINMENU);
								} else {
									menuPushRootDialog(&g_MpQuickGoMenuDialog, MENUROOT_MPSETUP);
								}
							} else if (g_Vars.mpsetupmenu == MPSETUPMENU_ADVSETUP) {
								// Player was waiting to join and we have just
								// reached the adv setup layer - open the dialog
								g_Vars.waitingtojoin[i] = false;
								g_MpNumJoined++;
								func0f17fcb0(false);
							}
						}
					}
				} else {
					g_Vars.mpsetupmenu = 0;
					g_Vars.waitingtojoin[i] = false;
				}

				// Note that MPENDSCREEN also refers to coop and anti modes.
				// Handle re-opening the endscreen by pressing B.
				if (g_MenuData.root == MENUROOT_MPENDSCREEN) {
					u32 buttons2 = joyGetButtonsPressedThisFrame(g_PlayerConfigsArray[i].contpad1, 0xffffffff);

					if (buttons2 & B_BUTTON) {
						s32 playernum = -1;
						s32 k;

						for (k = 0; k < PLAYERCOUNT(); k++) {
							if (g_Vars.playerstats[k].mpindex == i) {
								playernum = k;
							}
						}

						if (playernum >= 0) {
							if (g_Vars.coopplayernum >= 0) {
								s32 prevplayernum = g_Vars.currentplayernum;
								setCurrentPlayerNum(playernum);
								endscreenPushCoop();
								setCurrentPlayerNum(prevplayernum);
							} else if (g_Vars.antiplayernum >= 0) {
								s32 prevplayernum = g_Vars.currentplayernum;
								setCurrentPlayerNum(playernum);
								endscreenPushAnti();
								setCurrentPlayerNum(prevplayernum);
							} else {
								mpPushEndscreenDialog(playernum, i);
							}
						}
					}
				}
			}
		}

		if (sp340 &&
				(g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU)) {
			func0f0f820c(NULL, -5);
		}
	} else {
		var8006294c = 0;
	}

	if (var8006294c) {
		if (var80062948 == 0 &&
				(g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU)) {
			var80062948 = 1;
			filelistCreate(0, FILETYPE_MPPLAYER);
		}

		if (var80062944) {
			filelistsTick();
		}
	} else {
		if (var80062944 == 1) {
			menuStop();
		}
	}

	g_MpPlayerNum = 0;
	isdialogopen = false;

	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		if (g_Menus[i].curdialog) {
			isdialogopen = true;
		}
	}

	if ((g_MenuData.isdialogopen || g_MenuData.prevmenuroot != -1) && isdialogopen == false) {
		if ((g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU)
				&& g_MenuData.prevmenuroot == -1) {
			if (g_Vars.mpsetupmenu == MPSETUPMENU_GENERAL) {
				g_MenuData.prevmenuroot = MENUROOT_MAINMENU;
				g_MenuData.prevmenudialog = IS4MB() ? &g_CiMenuViaPauseMenuDialog : &g_CiMenuViaPcMenuDialog;
			} else if (IS4MB()) {
				g_MenuData.prevmenuroot = MENUROOT_4MBMAINMENU;
				g_MenuData.prevmenudialog = &g_MainMenu4MbMenuDialog;
			} else {
				g_MenuData.prevmenuroot = MENUROOT_MPSETUP;
				g_MenuData.prevmenudialog = &g_CombatSimulatorMenuDialog;
			}
		}

		if (g_MenuData.prevmenuroot != -1) {
			if (g_MenuData.prevmenuroot == -5) {
				// Match is beginning
				mpStartMatch();
				menuStop();

				if (g_Vars.modifiedfiles & MODFILE_MPSETUP) {
					bossfileSave();
					g_Vars.modifiedfiles &= ~MODFILE_MPSETUP;
				}
			} else if (g_MenuData.prevmenuroot == -6) {
				// Match is ending
				s32 playernum = 0;

				if (g_Vars.normmplayerisrunning) {
					func0f0fd548(4);
				}

				for (i = 0; i < MAX_PLAYERS; i++) {
					if (g_MpSetup.chrslots & (1 << i)) {
						if (g_Vars.coopplayernum >= 0) {
							if (g_Vars.stagenum == STAGE_DEEPSEA) {
								g_MissionConfig.stageindex++;
								g_MissionConfig.stagenum = g_SoloStages[g_MissionConfig.stageindex].stagenum;
								titleSetNextStage(g_MissionConfig.stagenum);
								lvSetDifficulty(g_MissionConfig.difficulty);
								titleSetNextMode(TITLEMODE_SKIP);
								mainChangeToStage(g_MissionConfig.stagenum);
							} else {
								s32 prevplayernum = g_Vars.currentplayernum;
								setCurrentPlayerNum(playernum);
								endscreenPushCoop();
								setCurrentPlayerNum(prevplayernum);
								isdialogopen = true;
							}
						} else if (g_Vars.antiplayernum >= 0) {
							s32 prevplayernum = g_Vars.currentplayernum;
							setCurrentPlayerNum(playernum);
							endscreenPushAnti();
							setCurrentPlayerNum(prevplayernum);
							isdialogopen = true;
						} else {
							mpPushEndscreenDialog(playernum, i);
							isdialogopen = true;

							if (g_PlayerConfigsArray[i].fileguid.fileid && g_PlayerConfigsArray[i].fileguid.deviceserial) {
								func0f0fd548(i);
							}
						}

						playernum++;
					}
				}
			} else if (g_MenuData.prevmenuroot == -7) {
				menuStop();
				g_FileState = FILESTATE_CHANGINGAGENT;
				gamefileLoadDefaults(&g_GameFile);
				gamefileApplyOptions(&g_GameFile);
				mainChangeToStage(IS4MB() ? STAGE_4MBMENU : STAGE_CITRAINING);
				musicQueueStopAllEvent();
			} else {
				bool startmusic = false;
				menuPushRootDialog(g_MenuData.prevmenudialog, g_MenuData.prevmenuroot);
				isdialogopen = true;

				if (g_MenuData.root == MENUROOT_MPSETUP || g_MenuData.root == MENUROOT_4MBMAINMENU) {
					startmusic = true;
#ifndef PLATFORM_N64
					if (!g_SkipToCombatSimulator) {
#endif
					sndStart(var80095200, SFX_EXPLOSION_8098, 0, -1, -1, -1, -1, -1);
#ifndef PLATFORM_N64
					}
#endif
				}

				if (g_MenuData.root == MENUROOT_MAINMENU || g_MenuData.root == MENUROOT_TRAINING) {
					struct trainingdata *dtdata = dtGetData();

					if ((g_Vars.stagenum == STAGE_CITRAINING || g_Vars.stagenum == STAGE_4MBMENU)
							&& ((g_Vars.currentplayer->prop->rooms[0] >= 0x16 && g_Vars.currentplayer->prop->rooms[0] <= 0x19)
								|| g_Vars.currentplayer->prop->rooms[0] == 0x0a
								|| g_Vars.currentplayer->prop->rooms[0] == 0x1e
								|| (dtdata && dtdata->intraining))) {
						startmusic = false;
#ifndef PLATFORM_N64
					} else if (g_SkipToCombatSimulator) {
						startmusic = false;
#endif
					} else {
						startmusic = true;
					}
				}

				if (startmusic) {
					musicStartMenu();
				}
			}

			g_MenuData.prevmenudialog = NULL;
			g_MenuData.prevmenuroot = -1;
		} else {
			switch (g_MenuData.root) {
			case MENUROOT_ENDSCREEN:
				if (g_Vars.restartlevel) {
					mainChangeToStage(mainGetStageNum());
				} else {
					mainChangeToStage(STAGE_TITLE);
				}
				break;
			case MENUROOT_MPPAUSE:
				break;
			case MENUROOT_MPENDSCREEN:
				if (g_Vars.normmplayerisrunning) {
					var80087260 = 3;
				} else if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
					struct mpplayerconfig tmp;

					tmp = g_PlayerConfigsArray[4];
					g_PlayerConfigsArray[4] = g_PlayerConfigsArray[0];
					g_PlayerConfigsArray[0] = tmp;

					tmp = g_PlayerConfigsArray[5];
					g_PlayerConfigsArray[5] = g_PlayerConfigsArray[1];
					g_PlayerConfigsArray[1] = tmp;
				}

				if (g_Vars.coopplayernum >= 0
						&& g_MissionConfig.stageindex <= SOLOSTAGEINDEX_SKEDARRUINS
						&& ((!g_CheatsActiveBank0 && !g_CheatsActiveBank1) || isStageDifficultyUnlocked(g_MissionConfig.stageindex + 1, g_MissionConfig.difficulty))) {
					endscreenPushSolo();
				} else if (g_Vars.restartlevel) {
					mainChangeToStage(mainGetStageNum());
				} else {
					mpSetPaused(MPPAUSEMODE_UNPAUSED);
					g_Vars.mplayerisrunning = false;
					g_Vars.normmplayerisrunning = false;
					g_Vars.lvmpbotlevel = 0;

					if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
						g_BossFile.locktype = MPLOCKTYPE_NONE;
					}

					if (IS8MB()) {
						titleSetNextStage(STAGE_CITRAINING);
						setNumPlayers(1);
						titleSetNextMode(TITLEMODE_SKIP);
						mainChangeToStage(STAGE_CITRAINING);
					} else {
						titleSetNextStage(STAGE_4MBMENU);
						setNumPlayers(1);
						titleSetNextMode(TITLEMODE_SKIP);
						mainChangeToStage(STAGE_4MBMENU);
					}
				}
				break;
			case MENUROOT_COOPCONTINUE:
				if (g_Vars.coopplayernum >= 0) {
					mpSetPaused(MPPAUSEMODE_UNPAUSED);
					g_Vars.mplayerisrunning = false;
					g_Vars.normmplayerisrunning = false;
					g_Vars.lvmpbotlevel = 0;
					titleSetNextStage(STAGE_CITRAINING);
					setNumPlayers(1);
					titleSetNextMode(TITLEMODE_SKIP);
					mainChangeToStage(STAGE_CITRAINING);
					g_MissionConfig.iscoop = false;
				}
				break;
			}
		}
	}

	menuCountDialogs();

	if (g_MenuData.count == 0) {
		if (g_MenuData.nextbg != 255) {
			if (g_MenuData.nextbg != 0) {
				g_MenuData.bg = g_MenuData.nextbg;
				g_MenuData.nextbg = 0;
				g_MenuData.unk010 = 1.0f - g_MenuData.unk010;
			}
		} else {
			if (g_MenuData.bg != 0) {
				g_MenuData.nextbg = 0;
			}
		}

		if (g_Vars.currentplayer->gunctrl.gunmemowner == GUNMEMOWNER_INVMENU && g_Vars.stagenum != STAGE_CITRAINING) {
			g_MenuData.unk5d5_01 = true;

			if (g_Menus[0].menumodel.allocstart) {
				bgunFreeGunMem();
				g_Menus[0].menumodel.allocstart = NULL;
			}
		}
	}

	g_Vars.paksneededformenu = 0;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		s32 mpindex = -1;

		if (g_Vars.mplayerisrunning) {
			mpindex = g_Vars.playerstats[i].mpindex;
		} else if (i == 0) {
			mpindex = 0;
		}

		if (mpindex >= 0 && g_Vars.players[i]) {
			if (g_MenuData.nextbg != 255U
					|| g_MenuData.bg
					|| g_MenuData.unk5d5_05
					|| g_MenuData.unk5d4
					|| g_Menus[mpindex].curdialog
					|| g_MenuData.bannernum != -1) {
				g_Vars.players[i]->menuisactive = true;
			} else {
				g_Vars.players[i]->menuisactive = false;
			}

			switch (g_MenuData.root) {
			case MENUROOT_ENDSCREEN:
			case MENUROOT_MAINMENU:
			case MENUROOT_MPSETUP:
			case MENUROOT_MPENDSCREEN:
			case MENUROOT_FILEMGR:
			case MENUROOT_BOOTPAKMGR:
			case MENUROOT_4MBFILEMGR:
			case MENUROOT_4MBMAINMENU:
			case MENUROOT_TRAINING:
				if (g_Menus[mpindex].curdialog) {
					g_Vars.paksneededformenu = 0x1f;
				}
				break;
			}

			g_Vars.players[i]->devicesinhibit = 0;

			if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
					&& PLAYERCOUNT() >= 2
					&& g_Menus[mpindex].curdialog) {
				g_Vars.players[i]->devicesinhibit = 0
					| DEVICE_NIGHTVISION
					| DEVICE_XRAYSCANNER
					| DEVICE_EYESPY
					| DEVICE_IRSCANNER;
			}
		}
	}

	g_ScaleX = 1;
	g_MenuData.isdialogopen = isdialogopen ? true : false;
}
