#include <PR/ultratypes.h>
#include <PR/ultrasched.h>
#include "lib/boot.h"
#include "lib/sched.h"
#include "constants.h"
#include "game/menugfx.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/audiomgr.h"
#include "lib/reset.h"
#include "lib/rzip.h"
#include "lib/crash.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/pimgr.h"
#include "lib/profile.h"
#include "lib/rmon.h"
#include "lib/lib_48150.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "data.h"
#include "types.h"

#include "video.h"
#include "audio.h"
#include "input.h"
#include "mixer.h"
#include "system.h"

/*
 * private typedefs and defines
 */
#define VIDEO_MSG       666
#define RSP_DONE_MSG    667
#define RDP_DONE_MSG    668
#define PRE_NMI_MSG     669

/*
 * OSScTask state
 */
#define OS_SC_DP                0x0001  /* set if still needs dp        */
#define OS_SC_SP                0x0002  /* set if still needs sp        */
#define OS_SC_YIELD             0x0010  /* set if yield requested       */
#define OS_SC_YIELDED           0x0020  /* set if yield completed       */

/*
 * OSScTask->flags type identifier
 */
#define OS_SC_XBUS      (OS_SC_SP | OS_SC_DP)
#define OS_SC_DRAM      (OS_SC_SP | OS_SC_DP | OS_SC_DRAM_DLIST)
#define OS_SC_DP_XBUS   (OS_SC_SP)
#define OS_SC_DP_DRAM   (OS_SC_SP | OS_SC_DRAM_DLIST)
#define OS_SC_SP_XBUS   (OS_SC_DP)
#define OS_SC_SP_DRAM   (OS_SC_DP | OS_SC_DRAM_DLIST)

/*
 * private functions
 */
void __scMain(void *arg);
void __scHandleTasks(OSSched *s);
void __scHandleRSP(OSSched *s);
void __scHandleRDP(OSSched *s);
void __scAppendList(OSSched *s, OSScTask *t);
OSScTask *__scTaskReady(OSScTask *t);
s32 __scTaskComplete(OSSched *s,OSScTask *t);
void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp);
void __scYield(OSSched *s);
s32 __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp, s32 availRCP);

OSViMode var8008dcc0[NUM_GFXTASKS];
OSViMode *var8008dd60[NUM_GFXTASKS];
OSViMode var8008dd68[NUM_GFXTASKS];
s32 var8008de08;
s32 g_ViCurVStart0;
s32 g_ViCurVStart1;
u32 var8008de14;
OSTimer g_SchedRspTimer;
u32 g_SchedDpCounters[4];
struct artifact g_ArtifactLists[MAX_PLAYERS][3][120];
u8 g_SchedSpecialArtifactIndexes[3];
s32 g_SchedWriteArtifactsIndex[MAX_PLAYERS];
s32 g_SchedFrontArtifactsIndex[MAX_PLAYERS];
s32 g_SchedPendingArtifactsIndex[MAX_PLAYERS];

bool g_SchedCrashedUnexpectedly = false;
bool g_SchedCrashEnable1 = false;
bool g_SchedCrashEnable2 = false;
u32 g_SchedCrashRenderInterval = 45000000;
u32 g_SchedCrashLastRendered = 0;

s32 var8005ce74 = 0;
f32 g_ViXScalesBySlot[NUM_GFXTASKS] = {1, 1};
f32 g_ViYScalesBySlot[NUM_GFXTASKS] = {1, 1};
bool g_SchedViModesPending[NUM_GFXTASKS] = {false, false};
s32 g_ViUnblackTimer = NUM_FRAMEBUFFERS + 1;
s32 g_ViShakeDirection = 1;
s32 g_ViShakeIntensity = 0;
f32 g_ViShakeIntensityMult = 1.f;
s32 g_ViShakeTimer = 0;
u32 var8005cea0 = 0;
u32 var8005cea4 = 0;
OSScMsg g_SchedRspMsg = {OS_SC_RSP_MSG};
bool g_SchedIsFirstTask = true;

