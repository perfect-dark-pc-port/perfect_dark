#include <ultra64.h>
#include "constants.h"
#include "game/quaternion.h"
#include "game/game_0b2150.h"
#include "game/camera.h"
#include "game/sky.h"
#include "game/game_152fa0.h"
#include "game/env.h"
#include "game/pad.h"
#include "game/tex.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/mtx.h"
#include "lib/sched.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#include "game/gfxmemory.h"
#include "game/artifact.h"
#include "game/player.h"
#endif

#define SKYABS(val) (val >= 0.0f ? (val) : -(val))

#define CORNERSTATE_NONE     0x0
#define CORNERSTATE_BR       0x1
#define CORNERSTATE_BL       0x2
#define CORNERSTATE_BOTTOM   0x3
#define CORNERSTATE_TR       0x4
#define CORNERSTATE_RIGHT    0x5
#define CORNERSTATE_TR_BL_BR 0x7
#define CORNERSTATE_TL       0x8
#define CORNERSTATE_LEFT     0xa
#define CORNERSTATE_TL_BL_BR 0xb
#define CORNERSTATE_TOP      0xc
#define CORNERSTATE_TL_TR_BR 0xd
#define CORNERSTATE_TL_TR_BL 0xe
#define CORNERSTATE_FULL     0xf

u32 g_SkyStageNum;
bool g_SkyLightningActive;
Mtxf g_SkyMtx;
struct coord g_SunPositions[3]; // relative to centre screen, with a huge scale
u32 var800a340c;
f32 g_SunScreenXPositions[4];
f32 g_SunScreenYPositions[4];

f32 g_SkyCloudOffset = 0;
f32 g_SkyWindSpeed = 1;
f32 g_SunAlphaFracs[3] = {0};
#ifdef PLATFORM_N64
s32 g_SunFlareTimers240[3] = {0};
#else
s32 *g_SunFlareTimers240;
#endif

void skyGetWorldPosFromScreenPos(f32 left, f32 top, struct coord *dst)
{
	Mtxf *mtx = camGetProjectionMtxF();
	f32 pos[2];

	pos[0] = left + camGetScreenLeft();
	pos[1] = top + camGetScreenTop() + envGetCurrent()->clouds_height;

	cam0f0b4c3c(pos, dst, 100);
	mtx4RotateVecInPlace(mtx, dst);
}

bool skyIsScreenCornerInSky(struct coord *corner3dpos, struct coord *dstpos, f32 *dstfrac)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	f32 f12 = 2.0f * corner3dpos->y / sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2] + 0.0001f);
	f32 sp2c;
	f32 f12_2;
	f32 sp24;
	u32 stack[2];

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*dstfrac = 1.0f - f12;

	if (corner3dpos->y == 0.0f) {
		sp24 = 0.01f;
	} else {
		sp24 = corner3dpos->y;
	}

	if (sp24 > 0.0f) {
		sp2c = (envGetCurrent()->clouds_scale - campos->y) / sp24;
		f12_2 = sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		dstpos->x = campos->x + sp2c * corner3dpos->f[0];
		dstpos->y = campos->y + sp2c * sp24;
		dstpos->z = campos->z + sp2c * corner3dpos->f[2];

		return true;
	}

	return false;
}

bool skyIsCornerInWater(struct coord *corner3dpos, struct coord *dstpos, f32 *dstfrac)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	f32 f12 = -2.0f * corner3dpos->y / sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2] + 0.0001f);
	f32 sp2c;
	f32 f12_2;
	f32 sp24;
	u32 stack[2];

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*dstfrac = 1.0f - f12;

	if (corner3dpos->y == 0.0f) {
		sp24 = -0.01f;
	} else {
		sp24 = corner3dpos->y;
	}

	if (sp24 < 0.0f) {
		sp2c = (envGetCurrent()->water_scale - campos->y) / sp24;
		f12_2 = sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		dstpos->x = campos->x + sp2c * corner3dpos->f[0];
		dstpos->y = campos->y + sp2c * sp24;
		dstpos->z = campos->z + sp2c * corner3dpos->f[2];

		return true;
	}

	return false;
}

/**
 * Scale base based on the height percentage between base and ref...
 * except the new y is zero.
 */
void skyCalculateEdgeVertex(struct coord *base, struct coord *ref, struct coord *out)
{
	f32 mult = base->y / (base->y - ref->y);

	out->x = (ref->x - base->x) * mult + base->x;
	out->y = 0;
	out->z = (ref->z - base->z) * mult + base->z;
}

f32 skyClamp(f32 value, f32 min, f32 max)
{
	if (value < min) {
		return min;
	}

	if (value > max) {
		return max;
	}

	return value;
}

f32 skyRound(f32 value)
{
	return (s32)(value + 0.5f);
}

void skyChooseCloudVtxColour(struct skyvtx3d *arg0, f32 arg1)
{
	struct environment *env = envGetCurrent();
	f32 scale = 1.0f - arg1;
	f32 r = env->sky_r;
	f32 g = env->sky_g;
	f32 b = env->sky_b;

	arg0->r = r + env->clouds_r * (1.0f - r * (1.0f / 255.0f)) * scale;
	arg0->g = g + env->clouds_g * (1.0f - g * (1.0f / 255.0f)) * scale;
	arg0->b = b + env->clouds_b * (1.0f - b * (1.0f / 255.0f)) * scale;

	if (g_SkyLightningActive) {
		arg0->r = arg0->g = arg0->b = 0xff;
	}

	arg0->a = 0xff;
}

void skyChooseWaterVtxColour(struct skyvtx3d *arg0, f32 arg1)
{
	struct environment *env = envGetCurrent();
	f32 scale = 1.0f - arg1;
	f32 r = env->sky_r;
	f32 g = env->sky_g;
	f32 b = env->sky_b;

	arg0->r = r + env->water_r * (1.0f - r * (1.0f / 255.0f)) * scale;
	arg0->g = g + env->water_g * (1.0f - g * (1.0f / 255.0f)) * scale;
	arg0->b = b + env->water_b * (1.0f - b * (1.0f / 255.0f)) * scale;
	arg0->a = 0xff;
}

Gfx *skyRender(Gfx *gdl)
{
	struct coord tl3dpos;
	struct coord tr3dpos;
	struct coord bl3dpos;
	struct coord br3dpos;
	struct coord sp674;
	struct coord sp668;
	struct coord sp65c;
	struct coord sp650;
	struct coord sp644;
	struct coord sp638;
	struct coord sp62c;
	struct coord sp620;
	struct coord sp614;
	struct coord sp608;
	struct coord sp5fc;
	struct coord sp5f0;
	struct coord sp5e4;
	struct coord sp5d8;
	struct coord sp5cc;
	struct coord sp5c0;
	struct coord sp5b4;
	struct coord sp5a8;
	struct coord sp59c;
	struct coord sp590;
	f32 sp58c;
	f32 sp588;
	f32 sp584;
	f32 sp580;
	f32 sp57c;
	f32 sp578;
	f32 sp574;
	f32 sp570;
	f32 sp56c;
	f32 sp568;
	f32 sp564;
	f32 sp560;
	f32 sp55c;
	f32 sp558;
	f32 sp554;
	f32 sp550;
	f32 sp54c;
	f32 sp548;
	s32 numvertices;
	s32 j;
	s32 cornerstate;
	s32 tlcornerissky;
	s32 trcornerissky;
	s32 blcornerissky;
	s32 brcornerissky;
	struct skyvtx3d skyvertices3d[5];
	struct skyvtx3d watervertices3d[5];
	f32 tmp;
	f32 scale;
	bool sp430;
	struct environment *env;

	sp430 = false;
	env = envGetCurrent();

	if (!env->clouds_enabled || g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		if (PLAYERCOUNT() == 1) {
			gDPSetCycleType(gdl++, G_CYC_FILL);

			if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
				gdl = viSetFillColour(gdl, 0, 0, 0);
			} else {
				gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
			}

			gDPFillRectangle(gdl++, viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth() - 1,
					viGetViewTop() + viGetViewHeight() - 1);

			gDPPipeSync(gdl++);
			return gdl;
		}

		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_FILL);

		if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
			gdl = viSetFillColour(gdl, 0, 0, 0);
		} else {
			gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
		}

		gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);

		gDPFillRectangle(gdl++,
				g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop,
				g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth - 1,
				g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight - 1);

		gDPPipeSync(gdl++);
		return gdl;
	}

	gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);

	if (&tl3dpos);

#ifdef PLATFORM_N64
	skyGetWorldPosFromScreenPos(0.0f, 0.0f, &tl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() - 0.1f, 0.0f, &tr3dpos);
	skyGetWorldPosFromScreenPos(0.0f, camGetScreenHeight() - 0.1f, &bl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() - 0.1f, camGetScreenHeight() - 0.1f, &br3dpos);
#else
	skyGetWorldPosFromScreenPos(-4.0f, -4.0f, &tl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() + 4.0f, -4.0f, &tr3dpos);
	skyGetWorldPosFromScreenPos(-4.0f, camGetScreenHeight() + 4.0f, &bl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() + 4.0f, camGetScreenHeight() + 4.0f, &br3dpos);
