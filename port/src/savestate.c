#include <stdlib.h>
#include <string.h>
#include <ultra64.h>
#include "constants.h"
#include "game/hudmsg.h"
#include "game/objectives.h"
#include "game/lv.h"
#include "lib/joy.h"
#include "bss.h"
#include "data.h"
#include "types.h"
#include "platform.h"
#include "system.h"
#include "config.h"
#include "savestate.h"

// In-memory mission checkpoint. See savestate.h.
//
// The game's whole mutable state is: its globals, the memp heap (g_MempHeap, allocated
// once at boot and never moved) and the checkpoint arena (savestate_arena.c). All three
// keep their addresses for the life of the process, so a byte copy of them is a complete,
// self-consistent snapshot: every pointer inside it is still valid when it is copied back,
// as long as the stage has not changed.
//
// The game's globals are found by link order: CMakeLists.txt links the game's objects
// between port/savestate/begin.c and port/savestate/end.c, whose markers therefore bound
// the game's part of .data and .bss. savestateSupported() checks that this actually held
// for the toolchain in use, and checkpoints stay hidden if it did not.

extern u8 *g_MempHeap;
extern u32 g_MempHeapSize;
extern s32 g_StageNum;
extern void gfx_texture_cache_clear(void);

#ifdef __APPLE__
// macOS: the game's globals are placed in their own sections (savestate_game.h)
extern u8 g_SavestateDataBegin[] __asm("section$start$__DATA$__pd_gdata");
extern u8 g_SavestateDataEnd[] __asm("section$end$__DATA$__pd_gdata");
extern u8 g_SavestateBssBegin[] __asm("section$start$__DATA$__pd_gbss");
extern u8 g_SavestateBssEnd[] __asm("section$end$__DATA$__pd_gbss");
#define SAVESTATE_MARKER_SIZE 0
#else
extern u8 g_SavestateDataBegin[], g_SavestateDataEnd[];
extern u8 g_SavestateBssBegin[], g_SavestateBssEnd[];
#define SAVESTATE_MARKER_SIZE 16
#endif

s32 g_SavestateAutoSave = 0;

static s32 g_SavestatePendingSave = 0;
static s32 g_SavestatePendingLoad = 0;
static s32 g_SavestateValid = 0;
static s32 g_SavestateChecked = 0;
static s32 g_SavestateLayoutOk = 0;

static u8 *g_SavestateBuf = NULL;
static size_t g_SavestateBufCap = 0;
static size_t g_SavestateArenaLen = 0;

// Game state that the port defines in its own files (pdmain.c replaces src/lib/main.c), so
// it is outside the game's ranges; copied into the checkpoint explicitly.
extern s32 g_MainIsEndscreen, g_MainGameLogicEnabled;
extern s32 g_ViShakeDirection, g_ViShakeIntensity, g_ViShakeTimer;

static void *const g_SavestateExtraVars[] = {
	&g_MainIsEndscreen, &g_MainGameLogicEnabled,
	&g_ViShakeDirection, &g_ViShakeIntensity, &g_ViShakeTimer,
};

// Objectives that have already produced an auto-save on this stage, so one that flips back
// and forth (an item dropped and picked up again) does not save again. Kept outside the
// checkpoint on purpose: a load must not re-arm it.
static u32 g_SavestateSavedObjectives = 0;

PD_CONSTRUCTOR static void savestateConfigInit(void)
{
	configRegisterInt("Game.AutoSaveOnObjective", &g_SavestateAutoSave, 0, 1);
}

static s32 savestateInRange(const void *ptr, const u8 *begin, const u8 *end)
{
	return (const u8 *)ptr >= begin && (const u8 *)ptr < end + SAVESTATE_MARKER_SIZE;
}

