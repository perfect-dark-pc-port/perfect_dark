#ifndef _IN_GAME_GAME_13C510_H
#define _IN_GAME_GAME_13C510_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void artifactsClear(void);
void artifactsTick(void);
u16 floatToN64Depth(f32 arg0);
s32 artifactsFloatToInt(f32 arg0);
void artifactsCalculateGlaresForRoom(s32 roomnum);
u8 artifactsClamp(u8 arg0, u8 arg1);
Gfx *artifactsConfigureForGlares(Gfx *gdl);
Gfx *artifactsUnconfigureForGlares(Gfx *gdl);
Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, s32 roomnum);
#ifndef PLATFORM_N64
bool artifactTestLos(struct coord *spec, struct coord *roompos, s32 xi, s32 yi);
void artifactsUpdateGlaresForPlayer(struct model *gunmodel, struct model *handmodel, bool hand, f32 znear, f32 zfar);
#endif

#endif
