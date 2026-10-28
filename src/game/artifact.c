#include <ultra64.h>
#include "lib/sched.h"
#include "constants.h"
#include "game/camera.h"
#include "game/dlights.h"
#include "game/env.h"
#include "game/game_0b2150.h"
#include "game/tex.h"
#include "game/sky.h"
#include "game/artifact.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/room.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "lib/collision.h"
#include "lib/lib_17ce0.h"
#include "game/player.h"
#include "game/prop.h"
#include "video.h"
#endif

/**
 * Artifacts are points of interest in the z-buffer.
 *
 * They correspond to:
 * - Individual circles in the lens flare effect from the sun.
 * - Individual corners of each light fixture's box.
 *
 * The game needs to do line of sight checks to these points. The collision
 * system would normally be used for line of sight checks, but it only works
 * with basic geometric volumes and also doesn't take into account the player's
 * gun. Instead, the checks are done by reading from the previous frame's
 * z-buffer.
 *
 * Typically, the z-buffer would be read by the scheduler after a graphics frame
 * is rendered, but this only works if the z-buffer is a complete image.
 * PD draws the scene, then clears the z-buffer before drawing the player's gun,
 * so the z-buffer at the end only contains the player's gun. To work around
 * this, the GPU is given commands to read the z-buffer as a texture and copy
 * depth values to a safe place before clearing it. Then once the frame is
 * rendered, the scheduler compares the saved depths with the current z-buffer
 * and updates the artifact, applying the minimum of the two depths.
 *
 * The scheduler maintains 3 arrays of artifacts, each referenced by indexes
 * which rotate between them.
 * - Artifacts are written by the main thread using the write index.
 *   The write and front indexes are then incremented.
 * - The same artifacts are updated by the scheduler using the pending index.
 *   The pending index is then updated.
 * - The artifacts are then rendered on a later frame using the front index.
 *
 * In each artifacts array, the first several slots are reserved for the sun
 * flare artifacts. The quantity depends on the number of suns in the stage
 * (Skedar Ruins has 3) and there are 8 artifacts per sun. The remaining slots
 * are used for light fixture corners. The array is big enough to handle at
 * least 90 lights on screen at a time.
 *
 * The initial state of the indexes are write=0, front=1, pending=0.
 * These are set in sched_reset_artifacts.
 *
 * The detailed workflow is:
 * - CPU: Light artifacts are determined and added to write artifacts (0) array
 * - CPU: Constructs the gdl in this order:
 *     - Render scene
 *     - Copy relevant parts of z-buffer to write depths (0) array
 *     - Clear z-buffer and render gun
 *     - Render artifacts by reading from front artifacts (1) array
 * - CPU: increments write (0 -> 1) and front (1 -> 2) indexes
 * - GPU: Executes above task. g_ArtifactDepths0 now contains depths, and is
 *       pointed to by pending.
 * - Scheduler: Reads the z-buffer, which at this point only contains the gun
 *       depth information, and updates the artifact with the minimum of the two
 * - Scheduler: Increments pending (0 -> 1)
 *
 * It appears that the front index should be initialised to 2 instead, so that
 * it's one frame behind the others rather than two frames behind.
 *
 * There's no doubt this went through several iterations before landing on this
 * implementation. It's likely that this was implemented using a single and full
 * z-buffer and it worked well until they decided to clear the z-buffer before
 * rendering the gun. There is evidence in zbuf.c that they allocated a second
 * z-buffer and swapped it. But when memory got too tight they had to change it
 * to this texture read method.
 */

u8 *var800a41a0;
u32 var800a41a4;
u32 var800a41a8;
u32 var800a41ac;

void artifactsClear(void)
{
	struct artifact *artifacts = schedGetWriteArtifacts();
	s32 i;

	for (i = 0; i < MAX_ARTIFACTS; i++) {
		artifacts[i].type = ARTIFACTTYPE_FREE;
	}
}

void artifactsTick(void)
{
	schedIncrementWriteArtifacts();
	schedIncrementFrontArtifacts();
}