#endif

	tlcornerissky = skyIsScreenCornerInSky(&tl3dpos, &sp644, &sp58c);
	trcornerissky = skyIsScreenCornerInSky(&tr3dpos, &sp638, &sp588);
	blcornerissky = skyIsScreenCornerInSky(&bl3dpos, &sp62c, &sp584);
	brcornerissky = skyIsScreenCornerInSky(&br3dpos, &sp620, &sp580);

	skyIsCornerInWater(&tl3dpos, &sp5e4, &sp56c);
	skyIsCornerInWater(&tr3dpos, &sp5d8, &sp568);
	skyIsCornerInWater(&bl3dpos, &sp5cc, &sp564);
	skyIsCornerInWater(&br3dpos, &sp5c0, &sp560);

	// For each screen edge, check if one vertex is off-sky and the other is on-sky.
	// If so, calculate where along the the edge the sky starts/ends.
	if (tlcornerissky != blcornerissky) {
		sp54c = camGetScreenTop() + camGetScreenHeight() * (tl3dpos.f[1] / (tl3dpos.f[1] - bl3dpos.f[1]));

		skyGetWorldPosFromScreenPos(0.0f, sp54c, &sp65c);
		skyCalculateEdgeVertex(&tl3dpos, &bl3dpos, &sp65c);
		skyIsScreenCornerInSky(&sp65c, &sp5fc, &sp574);
		skyIsCornerInWater(&sp65c, &sp59c, &sp554);
	} else {
		sp54c = 0.0f;
	}

	if (trcornerissky != brcornerissky) {
		sp548 = camGetScreenTop() + camGetScreenHeight() * (tr3dpos.f[1] / (tr3dpos.f[1] - br3dpos.f[1]));

		skyGetWorldPosFromScreenPos(camGetScreenWidth() - 0.1f, sp548, &sp650);
		skyCalculateEdgeVertex(&tr3dpos, &br3dpos, &sp650);
		skyIsScreenCornerInSky(&sp650, &sp5f0, &sp570);
		skyIsCornerInWater(&sp650, &sp590, &sp550);
	} else {
		sp548 = 0.0f;
	}

	if (tlcornerissky != trcornerissky) {
		skyGetWorldPosFromScreenPos(camGetScreenLeft() + camGetScreenWidth() * (tl3dpos.f[1] / (tl3dpos.f[1] - tr3dpos.f[1])), 0.0f, &sp674);
		skyCalculateEdgeVertex(&tl3dpos, &tr3dpos, &sp674);
		skyIsScreenCornerInSky(&sp674, &sp614, &sp57c);
		skyIsCornerInWater(&sp674, &sp5b4, &sp55c);
	}

	if (blcornerissky != brcornerissky) {
		tmp = camGetScreenLeft() + camGetScreenWidth() * (bl3dpos.f[1] / (bl3dpos.f[1] - br3dpos.f[1]));

		skyGetWorldPosFromScreenPos(tmp, camGetScreenHeight() - 0.1f, &sp668);
		skyCalculateEdgeVertex(&bl3dpos, &br3dpos, &sp668);
		skyIsScreenCornerInSky(&sp668, &sp608, &sp578);
		skyIsCornerInWater(&sp668, &sp5a8, &sp558);
	}

	cornerstate = (tlcornerissky << 3) | (trcornerissky << 2) | (blcornerissky << 1) | brcornerissky;

	/**
	 * Do maths stuff for the ground/water/below-horizon plane.
	 *
	 * The CORNERSTATE constants denote which corners are in the sky,
	 * which is why these cases appear to be inverted.
	 */
	switch (cornerstate) {
	case CORNERSTATE_FULL:
		// All four screen corners are in the sky.
		numvertices = 0;
		scale = 1.0f / 30.0f;
		break;
	case CORNERSTATE_NONE:
		// All four screen corners are on the ground.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp5cc.f[0] * scale;
		watervertices3d[2].y = sp5cc.f[1] * scale;
		watervertices3d[2].z = sp5cc.f[2] * scale;
		watervertices3d[3].x = sp5c0.f[0] * scale;
		watervertices3d[3].y = sp5c0.f[1] * scale;
		watervertices3d[3].z = sp5c0.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5cc.f[0];
		watervertices3d[2].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5c0.f[0];
		watervertices3d[3].t = sp5c0.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp564);
		skyChooseWaterVtxColour(&watervertices3d[3], sp560);
		break;
	case CORNERSTATE_BOTTOM:
		// The bottom corners are in the sky.
		// This is possible by turning the drugspy upside down in Air Base.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp59c.f[0] * scale;
		watervertices3d[2].y = sp59c.f[1] * scale;
		watervertices3d[2].z = sp59c.f[2] * scale;
		watervertices3d[3].x = sp590.f[0] * scale;
		watervertices3d[3].y = sp590.f[1] * scale;
		watervertices3d[3].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp59c.f[0];
		watervertices3d[2].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp590.f[0];
		watervertices3d[3].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp554);
		skyChooseWaterVtxColour(&watervertices3d[3], sp550);
		break;
	case CORNERSTATE_TOP:
		// The top corners are in the sky. A common occurrence.
		numvertices = 4;
		sp430 = true;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5cc.f[0] * scale;
		watervertices3d[1].y = sp5cc.f[1] * scale;
		watervertices3d[1].z = sp5cc.f[2] * scale;
		watervertices3d[2].x = sp590.f[0] * scale;
		watervertices3d[2].y = sp590.f[1] * scale;
		watervertices3d[2].z = sp590.f[2] * scale;
		watervertices3d[3].x = sp59c.f[0] * scale;
		watervertices3d[3].y = sp59c.f[1] * scale;
		watervertices3d[3].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5cc.f[0];
		watervertices3d[1].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp590.f[0];
		watervertices3d[2].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp59c.f[0];
		watervertices3d[3].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp564);
		skyChooseWaterVtxColour(&watervertices3d[2], sp550);
		skyChooseWaterVtxColour(&watervertices3d[3], sp554);
		break;
	case CORNERSTATE_LEFT:
		// The left side corners are in the sky.
		// This would happen if the camera rolls significantly.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp5c0.f[0] * scale;
		watervertices3d[1].y = sp5c0.f[1] * scale;
		watervertices3d[1].z = sp5c0.f[2] * scale;
		watervertices3d[2].x = sp5b4.f[0] * scale;
		watervertices3d[2].y = sp5b4.f[1] * scale;
		watervertices3d[2].z = sp5b4.f[2] * scale;
		watervertices3d[3].x = sp5a8.f[0] * scale;
		watervertices3d[3].y = sp5a8.f[1] * scale;
		watervertices3d[3].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5c0.f[0];
		watervertices3d[1].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5b4.f[0];
		watervertices3d[2].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5a8.f[0];
		watervertices3d[3].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp560);
		skyChooseWaterVtxColour(&watervertices3d[2], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[3], sp558);
		break;
	case CORNERSTATE_RIGHT:
		// The right side corners are in the sky.
		// This would happen if the camera rolls significantly.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp5e4.f[0] * scale;
		watervertices3d[1].y = sp5e4.f[1] * scale;
		watervertices3d[1].z = sp5e4.f[2] * scale;
		watervertices3d[2].x = sp5a8.f[0] * scale;
		watervertices3d[2].y = sp5a8.f[1] * scale;
		watervertices3d[2].z = sp5a8.f[2] * scale;
		watervertices3d[3].x = sp5b4.f[0] * scale;
		watervertices3d[3].y = sp5b4.f[1] * scale;
		watervertices3d[3].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5e4.f[0];
		watervertices3d[1].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5a8.f[0];
		watervertices3d[2].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5b4.f[0];
		watervertices3d[3].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp558);
		skyChooseWaterVtxColour(&watervertices3d[3], sp55c);
		break;
	case CORNERSTATE_TL_TR_BL:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5a8.f[0] * scale;
		watervertices3d[1].y = sp5a8.f[1] * scale;
		watervertices3d[1].z = sp5a8.f[2] * scale;
		watervertices3d[2].x = sp590.f[0] * scale;
		watervertices3d[2].y = sp590.f[1] * scale;
		watervertices3d[2].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5a8.f[0];
		watervertices3d[1].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp590.f[0];
		watervertices3d[2].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp558);
		skyChooseWaterVtxColour(&watervertices3d[2], sp550);
		break;
	case CORNERSTATE_TL_TR_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp59c.f[0] * scale;
		watervertices3d[1].y = sp59c.f[1] * scale;
		watervertices3d[1].z = sp59c.f[2] * scale;
		watervertices3d[2].x = sp5a8.f[0] * scale;
		watervertices3d[2].y = sp5a8.f[1] * scale;
		watervertices3d[2].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp59c.f[0];
		watervertices3d[1].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5a8.f[0];
		watervertices3d[2].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp554);
		skyChooseWaterVtxColour(&watervertices3d[2], sp558);
		break;
	case CORNERSTATE_TL_BL_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp590.f[0] * scale;
		watervertices3d[1].y = sp590.f[1] * scale;
		watervertices3d[1].z = sp590.f[2] * scale;
		watervertices3d[2].x = sp5b4.f[0] * scale;
		watervertices3d[2].y = sp5b4.f[1] * scale;
		watervertices3d[2].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp590.f[0];
		watervertices3d[1].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5b4.f[0];
		watervertices3d[2].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp550);
		skyChooseWaterVtxColour(&watervertices3d[2], sp55c);
		break;
	case CORNERSTATE_TR_BL_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5b4.f[0] * scale;
		watervertices3d[1].y = sp5b4.f[1] * scale;
		watervertices3d[1].z = sp5b4.f[2] * scale;
		watervertices3d[2].x = sp59c.f[0] * scale;
		watervertices3d[2].y = sp59c.f[1] * scale;
		watervertices3d[2].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5b4.f[0];
		watervertices3d[1].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp59c.f[0];
		watervertices3d[2].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp554);
		break;
	case CORNERSTATE_BR:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp5e4.f[0] * scale;
		watervertices3d[1].y = sp5e4.f[1] * scale;
		watervertices3d[1].z = sp5e4.f[2] * scale;
		watervertices3d[2].x = sp5d8.f[0] * scale;
		watervertices3d[2].y = sp5d8.f[1] * scale;
		watervertices3d[2].z = sp5d8.f[2] * scale;
		watervertices3d[3].x = sp590.f[0] * scale;
		watervertices3d[3].y = sp590.f[1] * scale;
		watervertices3d[3].z = sp590.f[2] * scale;
		watervertices3d[4].x = sp5a8.f[0] * scale;
		watervertices3d[4].y = sp5a8.f[1] * scale;
		watervertices3d[4].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5e4.f[0];
		watervertices3d[1].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5d8.f[0];
		watervertices3d[2].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp590.f[0];
		watervertices3d[3].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp5a8.f[0];
		watervertices3d[4].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp568);
		skyChooseWaterVtxColour(&watervertices3d[3], sp550);
		skyChooseWaterVtxColour(&watervertices3d[4], sp558);
		break;
	case CORNERSTATE_BL:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp5c0.f[0] * scale;
		watervertices3d[2].y = sp5c0.f[1] * scale;
		watervertices3d[2].z = sp5c0.f[2] * scale;
		watervertices3d[3].x = sp5a8.f[0] * scale;
		watervertices3d[3].y = sp5a8.f[1] * scale;
		watervertices3d[3].z = sp5a8.f[2] * scale;
		watervertices3d[4].x = sp59c.f[0] * scale;
		watervertices3d[4].y = sp59c.f[1] * scale;
		watervertices3d[4].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5c0.f[0];
		watervertices3d[2].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5a8.f[0];
		watervertices3d[3].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp59c.f[0];
		watervertices3d[4].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp560);
		skyChooseWaterVtxColour(&watervertices3d[3], sp558);
		skyChooseWaterVtxColour(&watervertices3d[4], sp554);
		break;
	case CORNERSTATE_TR:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5cc.f[0] * scale;
		watervertices3d[1].y = sp5cc.f[1] * scale;
		watervertices3d[1].z = sp5cc.f[2] * scale;
		watervertices3d[2].x = sp5e4.f[0] * scale;
		watervertices3d[2].y = sp5e4.f[1] * scale;
		watervertices3d[2].z = sp5e4.f[2] * scale;
		watervertices3d[3].x = sp5b4.f[0] * scale;
		watervertices3d[3].y = sp5b4.f[1] * scale;
		watervertices3d[3].z = sp5b4.f[2] * scale;
		watervertices3d[4].x = sp590.f[0] * scale;
		watervertices3d[4].y = sp590.f[1] * scale;
		watervertices3d[4].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5cc.f[0];
		watervertices3d[1].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5e4.f[0];
		watervertices3d[2].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5b4.f[0];
		watervertices3d[3].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp590.f[0];
		watervertices3d[4].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp564);
		skyChooseWaterVtxColour(&watervertices3d[2], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[3], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[4], sp550);
		break;
	case CORNERSTATE_TL:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp5c0.f[0] * scale;
		watervertices3d[1].y = sp5c0.f[1] * scale;
		watervertices3d[1].z = sp5c0.f[2] * scale;
		watervertices3d[2].x = sp5cc.f[0] * scale;
		watervertices3d[2].y = sp5cc.f[1] * scale;
		watervertices3d[2].z = sp5cc.f[2] * scale;
		watervertices3d[3].x = sp59c.f[0] * scale;
		watervertices3d[3].y = sp59c.f[1] * scale;
		watervertices3d[3].z = sp59c.f[2] * scale;
		watervertices3d[4].x = sp5b4.f[0] * scale;
		watervertices3d[4].y = sp5b4.f[1] * scale;
		watervertices3d[4].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5c0.f[0];
		watervertices3d[1].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5cc.f[0];
		watervertices3d[2].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp59c.f[0];
		watervertices3d[3].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp5b4.f[0];
		watervertices3d[4].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp560);
		skyChooseWaterVtxColour(&watervertices3d[2], sp564);
		skyChooseWaterVtxColour(&watervertices3d[3], sp554);
		skyChooseWaterVtxColour(&watervertices3d[4], sp55c);
		break;
	default:
		return gdl;
	}

	if (numvertices > 0) {
		// Some corners are not in the sky, so consider water
		Mtxf sp3cc;
		Mtxf sp38c;
		struct skyvtx2d watervertices2d[5];
		s32 i;

		mtx4MultMtx4(camGetMtxF1754(), camGetWorldToScreenMtxf(), &sp3cc);
		guScaleF(g_SkyMtx.m, 1.0f / scale, 1.0f / scale, 1.0f / scale);
		mtx4MultMtx4(&sp3cc, &g_SkyMtx, &sp38c);

		for (i = 0; i < numvertices; i++) {
			skyConvertVertex(&watervertices3d[i], &sp38c, 130, 65535.0f, 65535.0f, &watervertices2d[i]);

			watervertices2d[i].x = skyClamp(watervertices2d[i].x, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
			watervertices2d[i].y = skyClamp(watervertices2d[i].y, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);

			if (watervertices2d[i].y > camGetScreenTop() * 4.0f + 4.0f
					&& watervertices2d[i].y < (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 4.0f) {
				watervertices2d[i].y -= 4.0f;
			}
		}

		if (!env->water_enabled) {
			f32 x1 = 1279.0f;
			f32 y1 = 959.0f;
			f32 x2 = 0.0f;
			f32 y2 = 0.0f;

			for (j = 0; j < numvertices; j++) {
				if (watervertices2d[j].x < x1) {
					x1 = watervertices2d[j].x;
				}

				if (watervertices2d[j].x > x2) {
					x2 = watervertices2d[j].x;
				}

				if (watervertices2d[j].y < y1) {
					y1 = watervertices2d[j].y;
				}

				if (watervertices2d[j].y > y2) {
					y2 = watervertices2d[j].y;
				}
			}

			gDPPipeSync(gdl++);
			gDPSetCycleType(gdl++, G_CYC_FILL);
			gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
			gDPSetTexturePersp(gdl++, G_TP_NONE);
			gDPFillRectangle(gdl++, (s32)(x1 * 0.25f), (s32)(y1 * 0.25f), (s32)(x2 * 0.25f), (s32)(y2 * 0.25f));
			gDPPipeSync(gdl++);
			gDPSetTexturePersp(gdl++, G_TP_PERSP);
		} else {
			gDPPipeSync(gdl++);

			texSelect(&gdl, &g_TexSkyWaterConfigs[env->water_type], 1, 0, 2, 1, NULL);

			gDPSetRenderMode(gdl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

#ifdef PLATFORM_N64
			if (numvertices == 4) {
				gdl = skyRenderTri(gdl, &watervertices2d[0], &watervertices2d[1], &watervertices2d[3], 130.0f, true);

				if (sp430) {
					watervertices2d[0].y++;
					watervertices2d[1].y++;
					watervertices2d[2].y++;
					watervertices2d[3].y++;
				}

				gdl = skyRenderTri(gdl, &watervertices2d[3], &watervertices2d[2], &watervertices2d[0], 130.0f, true);
			} else if (numvertices == 5) {
				// 3 corners are on the ground
				gdl = skyRenderTri(gdl, &watervertices2d[0], &watervertices2d[1], &watervertices2d[2], 130.0f, true);
				gdl = skyRenderTri(gdl, &watervertices2d[0], &watervertices2d[2], &watervertices2d[3], 130.0f, true);
				gdl = skyRenderTri(gdl, &watervertices2d[0], &watervertices2d[3], &watervertices2d[4], 130.0f, true);
			} else if (numvertices == 3) {
				// 1 corner is on the ground
				gdl = skyRenderTri(gdl, &watervertices2d[0], &watervertices2d[1], &watervertices2d[2], 130.0f, true);
			}
#else
			Vtx *verts = gfxAllocateVertices(numvertices);
			Col *cols = gfxAllocateColours(numvertices);
			Mtxf *mtx = gfxAllocateMatrix();
			mtx4MultMtx4(camGetWorldToScreenMtxf(), &g_SkyMtx, mtx);
			mtxF2L(mtx, mtx);

			gSPSetExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
			gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
			gSPColor(gdl++, osVirtualToPhysical(cols), numvertices);
			gSPVertex(gdl++, osVirtualToPhysical(verts), numvertices, 0);

			for (s32 i = 0; i < numvertices; ++i) {
				verts[i].x = watervertices3d[i].x;
				verts[i].y = watervertices3d[i].y;
				verts[i].z = watervertices3d[i].z;
				verts[i].s = skyClamp(watervertices3d[i].s * 0.1f + g_SkyCloudOffset, -32768.f, 32767.f);
				verts[i].t = skyClamp((watervertices3d[i].t  - g_SkyCloudOffset) * 0.1f + g_SkyCloudOffset, -32768.f, 32767.f);
				verts[i].colour = i * 4;
				cols[i].r = watervertices3d[i].r;
				cols[i].g = watervertices3d[i].g;
				cols[i].b = watervertices3d[i].b;
				cols[i].a = watervertices3d[i].a;
			}

			if (numvertices == 4) {
				gSPTri2(gdl++, 0, 1, 3, 3, 2, 0);
			} else if (numvertices == 5) {
				gSPTri3(gdl++, 0, 1, 2, 0, 2, 3, 0, 3, 4);
			} else if (numvertices == 3) {
				gSPTri1(gdl++, 0, 1, 2);
			}

			gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
			gSPClearExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
#endif
		}
	}

	/**
	 * Maths for the upper half of the skydome.
	 */
	switch (cornerstate) {
	case CORNERSTATE_NONE:
		// All four screen corners are on the ground.
		return gdl;
	case CORNERSTATE_FULL:
		// All four screen corners are in the sky.
		numvertices = 4;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp62c.f[0] * scale;
		skyvertices3d[2].y = sp62c.f[1] * scale;
		skyvertices3d[2].z = sp62c.f[2] * scale;
		skyvertices3d[3].x = sp620.f[0] * scale;
		skyvertices3d[3].y = sp620.f[1] * scale;
		skyvertices3d[3].z = sp620.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp62c.f[0] * 0.1f;
		skyvertices3d[2].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp620.f[0] * 0.1f;
		skyvertices3d[3].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp580);
		break;
	case CORNERSTATE_TOP:
		numvertices = 4;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp5fc.f[0] * scale;
		skyvertices3d[2].y = sp5fc.f[1] * scale;
		skyvertices3d[2].z = sp5fc.f[2] * scale;
		skyvertices3d[3].x = sp5f0.f[0] * scale;
		skyvertices3d[3].y = sp5f0.f[1] * scale;
		skyvertices3d[3].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[2].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[3].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp570);
		break;
	case CORNERSTATE_BOTTOM:
		numvertices = 4;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp62c.f[0] * scale;
		skyvertices3d[1].y = sp62c.f[1] * scale;
		skyvertices3d[1].z = sp62c.f[2] * scale;
		skyvertices3d[2].x = sp5f0.f[0] * scale;
		skyvertices3d[2].y = sp5f0.f[1] * scale;
		skyvertices3d[2].z = sp5f0.f[2] * scale;
		skyvertices3d[3].x = sp5fc.f[0] * scale;
		skyvertices3d[3].y = sp5fc.f[1] * scale;
		skyvertices3d[3].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp62c.f[0] * 0.1f;
		skyvertices3d[1].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[2].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[3].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp574);
		break;
	case CORNERSTATE_RIGHT:
		numvertices = 4;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp620.f[0] * scale;
		skyvertices3d[1].y = sp620.f[1] * scale;
		skyvertices3d[1].z = sp620.f[2] * scale;
		skyvertices3d[2].x = sp614.f[0] * scale;
		skyvertices3d[2].y = sp614.f[1] * scale;
		skyvertices3d[2].z = sp614.f[2] * scale;
		skyvertices3d[3].x = sp608.f[0] * scale;
		skyvertices3d[3].y = sp608.f[1] * scale;
		skyvertices3d[3].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp620.f[0] * 0.1f;
		skyvertices3d[1].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp614.f[0] * 0.1f;
		skyvertices3d[2].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp608.f[0] * 0.1f;
		skyvertices3d[3].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp578);
		break;
	case CORNERSTATE_LEFT:
		numvertices = 4;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp644.f[0] * scale;
		skyvertices3d[1].y = sp644.f[1] * scale;
		skyvertices3d[1].z = sp644.f[2] * scale;
		skyvertices3d[2].x = sp608.f[0] * scale;
		skyvertices3d[2].y = sp608.f[1] * scale;
		skyvertices3d[2].z = sp608.f[2] * scale;
		skyvertices3d[3].x = sp614.f[0] * scale;
		skyvertices3d[3].y = sp614.f[1] * scale;
		skyvertices3d[3].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp644.f[0] * 0.1f;
		skyvertices3d[1].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp608.f[0] * 0.1f;
		skyvertices3d[2].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp614.f[0] * 0.1f;
		skyvertices3d[3].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp57c);
		break;
	case CORNERSTATE_BR:
		numvertices = 3;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp608.f[0] * scale;
		skyvertices3d[1].y = sp608.f[1] * scale;
		skyvertices3d[1].z = sp608.f[2] * scale;
		skyvertices3d[2].x = sp5f0.f[0] * scale;
		skyvertices3d[2].y = sp5f0.f[1] * scale;
		skyvertices3d[2].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp608.f[0] * 0.1f;
		skyvertices3d[1].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[2].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp570);
		break;
	case CORNERSTATE_BL:
		numvertices = 3;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp5fc.f[0] * scale;
		skyvertices3d[1].y = sp5fc.f[1] * scale;
		skyvertices3d[1].z = sp5fc.f[2] * scale;
		skyvertices3d[2].x = sp608.f[0] * scale;
		skyvertices3d[2].y = sp608.f[1] * scale;
		skyvertices3d[2].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[1].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp608.f[0] * 0.1f;
		skyvertices3d[2].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp578);
		break;
	case CORNERSTATE_TR:
		numvertices = 3;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp5f0.f[0] * scale;
		skyvertices3d[1].y = sp5f0.f[1] * scale;
		skyvertices3d[1].z = sp5f0.f[2] * scale;
		skyvertices3d[2].x = sp614.f[0] * scale;
		skyvertices3d[2].y = sp614.f[1] * scale;
		skyvertices3d[2].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[1].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp614.f[0] * 0.1f;
		skyvertices3d[2].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp57c);
		break;
	case CORNERSTATE_TL:
		numvertices = 3;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp614.f[0] * scale;
		skyvertices3d[1].y = sp614.f[1] * scale;
		skyvertices3d[1].z = sp614.f[2] * scale;
		skyvertices3d[2].x = sp5fc.f[0] * scale;
		skyvertices3d[2].y = sp5fc.f[1] * scale;
		skyvertices3d[2].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp614.f[0] * 0.1f;
		skyvertices3d[1].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[2].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp574);
		break;
	case CORNERSTATE_TL_TR_BL:
		numvertices = 5;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp644.f[0] * scale;
		skyvertices3d[1].y = sp644.f[1] * scale;
		skyvertices3d[1].z = sp644.f[2] * scale;
		skyvertices3d[2].x = sp638.f[0] * scale;
		skyvertices3d[2].y = sp638.f[1] * scale;
		skyvertices3d[2].z = sp638.f[2] * scale;
		skyvertices3d[3].x = sp5f0.f[0] * scale;
		skyvertices3d[3].y = sp5f0.f[1] * scale;
		skyvertices3d[3].z = sp5f0.f[2] * scale;
		skyvertices3d[4].x = sp608.f[0] * scale;
		skyvertices3d[4].y = sp608.f[1] * scale;
		skyvertices3d[4].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp644.f[0] * 0.1f;
		skyvertices3d[1].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp638.f[0] * 0.1f;
		skyvertices3d[2].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[3].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp608.f[0] * 0.1f;
		skyvertices3d[4].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp578);
		break;
	case CORNERSTATE_TL_TR_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp620.f[0] * scale;
		skyvertices3d[2].y = sp620.f[1] * scale;
		skyvertices3d[2].z = sp620.f[2] * scale;
		skyvertices3d[3].x = sp608.f[0] * scale;
		skyvertices3d[3].y = sp608.f[1] * scale;
		skyvertices3d[3].z = sp608.f[2] * scale;
		skyvertices3d[4].x = sp5fc.f[0] * scale;
		skyvertices3d[4].y = sp5fc.f[1] * scale;
		skyvertices3d[4].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp620.f[0] * 0.1f;
		skyvertices3d[2].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp608.f[0] * 0.1f;
		skyvertices3d[3].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[4].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp574);
		break;
	case CORNERSTATE_TL_BL_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp62c.f[0] * scale;
		skyvertices3d[1].y = sp62c.f[1] * scale;
		skyvertices3d[1].z = sp62c.f[2] * scale;
		skyvertices3d[2].x = sp644.f[0] * scale;
		skyvertices3d[2].y = sp644.f[1] * scale;
		skyvertices3d[2].z = sp644.f[2] * scale;
		skyvertices3d[3].x = sp614.f[0] * scale;
		skyvertices3d[3].y = sp614.f[1] * scale;
		skyvertices3d[3].z = sp614.f[2] * scale;
		skyvertices3d[4].x = sp5f0.f[0] * scale;
		skyvertices3d[4].y = sp5f0.f[1] * scale;
		skyvertices3d[4].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp62c.f[0] * 0.1f;
		skyvertices3d[1].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp644.f[0] * 0.1f;
		skyvertices3d[2].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp614.f[0] * 0.1f;
		skyvertices3d[3].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[4].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp570);
		break;
	case CORNERSTATE_TR_BL_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp620.f[0] * scale;
		skyvertices3d[1].y = sp620.f[1] * scale;
		skyvertices3d[1].z = sp620.f[2] * scale;
		skyvertices3d[2].x = sp62c.f[0] * scale;
		skyvertices3d[2].y = sp62c.f[1] * scale;
		skyvertices3d[2].z = sp62c.f[2] * scale;
		skyvertices3d[3].x = sp5fc.f[0] * scale;
		skyvertices3d[3].y = sp5fc.f[1] * scale;
		skyvertices3d[3].z = sp5fc.f[2] * scale;
		skyvertices3d[4].x = sp614.f[0] * scale;
		skyvertices3d[4].y = sp614.f[1] * scale;
		skyvertices3d[4].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp620.f[0] * 0.1f;
		skyvertices3d[1].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp62c.f[0] * 0.1f;
		skyvertices3d[2].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[3].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp614.f[0] * 0.1f;
		skyvertices3d[4].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp57c);
		break;
	default:
		return gdl;
	}

	gDPPipeSync(gdl++);

	texSelect(&gdl, &g_TexSkyWaterConfigs[env->clouds_type], 1, 0, 2, 1, NULL);

	gDPSetEnvColor(gdl++, env->sky_r, env->sky_g, env->sky_b, 0xff);
	gDPSetCombineLERP(gdl++,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE);

	if (1) {
		s32 stack;
		Mtxf sp1ec;
		Mtxf sp1ac;
		struct skyvtx2d skyvertices2d[5];
		s32 i;

		mtx4MultMtx4(camGetMtxF1754(), camGetWorldToScreenMtxf(), &sp1ec);
		guScaleF(g_SkyMtx.m, 1.0f / scale, 1.0f / scale, 1.0f / scale);
		mtx4MultMtx4(&sp1ec, &g_SkyMtx, &sp1ac);

		for (i = 0; i < numvertices; i++) {
			skyConvertVertex(&skyvertices3d[i], &sp1ac, 130, 65535.0f, 65535.0f, &skyvertices2d[i]);

			skyvertices2d[i].x = skyClamp(skyvertices2d[i].x, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
			skyvertices2d[i].y = skyClamp(skyvertices2d[i].y, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);
		}

#ifdef PLATFORM_N64
		if (numvertices == 4) {
			if (cornerstate == CORNERSTATE_TOP) {
				if (sp548 < sp54c) {
					if (skyvertices2d[3].y >= skyvertices2d[1].y + 4.0f) {
						skyvertices2d[0].x = camGetScreenLeft() * 4.0f;
						skyvertices2d[0].y = camGetScreenTop() * 4.0f;
						skyvertices2d[1].x = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;
						skyvertices2d[1].y = camGetScreenTop() * 4.0f;
						skyvertices2d[2].x = camGetScreenLeft() * 4.0f;
						skyvertices2d[3].x = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;

						gdl = skyRenderFull(gdl, &skyvertices2d[0], &skyvertices2d[1], &skyvertices2d[2], &skyvertices2d[3], 130.0f);
					} else {
						gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[1], &skyvertices2d[2], 130.0f, true);
					}
				} else if (skyvertices2d[2].y >= skyvertices2d[0].y + 4.0f) {
					skyvertices2d[0].x = camGetScreenLeft() * 4.0f;
					skyvertices2d[0].y = camGetScreenTop() * 4.0f;
					skyvertices2d[1].x = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;
					skyvertices2d[1].y = camGetScreenTop() * 4.0f;
					skyvertices2d[2].x = camGetScreenLeft() * 4.0f;
					skyvertices2d[3].x = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;

					gdl = skyRenderFull(gdl, &skyvertices2d[1], &skyvertices2d[0], &skyvertices2d[3], &skyvertices2d[2], 130.0f);
				} else {
					gdl = skyRenderTri(gdl, &skyvertices2d[1], &skyvertices2d[0], &skyvertices2d[3], 130.0f, true);
				}
			} else {
				gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[1], &skyvertices2d[3], 130.0f, true);
				gdl = skyRenderTri(gdl, &skyvertices2d[3], &skyvertices2d[2], &skyvertices2d[0], 130.0f, true);
			}
		} else if (numvertices == 5) {
			// Three corners are in the sky
			gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[1], &skyvertices2d[2], 130.0f, true);
			gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[2], &skyvertices2d[3], 130.0f, true);
			gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[3], &skyvertices2d[4], 130.0f, true);
		} else if (numvertices == 3) {
			// One corner is in the sky
			gdl = skyRenderTri(gdl, &skyvertices2d[0], &skyvertices2d[1], &skyvertices2d[2], 130.0f, true);
		}
