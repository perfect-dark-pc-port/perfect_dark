#ifndef _IN_SAVESTATE_H
#define _IN_SAVESTATE_H

#include <stddef.h>
#include <PR/ultratypes.h>

// In-memory mission checkpoint: one slot, valid until the stage changes or restarts.
// A save copies the game's globals, its memp heap and the checkpoint arena; a load
// copies them back. Both run at the top of mainTick(), between game frames.

// "Auto-Save on Objective" (Extended Options > Game; pd.ini Game.AutoSaveOnObjective), off by default.
extern s32 g_SavestateAutoSave;

s32 savestateSupported(void);
s32 savestateHasCheckpoint(void);

void savestateRequestLoad(void);

// Called by objectivesCheckAll() when objective <index> flips to complete.
void savestateOnObjectiveComplete(s32 index);

// Called by mainLoop() whenever a stage (re)starts; drops the checkpoint.
void savestateOnStageStart(void);

// Called at the top of mainTick(); performs a pending save or load.
void savestateTick(void);

// Checkpoint arena that the game's sysMem* calls are redirected to (savestate_game.h).
void *savestateArenaAlloc(const u32 size);
void *savestateArenaZeroAlloc(const u32 size);
void *savestateArenaRealloc(void *ptr, const u32 newSize);
void savestateArenaFree(void *ptr);
u8 *savestateArenaBase(void);
size_t savestateArenaTop(void);
s32 savestateArenaOverflowed(void);

#endif
