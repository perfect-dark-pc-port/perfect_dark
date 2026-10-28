#include <stdlib.h>
#include <stdio.h>
#include <PR/ultratypes.h>
#include <PR/ultrasched.h>
#include <PR/os_message.h>

#include "lib/main.h"
#include "bss.h"
#include "data.h"

#include "video.h"
#include "audio.h"
#include "input.h"
#include "fs.h"
#include "romdata.h"
#include "config.h"
#include "mod.h"
#include "system.h"
#include "utils.h"

u32 g_OsMemSize = 0;
s32 g_OsMemSizeMb = 16;
u8 g_Is4Mb = 0;
s8 g_Resetting = false;
OSSched g_Sched;

OSMesgQueue g_MainMesgQueue;
OSMesg g_MainMesgBuf[32];

u8 *g_MempHeap = NULL;
u32 g_MempHeapSize = 0;

u32 g_VmNumTlbMisses = 0;
u32 g_VmNumPageMisses = 0;
u32 g_VmNumPageReplaces = 0;
u8 g_VmShowStats = 0;

s32 g_TickRateDiv = 1;
s32 g_TickExtraSleep = true;

s32 g_SkipIntro = false;

s32 g_FileAutoSelect = -1;

extern s32 g_StageNum;

s32 bootGetMemSize(void)
{
	return (s32)g_OsMemSize;
}

void *bootAllocateStack(s32 threadid, s32 size)
{
	static u8 bruh[0x1000];
	return bruh;
}

void bootCreateSched(void)
{
	osCreateMesgQueue(&g_MainMesgQueue, g_MainMesgBuf, ARRAYCOUNT(g_MainMesgBuf));
	if (osTvType == OS_TV_MPAL) {
		osCreateScheduler(&g_Sched, NULL, OS_VI_MPAL_LAN1, 1);
	} else {
		osCreateScheduler(&g_Sched, NULL, OS_VI_NTSC_LAN1, 1);
	}
}

static void gameInit(void)
{
	osMemSize = g_OsMemSizeMb * 1024 * 1024;

	for (s32 i = 0; i < MAX_PLAYERS; ++i) {
		struct extplayerconfig *cfg = g_PlayerExtCfg + i;
		cfg->fovzoommult = cfg->fovzoom ? cfg->fovy / 60.0f : 1.0f;
	}

	if (g_HudCenter == HUDCENTER_NORMAL) {
		g_HudAlignModeL = G_ASPECT_CENTER_EXT;
		g_HudAlignModeR = G_ASPECT_CENTER_EXT;
	} else if (g_HudCenter == HUDCENTER_WIDE) {
		g_HudAlignModeL = G_ASPECT_LEFT_EXT | G_ASPECT_WIDE_EXT;
		g_HudAlignModeR = G_ASPECT_RIGHT_EXT | G_ASPECT_WIDE_EXT;
	}
}

static void cleanup(void)
{
	sysLogPrintf(LOG_NOTE, "shutdown");
	inputSaveBinds();
	configSave(CONFIG_PATH);
	videoShutdown();
	crashShutdown();
	// TODO: actually shut down all subsystems
}

int main(int argc, const char **argv)
{
	sysInitArgs(argc, argv);

	if (!sysArgCheck("--no-crash-handler")) {
		crashInit();
	}

	sysInit();
	fsInit();
	configInit();
	videoInit();
	inputInit();
	audioInit();
	romdataInit();

	g_ValidGbcRomFound = romdataCheckGbcRom();

	gameInit();

	if (fsGetModDir()) {
		modConfigLoad(MOD_CONFIG_FNAME);
	}

	atexit(cleanup);

	bootCreateSched();

	g_OsMemSize = osGetMemSize();

	g_MempHeapSize = g_OsMemSize;
	g_MempHeap = sysMemZeroAlloc(g_MempHeapSize);
	if (!g_MempHeap) {
		sysFatalError("Could not alloc %u bytes for memp heap.", g_MempHeapSize);
	}

	sysLogPrintf(LOG_NOTE, "memp heap at %p - %p", g_MempHeap, g_MempHeap + g_MempHeapSize);
	sysLogPrintf(LOG_NOTE, "rom  file at %p - %p", g_RomFile, g_RomFile + g_RomFileSize);

	g_SndDisabled = sysArgCheck("--no-sound");

	g_StageNum = sysArgGetInt("--boot-stage", STAGE_TITLE);

	if (g_StageNum == STAGE_TITLE && (sysArgCheck("--skip-intro") || g_SkipIntro)) {
		// shorthand for --boot-stage 0x26
		g_StageNum = STAGE_CITRAINING;
	} else if (g_StageNum < 0x01 || g_StageNum > 0x5d) {
		// stage num out of range
		g_StageNum = STAGE_TITLE;
	}

	if (g_StageNum != STAGE_TITLE) {
		sysLogPrintf(LOG_NOTE, "boot stage set to 0x%02x", g_StageNum);
	}

	g_FileAutoSelect = sysArgGetInt("--profile", -1);
	if (g_FileAutoSelect >= 0) {
		sysLogPrintf(LOG_NOTE, "player profile set to %d", g_FileAutoSelect);
	}

	mainProc();

	return 0;
}