#else
		Vtx *verts = gfxAllocateVertices(numvertices);
		Col *cols = gfxAllocateColours(numvertices);
		Mtxf *mtx = gfxAllocateMatrix();
		mtx4MultMtx4(camGetWorldToScreenMtxf(), &g_SkyMtx, mtx);
		mtxF2L(mtx, mtx);

		gSPSetExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
		gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
		gSPColor(gdl++, osVirtualToPhysical(cols), numvertices);
		gSPVertex(gdl++, osVirtualToPhysical(verts), numvertices, 0);

		for (s32 i = 0; i < numvertices; ++i) {
			verts[i].x = skyvertices3d[i].x;
			verts[i].y = skyvertices3d[i].y;
			verts[i].z = skyvertices3d[i].z;
			verts[i].s = skyClamp(skyvertices3d[i].s, -32768.f, 32767.f);
			verts[i].t = skyClamp(skyvertices3d[i].t, -32768.f, 32767.f);
			verts[i].colour = i * 4;
			cols[i].r = skyvertices3d[i].r;
			cols[i].g = skyvertices3d[i].g;
			cols[i].b = skyvertices3d[i].b;
			cols[i].a = skyvertices3d[i].a;
		}

		if (numvertices == 4) {
			gSPTri2(gdl++, 0, 1, 3, 3, 2, 0);
		} else if (numvertices == 5) {
			gSPTri3(gdl++, 0, 1, 2, 0, 2, 3, 0, 3, 4);
		} else if (numvertices == 3) {
			gSPTri1(gdl++, 0, 1, 2);
		}

		gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
		gSPClearExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
