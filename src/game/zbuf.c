#include <ultra64.h>
#include "constants.h"
#include "lib/sched.h"
#include "game/player.h"
#include "game/zbuf.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

u32 g_ZbufWidth;
u32 g_ZbufHeight;
u16 g_ArtifactsCfb0[0x180];
u16 g_ArtifactsCfb1[0x180];
u16 g_ArtifactsCfb2[0x180];

u16 *g_ZbufPtr1 = NULL;
u16 *g_ZbufPtr2 = NULL;

void *zbufGetAllocation(void)
{
	return g_ZbufPtr1;
}

void zbufReset(s32 stagenum)
{
	g_ZbufPtr1 = NULL;
	g_ZbufPtr2 = NULL;

	if (stagenum != STAGE_TITLE) {
		zbufAllocate();
	}
}

/**
 * In 4MB 2-player, the viewports are displayed with a vertical split and are
 * only half a screen height (the top 25% and bottom 25% of the screen are
 * black), so a half-height z-buffer is allocated.
 *
 * In 8MB, the full hi-res buffer is allocated. This makes sense for solo
 * missions because the player can switch to hi-res mid game. For normal
 * multiplayer this is wasteful but there's plenty of memory. For coop and anti
 * this is also wasteful, and memory is tight. They could have saved 137.5 KB.
 *
 * The allocation sizes need to enforce a minimum because the allocation is also
 * used by lighting initialisation code.
 */
void zbufAllocate(void)
{
	if (IS4MB()) {
		g_ZbufWidth = MAX(320, FBALLOC_WIDTH_LO);

		if (g_Vars.normmplayerisrunning && PLAYERCOUNT() >= 2) {
			g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_LO) / 2;
		} else {
			g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_LO);
		}
	} else {
		g_ZbufWidth = MAX(640, FBALLOC_WIDTH_HI);

		if (g_Vars.normmplayerisrunning && PLAYERCOUNT() >= 2) {
			g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_HI);
		} else {
			g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_HI);
		}
	}

	g_ZbufPtr1 = mempAlloc(g_ZbufWidth * g_ZbufHeight * sizeof(u16) + 0x40, MEMPOOL_STAGE);
	g_ZbufPtr1 = (void *) (((uintptr_t) g_ZbufPtr1 + 0x3f) & ~0x3f);
	g_ZbufPtr2 = g_ZbufPtr1;
}

/**
 * Note: There is only one z-buffer, so there is nothing to swap.
 * Both of these pointers always have the same value.
 *
 * We assume this is a swap function due to the context in which it's called.
 * Perhaps the developers implemented two buffers with swapping before realising
 * they only needed one.
 */
void zbufSwap(void)
{
	g_ZbufPtr2 = g_ZbufPtr1;
}

/**
 * In 8MB multiplayer, players on the bottom half of the screen have their
 * z-buffer shifted backwards by half a screen. This is safe because it's using
 * a scissor on the viewport.
 *
 * This allows the z-buffer allocation to be half a screen instead of a full
 * screen, however zbufAllocate allocates the full hi-res screen for 8MB,
 * so this benefit is not realised. The shifting code is likely from GE.
 */
Gfx *zbufConfigureRdp(Gfx *gdl)
{
	u32 subamount;
	uintptr_t addr;

	if (g_Vars.normmplayerisrunning
			&& (g_Vars.currentplayernum >= 2 || (PLAYERCOUNT() == 2 && g_Vars.currentplayernum == 1))) {
		subamount = playerGetFbWidth() * playerGetFbHeight();

		if (IS4MB() || optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			subamount = 0;
		}
	} else {
		subamount = 0;
	}

	addr = (uintptr_t) g_ZbufPtr2 - subamount;
	addr &= ~0x3f;

	gDPPipeSync(gdl++);
	gDPSetDepthImage(gdl++, addr);

	return gdl;
}

/**
 * Clear the current player's portion of the z-buffer.
 */