s32 g_PrevFrameFb = -1;
s32 g_BlurFb = -1;
s32 g_BlurFbCapTimer = -1;
bool g_BlurFbDirty = true;

u8 *g_LightTable[MAX_PLAYERS] = {NULL, NULL, NULL, NULL};
s32 g_SunFlareTimers[MAX_PLAYERS][3] = {0};

void schedSetCrashEnable1(bool enable)
{
	g_SchedCrashEnable1 = enable;
}

void schedSetCrashedUnexpectedly(bool enable)
{
	g_SchedCrashedUnexpectedly = enable;
}

void schedSetCrashEnable2(bool enable)
{
	g_SchedCrashEnable2 = enable;
}

void schedSetCrashRenderInterval(u32 cycles)
{
	g_SchedCrashRenderInterval = cycles;
}

void schedRenderCrashOnBuffer(void *framebuffer)
{
	if ((g_SchedCrashEnable2 && g_SchedCrashEnable1) || g_SchedCrashedUnexpectedly) {
		// crashRenderFrame(framebuffer);
		g_SchedCrashLastRendered = osGetCount();
	}
}

void schedRenderCrashPeriodically(u32 framecount)
{
	if ((framecount & 0xf) == 0 && ((g_SchedCrashEnable2 && g_SchedCrashEnable1) || g_SchedCrashedUnexpectedly)) {
		if (osGetCount() - g_SchedCrashLastRendered > g_SchedCrashRenderInterval) {
			// crashRenderFrame(g_FrameBuffers[0]);
			// crashRenderFrame(g_FrameBuffers[1]);
		}
	}
}

void schedInitCrashLastRendered(void)
{
	g_SchedCrashLastRendered = osGetCount();
}

void osCreateScheduler(OSSched *sc, OSThread *thread, u8 mode, u32 numFields)
{
	sc->curRSPTask = 0;
	sc->curRDPTask = 0;
	sc->clientList = 0;
	sc->frameCount = 0;
	sc->audioListHead = 0;
	sc->gfxListHead = 0;
	sc->audioListTail = 0;
	sc->gfxListTail = 0;
	sc->retraceMsg.type = OS_SC_RETRACE_MSG;
	sc->prenmiMsg.type = OS_SC_PRE_NMI_MSG;
	sc->thread = thread;

	osCreateMesgQueue(&sc->interruptQ, sc->intBuf, OS_SC_MAX_MESGS);
	osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, OS_SC_MAX_MESGS);

	osCreateViManager(OS_PRIORITY_VIMGR);

	var8008de08 = osViModeTable[mode].comRegs.hStart;
	g_ViCurVStart0 = osViModeTable[mode].fldRegs[0].vStart;
	g_ViCurVStart1 = osViModeTable[mode].fldRegs[1].vStart;

	var8008dd60[0] = &var8008dd68[0];
	var8008dd60[1] = &var8008dd68[1];

	var8008dd68[0] = osViModeTable[mode];
	var8008dd68[1] = osViModeTable[mode];

	osViSetEvent(&sc->interruptQ, (OSMesg)VIDEO_MSG, numFields);
	schedInitCrashLastRendered();

	g_PrevFrameFb = videoCreateFramebuffer(0, 0, false, true);
	g_BlurFb = videoCreateFramebuffer(0, 0, false, true);
}

void osScAddClient(OSSched *sc, OSScClient *c, OSMesgQueue *msgQ, bool is30fps)
{
	c->msgQ = msgQ;
	c->is30fps = is30fps;
	c->next = sc->clientList;
	sc->clientList = c;
}

OSMesgQueue *osScGetCmdQ(OSSched *sc)
{
	return &sc->cmdQ;
}