#endif
	}

	return gdl;
}

/**
 * Convert a 3D vertex to 2D.
 */
void skyConvertVertex(struct skyvtx3d *srcvtx, Mtxf *mtx, u16 arg2, f32 arg3, f32 arg4, struct skyvtx2d *dstvtx)
{
	f32 sp68[4];
	f32 t;
	f32 s;
	f32 f22;
	f32 f0;
	f32 sp48[4];
	f32 sp38[4];
	f32 sp34;
	f32 sp30;
	f32 mult;

	mult = arg2 / 65536.0f;

	sp68[0] = (srcvtx->x * mtx->m[0][0] + srcvtx->y * mtx->m[1][0] + srcvtx->z * mtx->m[2][0]) + mtx->m[3][0];
	sp68[1] = (srcvtx->x * mtx->m[0][1] + srcvtx->y * mtx->m[1][1] + srcvtx->z * mtx->m[2][1]) + mtx->m[3][1];
	sp68[2] = (srcvtx->x * mtx->m[0][2] + srcvtx->y * mtx->m[1][2] + srcvtx->z * mtx->m[2][2]) + mtx->m[3][2];
	sp68[3] = (srcvtx->x * mtx->m[0][3] + srcvtx->y * mtx->m[1][3] + srcvtx->z * mtx->m[2][3]) + mtx->m[3][3];

	s = srcvtx->s * (arg3 * (1.0f / 65536.0f));
	t = srcvtx->t * (arg4 * (1.0f / 65536.0f));

	if (sp68[3] == 0.0f) {
		f22 = 32767.0f;
	} else {
		f22 = 1.0f / (sp68[3] * mult);
	}

	f0 = f22;

	if (f0 < 0.0f) {
		f0 = 32767.0f;
	}

	sp48[0] = sp68[0] * f0 * mult;
	sp48[1] = sp68[1] * f0 * mult;
	sp48[2] = sp68[2] * f0 * mult;
	sp48[3] = sp68[3] * f0 * mult;

	sp34 = camGetScreenWidth();
	sp30 = camGetScreenWidth();
	sp38[0] = sp48[0] * (sp34 + sp34) + (sp30 + sp30 + camGetScreenLeft() * 4);

	sp34 = camGetScreenHeight();
	sp30 = camGetScreenHeight();
	sp38[1] = -sp48[1] * (sp34 + sp34) + (sp30 + sp30 + camGetScreenTop() * 4);

	sp34 = 511.0f;
	sp30 = 511.0f;
	sp38[2] = sp48[2] * sp34 + sp30;

	sp34 = 0;
	sp30 = 0;
	sp38[3] = sp48[3] * sp34 + sp30;

	sp38[0] = skyClamp(sp38[0], -4090.0f, 4090.0f);
	sp38[1] = skyClamp(sp38[1], -4090.0f, 4090.0f);
	sp38[2] = skyClamp(sp38[2], 0.0f, 32767.0f);
	sp38[3] = skyClamp(sp38[3], 0.0f, 32767.0f);

	dstvtx->unk00 = sp68[0];
	dstvtx->unk04 = sp68[1];
	dstvtx->unk08 = sp68[2];
	dstvtx->unk0c = sp68[3];
	dstvtx->s = s;
	dstvtx->t = t;
	dstvtx->x = sp38[0];
	dstvtx->y = sp38[1] - envGetCurrent()->clouds_height * 4.0f;
	dstvtx->unk30 = sp38[2];
	dstvtx->unk34 = f22;

	dstvtx->r = srcvtx->r;
	dstvtx->g = srcvtx->g;
	dstvtx->b = srcvtx->b;
	dstvtx->a = srcvtx->a;
}

bool skyVerticesAreSame(struct skyvtx2d *vtx0, struct skyvtx2d *vtx1)
{
	f32 xdiff = vtx0->x - vtx1->x;
	f32 ydiff = vtx0->y - vtx1->y;

	return sqrtf(xdiff * xdiff + ydiff * ydiff) < 1.0f ? true : false;
}

