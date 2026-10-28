#ifndef IN_GAME_MAINMENU_H
#define IN_GAME_MAINMENU_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern u16 g_ControlStyleOptions[];

extern struct menudialogdef g_AcceptMissionMenuDialog;
extern struct menudialogdef g_PreAndPostMissionBriefingMenuDialog;
extern struct menudialogdef g_RetryMissionMenuDialog;
extern struct menudialogdef g_NextMissionMenuDialog;
#ifndef PLATFORM_N64
extern struct menudialogdef g_ExitGameMenuDialog;
#endif

char *menuTextCurrentStageName(struct menuitem *item);
char *soloMenuTextDifficulty(struct menuitem *item);
MenuItemHandlerResult menuhandlerControlStyleImpl(s32 operation, struct menuitem *item, union handlerdata *data, s32 mpindex);
char *soloMenuTitleStageOverview(struct menudialogdef *dialogdef);
f32 func0f1036ac(u8 value, s32 prop);
bool isStageDifficultyUnlocked(s32 stageindex, s32 difficulty);
char *soloMenuTextBestTime(struct menuitem *item);
s32 getMaxAiBuddies(void);
s32 getNumUnlockedSpecialStages(void);
s32 func0f104720(s32 value);
char *func0f105664(struct menuitem *item);
char *func0f1056a0(struct menuitem *item);
char *invMenuTextPrimaryFunction(struct menuitem *item);
char *invMenuTextSecondaryFunction(struct menuitem *item);
void func0f105948(s32 weaponnum);
char *invMenuTextWeaponName(struct menuitem *item);
char *invMenuTextWeaponManufacturer(struct menuitem *item);
char *invMenuTextWeaponDescription(struct menuitem *item);
bool soloChoosePauseDialog(void);
MenuDialogHandlerResult menudialogBriefing(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialog00103608(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialogCoopAntiOptions(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialog0010559c(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult inventoryMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialogAbortMission(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult soloMenuDialogPauseStatus(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult menuhandler001024dc(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler001024fc(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAcceptMission(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAcceptPdModeSettings(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerBuddyOptionsContinue(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCoopDifficulty(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAntiDifficulty(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMissionList(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerControlStyle(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerFrInventoryList(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerInventoryList(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerLoadCheckpoint(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAbortMission(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCinema(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAimControl(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAlternativeTitle(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAlwaysShowTarget(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAmmoOnScreen(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAntiPlayer(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAntiRadar(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerAutoAim(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerChangeAgent(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCoopBuddy(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCoopFriendlyFire(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCoopRadar(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerCutsceneSubtitles(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerHeadRoll(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerHiRes(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerInGameSubtitles(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerLangFilter(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerLookAhead(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMusicVolume(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerPaintball(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerPdMode(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerPdModeSetting(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerReversePitch(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerScreenRatio(s32 operation, struct menuitem *item, union handlerdata *data);
#if PAL
MenuItemHandlerResult menuhandlerLanguage(s32 operation, struct menuitem *item, union handlerdata *data);
#endif
MenuItemHandlerResult menuhandlerScreenSize(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerScreenSplit(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerSfxVolume(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerShowGunFunction(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerShowMissionTime(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerShowZoomRange(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerSightOnScreen(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerSoloDifficulty(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerSoundMode(s32 operation, struct menuitem *item, union handlerdata *data);

#endif