void __scUpdateViMode(void)
{
	if (g_SchedIsFirstTask) {
		osViBlack(false);
		g_SchedIsFirstTask = false;
	}

	var8005ce74 = (var8005ce74 + 1) % 2;

	if (g_SchedViModesPending[1 - var8005ce74]) {
		// TODO: make this a little less awkward
		extern struct rend_vidat *g_ViBackData;
		videoUpdateNativeResolution(g_ViBackData->bufx, g_ViBackData->bufy);
		g_SchedViModesPending[1 - var8005ce74] = false;
	}

	if (g_ViUnblackTimer != 0 && g_ViUnblackTimer <= NUM_FRAMEBUFFERS) {
		g_ViUnblackTimer--;
	}
}

/**
 * Nintendo's sheduler accepts tasks on a "command" message queue.
 * This isn't used here.
 *
 * In PD, the main and audio threads submit tasks by calling this function
 * instead. It temporarily increases the calling thread's priority above the
 * scheduler, adds the task to the linked list directly and attempts to execute
 * it. This is faster than the queue method because it avoids switching threads.
 */
void schedSubmitTask(OSSched *sc, OSScTask *t)
{
	if (sc->doAudio && t->list.t.type == M_AUDTASK) {
		// send this into the mixer
	} else if (t->list.t.type == M_GFXTASK) {
		videoSubmitCommands((Gfx *)t->list.t.data_ptr);
	}
}

void schedStartFrame(OSSched *sc)
{
	videoStartFrame();
}

void schedAudioFrame(OSSched *sc)
{
	s32 i;

	if (!g_SndDisabled) {
		for (i = 0; i < g_Vars.diffframe60; i++) {
			amgrFrame();
			audioEndFrame();
		}
	}
}

/**
 * Handle a retrace (vsync) event.
 *
 * Audio tasks are scheduled based on retrace + a timer (approximately 6ms).
 * On NTSC, this is done on every second frame if 8MB, or every second frame
 * if 4MB. I guess less memory means the audio queue has to be kept smaller
 * and processed more frequently. On PAL, it's every second frame regardless.
 *
 * Controller input is polled here.
 *
 * Lastly, if there's crash information available then it will be checked and
 * rendered periodically (once every 16 retraces). I guess this makes it render
 * if the RDP has hung.
 */
void schedEndFrame(OSSched *sc)
{
	sc->frameCount++;

#if PAL
	if (!g_Resetting && (sc->frameCount & 1)) {
		// osStopTimer(&g_SchedRspTimer);
		// osSetTimer(&g_SchedRspTimer, 280000, 0, amgrGetFrameMesgQueue(), &g_SchedRspMsg);
	}
#else
	if (!g_Resetting && ((sc->frameCount & 1) || IS4MB())) {
		// osStopTimer(&g_SchedRspTimer);
		// osSetTimer(&g_SchedRspTimer, 280000, 0, amgrGetFrameMesgQueue(), &g_SchedRspMsg);
	}
#endif

	if (!g_Resetting) {
		viHandleRetrace();
	}

	inputUpdate();

	joyStartReadData(&g_PiMesgQueue);
	joyReadData();
	joy00014238();

	sndHandleRetrace();
	schedAudioFrame(sc);
	schedRenderCrashPeriodically(sc->frameCount);
	videoEndFrame();

	if (g_MainIsBooting == 0) {
		schedConsiderScreenshot();
	}

	// check for vid mode changes
	__scUpdateViMode();
}

void schedInitArtifacts(void)
{
	s32 i;
	s32 j;
	s32 player;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < MAX_ARTIFACTS; j++) {
			for (player = 0; player < MAX_PLAYERS; player++) {
				g_ArtifactLists[player][i][j].type = ARTIFACTTYPE_FREE;
			}
		}

		g_SchedSpecialArtifactIndexes[i] = 0;
	}
}

/**
 * The write list is an artifact list that is not currently being displayed on
 * the screen. Update logic can write here to put artifacts on the next frame.
 */
struct artifact *schedGetWriteArtifacts(void)
{
	return g_ArtifactLists[g_Vars.currentplayernum][g_SchedWriteArtifactsIndex[g_Vars.currentplayernum]];
}