Gfx *zbufClear(Gfx *gdl)
{
#ifdef PLATFORM_N64
	s32 left;
	s32 right;

	gDPPipeSync(gdl++);
	gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetWidth(), OS_PHYSICAL_TO_K0(g_ZbufPtr2));
	gDPSetCycleType(gdl++, G_CYC_FILL);
	gDPSetFillColor(gdl++, 0xfffcfffc);
	gDPSetScissorFrac(gdl++, G_SC_NON_INTERLACE, 0, 0, playerGetFbWidth() * 4.0f, playerGetFbHeight() * 4.0f);

	if (PLAYERCOUNT() <= 2) {
		left = 0;
		right = playerGetFbWidth() - 1;
	} else if (g_Vars.currentplayernum == 0 || g_Vars.currentplayernum == 2) {
		left = 0;
		right = playerGetFbWidth() / 2 - 1;
	} else {
		left = playerGetFbWidth() / 2;
		right = playerGetFbWidth() - 1;
	}

	gDPFillRectangle(gdl++, left, 0, right, playerGetFbHeight() - 1);
	gDPPipeSync(gdl++);
#else
	gDPClearDepthEXT(gdl++);
#endif

	return gdl;
}

u16 *zbufGetArtifactsCfb(s32 index)
{
	u16 *addr;

	if (index == 0) {
		addr = g_ArtifactsCfb0;
	}

	if (index == 1) {
		addr = g_ArtifactsCfb1;
	}

	if (index == 2) {
		addr = g_ArtifactsCfb2;
	}

	addr = (u16 *) (((uintptr_t) addr + 0x3f) & ~0x3f);

	return addr;
}

/**
 * This method is designed to save artifact depths
 * prior to drawing the hands and weapon on-screen
 * since drawing them requires clearing the zbuffer
 * to avoid clipping the gun model on floors and walls.
 *
 * Note: this is not needed with the PC port. Use shotTestLos() instead.
 */
Gfx *zbufSaveArtifactDepths(Gfx *gdl)
{
#ifdef PLATFORM_N64
	struct artifact *artifacts = schedGetWriteArtifacts();
	u32 stack;
	u16 *zbuf = g_ZbufPtr1;
	u32 numsamples = 0;
	u16 *samples;
	u16 *thissample;
	u16 *zbufrow;
	s32 i;

	viGetBackBuffer();
	samples = zbufGetArtifactsCfb(g_SchedWriteArtifactsIndex);
	g_SchedSpecialArtifactIndexes[g_SchedWriteArtifactsIndex] = 1;

	gDPPipeSync(gdl++);
	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), OS_PHYSICAL_TO_K0(samples));
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_320, SCREEN_240);
	gDPSetCycleType(gdl++, G_CYC_COPY);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0080, 4, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 160, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_I, G_IM_SIZ_8b, 160, 0x0080, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, 15);
	gSPTexture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
	gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, 0xff);
	gDPSetPrimColor(gdl++, 0, 0, 0xff, 0xff, 0xff, 0xff);
	gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetAlphaDither(gdl++, G_AD_DISABLE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureDetail(gdl++, G_TD_CLAMP);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gSPClearGeometryMode(gdl++, G_ZBUFFER);
	gDPTileSync(gdl++);

	for (i = 0; i < MAX_ARTIFACTS; i++) {
		if (1);

		if (artifacts[i].type != ARTIFACTTYPE_FREE) {
			thissample = &samples[numsamples];
			zbufrow = &zbuf[artifacts[i].screeny * viGetWidth()];

			gDPPipeSync(gdl++);
			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_320, zbufrow);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, 5, 0, 0, viGetWidth() - 1, 0);
			gDPPipeSync(gdl++);

			gSPTextureRectangle(gdl++,
					numsamples << 2, 0,
					(numsamples + 3) << 2, 0,
					G_TX_RENDERTILE, (artifacts[i].screenx * 32) + 16, 0x0010, 0x1000, 0);

			artifacts[i].depthptr = thissample;
			numsamples++;

			if (thissample);
		}
	}

	gDPPipeSync(gdl++);
	gDPLoadSync(gdl++);
	gDPTileSync(gdl++);
	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), OS_PHYSICAL_TO_K0(viGetBackBuffer()));
	gDPSetScissorFrac(gdl++, G_SC_NON_INTERLACE, 0, 0, viGetWidth() * 4.0f, viGetHeight() * 4.0f);
	gSPSetGeometryMode(gdl++, G_ZBUFFER);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetColorDither(gdl++, G_CD_BAYER);

	if (samples);
#endif
	return gdl;
}