u16 floatToN64Depth(f32 arg0)
{
	/**
	 * Method to convert a 32 bit floating point depth value to the
	 * unsigned 16 bit integer format used by the zbuffer on the N64.
	 * The argument arg0 represents z normalized to [0, 1] * 32704.0f.
	 *
	 * It works by converting the depth value to a large, unsigned 32 bit
	 * integer before scaling back down to an unsigned 16 bit integer.
	 * The scaling is done using bit shift operations on the most & least
	 * significant bytes of the resulting 16 bit integer, likely because
	 * bit shift operations are faster than division. The segmentation of
	 * this calculation at values 0x3f800, 0x3f000, etc. is probably
	 * intended to reduce z-fighting by adding more differentiation for
	 * distant z values.
	 */
	u32 value = arg0 * 8.0f;
	u32 left; // forms the most significant byte of the final u16
	u32 right = value; // forms the least significant byte of the final u16

	if (value > 0x3f800) {
		right = value & 0x7ff;
		right &= 0x7ff;
		left = 7;
	} else if (value > 0x3f000) {
		right = value & 0x7ff;
		right &= 0x7ff;
		left = 6;
	} else if (value > 0x3e000) {
		right = (value >> 1) & 0x7ff;
		right &= 0x7ff;
		left = 5;
	} else if (value > 0x3c000) {
		right = (value >> 2) & 0x7ff;
		right &= 0x7ff;
		left = 4;
	} else if (value > 0x38000) {
		right = (value >> 3) & 0x7ff;
		right &= 0x7ff;
		left = 3;
	} else if (value > 0x30000) {
		right = (value >> 4) & 0x7ff;
		right &= 0x7ff;
		left = 2;
	} else if (value > 0x20000) {
		right = (value >> 5) & 0x7ff;
		right &= 0x7ff;
		left = 1;
	} else {
		right = (value >> 6) & 0x7ff;
		right &= 0x7ff;
		left = 0;
	}

	return left << 13 | (right << 2);
}

s32 artifactsFloatToInt(f32 arg0)
{
	if (arg0 > 0.0f) {
		if (arg0 > 2147483520.0f) {
			arg0 = 2147483520;
		}
	} else {
		if (arg0 < -2147483520) {
			arg0 = -2147483520;
		}
	}

	return arg0;
}

#ifndef PLATFORM_N64

bool artifactTestLos(struct coord *spec, struct coord *roompos, s32 xi, s32 yi)
{
	s32 i = 0;

	if (!g_Vars.currentplayer) {
		return false;
	}

	struct coord endpos;
	endpos.x = roompos->x + spec->x;
	endpos.y = roompos->y + spec->y;
	endpos.z = roompos->z + spec->z;

	struct coord gundir2d;
	struct coord gunpos2d = {{ 0.f, 0.f, 0.f }};
	struct coord gundir3d;
	struct coord gunpos3d = g_Vars.currentplayer->cam_pos;
	f32 crosspos[2] = { (f32)xi, (f32)yi };
	cam0f0b4c3c(crosspos, &gundir2d, 1.f);
	mtx4RotateVec(camGetProjectionMtxF(), &gundir2d, &gundir3d);

	return shotTestLos(&gunpos2d, &gundir2d, &gunpos3d, &gundir3d, &endpos);
}

#endif