/**
 * The front list is the artifact list that is currently being displayed on the
 * screen. Rendering logic reads this list. The list may be re-used for multiple
 * frames in a row during lag.
 */
struct artifact *schedGetFrontArtifacts(void)
{
	return g_ArtifactLists[g_Vars.currentplayernum][g_SchedFrontArtifactsIndex[g_Vars.currentplayernum]];
}

/**
 * The pending list is possibly misnamed. I'm not sure how this list works.
 *
 * @TODO: Investigate.
 */
struct artifact *schedGetPendingArtifacts(void)
{
	return g_ArtifactLists[g_Vars.currentplayernum][g_SchedPendingArtifactsIndex[g_Vars.currentplayernum]];
}

void schedIncrementWriteArtifacts(void)
{
	g_SchedWriteArtifactsIndex[g_Vars.currentplayernum] = (g_SchedWriteArtifactsIndex[g_Vars.currentplayernum] + 1) % 3;
}

void schedIncrementFrontArtifacts(void)
{
	g_SchedFrontArtifactsIndex[g_Vars.currentplayernum] = (g_SchedFrontArtifactsIndex[g_Vars.currentplayernum] + 1) % 3;
}

void schedIncrementPendingArtifacts(void)
{
	g_SchedPendingArtifactsIndex[g_Vars.currentplayernum] = (g_SchedPendingArtifactsIndex[g_Vars.currentplayernum] + 1) % 3;
}

void schedResetArtifacts(void)
{
	s32 numlights = 0;

	if (g_Vars.stagenum < STAGE_TITLE && g_Rooms != NULL) {
		for (s32 i = 1; i < g_Vars.roomcount; i++) {
			numlights += g_Rooms[i].numlights;
		}
	}

	for (s32 player = 0; player < MAX_PLAYERS; player++) {
		g_SchedWriteArtifactsIndex[player] = 0;
		g_SchedFrontArtifactsIndex[player] = 1;
		g_SchedPendingArtifactsIndex[player] = 0;

		if (g_LightTable[player]) {
			sysMemFree(g_LightTable[player]);
			g_LightTable[player] = NULL;
		}
		if (numlights > 0) {
			g_LightTable[player] = (u8 *)sysMemZeroAlloc(ALIGN16(numlights * 3));
		}
	}
}

void schedUpdatePendingArtifacts(void)
{
	g_SchedSpecialArtifactIndexes[g_SchedPendingArtifactsIndex[g_Vars.currentplayernum]] = 0;
	schedIncrementPendingArtifacts();
}

void schedConsiderScreenshot(void)
{
	if (g_MenuData.screenshottimer == 1) {
		if (IS8MB()) {
			menugfxCreateBlur();
		}

		g_MenuData.screenshottimer = 0;
	}

	if (g_BlurFbCapTimer == 0) {
		videoCopyFramebuffer(g_BlurFb, 0, -1, -1);
		g_BlurFbCapTimer = -1;
		g_BlurFbDirty = false;
	} else if (g_BlurFbCapTimer > 0) {
		--g_BlurFbCapTimer;
	} else if (g_BlurFbCapTimer < 0) {
		// no blur requested this frame, mark blur fb dirty
		g_BlurFbDirty = true;
	}

	if (g_MenuData.screenshottimer >= 2) {
		g_MenuData.screenshottimer--;
	}
}

u8 *schedGetPlayerLightTable(void)
{
	/**
	 * Return the table which tracks brightness values
	 * for light artifacts. This is used to smooth the transition
	 * between visible and non-visible light states so that
	 * it looks more natural than a binary on/off. Do this
	 * on a per-player basis so that it works in splitscreen.
	 */
	return g_LightTable[g_Vars.currentplayernum];
}

s32 *schedGetPlayerSunTimers(void)
{
	/**
	 * Return the timers which handle Sun brightness timing.
	 * Do this on a per-player basis so that it works in splitscreen.
	 */
	return g_SunFlareTimers[g_Vars.currentplayernum];
}