static s32 savestateCheckLayout(void)
{
	// game globals, one initialised and one zeroed, must be inside the ranges...
	const s32 gameIn = savestateInRange(&g_ObjectiveLastIndex, g_SavestateDataBegin, g_SavestateDataEnd)
		&& savestateInRange(&g_Vars, g_SavestateBssBegin, g_SavestateBssEnd)
		&& savestateInRange(&g_MenuData, g_SavestateBssBegin, g_SavestateBssEnd);

	// ...and the port's own state must be outside them
	const s32 portOut = !savestateInRange(&g_MempHeap, g_SavestateDataBegin, g_SavestateDataEnd)
		&& !savestateInRange(&g_MempHeap, g_SavestateBssBegin, g_SavestateBssEnd)
		&& !savestateInRange(&g_SavestateAutoSave, g_SavestateDataBegin, g_SavestateDataEnd)
		&& !savestateInRange(&g_SavestateAutoSave, g_SavestateBssBegin, g_SavestateBssEnd);

	const s32 ordered = g_SavestateDataBegin < g_SavestateDataEnd && g_SavestateBssBegin < g_SavestateBssEnd;

	if (!(gameIn && portOut && ordered)) {
		sysLogPrintf(LOG_NOTE, "savestate: game globals are not contiguous on this build, checkpoints disabled");
		return 0;
	}

	return 1;
}

s32 savestateSupported(void)
{
	if (!g_SavestateChecked) {
		g_SavestateChecked = 1;
		g_SavestateLayoutOk = savestateCheckLayout();
	}

	return g_SavestateLayoutOk && savestateArenaBase() != NULL && !savestateArenaOverflowed();
}

s32 savestateHasCheckpoint(void)
{
	return g_SavestateValid;
}

static s32 savestateAnyObjectiveFailed(void)
{
	for (s32 i = 0; i <= g_ObjectiveLastIndex; i++) {
		if ((objectiveGetDifficultyBits(i) & (1 << lvGetDifficulty())) && g_ObjectiveStatuses[i] == OBJECTIVE_FAILED) {
			return true;
		}
	}
	return false;
}

static s32 savestateCanSave(void)
{
	return savestateSupported()
		&& g_StageNum < STAGE_TITLE
		&& !g_Vars.mplayerisrunning
		&& g_Vars.coopplayernum < 0
		&& g_Vars.antiplayernum < 0
		&& g_Vars.bond
		&& !g_Vars.bond->isdead
		&& !g_Vars.bond->aborted
		// a mission that can no longer be won is not worth a checkpoint
		&& !savestateAnyObjectiveFailed();
}

void savestateRequestLoad(void)
{
	if (g_SavestateValid) {
		g_SavestatePendingLoad = 1;
	}
}

void savestateOnObjectiveComplete(s32 index)
{
	// Protect-type objectives ('don't let X die') read as complete from the first check after
	// stage start, before the player has done anything, so the first second does not count
	if (!g_SavestateAutoSave || g_Vars.lvframenum < TICKS(60) || index < 0 || index >= 32) {
		return;
	}

	if (!(g_SavestateSavedObjectives & (1u << index))) {
		g_SavestateSavedObjectives |= 1u << index;
		g_SavestatePendingSave = 1;
	}
}

void savestateOnStageStart(void)
{
	g_SavestateValid = 0;
	g_SavestateSavedObjectives = 0;
	g_SavestatePendingSave = 0;
	g_SavestatePendingLoad = 0;
}

static void savestateSave(void)
{
	const size_t datalen = g_SavestateDataEnd + SAVESTATE_MARKER_SIZE - g_SavestateDataBegin;
	const size_t bsslen = g_SavestateBssEnd + SAVESTATE_MARKER_SIZE - g_SavestateBssBegin;
	const size_t arenalen = savestateArenaTop();
	const size_t total = datalen + bsslen + g_MempHeapSize + arenalen + ARRAYCOUNT(g_SavestateExtraVars) * 4;
	const u64 t0 = sysGetMicroseconds();

	if (total > g_SavestateBufCap) {
		free(g_SavestateBuf);
		g_SavestateBufCap = total + total / 8;
		g_SavestateBuf = malloc(g_SavestateBufCap);
		if (!g_SavestateBuf) {
			g_SavestateBufCap = 0;
			g_SavestateValid = 0;
			sysLogPrintf(LOG_ERROR, "savestate: could not allocate %u bytes", (u32)total);
			return;
		}
	}

	u8 *dst = g_SavestateBuf;
	memcpy(dst, g_SavestateDataBegin, datalen); dst += datalen;
	memcpy(dst, g_SavestateBssBegin, bsslen); dst += bsslen;
	memcpy(dst, g_MempHeap, g_MempHeapSize); dst += g_MempHeapSize;
	memcpy(dst, savestateArenaBase(), arenalen); dst += arenalen;
	for (u32 i = 0; i < ARRAYCOUNT(g_SavestateExtraVars); i++, dst += 4) {
		memcpy(dst, g_SavestateExtraVars[i], 4);
	}
	g_SavestateArenaLen = arenalen;
	g_SavestateValid = 1;

	sysLogPrintf(LOG_NOTE, "savestate: saved stage %d frame %d pos=(%.0f,%.0f,%.0f) health=%.2f (%u KB) in %u ms",
		g_StageNum, g_Vars.lvframenum, g_Vars.bond->prop->pos.x, g_Vars.bond->prop->pos.y, g_Vars.bond->prop->pos.z,
		g_Vars.bond->bondhealth, (u32)(total / 1024), (u32)((sysGetMicroseconds() - t0) / 1000));

	// after the copy, so the message is not part of the checkpoint itself
	hudmsgCreate("Checkpoint Saved", HUDMSGTYPE_DEFAULT);
}