void artifactsCalculateGlaresForRoom(s32 roomnum)
{
	s32 i;
	s32 j;
	s32 k;
	s32 l;
	f32 f0;
	s32 numlights;
	f32 viewwidth;
	f32 viewheight;
	f32 viewleft;
	f32 viewtop;
	u8 *s1;
	f32 x;
	f32 y;
	f32 f16;
	f32 f20;
	s32 xi;
	s32 yi;
	f32 sp190;
	f32 brightnessfrac;
	f32 thisfrac;
	f32 tmp;
	f32 tmp2;
	f32 tmp3;
	f32 sp178;
	Mtxf sp138;
	Mtxf spf8;
	struct coord spec;
	f32 spdc[4];
	struct coord origin;
	struct coord spc4;
	struct light *roomlights;
	s32 index;
	struct artifact *artifacts = schedGetWriteArtifacts();
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	struct artifact *artifact;

#ifndef PLATFORM_N64
	if (videoGetGlareBrightness() <= 0.f) {
		return;
	}
	var800a41a0 = schedGetPlayerLightTable();
#endif

	if (g_Rooms[roomnum].gfxdata != NULL && g_Rooms[roomnum].loaded240) {
		numlights = g_Rooms[roomnum].gfxdata->numlights;

		if (numlights != 0) {
			roomlights = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].gfxdata->lightsindex * 0x22];
			s1 = &var800a41a0[g_Rooms[roomnum].gfxdata->lightsindex * 3];

			roomPopulateMtx(&sp138, roomnum);
			mtx00015f88(bgGetScaleBg2Gfx(), &sp138);
			mtx4MultMtx4(camGetMtxF006c(), &sp138, &spf8);

			viewwidth = viGetViewWidth();
			viewheight = viGetViewHeight();
			viewleft = viGetViewLeft();
			viewtop = viGetViewTop();

			for (i = 0; i < numlights; i++) {
				origin.x = 0.0f;
				origin.y = 0.0f;
				origin.z = 0.0f;

				for (j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
					origin.x += roomlights[i].bbox[j].x;
					origin.y += roomlights[i].bbox[j].y;
					origin.z += roomlights[i].bbox[j].z;
				}

				origin.x /= 4.0f;
				origin.y /= 4.0f;
				origin.z /= 4.0f;

				for (j = 0; j != 3; j++) {
					spc4.f[j] = origin.f[j] - (campos->f[j] - g_BgRooms[roomnum].pos.f[j]);
				}

				s1[i * 3 + 1] = 0;
				s1[i * 3 + 2] = 0;

				tmp = roomlights[i].dirx * roomlights[i].dirx + roomlights[i].diry * roomlights[i].diry + roomlights[i].dirz * roomlights[i].dirz;
				f16 = spc4.f[0] * spc4.f[0] + spc4.f[1] * spc4.f[1] + spc4.f[2] * spc4.f[2];

				if (tmp > 0.0001f && f16 > 0.0001f) {
					sp190 = -((roomlights[i].dirx * spc4.f[0] + roomlights[i].diry * spc4.f[1] + roomlights[i].dirz * spc4.f[2]) / sqrtf(tmp * f16));

					if (sp190 > 0.4f) {
						sp190 = 0.4f;
					}

					sp190 *= 2.5f;
				} else {
					sp190 = 0.0f;
				}

				if (sp190 > 0.0f) {
					for (l = 3; l >= 0; l--) {
						spdc[l] = origin.f[0] * spf8.m[0][l] + origin.f[1] * spf8.m[1][l] + origin.f[2] * spf8.m[2][l] + spf8.m[3][l];

						if (l == 3 && spdc[l] <= 0.0f) {
							break;
						}
					}

					if (spdc[3] > 0.0001f) {
						f20 = 1.0f / spdc[3];
						x = artifactsFloatToInt(viewleft + (1.0f + spdc[0] * f20) * (viewwidth * 0.5f));
						y = artifactsFloatToInt(viewtop + (1.0f - spdc[1] * f20) * (viewheight * 0.5f));
						f0 = (spdc[2] * f20 * 511.0f + 511.0f) * 32.0f;

						if (f0 < 32576.0f) {
							brightnessfrac = 1.0f;
							tmp2 = (brightnessfrac - 1.00f);

							if (x <= 10.0f + viewleft) {
								brightnessfrac = 0.0f;
							} else if (y <= 30.0f + viewtop) {
								brightnessfrac = 0.0f;
							} else if (x >= -10.0f + viewleft + viewwidth) {
								brightnessfrac = 0.0f;
							} else if (y >= -30.0f + viewtop + viewheight) {
								brightnessfrac = 0.0f;
							}

							sp178 = 1.0f - 2.0f * tmp2;

							if (brightnessfrac != 0.0f) {
								brightnessfrac = 1.0f;

								if (x < viewleft + 90.0f) {
									thisfrac = (x - (10.0f + viewleft)) / 80.0f;

									if (thisfrac < brightnessfrac) {
										brightnessfrac = thisfrac;
									}
								}

								if (y < viewtop + 100.0f) {
									thisfrac = (y - (viewtop + 30.0f)) / 70.0f;

									if (thisfrac < brightnessfrac) {
										brightnessfrac = thisfrac;
									}
								}

								if (x > viewleft + viewwidth - 90.0f) {
									thisfrac = (viewleft + viewwidth - 10.0f - x) / 80.0f;

									if (thisfrac < brightnessfrac) {
										brightnessfrac = thisfrac;
									}
								}

								if (y > viewtop + viewheight - 100.0f) {
									thisfrac = (viewtop + viewheight - 30.0f - y) / 70.0f;

									if (thisfrac < brightnessfrac) {
										brightnessfrac = thisfrac;
									}
								}
							}

							tmp3 = 32300.0f - f0;

							if (tmp3 < 0.0f) {
								tmp3 = 0.0f;
							}

							if (tmp3 > 1300.0f) {
								tmp3 = 1300.0f;
							}

							tmp3 *= 1.0f / 1300.0f;

							if (2.0f * tmp2 > 1.0f) {
								sp178 = 0.0f;
							}

							s1[i * 3 + 1] = sp190 * 255.0f * sp178;
							s1[i * 3 + 2] = brightnessfrac * tmp3 * sp190 * 64.0f * 1;
						}
					}
				}

				if (s1[i * 3 + 1] > 0) {
					for (j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
						spec.x = origin.x + (roomlights[i].bbox[j].x - origin.x) * 0.6f;
						spec.y = origin.y + (roomlights[i].bbox[j].y - origin.y) * 0.6f;
						spec.z = origin.z + (roomlights[i].bbox[j].z - origin.z) * 0.6f;

						for (k = 3; k >= 0; k--) {
							spdc[k] = spec.f[0] * spf8.m[0][k] + spec.f[1] * spf8.m[1][k] + spec.f[2] * spf8.m[2][k] + spf8.m[3][k];

							if (k == 3 && spdc[k] <= 0.0f) {
								break;
							}
						}

						if (spdc[3] > 0.0f) {
							f20 = 1.0f / spdc[3];

							if (f20 > 9999.0f) {
								f20 = 9999.0f;
							}

							if (f20 < -9999.0f) {
								f20 = -9999.0f;
							}

							xi = artifactsFloatToInt(viewleft + (1.0f + spdc[0] * f20) * (viewwidth * 0.5f));
							yi = artifactsFloatToInt(viewtop + (1.0f - spdc[1] * f20) * (viewheight * 0.5f));
							f0 = (spdc[2] * f20 * 511.0f + 511.0f) * 32.0f;

							if (g_ZbufPtr1
									&& xi >= (s32)viewleft
									&& xi < (s32)(viewleft + viewwidth)
									&& yi >= (s32)viewtop
									&& yi < (s32)(viewtop + viewheight)
									&& f0 < 32576.0f) {
								index = envGetCurrent()->numsuns;
								index *= 8;
								artifact = artifacts;
								artifact += index;

								while (artifact->type != ARTIFACTTYPE_FREE) {
									index++;
									artifact++;
								}

								if (index < MAX_ARTIFACTS) {
#ifndef PLATFORM_N64
									artifact->visiblelos = artifactTestLos(&spec, &g_BgRooms[roomnum].pos, xi, yi);
#endif
									/**
									 * the original game performs artifact depth comparison
									 * using the N64 depth values divided by 4. This is
									 * accomplished by bit shifting the N64 depth value
									 * to the right using >> 2
									 */
									artifact->expecteddepth = floatToN64Depth(f0) >> 2;
									artifact->zbufptr = &g_ZbufPtr1[viGetWidth() * yi + xi];
									artifact->light = &roomlights[i];
									artifact->type = ARTIFACTTYPE_GLARE;
									artifact->screenx = xi;
									artifact->screeny = yi;
								}
							}
						}
					}
				}
			}
		}
	}
}