Gfx *skyRenderTri(Gfx *gdl, struct skyvtx2d *vtx0, struct skyvtx2d *vtx1, struct skyvtx2d *vtx2, f32 arg4, bool textured)
{
	struct skyvtx2d *svtx0;
	struct skyvtx2d *svtx1;
	struct skyvtx2d *svtx2;
	s32 i;
	f32 xdiff1;
	f32 ydiff1;
	f32 xdiff2;
	f32 ydiff2;
	f32 xdiff3;
	f32 ydiff3;

	f32 svtx0y[1];
	f32 svtx0x[1];
	f32 svtx1y[1];
	f32 svtx1x[1];
	f32 svtx2y[1];
	f32 svtx2x[1];

	f32 sp444;
	f32 sp440;

	f32 sp43c[1];
	f32 sp438[1];
	f32 sp434[1];
	f32 sp430[1];
	f32 sp42c[1];
	f32 sp428[1];
	f32 sp424[1];
	f32 sp420[1];

	f32 sp41c;
	f32 sp418;
	f32 sp414;
	f32 sp410;
	f32 sp40c;
	f32 sp408;
	f32 sp404;
	f32 sp400;

	f32 sp3fc[1];
	f32 sp3f8[1];
	f32 sp3f4[1];
	f32 sp3f0[1];
	f32 sp3ec[1];
	f32 sp3e8[1];
	f32 sp3e4[1];
	f32 sp3e0[1];

	f32 sp3dc[1];
	f32 sp3d8[1];
	f32 sp3d4[1];
	f32 sp3d0[1];
	f32 sp3cc[1];
	f32 sp3c8[1];
	f32 sp3c4[1];
	f32 sp3c0[1];

	f32 sp3bc[1];
	f32 sp3b8[1];
	f32 sp3b4[1];
	f32 sp3b0[1];
	f32 sp3ac[1];
	f32 sp3a8[1];
	f32 sp3a4[1];
	f32 sp3a0[1];
	f32 sp39c[1];
	f32 sp398[1];
	f32 sp394[1];
	f32 sp390[1];
	f32 sp38c[1];
	f32 sp388[1];
	f32 sp384[1];
	f32 sp380[1];
	f32 sp37c;
	f32 sp378;
	f32 sp374[1];
	f32 sp370[1];
	f32 sp36c[1];
	f32 sp368;
	f32 sp364[1];
	f32 sp360[1];
	f32 sp35c[1];
	f32 sp358[1];
	f32 sp354[1];
	f32 sp350[1];
	f32 sp34c[1];
	f32 sp348[1];
	f32 sp344[1];
	f32 sp340[1];
	f32 sp33c[1];
	f32 sp338[1];
	f32 sp334[1];
	f32 sp330[1];
	f32 sp310[8];
	f32 sp2f0[8];
	f32 sp2d0[8];
	f32 sp2b0[8];
	f32 sp290[8];
	f32 sp270[8];
	f32 sp250[8];
	f32 sp230[8];
	f32 sp210[8];
	f32 f2;
	f32 sp208[1];
	f32 sp204[1];
	f32 sp200[1];
	u32 stack[4];
	f32 sp1d0[8];
	f32 sp1b0[8];
	u32 stack2;
	f32 sp1a8[1];
	f32 sp1a4[1];
	f32 sp1a0[1];
	struct skyvtx2d *swap1;
	struct skyvtx2d *swap2;
	struct skyvtx2d *swap3;
	f32 sp190[1];
	u32 stack3;

	if (skyVerticesAreSame(vtx0, vtx1) || skyVerticesAreSame(vtx1, vtx2) || skyVerticesAreSame(vtx2, vtx0)) {
		return gdl;
	}

	sp378 = arg4 / 65536.0f;

	xdiff1 = vtx1->x - vtx0->x;
	ydiff1 = vtx1->y - vtx0->y;
	xdiff2 = vtx2->x - vtx0->x;
	ydiff2 = vtx2->y - vtx0->y;

	sp444 = ((xdiff2 * ydiff1) - (xdiff1 * ydiff2)) * (1.0f / 65536.0f);

	if (sp444 == 0.0f) {
		return gdl;
	}

	sp440 = 1.0f / sp444;

	// svtx = sorted vertex
	svtx0 = vtx0;
	svtx1 = vtx1;
	svtx2 = vtx2;

	if (svtx1->y < svtx0->y) {
		swap1 = svtx1;
		svtx1 = svtx0;
		svtx0 = swap1;

		sp444 *= -1.0f;
		sp440 *= -1.0f;
	}

	if (svtx2->y < svtx1->y) {
		swap2 = svtx1;
		svtx1 = svtx2;
		svtx2 = swap2;

		sp444 *= -1.0f;
		sp440 *= -1.0f;
	}

	if (svtx1->y < svtx0->y) {
		swap3 = svtx1;
		svtx1 = svtx0;
		svtx0 = swap3;

		sp444 *= -1.0f;
		sp440 *= -1.0f;
	}

	sp420[0] = svtx1->x * 0.25f;
	sp424[0] = 0.0f;
	sp428[0] = svtx0->x * 0.25f;
	sp42c[0] = 0.0f;
	sp430[0] = svtx0->x * 0.25f;
	sp434[0] = 0.0f;
	sp43c[0] = 0.0f;
	sp438[0] = 0.0f;

	svtx2x[0] = svtx2->x;
	svtx2y[0] = svtx2->y;
	svtx1x[0] = svtx1->x;
	svtx1y[0] = svtx1->y;
	svtx0x[0] = svtx0->x;
	svtx0y[0] = svtx0->y;

	xdiff1 = svtx1x[0] - svtx0x[0];
	ydiff1 = svtx1y[0] - svtx0y[0];
	xdiff2 = svtx2x[0] - svtx0x[0];
	ydiff2 = svtx2y[0] - svtx0y[0];
	xdiff3 = svtx2x[0] - svtx1x[0];
	ydiff3 = svtx2y[0] - svtx1y[0];

	sp3fc[0] = 0.0f;
	sp3f8[0] = 0.0f;
	sp3dc[0] = 0.0f;
	sp3d8[0] = 0.0f;
	sp3a0[0] = 0.0f;
	sp3a8[0] = 0.0f;
	sp3b0[0] = 0.0f;
	sp3bc[0] = 0.0f;
	sp3b8[0] = 0.0f;
	sp380[0] = 0.0f;
	sp388[0] = 0.0f;
	sp390[0] = 0.0f;
	sp39c[0] = 0.0f;
	sp398[0] = 0.0f;

	sp3e0[0] = xdiff3 / 4.0f;
	sp3e4[0] = ydiff3 / 4.0f;
	sp3e8[0] = xdiff1 / 4.0f;
	sp3ec[0] = ydiff1 / 4.0f;
	sp3f0[0] = xdiff2 / 4.0f;
	sp3f4[0] = ydiff2 / 4.0f;

	sp3c0[0] = xdiff3 * 4.0f;
	sp3c4[0] = ydiff3 * 4.0f;
	sp3c8[0] = xdiff1 * 4.0f;
	sp3cc[0] = ydiff1 * 4.0f;
	sp3d0[0] = xdiff2 * 4.0f;
	sp3d4[0] = ydiff2 * 4.0f;

	sp3a4[0] = 4.0f / ydiff3;
	sp3ac[0] = 4.0f / ydiff1;
	sp3b4[0] = 4.0f / ydiff2;

	sp384[0] = xdiff3 / ydiff3;
	sp38c[0] = xdiff1 / ydiff1;
	sp394[0] = xdiff2 / ydiff2;

	sp384[0] = skyClamp(sp384[0], -1878.0f, 1877.0f);
	sp38c[0] = skyClamp(sp38c[0], -1878.0f, 1877.0f);
	sp394[0] = skyClamp(sp394[0], -1878.0f, 1877.0f);

	f2 = (svtx0->y * 0.25f);
	sp37c = f2 - (s32) f2;
	sp408 = sp428[0] - skyRound(sp38c[0] * 8192.0f) * (1.0f / 8192.0f) * sp37c;
	sp410 = sp430[0] - skyRound(sp394[0] * 8192.0f) * (1.0f / 8192.0f) * sp37c;

	gImmp1(gdl++, G_RDPHALF_1, (textured ? (G_TRI_SHADE_TXTR << 24) : (G_TRI_FILL << 24))
			| (sp444 < 0.0f ? 0x00800000 : 0)
			| (u32) svtx2->y);
	gImmp1(gdl++, G_RDPHALF_CONT, (s32) svtx1->y << 16 | (s32) svtx0->y);

	gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(svtx1->x * 0.25f));
	gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(sp384[0]));

	gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(sp410));
	gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(sp394[0]));

	gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(sp408));
	gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(sp38c[0]));

	if (!textured) {
		return gdl;
	}

	sp36c[0] = svtx0->unk0c * sp378;
	sp370[0] = svtx1->unk0c * sp378;
	sp374[0] = svtx2->unk0c * sp378;

	sp368 = sp36c[0];

	if (sp370[0] < sp368) {
		sp368 = sp370[0];
	}

	if (sp374[0] < sp368) {
		sp368 = sp374[0];
	}

	sp368 *= 0.5f;

	sp35c[0] = svtx0->unk34 * sp368;
	sp360[0] = svtx1->unk34 * sp368;
	sp364[0] = svtx2->unk34 * sp368;

	sp338[0] = sp35c[0] * svtx0->s;
	sp33c[0] = sp35c[0] * svtx0->t;
	sp340[0] = sp35c[0] * 32767.0f;
	sp344[0] = sp360[0] * svtx1->s;
	sp348[0] = sp360[0] * svtx1->t;
	sp34c[0] = sp360[0] * 32767.0f;
	sp350[0] = sp364[0] * svtx2->s;
	sp354[0] = sp364[0] * svtx2->t;
	sp358[0] = sp364[0] * 32767.0f;

	sp330[0] = SKYABS(sp338[0]);
	sp334[0] = SKYABS(sp33c[0]);

	if (sp330[0] < SKYABS(sp344[0])) {
		sp330[0] = SKYABS(sp344[0]);
	}

	if (sp334[0] < SKYABS(sp348[0])) {
		sp334[0] = SKYABS(sp348[0]);
	}

	if (sp330[0] < SKYABS(sp350[0])) {
		sp330[0] = SKYABS(sp350[0]);
	}

	if (sp334[0] < SKYABS(sp354[0])) {
		sp334[0] = SKYABS(sp354[0]);
	}

	sp310[0] = svtx0->r + 0.5f;
	sp310[1] = svtx0->g + 0.5f;
	sp310[2] = svtx0->b + 0.5f;
	sp310[3] = svtx0->a + 0.5f;

	sp2f0[0] = svtx1->r + 0.5f;
	sp2f0[1] = svtx1->g + 0.5f;
	sp2f0[2] = svtx1->b + 0.5f;
	sp2f0[3] = svtx1->a + 0.5f;

	sp2d0[0] = svtx2->r + 0.5f;
	sp2d0[1] = svtx2->g + 0.5f;
	sp2d0[2] = svtx2->b + 0.5f;
	sp2d0[3] = svtx2->a + 0.5f;

	sp310[4] = sp338[0]; sp310[5] = sp33c[0]; sp310[6] = sp340[0];
	sp2f0[4] = sp344[0]; sp2f0[5] = sp348[0]; sp2f0[6] = sp34c[0];
	sp2d0[4] = sp350[0]; sp2d0[5] = sp354[0]; sp2d0[6] = sp358[0];

	sp310[7] = svtx0->unk30;
	sp2f0[7] = svtx1->unk30;
	sp2d0[7] = svtx2->unk30;

	for (i = 0; i < 8; i++) {
		sp2b0[i] = sp2f0[i] - sp310[i];
		sp290[i] = sp2d0[i] - sp310[i];
	}

	for (i = 0; i < 8; i++) {
		sp250[i] = (sp290[i] * sp3cc[0] - sp2b0[i] * sp3d4[0]) * (1.0f / 65536.0f);
		sp270[i] = (sp2b0[i] * sp3d0[0] - sp290[i] * sp3c8[0]) * (1.0f / 65536.0f);
		sp290[i] = sp250[i] * sp440;
		sp2b0[i] = sp270[i] * sp440;
		sp230[i] = sp2b0[i] + sp290[i] * sp394[0];
		sp210[i] = sp310[i] - sp230[i] * sp37c;
	}

	{
		u32 sp168;
		u32 sp164;
		u32 sp160;
		u32 sp15c;
		u32 sp158;
		u32 sp154;
		u32 sp150;
		u32 sp14c;
		u32 sp148;
		u32 sp144;
		u32 sp140;
		u32 sp13c;
		u32 sp138;
		u32 sp134;
		u32 sp130;
		u32 sp12c;

		sp168 = func0f152fa0(sp210[0]);
		sp164 = func0f152fa0(sp210[1]);
		sp160 = func0f152fa0(sp210[2]);
		sp15c = func0f152fa0(sp210[3]);

		sp158 = func0f152fa0(sp290[0]);
		sp154 = func0f152fa0(sp290[1]);
		sp150 = func0f152fa0(sp290[2]);
		sp14c = func0f152fa0(sp290[3]);

		sp138 = func0f152fa0(sp2b0[0]);
		sp134 = func0f152fa0(sp2b0[1]);
		sp130 = func0f152fa0(sp2b0[2]);
		sp12c = func0f152fa0(sp2b0[3]);

		sp148 = func0f152fa0(sp230[0]);
		sp144 = func0f152fa0(sp230[1]);
		sp140 = func0f152fa0(sp230[2]);
		sp13c = func0f152fa0(sp230[3]);

		gImmp1(gdl++, G_RDPHALF_1, (sp168 & 0xffff0000) | (sp164 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp160 & 0xffff0000) | (sp15c & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp158 & 0xffff0000) | (sp154 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp150 & 0xffff0000) | (sp14c & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp168 & 0x0000ffff) << 16 | (sp164 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp160 & 0x0000ffff) << 16 | (sp15c & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp158 & 0x0000ffff) << 16 | (sp154 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp150 & 0x0000ffff) << 16 | (sp14c & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp148 & 0xffff0000) | (sp144 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp140 & 0xffff0000) | (sp13c & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp138 & 0xffff0000) | (sp134 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp130 & 0xffff0000) | (sp12c & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp148 & 0x0000ffff) << 16 | (sp144 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp140 & 0x0000ffff) << 16 | (sp13c & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp138 & 0x0000ffff) << 16 | (sp134 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp130 & 0x0000ffff) << 16 | (sp12c & 0x0000ffff));
	}

	sp200[0] = sp330[0] * (1.0f / 32.0f);
	sp204[0] = sp334[0] * (1.0f / 32.0f);
	sp208[0] = sp368 * (1.0f / 32.0f);

	for (i = 0; i < 8; i++) {
		sp1d0[i] = SKYABS(sp290[i]) * (1.0f / 32.0f);
		sp1b0[i] = SKYABS(sp2b0[i]) * (1.0f / 32.0f);
	}

	sp1a0[0] = sp200[0] + (2.0f * sp1d0[4]) + sp1b0[4];
	sp1a4[0] = sp204[0] + (2.0f * sp1d0[5]) + sp1b0[5];
	sp1a8[0] = sp208[0] + (2.0f * sp1d0[6]) + sp1b0[6];

	if (sp1a0[0] < sp1a4[0]) {
		sp1a0[0] = sp1a4[0];
	}

	if (sp1a0[0] < sp1a8[0]) {
		sp1a0[0] = sp1a8[0];
	}

	sp1a0[0] *= 1.0f / 1024.0f;

	if (sp1a0[0] > 1.0f) {
		sp190[0] = 1.0f / sp1a0[0];
	} else {
		sp190[0] = 1.0f;
	}

	{
		u32 spe8;
		u32 spe4;
		u32 spe0;
		u32 spdc;
		u32 spd8;
		u32 spd4;
		u32 spd0;
		u32 spcc;
		u32 spc8;
		u32 spc4;
		u32 spc0;
		u32 spbc;
		u32 spb8;
		u32 spb4;
		u32 spb0;
		u32 spac;

		spe8 = func0f152fa0(sp210[4] * sp190[0]);
		spe4 = func0f152fa0(sp210[5] * sp190[0]);
		spe0 = func0f152fa0(sp210[6] * sp190[0]);
		spdc = 0;

		spd8 = func0f152fa0(sp290[4] * sp190[0]);
		spd4 = func0f152fa0(sp290[5] * sp190[0]);
		spd0 = func0f152fa0(sp290[6] * sp190[0]);
		spcc = 0;

		spb8 = func0f152fa0(sp2b0[4] * sp190[0]);
		spb4 = func0f152fa0(sp2b0[5] * sp190[0]);
		spb0 = func0f152fa0(sp2b0[6] * sp190[0]);
		spac = 0;

		spc8 = func0f152fa0(sp230[4] * sp190[0]);
		spc4 = func0f152fa0(sp230[5] * sp190[0]);
		spc0 = func0f152fa0(sp230[6] * sp190[0]);
		spbc = 0;

		gImmp1(gdl++, G_RDPHALF_1, (spe8 & 0xffff0000) | (spe4 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spe0 & 0xffff0000) | (spdc & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spd8 & 0xffff0000) | (spd4 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spd0 & 0xffff0000) | (spcc & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spe8 & 0x0000ffff) << 16 | (spe4 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spe0 & 0x0000ffff) << 16 | (spdc & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spd8 & 0x0000ffff) << 16 | (spd4 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spd0 & 0x0000ffff) << 16 | (spcc & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spc8 & 0xffff0000) | (spc4 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spc0 & 0xffff0000) | (spbc & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spb8 & 0xffff0000) | (spb4 & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spb0 & 0xffff0000) | (spac & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spc8 & 0x0000ffff) << 16 | (spc4 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spc0 & 0x0000ffff) << 16 | (spbc & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spb8 & 0x0000ffff) << 16 | (spb4 & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_2, (spb0 & 0x0000ffff) << 16 | (spac & 0x0000ffff));
	}

	return gdl;
}

/**
 * Render the sky when all four corners of the viewport are above the horizon.
 * ie. The sky takes up the full screen.
 */
