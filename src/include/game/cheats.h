#ifndef IN_GAME_CHEATS_H
#define IN_GAME_CHEATS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_CheatsMenuDialog;
extern struct menudialogdef g_CheatsFunMenuDialog;
extern struct menudialogdef g_CheatsGameplayMenuDialog;
extern struct menudialogdef g_CheatsSoloWeaponsMenuDialog;
extern struct menudialogdef g_CheatsClassicWeaponsMenuDialog;
extern struct menudialogdef g_CheatsWeaponsMenuDialog;
extern struct menudialogdef g_CheatsBuddiesMenuDialog;
extern struct menudialogdef g_ExtendedCheatsMenuDialog;

u32 cheatIsUnlocked(s32 cheat_id);
bool cheatIsActive(s32 cheat_id);
void cheatActivate(s32 cheat_id);
void cheatDeactivate(s32 cheat_id);
void cheatsInit(void);
void cheatsReset(void);
char *cheatGetNameIfUnlocked(struct menuitem *item);
char *cheatGetMarquee(struct menuitem *item);
s32 cheatGetByTimedStageIndex(s32 stage_index, s32 difficulty);
s32 cheatGetByCompletedStageIndex(s32 stage_index);
s32 cheatGetTime(s32 cheat_id);
const char *cheatGetName(s32 cheat_id);
MenuDialogHandlerResult cheatMenuHandleDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult cheatCheckboxMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult cheatMenuHandleBuddyCheckbox(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult cheatMenuHandleTurnOffAllCheats(s32 operation, struct menuitem *item, union handlerdata *data);

#endif