static void savestateLoad(void)
{
	const size_t datalen = g_SavestateDataEnd + SAVESTATE_MARKER_SIZE - g_SavestateDataBegin;
	const size_t bsslen = g_SavestateBssEnd + SAVESTATE_MARKER_SIZE - g_SavestateBssBegin;
	const u64 t0 = sysGetMicroseconds();

	// things that must survive the load: settings, and the frame clock, which counts real
	// time and would otherwise see the whole gap since the save as one enormous frame
	const u32 cfglen = configValuesSize();
	u8 *cfg = malloc(cfglen ? cfglen : 1);
	if (cfg) {
		configValuesCopyOut(cfg);
	}

	const s32 prevframestartt = g_Vars.prevframestartt;
	const s32 thisframestartt = g_Vars.thisframestartt;
	const s32 diffframet = g_Vars.diffframet;
	const s32 lostframetime60t = g_Vars.lostframetime60t;
	const s32 lostframetime240t = g_Vars.lostframetime240t;

	const u8 *src = g_SavestateBuf;
	memcpy(g_SavestateDataBegin, src, datalen); src += datalen;
	memcpy(g_SavestateBssBegin, src, bsslen); src += bsslen;
	memcpy(g_MempHeap, src, g_MempHeapSize); src += g_MempHeapSize;
	memcpy(savestateArenaBase(), src, g_SavestateArenaLen); src += g_SavestateArenaLen;
	for (u32 i = 0; i < ARRAYCOUNT(g_SavestateExtraVars); i++, src += 4) {
		memcpy(g_SavestateExtraVars[i], src, 4);
	}

	if (cfg) {
		configValuesCopyIn(cfg);
		free(cfg);
	}

	g_Vars.prevframestartt = prevframestartt;
	g_Vars.thisframestartt = thisframestartt;
	g_Vars.diffframet = diffframet;
	g_Vars.lostframetime60t = lostframetime60t;
	g_Vars.lostframetime240t = lostframetime240t;

	// GPU textures are cached by the address of their source data, which now holds
	// whatever was loaded there at save time
	gfx_texture_cache_clear();

	// the button that chose Load Checkpoint is probably still held; without this the restored
	// input state would see it as a fresh press and fire or activate on the first frame
	joyDisableTemporarily();

	// the checkpoint was taken just after an objective completed, so its own
	// "Objective N: Completed" message is still in it
	hudmsgRemoveAll();

	sysLogPrintf(LOG_NOTE, "savestate: loaded stage %d frame %d pos=(%.0f,%.0f,%.0f) health=%.2f in %u ms",
		g_StageNum, g_Vars.lvframenum, g_Vars.bond->prop->pos.x, g_Vars.bond->prop->pos.y, g_Vars.bond->prop->pos.z,
		g_Vars.bond->bondhealth, (u32)((sysGetMicroseconds() - t0) / 1000));

	hudmsgCreate("Checkpoint Loaded", HUDMSGTYPE_DEFAULT);
}

void savestateTick(void)
{
	if (savestateArenaOverflowed()) {
		g_SavestateValid = 0;
	}

	if (g_SavestatePendingLoad) {
		g_SavestatePendingLoad = 0;
		g_SavestatePendingSave = 0;
		if (g_SavestateValid) {
			savestateLoad();
		}
		return;
	}

	if (g_SavestatePendingSave) {
		g_SavestatePendingSave = 0;
		if (savestateCanSave()) {
			savestateSave();
		}
	}
}