u8 artifactsClamp(u8 arg0, u8 arg1)
{
	if (arg1 >= arg0 + 7) {
		return arg0 + 7;
	}

	if (arg1 <= arg0 - 7) {
		return arg0 - 7;
	}

	return arg1;
}

Gfx *artifactsConfigureForGlares(Gfx *gdl)
{
	struct stagetableentry *stage = stageGetCurrent();

	texSelect(&gdl, &g_TexLightGlareConfigs[stage->light_type], 4, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
	gDPSetTexturePersp(gdl++, G_TP_NONE);

	return gdl;
}

Gfx *artifactsUnconfigureForGlares(Gfx *gdl)
{
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	return gdl;
}

Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, s32 roomnum)
{
	s32 i;
	s32 j;
	s32 lightindex;
	struct artifact *artifacts;
	u16 min;
	u16 max;
	f32 lightop_cur_frac;
	s32 numgood;
	struct light *light;
	u8 *s3;
	s32 k;
	s32 count;
	u16 actualdepth;
	f32 add;
	s32 l;
	f32 brightness;
	s32 tolerance;
	f32 f0;
	s32 difference;
	s32 r;
	s32 g;
	s32 b;
	s32 stack;
	u8 colour[4];
	s16 lightroompos[3];
	struct coord lightworldpos;
	struct coord lightscreenpos;
	f32 spdc[2];
	f32 spd4[2];
	f32 f24;
	bool extra;
	f32 f26;
#ifndef PLATFORM_N64
	const f32 pfov = g_Vars.currentplayerstats ? PLAYER_DEFAULT_FOV : 60.f;
	const f32 brightscale = videoGetGlareBrightness();
	if (brightscale <= 0.f) {
		return gdl;
	}
#endif

	artifacts = schedGetFrontArtifacts();
	lightop_cur_frac = roomGetLightOpCurFrac(roomnum);

	if (g_Rooms[roomnum].gfxdata == NULL || g_Rooms[roomnum].loaded240 == 0) {
		return gdl;
	}

	for (i = envGetCurrent()->numsuns * 8; i < MAX_ARTIFACTS; i++) {
		struct light *light2 = artifacts[i].light;
		count = 0;

		/**
		 * light arifacts are created from several, closely spaced
		 * textures that give the appearance of a dynamic light glare
		 * as the character moves. Loop to count all the sub-artifacts
		 * in this light.
		 */
		for (j = i; j < MAX_ARTIFACTS && artifacts[j].type == ARTIFACTTYPE_GLARE && artifacts[j].light == light2; j++) {
			count++;
		}

		light = artifacts[i].light;

		if (count > 0) {
			if (roomnum == light->roomnum) {
				lightindex = ((uintptr_t)light - (uintptr_t)g_BgLightsFileData) / sizeof(struct light);
				s3 = &var800a41a0[lightindex * 3];
				numgood = 0;
				min = 0xffff;
				max = 0;

				/**
				 * loop to determine the min & max depth of
				 * the sub-artifacts composing this room light.
				 */
				for (k = i; k < i + count; k++) {
					if (artifacts[k].expecteddepth > max) {
						max = artifacts[k].expecteddepth;
					}

					if (artifacts[k].expecteddepth < min) {
						min = artifacts[k].expecteddepth;
					}
				}

				/**
				 * Define a depth tolerance from the min & max
				 * depths of sub-artifacts for a given light.
				 * This will be used to determine which light
				 * artifacts are visible when comparing depth
				 * values of lights to other items rendered
				 * on screen. Without this, the lights would
				 * constantly flicker due to z-fighting caused
				 * by the low precision of the N64 depth.
				 */
				tolerance = (max - min) >> 1;

				if (tolerance < 25) {
					tolerance = 25;
				}

				for (k = i; k < i + count; k++) {
#ifdef PLATFORM_N64
					u16 expecteddepth;
					actualdepth = (artifacts[k].actualdepth & 0xfffc) >> 2;
					expecteddepth = artifacts[k].expecteddepth;

					if (expecteddepth < actualdepth) {
						difference = actualdepth - expecteddepth;
					} else {
						difference = expecteddepth - actualdepth;
					}

					if (difference <= tolerance) {
						numgood++;
					}
#else
					numgood += artifacts[k].visiblelos;
#endif

					artifacts[k].type = ARTIFACTTYPE_FREE;
				}

				s3[0] = artifactsClamp(s3[0], numgood * 2);

				if (numgood > 0) {
#ifndef PLATFORM_N64
					brightness = viGetFovY() / pfov;
#else
					brightness = viGetFovY() * 0.017453292f;
#endif
					add = cosf(brightness) / sinf(brightness) * 14.6f;

					if (lightIsHealthy(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
						if (!lightIsOn(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
							continue;
						}

						brightness = 1.0f;
					} else if (lightTickBroken(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
						brightness = 0.4f;
					} else {
						continue;
					}

					r = ((light->colour >> 12) & 0xf) * 17;
					g = ((light->colour >> 8) & 0xf) * 17;
					b = ((light->colour >> 4) & 0xf) * 17;

					if ((r == 0xff && g == 0xff && b == 0xff) || (r == 0xff && g + b < 35)) {
						extra = false;
					} else {
						extra = true;
					}

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						s3[2] *= (s32) (lightop_cur_frac * 7.0f);
					}

					f0 = s3[2] * (1.0f / 255.0f);
#ifndef PLATFORM_N64
					f0 *= (60.f / pfov);
#endif

					skySetOverexposure((s32) ((f32)f0 * r), (s32) ((f32)f0 * g), (s32) ((f32)f0 * b));

					for (l = 0; l < 3; l++) {
						lightroompos[l] = (light->bbox[0].s[l] + light->bbox[1].s[l] + light->bbox[2].s[l] + light->bbox[3].s[l]) / 4;
						lightworldpos.f[l] = lightroompos[l] + g_BgRooms[roomnum].pos.f[l];
						lightscreenpos.f[l] = lightworldpos.f[l] - g_Vars.currentplayer->cam_pos.f[l];
					}

					mtx4RotateVecInPlace(camGetWorldToScreenMtxf(), &lightscreenpos);

					cam0f0b4d04(&lightscreenpos, spdc);

					brightness *= 27500.0f / (-lightscreenpos.z < 1.0f ? 1.0f : -lightscreenpos.z);

					if (light->brightnessmult != 0) {
						brightness *= light->brightnessmult * (1.0f / 32.0f);
					}

					brightness *= s3[1] * (1.0f / 255.0f);

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						brightness *= 14.0f * lightop_cur_frac;
					}

					brightness += add;
					brightness *= 2.0f * roomGetSettledLocalBrightnessFrac(roomnum);

					if (brightness > 750.0f) {
						brightness = 750.0f;
					}

					f24 = stageGetCurrent()->light_width * brightness * 0.01f;
					f26 = stageGetCurrent()->light_height * brightness * 0.01f;

					f24 *= viGetViewWidth() * (1.0f / 240.0f) / camGetPerspAspect();
					f26 *= viGetViewHeight() * (1.0f / 240.0f);

					if (brightness > 3.0f) {
						f32 alpha = (light->colour & 0xf) * 17;

						colour[0] = r;
						colour[1] = g;
						colour[2] = b;

						alpha *= stageGetCurrent()->light_alpha / 255.0f;
						alpha *= (s3[1] / 255.0f);
						alpha *= (s3[0] / 8.0f);

#ifndef PLATFORM_N64
						alpha *= brightscale;
#endif

						if (USINGDEVICE(DEVICE_NIGHTVISION)) {
							alpha *= lightop_cur_frac * 7.0f;
						}

						if (alpha > 255.0f) {
							alpha = 255.0f;
						}

						colour[3] = alpha;

						gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], colour[3]);

						spd4[0] = f24;
						spd4[1] = f26;

						func0f0b2740(&gdl, spdc, spd4, 64, 64, false, false, false, 1);

						if (extra) {
							colour[0] = 0xff;
							colour[1] = 0xff;
							colour[2] = 0xff;
							colour[3] = stageGetCurrent()->light_alpha;
							colour[3] = s3[0] * colour[3] / 8;

							gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], colour[3]);

							spd4[0] = f24 * 0.4f;
							spd4[1] = f26 * 0.4f;

							func0f0b2740(&gdl, spdc, spd4, 64, 64, false, false, false, 1);
						}
					}
				}

				s3[1] = 0;
				s3[2] = 0;
			}

			// This is incrementing i past all the artifacts for this particular
			// light, then subtracting 1 because the for loop will add 1.
			i = i + count - 1;
		}
	}

	return gdl;
}