PD_CONSTRUCTOR static void gameConfigInit(void)
{
	configRegisterInt("Game.MemorySize", &g_OsMemSizeMb, 4, 2048);
	configRegisterInt("Game.CenterHUD", &g_HudCenter, 0, 2);
	configRegisterInt("Game.MenuMouseControl", &g_MenuMouseControl, 0, 1);
	configRegisterFloat("Game.ScreenShakeIntensity", &g_ViShakeIntensityMult, 0.f, 10.f);
	configRegisterInt("Game.TickRateDivisor", &g_TickRateDiv, 0, 10);
	configRegisterInt("Game.ExtraSleep", &g_TickExtraSleep, 0, 1);
	configRegisterInt("Game.SkipIntro", &g_SkipIntro, 0, 1);
	configRegisterInt("Game.DisableMpDeathMusic", &g_MusicDisableMpDeath, 0, 1);
	configRegisterInt("Game.GEMuzzleFlashes", &g_BgunGeMuzzleFlashes, 0, 1);
	configRegisterInt("Game.SplitscreenGlares", &g_SplitscreenGlares, 0, 1);
	configRegisterInt("Game.MaxExplosions", &g_MaxExplosions, 6, 96);
	for (s32 j = 0; j < MAX_PLAYERS; ++j) {
		const s32 i = j + 1;
		configRegisterFloat(strFmt("Game.Player%d.FovY", i), &g_PlayerExtCfg[j].fovy, 5.f, 175.f);
		configRegisterInt(strFmt("Game.Player%d.FovAffectsZoom", i), &g_PlayerExtCfg[j].fovzoom, 0, 1);
		configRegisterInt(strFmt("Game.Player%d.MouseAimMode", i), &g_PlayerExtCfg[j].mouseaimmode, 0, 1);
		configRegisterFloat(strFmt("Game.Player%d.MouseAimSpeedX", i), &g_PlayerExtCfg[j].mouseaimspeedx, 0.f, 10.f);
		configRegisterFloat(strFmt("Game.Player%d.MouseAimSpeedY", i), &g_PlayerExtCfg[j].mouseaimspeedy, 0.f, 10.f);
		configRegisterFloat(strFmt("Game.Player%d.RadialMenuSpeed", i), &g_PlayerExtCfg[j].radialmenuspeed, 0.f, 10.f);
		configRegisterFloat(strFmt("Game.Player%d.CrosshairSway", i), &g_PlayerExtCfg[j].crosshairsway, 0.f, 10.f);
		configRegisterFloat(strFmt("Game.Player%d.CrosshairEdgeBoundary", i), &g_PlayerExtCfg[j].crosshairedgeboundary, 0.0f, 1.0f);
		configRegisterInt(strFmt("Game.Player%d.CrouchMode", i), &g_PlayerExtCfg[j].crouchmode, 0, CROUCHMODE_TOGGLE_ANALOG);
		configRegisterInt(strFmt("Game.Player%d.ExtendedControls", i), &g_PlayerExtCfg[j].extcontrols, 0, 1);
		configRegisterUInt(strFmt("Game.Player%d.CrosshairColour", i), &g_PlayerExtCfg[j].crosshaircolour, 0, 0xFFFFFFFF);
		configRegisterUInt(strFmt("Game.Player%d.CrosshairSize", i), &g_PlayerExtCfg[j].crosshairsize, 0, 4);
		configRegisterInt(strFmt("Game.Player%d.CrosshairHealth", i), &g_PlayerExtCfg[j].crosshairhealth, 0, CROSSHAIR_HEALTH_ON_WHITE);
		configRegisterInt(strFmt("Game.Player%d.UseKeyReloads", i), &g_PlayerExtCfg[j].usereloads, 0, false);
	}
}