Gfx *skyRenderFull(Gfx *gdl, struct skyvtx2d *vtx0, struct skyvtx2d *vtx1, struct skyvtx2d *vtx2, struct skyvtx2d *vtx3, f32 arg5)
{
	struct skyvtx2d *sp4cc;
	struct skyvtx2d *sp4c8;
	struct skyvtx2d *sp4c4;
	s32 i;
	u32 stack;
	f32 sp4b8;
	f32 sp4b4;
	f32 sp4b0;
	f32 sp4ac;
	f32 sp4a8;
	f32 sp4a4;
	f32 sp4a0[1];
	f32 sp49c[1];
	f32 sp498[1];
	f32 sp494[1];
	f32 sp490[1];
	f32 sp48c[1];
	f32 sp488;
	f32 sp484;
	f32 sp480[1];
	f32 sp47c[1];
	f32 sp478[1];
	f32 sp474[1];
	f32 sp470[1];
	f32 sp46c[1];
	f32 sp468[1];
	f32 sp464[1];
	struct skyvtx2d *swap1;
	struct skyvtx2d *swap2;
	struct skyvtx2d *swap3;
	f32 sp454[1];
	u32 stack07;
	f32 svtx2y[1];
	u32 stack08;
	u32 stack09;
	f32 sp440[1];
	f32 sp43c[1];
	f32 sp438[1];
	f32 sp434[1];
	f32 sp430[1];
	f32 sp42c[1];
	f32 sp428[1];
	f32 sp424[1];
	f32 sp420[1];
	f32 sp41c[1];
	f32 sp418[1];
	f32 sp414[1];
	f32 sp410[1];
	f32 sp40c[1];
	f32 sp408[1];
	f32 sp404[1];
	f32 sp400[1];
	f32 sp3fc[1];
	f32 sp3f8[1];
	f32 sp3f4[1];
	f32 sp3f0[1];
	f32 sp3ec[1];
	f32 sp3e8[1];
	f32 sp3e4[1];
	f32 sp3e0[1];
	f32 sp3dc[1];
	f32 sp3d8[1];
	f32 sp3d4[1];
	f32 sp3d0[1];
	f32 sp3cc[1];
	f32 sp3c8[1];
	f32 sp3c4[1];
	f32 sp3c0;
	f32 sp3bc[1];
	f32 sp3b8[1];
	f32 sp3b4[1];
	f32 sp3b0[1];
	f32 sp3ac;
	f32 sp3a8[1];
	f32 sp3a4[1];
	f32 sp3a0[1];
	f32 sp39c[1];
	f32 sp398[1];
	f32 sp394[1];
	f32 sp390[1];
	f32 sp38c[1];
	f32 sp388[1];
	f32 sp384[1];
	f32 sp380[1];
	f32 sp37c[1];
	f32 sp378[1];
	f32 sp374[1];
	f32 sp370[1];
	f32 sp36c[1];
	f32 sp368[1];
	f32 sp364[1];
	f32 sp354[4];
	f32 sp334[8];
	f32 sp314[8];
	f32 sp2f4[8];
	f32 sp2d4[8];
	f32 sp2b4[8];
	f32 sp294[8];
	f32 sp274[8];
	f32 sp254[8];
	u32 stack10;
	u32 stack11;
	u32 stack12;
	u32 stack13;
	u32 stack00;
	f32 sp23c[1];
	f32 sp238[1];
	f32 sp234[1];
	f32 sp214[8];
	f32 sp1f4[8];
	u32 stack03;
	u32 stack04;
	u32 stack05;
	u32 stack06;
	u32 stack15;
	f32 sp1dc[1];
	f32 sp1d8[1];
	f32 sp1d4[1];
	u32 stack01;
	u32 stack02;
	u32 stack14;
	f32 sp1c4[1];
	u32 stack16;

	if (skyVerticesAreSame(vtx0, vtx1)
			|| skyVerticesAreSame(vtx1, vtx2)
			|| skyVerticesAreSame(vtx2, vtx0)
			|| skyVerticesAreSame(vtx3, vtx0)
			|| skyVerticesAreSame(vtx3, vtx1)
			|| skyVerticesAreSame(vtx3, vtx2)) {
		return gdl;
	}

	sp3c0 = arg5 * (1.0f / 65536.0f);

	sp4b8 = vtx1->x - vtx0->x;
	sp4b4 = vtx1->y - vtx0->y;
	sp4b0 = vtx2->x - vtx0->x;
	sp4ac = vtx2->y - vtx0->y;

	sp488 = ((sp4b0 * sp4b4) - (sp4b8 * sp4ac)) * (1.0f / 65536.0f);

	sp484 = 1.0f / sp488;

	sp4cc = vtx0;
	sp4c8 = vtx1;
	sp4c4 = vtx2;

	if (sp4c8->y < sp4cc->y) {
		swap1 = sp4c8;
		sp4c8 = sp4cc;
		sp4cc = swap1;

		sp488 *= -1.0f;
		sp484 *= -1.0f;
	}

	if (sp4c4->y < sp4c8->y) {
		swap2 = sp4c8;
		sp4c8 = sp4c4;
		sp4c4 = swap2;

		sp488 *= -1.0f;
		sp484 *= -1.0f;
	}

	if (sp4c8->y < sp4cc->y) {
		swap3 = sp4c8;
		sp4c8 = sp4cc;
		sp4cc = swap3;

		sp488 *= -1.0f;
		sp484 *= -1.0f;
	}

	sp464[0] = sp4c8->x * 0.25f;
	sp468[0] = 0.0f;
	sp46c[0] = sp4cc->x * 0.25f;
	sp470[0] = 0.0f;
	sp474[0] = sp4cc->x * 0.25f;
	sp478[0] = 0.0f;
	sp480[0] = 0.0f;
	sp47c[0] = 0.0f;

	sp48c[0] = sp4c4->x;
	sp490[0] = sp4c4->y;
	sp494[0] = sp4c8->x;
	sp498[0] = sp4c8->y;
	sp49c[0] = sp4cc->x;
	sp4a0[0] = sp4cc->y;

	sp4b8 = sp494[0] - sp49c[0];
	sp4b4 = sp498[0] - sp4a0[0];
	sp4b0 = sp48c[0] - sp49c[0];
	sp4ac = sp490[0] - sp4a0[0];
	sp4a8 = sp48c[0] - sp494[0];
	sp4a4 = sp490[0] - sp498[0];

	sp440[0] = 0.0f;
	sp43c[0] = 0.0f;
	sp420[0] = 0.0f;
	sp41c[0] = 0.0f;
	sp3e4[0] = 0.0f;
	sp3ec[0] = 0.0f;
	sp3f4[0] = 0.0f;
	sp400[0] = 0.0f;
	sp3fc[0] = 0.0f;
	sp3c4[0] = 0.0f;
	sp3cc[0] = 0.0f;
	sp3d4[0] = 0.0f;
	sp3e0[0] = 0.0f;
	sp3dc[0] = 0.0f;

	sp424[0] = sp4a8 / 4.0f;
	sp428[0] = sp4a4 / 4.0f;
	sp42c[0] = sp4b8 / 4.0f;
	sp430[0] = sp4b4 / 4.0f;
	sp434[0] = sp4b0 / 4.0f;
	sp438[0] = sp4ac / 4.0f;

	sp404[0] = sp4a8 * 4.0f;
	sp408[0] = sp4a4 * 4.0f;
	sp40c[0] = sp4b8 * 4.0f;
	sp410[0] = sp4b4 * 4.0f;
	sp414[0] = sp4b0 * 4.0f;
	sp418[0] = sp4ac * 4.0f;

	sp3e8[0] = 4.0f / sp4a4;
	sp3f0[0] = 4.0f / sp4b4;
	sp3f8[0] = 4.0f / sp4ac;

	sp3c8[0] = sp4a8 / sp4a4;
	sp3d0[0] = sp4b8 / sp4b4;
	sp3d8[0] = sp4b0 / sp4ac;

	sp3c8[0] = skyClamp(sp3c8[0], -1878.0f, 1877.0f);
	sp3d0[0] = skyClamp(sp3d0[0], -1878.0f, 1877.0f);
	sp3d8[0] = skyClamp(sp3d8[0], -1878.0f, 1877.0f);

	svtx2y[0] = sp46c[0];
	sp454[0] = sp474[0];

	if (vtx0->x < vtx1->x) {
		f32 sp1bc;

		if (vtx2->y - vtx3->y < 1.0f) {
			sp1bc = -1878.0f;
		} else {
			sp1bc = -(camGetScreenWidth() - 0.25f) / ((vtx2->y - vtx3->y) / 4.0f);
		}

		gImmp1(gdl++, G_RDPHALF_1, (G_TRI_SHADE_TXTR << 24) | 0x00800000 | (u32) vtx2->y);
		gImmp1(gdl++, G_RDPHALF_CONT, (s32) vtx3->y << 16 | (s32) vtx0->y);

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft() + camGetScreenWidth() - 0.25f));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(sp1bc));

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft()));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(0.0f));

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft() + camGetScreenWidth() - 0.25f));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(0.0f));
	} else {
		f32 sp198;

		if (vtx2->y - vtx3->y < 1.0f) {
			sp198 = 1877.0f;
		} else {
			sp198 = (camGetScreenWidth() - 0.25f) / ((vtx2->y - vtx3->y) / 4.0f);
		}

		gImmp1(gdl++, G_RDPHALF_1, 0xce000000 | (u32) vtx2->y);
		gImmp1(gdl++, G_RDPHALF_CONT, (s32) vtx3->y << 16 | (s32) vtx0->y);

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft()));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(sp198));

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft() + camGetScreenWidth() - 0.25f));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(0.0f));

		gImmp1(gdl++, G_RDPHALF_1, func0f152fa0(camGetScreenLeft()));
		gImmp1(gdl++, G_RDPHALF_CONT, func0f152fa0(0.0f));
	}

	sp3b0[0] = sp4cc->unk0c * sp3c0;
	sp3b4[0] = sp4c8->unk0c * sp3c0;
	sp3b8[0] = sp4c4->unk0c * sp3c0;
	sp3bc[0] = vtx3->unk0c * sp3c0;

	sp3ac = sp3b0[0];

	if (sp3b4[0] < sp3ac) {
		sp3ac = sp3b4[0];
	}

	if (sp3b8[0] < sp3ac) {
		sp3ac = sp3b8[0];
	}

	if (sp3bc[0] < sp3ac) {
		sp3ac = sp3bc[0];
	}

	sp3ac *= 0.5f;

	sp39c[0] = sp4cc->unk34 * sp3ac;
	sp3a0[0] = sp4c8->unk34 * sp3ac;
	sp3a4[0] = sp4c4->unk34 * sp3ac;
	sp3a8[0] = vtx3->unk34 * sp3ac;

	sp36c[0] = sp39c[0] * sp4cc->s;
	sp370[0] = sp39c[0] * sp4cc->t;
	sp374[0] = sp39c[0] * 32767.0f;
	sp378[0] = sp3a0[0] * sp4c8->s;
	sp37c[0] = sp3a0[0] * sp4c8->t;
	sp380[0] = sp3a0[0] * 32767.0f;
	sp384[0] = sp3a4[0] * sp4c4->s;
	sp388[0] = sp3a4[0] * sp4c4->t;
	sp38c[0] = sp3a4[0] * 32767.0f;
	sp390[0] = sp3a8[0] * vtx3->s;
	sp394[0] = sp3a8[0] * vtx3->t;
	sp398[0] = sp3a8[0] * 32767.0f;

	sp364[0] = SKYABS(sp36c[0]);
	sp368[0] = SKYABS(sp370[0]);

	if (sp364[0] < SKYABS(sp378[0])) {
		sp364[0] = SKYABS(sp378[0]);
	}

	if (sp368[0] < SKYABS(sp37c[0])) {
		sp368[0] = SKYABS(sp37c[0]);
	}

	if (sp364[0] < SKYABS(sp384[0])) {
		sp364[0] = SKYABS(sp384[0]);
	}

	if (sp368[0] < SKYABS(sp388[0])) {
		sp368[0] = SKYABS(sp388[0]);
	}

	if (sp364[0] < SKYABS(sp390[0])) {
		sp364[0] = SKYABS(sp390[0]);
	}

	if (sp368[0] < SKYABS(sp394[0])) {
		sp368[0] = SKYABS(sp394[0]);
	}

	sp354[0] = sp36c[0]; sp354[1] = sp370[0]; sp354[2] = sp374[0];
	sp334[0] = sp378[0]; sp334[1] = sp37c[0]; sp334[2] = sp380[0];
	sp314[0] = sp384[0]; sp314[1] = sp388[0]; sp314[2] = sp38c[0];

	sp354[3] = sp4cc->unk30;
	sp334[3] = sp4c8->unk30;
	sp314[3] = sp4c4->unk30;

	for (i = 0; i < 4; i++) {
		sp2f4[i] = sp334[i] - sp354[i];
		sp2d4[i] = sp314[i] - sp354[i];
	}

	for (i = 0; i < 4; i++) {
		sp294[i] = ((sp2d4[i] * sp410[0]) - (sp2f4[i] * sp418[0])) * (1.0f / 65536.0f);
		sp2b4[i] = ((sp2f4[i] * sp414[0]) - (sp2d4[i] * sp40c[0])) * (1.0f / 65536.0f);
		sp2d4[i] = sp294[i] * sp484;
		sp2f4[i] = sp2b4[i] * sp484;
		sp274[i] = sp2b4[i] * sp484;
		sp254[i] = sp354[i];
	}

	{
		f32 mult = vtx3->y / vtx2->y;

		f32 sp170 = vtx3->r + ((vtx0->r - vtx2->r) * mult);
		f32 sp16c = vtx3->g + ((vtx0->g - vtx2->g) * mult);
		f32 sp168 = vtx3->b + ((vtx0->b - vtx2->b) * mult);
		f32 sp164 = vtx3->a + ((vtx0->a - vtx2->a) * mult);

		u32 sp160 = vtx0->r * 65536.0f;
		u32 sp15c = vtx0->g * 65536.0f;
		u32 sp158 = vtx0->b * 65536.0f;
		u32 sp154 = vtx0->a * 65536.0f;

		u32 sp150 = func0f152fa0((sp170 - vtx0->r) / ((vtx1->x - vtx0->x) * 0.25f));
		u32 sp14c = func0f152fa0((sp16c - vtx0->g) / ((vtx1->x - vtx0->x) * 0.25f));
		u32 sp148 = func0f152fa0((sp168 - vtx0->b) / ((vtx1->x - vtx0->x) * 0.25f));
		u32 sp144 = func0f152fa0((sp164 - vtx0->a) / ((vtx1->x - vtx0->x) * 0.25f));

		u32 sp140;
		u32 sp13c;
		u32 sp138;
		u32 sp134;
		u32 sp130;
		u32 sp12c;
		u32 sp128;
		u32 sp124;

		sp140 = sp130 = func0f152fa0((vtx2->r - vtx0->r) / ((vtx2->y - vtx0->y) * 0.25f));
		sp13c = sp12c = func0f152fa0((vtx2->g - vtx0->g) / ((vtx2->y - vtx0->y) * 0.25f));
		sp138 = sp128 = func0f152fa0((vtx2->b - vtx0->b) / ((vtx2->y - vtx0->y) * 0.25f));
		sp124 = sp134 = func0f152fa0((vtx2->a - vtx0->a) / ((vtx2->y - vtx0->y) * 0.25f));

		gImmp1(gdl++, G_RDPHALF_1, (sp160 & 0xffff0000) | (sp15c & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp158 & 0xffff0000) | (sp154 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp150 & 0xffff0000) | (sp14c & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp148 & 0xffff0000) | (sp144 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp160 & 0x0000ffff) << 16 | (sp15c & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp158 & 0x0000ffff) << 16 | (sp154 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp150 & 0x0000ffff) << 16 | (sp14c & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp148 & 0x0000ffff) << 16 | (sp144 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp140 & 0xffff0000) | (sp13c & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp138 & 0xffff0000) | (sp134 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp130 & 0xffff0000) | (sp12c & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (sp128 & 0xffff0000) | (sp124 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (sp140 & 0x0000ffff) << 16 | (sp13c & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp138 & 0x0000ffff) << 16 | (sp134 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (sp130 & 0x0000ffff) << 16 | (sp12c & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (sp128 & 0x0000ffff) << 16 | (sp124 & 0x0000ffff));
	}

	sp234[0] = sp364[0] * (1.0f / 32.0f);
	sp238[0] = sp368[0] * (1.0f / 32.0f);
	sp23c[0] = sp3ac * (1.0f / 32.0f);

	for (i = 0; i < 4; i++) {
		sp214[i] = SKYABS(sp2d4[i]) * (1.0f / 32.0f);
		sp1f4[i] = SKYABS(sp2f4[i]) * (1.0f / 32.0f);
	}

	sp1d4[0] = sp234[0] + (2.0f * sp214[0]) + sp1f4[0];
	sp1d8[0] = sp238[0] + (2.0f * sp214[1]) + sp1f4[1];
	sp1dc[0] = sp23c[0] + (2.0f * sp214[2]) + sp1f4[2];

	if (sp1d4[0] < sp1d8[0]) {
		sp1d4[0] = sp1d8[0];
	}

	if (sp1d4[0] < sp1dc[0]) {
		sp1d4[0] = sp1dc[0];
	}

	sp1d4[0] *= (1.0f / 1024.0f);

	if (sp1d4[0] > 1.0f) {
		sp1c4[0] = 1.0f / sp1d4[0];
	} else {
		sp1c4[0] = 1.0f;
	}

	{
		u32 spe0;
		u32 spdc;
		u32 spd8;
		u32 spd4;
		u32 spd0;
		u32 spcc;
		u32 spc8;
		u32 spc4;
		u32 spc0;
		u32 spbc;
		u32 spb8;
		u32 spb4;
		u32 spb0;
		u32 spac;
		u32 spa8;
		u32 spa4;

		spe0 = func0f152fa0(sp254[0] * sp1c4[0]);
		spdc = func0f152fa0(sp254[1] * sp1c4[0]);
		spd8 = func0f152fa0(sp254[2] * sp1c4[0]);
		spd4 = func0f152fa0(sp254[3] * sp1c4[0]);

		spd0 = func0f152fa0(sp2d4[0] * sp1c4[0]);
		spcc = func0f152fa0(sp2d4[1] * sp1c4[0]);
		spc8 = func0f152fa0(sp2d4[2] * sp1c4[0]);
		spc4 = func0f152fa0(sp2d4[3] * sp1c4[0]);

		spb0 = func0f152fa0(sp2f4[0] * sp1c4[0]);
		spac = func0f152fa0(sp2f4[1] * sp1c4[0]);
		spa8 = func0f152fa0(sp2f4[2] * sp1c4[0]);
		spa4 = func0f152fa0(sp2f4[3] * sp1c4[0]);

		spc0 = func0f152fa0(sp274[0] * sp1c4[0]);
		spbc = func0f152fa0(sp274[1] * sp1c4[0]);
		spb8 = func0f152fa0(sp274[2] * sp1c4[0]);
		spb4 = func0f152fa0(sp274[3] * sp1c4[0]);

		gImmp1(gdl++, G_RDPHALF_1, (spe0 & 0xffff0000) | (spdc & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spd8 & 0xffff0000) | (spd4 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spd0 & 0xffff0000) | (spcc & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spc8 & 0xffff0000) | (spc4 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spe0 & 0x0000ffff) << 16 | (spdc & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spd8 & 0x0000ffff) << 16 | (spd4 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spd0 & 0x0000ffff) << 16 | (spcc & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spc8 & 0x0000ffff) << 16 | (spc4 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spc0 & 0xffff0000) | (spbc & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spb8 & 0xffff0000) | (spb4 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spb0 & 0xffff0000) | (spac & 0xffff0000) >> 16);
		gImmp1(gdl++, G_RDPHALF_CONT, (spa8 & 0xffff0000) | (spa4 & 0xffff0000) >> 16);

		gImmp1(gdl++, G_RDPHALF_1, (spc0 & 0x0000ffff) << 16 | (spbc & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_CONT, (spb8 & 0x0000ffff) << 16 | (spb4 & 0x0000ffff));

		gImmp1(gdl++, G_RDPHALF_1, (spb0 & 0x0000ffff) << 16 | (spac & 0x0000ffff));
		gImmp1(gdl++, G_RDPHALF_2, (spa8 & 0x0000ffff) << 16 | (spa4 & 0x0000ffff));
	}

	return gdl;
}

void skyCreateSunArtifact(struct artifact *artifact, s32 x, s32 y)
{
	s32 viewleft = viGetViewLeft();
	s32 viewtop = viGetViewTop();
	s32 viewwidth = viGetViewWidth();
	s32 viewheight = viGetViewHeight();

	if (x >= viewleft && x < viewleft + viewwidth && y >= viewtop && y < viewtop + viewheight) {
#ifndef PLATFORM_N64
		const s32 i = (artifact - schedGetWriteArtifacts()) >> 3;
		struct coord zero = { 0.f };
		struct environment *env = envGetCurrent();
		struct coord sunpos;
		sunpos.x = env->suns[i].pos[0];
		sunpos.y = env->suns[i].pos[1];
		sunpos.z = env->suns[i].pos[2];
		artifact->visiblelos = artifactTestLos(&sunpos, &zero, x, y) * 0xfffc;
#endif
		artifact->zbufptr = &g_ZbufPtr1[(s32)camGetScreenWidth() * y + x];
		artifact->screenx = x;
		artifact->screeny = y;
		artifact->type = ARTIFACTTYPE_CIRCLE;
	}
}

f32 skyGetArtifactGroupIntensityFrac(struct artifact *artifacts)
{
	f32 sum = 0;
	s32 i;

	for (i = 0; i < 8; i++) {
		const u16 test =
#ifdef PLATFORM_N64
			artifacts[i].actualdepth;
#else
			artifacts[i].visiblelos;
#endif
		if (artifacts[i].type == ARTIFACTTYPE_CIRCLE && test == 0xfffc) {
			sum += 0.125f;
		}
	}

	return sum;
}

Gfx *skyRenderSuns(Gfx *gdl, bool xray)
{
	Mtxf *sp16c;
	Mtxf *sp168;
	s16 viewleft;
	s16 viewtop;
	s16 viewwidth;
	s16 viewheight;
	f32 viewleftf;
	f32 viewtopf;
	f32 viewwidthf;
	f32 viewheightf;
	struct artifact *artifacts;
	u8 colour[3];
	struct environment *env;
	struct sun *sun;
	s32 i;
	f32 sp134[2];
	f32 sp12c[2];
	s32 xscale;
	f32 sp124;
	bool onscreen;
	f32 radius;

	sp16c = camGetWorldToScreenMtxf();
	sp168 = camGetMtxF1754();
	env = envGetCurrent();

	xscale = 1;
#ifdef PLATFORM_N64
	if (env->numsuns <= 0 || !g_ZbufPtr1 || g_Vars.mplayerisrunning) {
#else
	g_SunFlareTimers240 = schedGetPlayerSunTimers();
	if (env->numsuns <= 0 || !g_ZbufPtr1 || (g_Vars.mplayerisrunning && g_SplitscreenGlares == false)) {
#endif
		return gdl;
	}

#if !PAL
	if (g_ViRes == 1) {
		xscale = 2;
	}
#endif

	viewleft = viGetViewLeft();
	viewtop = viGetViewTop();
	viewwidth = viGetViewWidth();
	viewheight = viGetViewHeight();

	viewleftf = viewleft;
	viewtopf = viewtop;
	viewwidthf = viewwidth;
	viewheightf = viewheight;

	sun = env->suns;

	for (i = 0; i < env->numsuns; i++) {
		g_SunPositions[i].f[0] = sun->pos[0];
		g_SunPositions[i].f[1] = sun->pos[1];
		g_SunPositions[i].f[2] = sun->pos[2];

		colour[0] = sun->red;
		colour[1] = sun->green;
		colour[2] = sun->blue;

		if (!xray) {
			mtx4TransformVecInPlace(sp16c, &g_SunPositions[i]);
			mtx4TransformVecInPlace(sp168, &g_SunPositions[i]);

			if (g_SunPositions[i].f[2] > 1.0f) {
				g_SunScreenXPositions[i] = (g_SunPositions[i].f[0] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewwidthf + viewleftf;
				g_SunScreenYPositions[i] = (-g_SunPositions[i].f[1] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewheightf + viewtopf;
				radius = 60.0f / viGetFovY() * sun->texture_size;
				onscreen = false;

				if (g_SunScreenXPositions[i] >= viewleftf - radius
						&& g_SunScreenXPositions[i] < viewleftf + viewwidth + radius
						&& g_SunScreenYPositions[i] >= viewtopf - radius
						&& g_SunScreenYPositions[i] < viewtopf + viewheightf + radius) {
					// Sun is at least partially on screen
					if (g_SunScreenXPositions[i] >= viewleftf
							&& g_SunScreenXPositions[i] < viewleftf + viewwidthf
							&& g_SunScreenYPositions[i] >= viewtopf
							&& g_SunScreenYPositions[i] < viewtopf + viewheightf) {
						// Sun's centre point is on-screen
						f32 distfromedge;
						f32 mindistfromedge;
						artifacts = schedGetWriteArtifacts();
						onscreen = true;
						mindistfromedge = 1000;

						if ((s32)g_SunScreenXPositions[i] < viewleft + 15) {
							distfromedge = g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if (1);

						if ((s32)g_SunScreenYPositions[i] < viewtop + 15) {
							distfromedge = g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((s32)g_SunScreenXPositions[i] > viewleft + viewwidth - 16) {
							distfromedge = viewleft + viewwidth - 1 - g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((s32)g_SunScreenYPositions[i] > viewtop + viewheight - 16) {
							distfromedge = viewtop + viewheight - 1 - g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						mindistfromedge -= 1.0f;

						if (mindistfromedge < 0.0f) {
							mindistfromedge = 0.0f;
						}

						g_SunAlphaFracs[i] = mindistfromedge * (1.0f / 15.0f);

						if (g_SunAlphaFracs[i] > 1.0f) {
							g_SunAlphaFracs[i] = 1.0f;
						}

#ifndef PLATFORM_N64
						const bool prevperim = g_Vars.currentplayer->bondperimenabled;
						playerSetPerimEnabled(g_Vars.currentplayer->prop, false);
#endif

						skyCreateSunArtifact(&artifacts[i * 8 + 0], (s32)g_SunScreenXPositions[i] - 7, (s32)g_SunScreenYPositions[i] + 1);
						skyCreateSunArtifact(&artifacts[i * 8 + 1], (s32)g_SunScreenXPositions[i] - 5, (s32)g_SunScreenYPositions[i] - 3);
						skyCreateSunArtifact(&artifacts[i * 8 + 2], (s32)g_SunScreenXPositions[i] - 3, (s32)g_SunScreenYPositions[i] + 5);
						skyCreateSunArtifact(&artifacts[i * 8 + 3], (s32)g_SunScreenXPositions[i] - 1, (s32)g_SunScreenYPositions[i] - 7);
						skyCreateSunArtifact(&artifacts[i * 8 + 4], (s32)g_SunScreenXPositions[i] + 1, (s32)g_SunScreenYPositions[i] + 7);
						skyCreateSunArtifact(&artifacts[i * 8 + 5], (s32)g_SunScreenXPositions[i] + 3, (s32)g_SunScreenYPositions[i] - 5);
						skyCreateSunArtifact(&artifacts[i * 8 + 6], (s32)g_SunScreenXPositions[i] + 5, (s32)g_SunScreenYPositions[i] + 3);
						skyCreateSunArtifact(&artifacts[i * 8 + 7], (s32)g_SunScreenXPositions[i] + 7, (s32)g_SunScreenYPositions[i] - 1);

#ifndef PLATFORM_N64
						playerSetPerimEnabled(g_Vars.currentplayer->prop, prevperim);
#endif
					}

					if (1);

					g_SunFlareTimers240[i] += g_Vars.lvupdate240;

					texSelect(&gdl, &g_TexLightGlareConfigs[5], 4, 0, 2, 1, NULL);

					gDPSetCycleType(gdl++, G_CYC_1CYCLE);
					gDPSetColorDither(gdl++, G_CD_DISABLE);
					gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
					gDPSetTexturePersp(gdl++, G_TP_NONE);
					gDPSetAlphaCompare(gdl++, G_AC_NONE);
					gDPSetTextureLOD(gdl++, G_TL_TILE);
					gDPSetTextureConvert(gdl++, G_TC_FILT);
					gDPSetTextureLUT(gdl++, G_TT_NONE);
					gDPSetTextureFilter(gdl++, G_TF_BILERP);
					gDPSetCombineLERP(gdl++,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0);
					gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], (s32)(g_SunAlphaFracs[i] * 255.0f));

					sp134[0] = g_SunScreenXPositions[i];
					sp134[1] = g_SunScreenYPositions[i];
					sp12c[0] = radius * 0.50f * xscale;
					sp12c[1] = radius * 0.50f;

#ifndef PLATFORM_N64
					sp12c[0] *=  SCREEN_ASPECT / videoGetAspect();
#endif

					func0f0b2150(&gdl, sp134, sp12c, g_TexLightGlareConfigs[5].width, g_TexLightGlareConfigs[5].height, 0, 1, 1, 1, 0, 1);

					gDPPipeSync(gdl++);
					gDPSetColorDither(gdl++, G_CD_BAYER);
					gDPSetTexturePersp(gdl++, G_TP_PERSP);
					gDPSetTextureLOD(gdl++, G_TL_LOD);

					sp124 = skyGetArtifactGroupIntensityFrac(&schedGetFrontArtifacts()[i * 8]);
				}

				if (onscreen && sp124 > 0.0f) {
					g_SunFlareTimers240[i] += g_Vars.lvupdate240;
				} else {
					g_SunFlareTimers240[i] = 0;
				}
			}
		}

		sun++;
	}

	return gdl;
}

/**
 * Render a lens flare.
 *
 * Used for the sun and the Deep Sea teleports.
 */
Gfx *skyRenderFlare(Gfx *gdl, f32 x, f32 y, f32 intensityfrac, f32 size, s32 flaretimer240, f32 alphafrac)
{
	s32 i;
	f32 f2;
	f32 f12;
	f32 sp17c[2];
	f32 sp174[2];
	s32 sp15c[] = { 16, 32, 12, 32, 24, 64 }; // diameters?
	s32 sp144[] = { 60, 80, 225, 275, 470, 570 }; // distances from the source?

	u32 colours[] = {
		0xff99ffff, // pinkish/purple
		0x9999ffff, // blue
		0x99ffffff, // very light blue
		0x99ff99ff, // green
		0xffff99ff, // yellow
		0xff9999ff, // red
	};

	f32 xdist;
	f32 ydist;
	s32 scale;
	f32 fovy;

	scale = 1;

#if !PAL
	if (g_ViRes == VIRES_HI) {
		scale = 2;
	}
#endif

	xdist = (x - viGetViewWidth() / 2.0f) * 0.01f;
	ydist = (y - viGetViewHeight() / 2.0f) * 0.01f;
#ifndef PLATFORM_N64
	if (g_Vars.mplayerisrunning && g_SplitscreenGlares) {
		xdist -= viGetViewLeft() * 0.01f;
		ydist -= viGetViewTop() * 0.01f;
	}
#endif

	// Render the source artifact (eg. the artifact that is on top of the sun)
	texSelect(&gdl, &g_TexLightGlareConfigs[6], 4, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);

	fovy = viGetFovY();

	gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, (s32) (alphafrac * intensityfrac * 255.0f));
	f2 = ((s32) ((60.0f / fovy) * (size * (0.5f + (0.5f * intensityfrac)))));

	sp17c[0] = x;
	sp17c[1] = y;
	sp174[1] = f2 * 0.5f;
	sp174[0] = f2 * 0.5f * scale;

#ifndef PLATFORM_N64
	sp174[0] *=  SCREEN_ASPECT / videoGetAspect();
#endif

	func0f0b2150(&gdl, sp17c, sp174, g_TexLightGlareConfigs[6].width, g_TexLightGlareConfigs[6].height, 0, 1, 1, 1, 0, 1);

	// Render the other artifacts
	texSelect(&gdl, &g_TexLightGlareConfigs[1], 4, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);

	for (i = 0; i < 6; i++) {
		f32 f12;
		f32 f14;
		f32 tmp;

		if (flaretimer240 < TICKS(90)) {
			if (flaretimer240 < TICKS(30)) {
				f2 = flaretimer240 * (1.0f / TICKS(30.0f));
			} else {
				f2 = 1.0f;
			}
		} else {
			f2 = (TICKS(180.0f) - (flaretimer240 - TICKS(90))) * (1.0f / TICKS(180.0f)) * 0.5f;

			if (f2 < 0.0f) {
				f2 =  0.0f;
			}

			f2 += 0.5f;
		}

		f12 = x - sp144[i] * xdist;
		f14 = y - sp144[i] * ydist;

		tmp = sp15c[i];

		gDPSetEnvColor(gdl++,
				(colours[i] >> 24) & 0xff,
				(colours[i] >> 16) & 0xff,
				(colours[i] >> 8) & 0xff,
				(s32) ((colours[i] & 0xff) * (alphafrac * f2)));

		sp17c[0] = f12;
		sp17c[1] = f14;

		sp174[1] = tmp * 0.5f;
		sp174[0] = tmp * 0.5f * scale;

#ifndef PLATFORM_N64
		sp174[0] *=  SCREEN_ASPECT / videoGetAspect();
#endif

		func0f0b2150(&gdl, sp17c, sp174, g_TexLightGlareConfigs[1].width, g_TexLightGlareConfigs[1].height, 0, 0, 0, 0, 0, 1);
	}

	// Check if the source is close to the center of the screen and create the bloom effect if so
	xdist = viGetViewWidth() / 2.0f - x;
	ydist = viGetViewHeight() / 2.0f - y;
#ifndef PLATFORM_N64
	if (g_Vars.mplayerisrunning && g_SplitscreenGlares) {
		xdist += viGetViewLeft();
		ydist += viGetViewTop();
	}
#endif

	f12 = (40.0f - sqrtf(xdist * xdist + ydist * ydist)) * 0.0125f;

	if (f12 < 0.0f) {
		f12 = 0.0f;
	}

	f12 += 0.1f;

	if (flaretimer240 <= g_Vars.lvupdate240) {
		f12 = 0.0f;
	}

	if (f12 > 0.0f) {
		skySetOverexposure(alphafrac * f12 * 255.0f, alphafrac * f12 * 255.0f, alphafrac * f12 * 255.0f);
	}

	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetAlphaDither(gdl++, G_AD_PATTERN | G_CD_DISABLE);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetTextureLOD(gdl++, G_TL_LOD);

	return gdl;
}

struct coord g_TeleportToPos = {0, 0, 0};
struct coord g_TeleportToUp = {0, 0, 1};
struct coord g_TeleportToLook = {0, 1, 0};

Gfx *skyRenderTeleportFlare(Gfx *gdl, f32 x, f32 y, f32 z, f32 size, f32 intensityfrac)
{
	struct coord sp64;

	sp64.x = x;
	sp64.y = y;
	sp64.z = z;

	mtx4TransformVecInPlace(camGetWorldToScreenMtxf(), &sp64);
	mtx4TransformVecInPlace(camGetMtxF1754(), &sp64);

	if (sp64.z > 1.0f) {
		f32 xpos;
		f32 ypos;
		s16 viewlefti = viGetViewLeft();
		s16 viewtopi = viGetViewTop();
		s16 viewwidthi = viGetViewWidth();
		s16 viewheighti = viGetViewHeight();
		f32 viewleft = viewlefti;
		f32 viewwidth = viewwidthi;
		f32 viewtop = viewtopi;
		f32 viewheight = viewheighti;

		xpos = viewleft + (sp64.f[0] / sp64.f[2] + 1.0f) * 0.5f * viewwidth;
		ypos = viewtop + (-sp64.f[1] / sp64.f[2] + 1.0f) * 0.5f * viewheight;

		if (xpos >= viewleft && xpos < viewleft + viewwidth
				&& ypos >= viewtop && ypos < viewtop + viewheight) {
			gdl = skyRenderFlare(gdl, xpos, ypos, intensityfrac, size, TICKS(90), 1.0f);
		}
	}

	return gdl;
}

/**
 * Render lens flares during teleport.
 */
Gfx *skyRenderTeleportFlares(Gfx *gdl)
{
	f32 sp154 = g_20SecIntervalFrac * M_BADTAU;
	s32 i;
	f32 sizefrac = 0.0f;
	f32 f20_2;
	f32 f22;
	f32 f22_3;
	struct pad pad;
	struct coord spe0;
	f32 spd0[4];
	Mtxf mtx;
	f32 f24;
	f32 intensityfrac;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER) {
		sizefrac = g_Vars.currentplayer->teleporttime / 24.0f * 0.33f;
	} else if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		sizefrac = g_Vars.currentplayer->teleporttime / 48.0f * 0.66f + 0.33f;
	}

	intensityfrac = sizefrac * 6.0f;
	f22 = sizefrac * 1.3f;

	if (f22 > 1.0f) {
		f22 = 1.0f;
	}

	if (intensityfrac > 1.0f) {
		intensityfrac = 1.0f;
	}

	sizefrac *= 1.7f;

	if (sizefrac > 1.0f) {
		sizefrac = 1.0f;
	}

	padUnpack(g_Vars.currentplayer->teleportpad, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP, &pad);

	g_TeleportToPos.x = pad.pos.x;
	g_TeleportToPos.y = pad.pos.y;
	g_TeleportToPos.z = pad.pos.z;
	g_TeleportToLook.x = pad.look.x;
	g_TeleportToLook.y = pad.look.y;
	g_TeleportToLook.z = pad.look.z;
	g_TeleportToUp.x = pad.up.x;
	g_TeleportToUp.y = pad.up.y;
	g_TeleportToUp.z = pad.up.z;

	f22 = -cosf(f22 * M_PI) * 0.5f + .5f;
	f24 = 100 * f22;

	for (i = 0; i < 5; i++) {
		spe0.x = g_TeleportToLook.f[0] * f24;
		spe0.y = g_TeleportToLook.f[1] * f24;
		spe0.z = g_TeleportToLook.f[2] * f24;

		f22_3 = sp154 + i * 1.2564370632172f;
		f20_2 = sinf(f22_3);

		spd0[0] = cosf(f22_3);
		spd0[1] = g_TeleportToUp.f[0] * f20_2;
		spd0[2] = g_TeleportToUp.f[1] * f20_2;
		spd0[3] = g_TeleportToUp.f[2] * f20_2;

		quaternionToMtx(spd0, &mtx);
		mtx4RotateVecInPlace(&mtx, &spe0);

		spe0.x += g_TeleportToPos.x;
		spe0.y += g_TeleportToPos.y;
		spe0.z += g_TeleportToPos.z;

		gdl = skyRenderTeleportFlare(gdl, spe0.x, spe0.y, spe0.z, sizefrac * 200, intensityfrac);
	}

	return gdl;
}

/**
 * Render teleport artifacts, and all suns and their artifacts.
 */
Gfx *skyRenderArtifacts(Gfx *gdl)
{
	struct environment *env = envGetCurrent();
	struct sun *sun;
	s32 i;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER
			|| g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		gdl = skyRenderTeleportFlares(gdl);
	}

#ifdef PLATFORM_N64
	if (env->numsuns <= 0 || !g_ZbufPtr1 || g_Vars.mplayerisrunning) {
#else
	if (env->numsuns <= 0 || !g_ZbufPtr1 || (g_Vars.mplayerisrunning && g_SplitscreenGlares == false)) {
#endif
		return gdl;
	}

	sun = env->suns;

	for (i = 0; i < env->numsuns; i++) {
		if (sun->lens_flare && g_SunPositions[i].z > 1) {
			struct artifact *artifacts = schedGetFrontArtifacts() + i * 8;
			f32 intensityfrac = skyGetArtifactGroupIntensityFrac(artifacts);

			if (intensityfrac > 0.0f) {
				gdl = skyRenderFlare(gdl, g_SunScreenXPositions[i], g_SunScreenYPositions[i], intensityfrac, sun->orb_size, g_SunFlareTimers240[i], g_SunAlphaFracs[i]);
			}
		}

		sun++;
	}

	return gdl;
}

void skySetOverexposure(s32 r, s32 g, s32 b)
{
#ifndef PLATFORM_N64
	const f32 scale = videoGetOverexposureScale();
	r *= scale;
	g *= scale;
	b *= scale;
#endif

	g_Vars.currentplayer->overexposurered = sqrtf(g_Vars.currentplayer->overexposurered * g_Vars.currentplayer->overexposurered + r * r);
	g_Vars.currentplayer->overexposuregreen = sqrtf(g_Vars.currentplayer->overexposuregreen * g_Vars.currentplayer->overexposuregreen + g * g);
	g_Vars.currentplayer->overexposureblue = sqrtf(g_Vars.currentplayer->overexposureblue * g_Vars.currentplayer->overexposureblue + b * b);

	if (g_Vars.currentplayer->overexposurered > 0xcc) {
		g_Vars.currentplayer->overexposurered = 0xcc;
	}

	if (g_Vars.currentplayer->overexposuregreen > 0xcc) {
		g_Vars.currentplayer->overexposuregreen = 0xcc;
	}

	if (g_Vars.currentplayer->overexposureblue > 0xcc) {
		g_Vars.currentplayer->overexposureblue = 0xcc;
	}
}

s32 skyCalculateOverexposureComponent(s32 old, s32 new)
{
	if (new >= old) {
		if (new - old > 8) {
			return old + 8;
		} else {
			return new;
		}
	} else {
		if (old - new > 8) {
			return old - 8;
		} else {
			return new;
		}
	}
}

/**
 * Overexposure is used when the player looks at the sun, and when night vision
 * is overloaded. An almost-transparent rectangle is drawn across the viewport.
 */
Gfx *skyRenderOverexposure(Gfx *gdl)
{
	s32 value;
	u32 stack;

	g_Vars.currentplayer->overexposurered = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposurered, g_Vars.currentplayer->overexposurered);
	g_Vars.currentplayer->overexposuregreen = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposuregreen, g_Vars.currentplayer->overexposuregreen);
	g_Vars.currentplayer->overexposureblue = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposureblue, g_Vars.currentplayer->overexposureblue);

	value = (g_Vars.currentplayer->overexposurered > g_Vars.currentplayer->overexposuregreen && g_Vars.currentplayer->overexposurered > g_Vars.currentplayer->overexposureblue)
		? g_Vars.currentplayer->overexposurered
		: g_Vars.currentplayer->overexposuregreen > g_Vars.currentplayer->overexposureblue
		? g_Vars.currentplayer->overexposuregreen
		: g_Vars.currentplayer->overexposureblue;

	if (!g_InCutscene && EYESPYINACTIVE() && value > 0) {
		f32 r = g_Vars.currentplayer->overexposurered * (255.0f / value);
		f32 g = g_Vars.currentplayer->overexposuregreen * (255.0f / value);
		f32 b = g_Vars.currentplayer->overexposureblue * (255.0f / value);

		f32 a = (g_Vars.currentplayer->overexposurered
			+ g_Vars.currentplayer->overexposuregreen
			+ g_Vars.currentplayer->overexposureblue) * (1.0f / 3.0f);

		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
		gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

		if (USINGDEVICE(DEVICE_NIGHTVISION)) {
			r *= 0.5f;
			g *= 0.75f;
			b *= 0.5f;
		} else if (USINGDEVICE(DEVICE_IRSCANNER)) {
			r *= 0.75f;
			g *= 0.5f;
			b *= 0.5f;
		}

		gDPSetPrimColor(gdl++, 0, 0, (s32)r, (s32)g, (s32)b, (s32)a);

		gDPFillRectangle(gdl++,
				viGetViewLeft(),
				viGetViewTop(),
				viGetViewLeft() + viGetViewWidth(),
				viGetViewTop() + viGetViewHeight());

		gDPPipeSync(gdl++);
	}

	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	g_Vars.currentplayer->prevoverexposurered = g_Vars.currentplayer->overexposurered;
	g_Vars.currentplayer->prevoverexposuregreen = g_Vars.currentplayer->overexposuregreen;
	g_Vars.currentplayer->prevoverexposureblue = g_Vars.currentplayer->overexposureblue;
	g_Vars.currentplayer->overexposurered = 0;
	g_Vars.currentplayer->overexposuregreen = 0;
	g_Vars.currentplayer->overexposureblue = 0;

	return gdl;
}
