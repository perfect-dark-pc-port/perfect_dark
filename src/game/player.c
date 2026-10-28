#include <ultra64.h>
#include "constants.h"
#include "game/bondeyespy.h"
#include "game/bondmove.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/floor.h"
#include "game/inv.h"
#include "game/nbomb.h"
#include "game/title.h"
#include "game/chr.h"
#include "game/debug.h"
#include "game/body.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/objectives.h"
#include "game/atan2f.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/env.h"
#include "game/gunfx.h"
#include "game/game_0b0fd0.h"
#include "game/game_0b2150.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/modeldef.h"
#include "game/healthbar.h"
#include "game/hudmsg.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/file.h"
#include "game/filemgr.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/explosions.h"
#include "game/bondview.h"
#include "game/game_1531a0.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/room.h"
#include "game/gfxmemory.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/texdecompress.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/training.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "game/pak.h"
#include "game/options.h"
#include "game/propobj.h"
#include "game/splat.h"
#include "game/mpstats.h"
#include "bss.h"
#include "lib/ailist.h"
#include "lib/collision.h"
#include "lib/joy.h"
#include "lib/lib_17ce0.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#include "input.h"
#include "platform.h"
#endif

s32 g_DefaultWeapons[2];
f32 g_MpSwirlRotateSpeed;
f32 g_MpSwirlAngleDegrees;
f32 g_MpSwirlForwardSpeed;
f32 g_MpSwirlDistance;
s16 g_WarpType1Pad;
struct warpparams *g_WarpType2Params;
f32 g_WarpType3PosAngle;
f32 g_WarpType3RotAngle;
f32 g_WarpType3Range;
f32 g_WarpType3Height;
f32 g_WarpType3MoreHeight;
u32 g_WarpType3Pad;
s32 g_WarpType2HasDirection;
u32 g_WarpType2Arg2;
s32 g_CutsceneCurAnimFrame60;

#if VERSION == VERSION_JPN_FINAL
s32 g_CutsceneCurAnimFrame240;
s32 g_CutsceneFrameOverrun240;
s16 g_CutsceneAnimNum;
f32 g_CutsceneBlurFrac;
#elif PAL
f32 g_CutsceneCurAnimFrame240;
f32 var8009e388pf;
s16 g_CutsceneAnimNum;
f32 g_CutsceneBlurFrac;
#else
s32 g_CutsceneCurAnimFrame240;
s16 g_CutsceneAnimNum;
f32 g_CutsceneBlurFrac;
s32 g_CutsceneFrameOverrun240;
#endif

bool g_CutsceneSkipRequested;
f32 g_CutsceneCurTotalFrame60f;
s32 g_CutsceneTweenDuration60;
f32 g_CutsceneTweenFrac; // 0 when bars across the top and bottom, 1 when fullscreen
u32 var8009de34;
s16 g_SpawnPoints[24];
s32 g_NumSpawnPoints;

struct vimode g_ViModes[] = {
	// fbwidth
	// |               fbheight
	// |               |                 width
	// |               |                 |                yscale
	// |               |                 |                |                 xscale
	// |               |                 |                |                 |          fullheight
	// |               |                 |                |                 |          |                 fulltop
	// |               |                 |                |                 |          |                 |     wideheight
	// |               |                 |                |                 |          |                 |     |  widetop
	// |               |                 |                |                 |          |                 |     |  |     cinemaheight
	// |               |                 |                |                 |          |                 |     |  |     |  cinematop
	// |               |                 |                |                 |          |                 |     |  |     |  |
#if VERSION >= VERSION_JPN_FINAL
	{ SCREEN_WIDTH_LO, SCREEN_HEIGHT_LO, SCREEN_WIDTH_LO, 1,                VIMODE_LO, SCREEN_HEIGHT_LO, 0,  180, 20, 136, 42  }, // default
	{ SCREEN_WIDTH_HI, SCREEN_HEIGHT_HI, SCREEN_WIDTH_HI, 0.5,              VIMODE_LO, SCREEN_HEIGHT_HI, 0,  180, 20, 136, 42  }, // hi-res
#elif VERSION >= VERSION_PAL_BETA
	{ SCREEN_WIDTH_LO, SCREEN_HEIGHT_LO, SCREEN_WIDTH_LO, 1,                VIMODE_LO, SCREEN_HEIGHT_LO, 0,  212, 20, 168, 42 }, // default
	{ SCREEN_WIDTH_HI, SCREEN_HEIGHT_HI, SCREEN_WIDTH_HI, 0.71428567171097, VIMODE_LO, SCREEN_HEIGHT_HI, 0,  212, 20, 168, 42 }, // hi-res
#else
	{ SCREEN_WIDTH_LO, SCREEN_HEIGHT_LO, SCREEN_WIDTH_LO, 1,                VIMODE_LO, SCREEN_HEIGHT_LO, 0,  180, 20, 136, 42  }, // default
	{ SCREEN_WIDTH_HI, SCREEN_HEIGHT_HI, SCREEN_WIDTH_HI, 0.5,              VIMODE_LO, SCREEN_HEIGHT_HI, 0,  180, 20, 136, 42  }, // hi-res
	{ 320,             480,              320,             2,                VIMODE_HI, 440,              20, 360, 60, 272, 104 }, // unused
	{ 440,             330,              440,             1,                VIMODE_LO, 330,              0,  330, 0,  330, 0   }, // unused
	{ 440,             240,              440,             (1.0f / 1.375f),  VIMODE_LO, 220,              0,  180, 0,  136, 0   }, // unused
	{ 400,             300,              400,             1,                VIMODE_HI, 300,              0,  300, 0,  300, 0   }, // unused
#endif
};

s32 g_ViRes = VIRES_LO;
bool g_HiResEnabled = false;
u32 var800706d0 = 0x00000000;
u32 var800706d4 = 0x00000000;
u32 var800706d8 = 0x00000000;
u32 var800706dc = 0x00000000;
u32 var800706e0 = 0x00000000;
u32 var800706e4 = 0xbf800000;
u32 var800706e8 = 0x00000000;
u32 var800706ec = 0x3f800000;
u32 var800706f0 = 0x00000000;
u32 var800706f4 = 0x00000000;
u32 var800706f8 = 0x3f800000;
u32 var800706fc = 0x00000000;
u32 var80070700 = 0x00000000;
u32 var80070704 = 0x3f800000;
u32 var80070708 = 0x00000000;
u32 var8007070c = 0x00000000;
u32 var80070710 = 0x00000000;
u32 var80070714 = 0x00000000;
u32 var80070718 = 0x00000000;
u32 var8007071c = 0x00000000;
u32 var80070720 = 0x00000000;
u32 var80070724 = 0x00000000;
u32 var80070728 = 0x3f800000;
s32 var8007072c = 1;
u32 var80070730 = 0xffffffff;
u32 var80070734 = 0xffffffff;
u32 var80070738 = 0;
u32 var8007073c = 0;
struct gecreditsdata *g_CurrentGeCreditsData = NULL;
bool g_PlayerTriggerGeFadeIn = false;
u32 var80070748 = 0;
u32 var8007074c = 0;

bool g_PlayersWithControl[] = {
	true, true, true, true
};

bool g_PlayerInvincible = false;
s32 g_InCutscene = 0x00000000;

s16 g_DeathAnimations[] = {
	ANIM_DEATH_001A,
	ANIM_DEATH_001C,
	ANIM_DEATH_0020,
	ANIM_DEATH_0021,
	ANIM_DEATH_0022,
	ANIM_DEATH_0023,
	ANIM_DEATH_0024,
	ANIM_DEATH_0025,
	0,
};

s32 g_NumDeathAnimations = 0;

/**
 * Choose which location to spawn into from the given pads. Write the position
 * and rooms to the dstpos and dstrooms pointers and return the angle that the
 * player should be facing.
 *
 * It works by splitting each pad into one of three categories: good pads, bad
 * pads, and very bad pads. Categorisation logic is based on distances to enemy
 * chrs and room visibility. A shortlist of 4 pads is then created based on the
 * best pads, and a random pad is selected from the shortlist.
 *
 * @dangerous: If there are too many pads (24+) in the setup then array
 * overflows may occur.
 */
f32 playerChooseSpawnLocation(f32 chrradius, struct coord *dstpos, RoomNum *dstrooms, struct prop *prop, s16 *pads, s32 numpads)
{
	u8 verybadpads[24];
	u8 badpads[24];
	f32 padsqdists[24];

	u8 stack1[0x10];
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;
	f32 sqdist;

	// "sl" prefixes are for shortlist
	s16 slpadindexes[8];
	struct coord slpositions[4];
	RoomNum slrooms[4][8];
	f32 slangles[4];
	s32 sllen = 0;

	s32 i;
	s32 p;
	s32 playercount = PLAYERCOUNT();
	f32 dstangle;
	u8 stack2[0x10];
	struct pad pad;
	s32 stack3[2];
	RoomNum tmppadrooms[2];
	f32 bestsqdist;
#ifdef AVOID_UB
	RoomNum neighbours[21]; // prevent bgRoomGetNeighbours from writing out of bounds
#else
	RoomNum neighbours[20];
#endif

	// Iterate all spawn pads and populate the category arrays
	for (p = 0; p < numpads; p++) {
		bestsqdist = U32_MAX;
		padUnpack(pads[p], PADFIELD_POS | PADFIELD_ROOM, &pad);
		verybadpads[p] = false;
		badpads[p] = false;

		// Iterate players other than the one being spawned.
		// Note the closest chr's distance.
		// Decide whether the pad is considered to be ok, bad or very bad.
		for (i = 0; i < playercount; i++) {
			if (g_Vars.players[i]->prop
					&& g_Vars.players[i]->prop != prop
					&& (!prop || chrCompareTeams(prop->chr, g_Vars.players[i]->prop->chr, COMPARE_ENEMIES))) {
				xdiff = g_Vars.players[i]->prop->pos.x - pad.pos.x;
				ydiff = g_Vars.players[i]->prop->pos.y - pad.pos.y;
				zdiff = g_Vars.players[i]->prop->pos.z - pad.pos.z;

				sqdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

				if (sqdist < bestsqdist) {
					bestsqdist = sqdist;
				}

				if (bgRoomIsOnPlayerScreen(pad.room, i)) {
					verybadpads[p] = true;
				}

				if (verybadpads[p] || bgRoomIsOnPlayerStandby(pad.room, i)) {
					badpads[p] = true;
				}
			}
		}

		// Do the same as above, but for simulants
		tmppadrooms[0] = pad.room;
		tmppadrooms[1] = -1;

		bgRoomGetNeighbours(pad.room, neighbours, 20);

		for (i = 0; i < g_BotCount; i++) {
			if (g_MpBotChrPtrs[i]->prop
					&& g_MpBotChrPtrs[i]->prop != prop
					&& (!prop || chrCompareTeams(prop->chr, g_MpBotChrPtrs[i], COMPARE_ENEMIES))) {
				xdiff = g_MpBotChrPtrs[i]->prop->pos.x - pad.pos.x;
				ydiff = g_MpBotChrPtrs[i]->prop->pos.y - pad.pos.y;
				zdiff = g_MpBotChrPtrs[i]->prop->pos.z - pad.pos.z;

				sqdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

				if (sqdist < bestsqdist) {
					bestsqdist = sqdist;
				}

				if (arrayIntersects(tmppadrooms, g_MpBotChrPtrs[i]->prop->rooms)) {
					verybadpads[p] = true;
				}

				if (verybadpads[p] || arrayIntersects(neighbours, g_MpBotChrPtrs[i]->prop->rooms)) {
					badpads[p] = true;
				}
			}
		}

		padsqdists[p] = bestsqdist;
	}

	// Now start building the shortlist arrays.
	// Start with a random index into the array, then process it circularly
	// until the shortlist is full or a full iteration is done.
	// Look for pads that aren't bad (and therefore aren't very bad either) and
	// are at least 10m away. For each pad added, set their distance to -1 so
	// they don't get reused later.
	i = rngRandom() % numpads;
	p = i; \
	while (sllen < 4) {
		if (padsqdists[p] > 1000 * 1000 && !badpads[p]) {
			padUnpack(pads[p], PADFIELD_POS | PADFIELD_ROOM | PADFIELD_LOOK, &pad);

			slrooms[sllen][0] = pad.room;
			slrooms[sllen][1] = -1;

			slpositions[sllen].x = pad.pos.x;
			slpositions[sllen].y = pad.pos.y;
			slpositions[sllen].z = pad.pos.z;

			slangles[sllen] = atan2f(pad.look.x, pad.look.z);

#if VERSION >= VERSION_NTSC_1_0
			if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false, false)) {
				slpadindexes[sllen] = p;
				sllen++;
			}
#else
			if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false)) {
				slpadindexes[sllen] = p;
				sllen++;
			}
#endif

			padsqdists[p] = -1.0f;
		}

		p = (p + 1) % numpads;

		if (p == i) {
			break;
		}
	}

	// If the shortlist still has vacant slots, iterate the pads again but this
	// time take the bad pads. Keep the very bad pads out of contention for now.
	p = i = rngRandom() % numpads;

	while (sllen < 4) {
		if (padsqdists[p] > 1000 * 1000 && !verybadpads[p]) {
			padUnpack(pads[p], PADFIELD_POS | PADFIELD_ROOM | PADFIELD_LOOK, &pad);

			slrooms[sllen][0] = pad.room;
			slrooms[sllen][1] = -1;

			slpositions[sllen].x = pad.pos.x;
			slpositions[sllen].y = pad.pos.y;
			slpositions[sllen].z = pad.pos.z;

			slangles[sllen] = atan2f(pad.look.x, pad.look.z);

#if VERSION >= VERSION_NTSC_1_0
			if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false, false)) {
				slpadindexes[sllen] = p;
				sllen++;
			}
#else
			if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false)) {
				slpadindexes[sllen] = p;
				sllen++;
			}
#endif

			padsqdists[p] = -1.0f;
		}

		if (numpads);

		p = (p + 1) % numpads;

		if (p == i) {
			break;
		}
	}

	// If there's still vacancies in the shortlist, fill them using any
	// remaining pads in distance order.
	while (sllen < 4) {
		i = -1;
		bestsqdist = -1.0f;

		for (p = 0; p < numpads; p++) {
			if (padsqdists[p] > bestsqdist) {
				bestsqdist = padsqdists[p];
				i = p;
			}
		}

		// If there's no more pads, bail out of the loop
		if (i < 0) {
			break;
		}

		// If the pad with the furtherest chr is less than 2m away from that
		// chr, bail out of the loop provided there's at least something in the
		// shortlist.
		if (!(bestsqdist > 200 * 200) && sllen != 0) {
			break;
		}

		// Add this pad to the shortlist
		padUnpack(pads[i], PADFIELD_POS | PADFIELD_ROOM | PADFIELD_LOOK, &pad);

		slrooms[sllen][0] = pad.room;
		slrooms[sllen][1] = -1;

		slpositions[sllen].x = pad.pos.x;
		slpositions[sllen].y = pad.pos.y;
		slpositions[sllen].z = pad.pos.z;

		slangles[sllen] = atan2f(pad.look.x, pad.look.z);

#if VERSION >= VERSION_NTSC_1_0
		if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false, false)) {
			slpadindexes[sllen] = i;
			sllen++;
		}
#else
		if (chrAdjustPosForSpawn(chrradius, &slpositions[sllen], slrooms[sllen], slangles[sllen], true, false)) {
			slpadindexes[sllen] = i;
			sllen++;
		}
#endif

		padsqdists[i] = -1.0f;
	}

	// Finally, choose a random pad from the shortlist
	if (sllen > 0) {
		p = rngRandom() % sllen;

		dstpos->x = slpositions[p].x;
		dstpos->y = slpositions[p].y;
		dstpos->z = slpositions[p].z;

		roomsCopy(slrooms[p], dstrooms);

		dstangle = slangles[p];
	} else {
		// No shortlisted pads, so pick a random one from the full selection
		padUnpack(pads[rngRandom() % numpads], PADFIELD_POS | PADFIELD_LOOK | PADFIELD_ROOM, &pad);

		dstrooms[0] = pad.room;
		dstrooms[1] = -1;

		dstpos->x = pad.pos.x;
		dstpos->y = pad.pos.y;
		dstpos->z = pad.pos.z;

		dstangle = atan2f(pad.look.x, pad.look.z);
	}

	return dstangle;
}

f32 playerChooseGeneralSpawnLocation(f32 chrradius, struct coord *pos, RoomNum *rooms, struct prop *prop)
{
	return playerChooseSpawnLocation(chrradius, pos, rooms, prop, g_SpawnPoints, g_NumSpawnPoints);
}

void playerStartNewLife(void)
{
	struct coord pos = {0, 0, 0};
	RoomNum rooms[8];
	f32 angle;
	s32 *cmd = g_StageSetup.intro;
	f32 groundy;
	s32 i;

	pakEnableRumbleForPlayer(g_Vars.currentplayernum);

	g_Vars.currentplayer->dostartnewlife = false;

	if (g_Vars.coopplayernum < 0) {
		struct prop *prop = g_Vars.currentplayer->prop->child;

		while (prop) {
			struct defaultobj *obj = prop->obj;

			if (obj) {
				obj->hidden |= OBJHFLAG_DELETING;
			}

			prop = prop->next;
		}
	}

	splatResetChr(g_Vars.currentplayer->prop->chr);
	playerLoadDefaults();
	g_Vars.currentplayer->isdead = false;
	g_Vars.currentplayer->healthdamagetype = DAMAGETYPE_7;
	g_Vars.currentplayer->damagetype = DAMAGETYPE_7;
	g_Vars.currentplayer->gunammooff = 0;
	g_Vars.currentplayer->gunsightoff = 2;
#ifndef PLATFORM_N64
	g_Vars.currentplayer->prop->chr->blurdrugamount = 0;
	g_Vars.currentplayer->prop->chr->poisoncounter = 0;
#endif

	hudmsgsSetOn(0xffffffff);

	angle = M_BADTAU - scenarioChooseSpawnLocation(30, &pos, rooms, g_Vars.currentplayer->prop); // var7f1ad534

	groundy = cdFindGroundInfoAtCyl(&pos, 30, rooms,
			&g_Vars.currentplayer->floorcol,
			&g_Vars.currentplayer->floortype,
			&g_Vars.currentplayer->floorflags,
			&g_Vars.currentplayer->floorroom,
			NULL, NULL);

	pos.y = groundy + g_Vars.currentplayer->vv_eyeheight;

	g_Vars.currentplayer->vv_manground = groundy;
	g_Vars.currentplayer->vv_theta = angle * 360.0f / M_BADTAU;
	g_Vars.currentplayer->vv_ground = groundy;

	playerResetBond(&g_Vars.currentplayer->bond2, &pos);

	g_Vars.currentplayer->bond2.unk00.x = -sinf(angle);
	g_Vars.currentplayer->bond2.unk00.y = 0;
	g_Vars.currentplayer->bond2.unk00.z = cosf(angle);

	g_Vars.currentplayer->prop->pos.f[0] = g_Vars.currentplayer->bondprevpos.f[0] = pos.f[0];
	g_Vars.currentplayer->prop->pos.f[1] = g_Vars.currentplayer->bondprevpos.f[1] = pos.f[1];
	g_Vars.currentplayer->prop->pos.f[2] = g_Vars.currentplayer->bondprevpos.f[2] = pos.f[2];

	propDeregisterRooms(g_Vars.currentplayer->prop);

	g_Vars.currentplayer->prop->rooms[0] = rooms[0];
	g_Vars.currentplayer->prop->rooms[1] = -1;

	playerSetCamPropertiesWithRoom(&pos, &g_Vars.currentplayer->bond2.unk28,
			&g_Vars.currentplayer->bond2.unk1c, rooms[0]);

	if (g_Vars.coopplayernum >= 0) {
		u32 stack;
		bool ammotypesheld[33];
		s32 stack2[2];

		for (i = 0; i != ARRAYCOUNT(ammotypesheld); i++) {
			ammotypesheld[i] = false;
		}

		for (i = 1; i != ARRAYCOUNT(g_Weapons); i++) {
			if (invHasSingleWeaponOrProp(i)) {
				s32 ammotype = bgunGetAmmoTypeForWeapon(i, FUNC_PRIMARY);

				if (ammotype >= 0 && ammotype <= AMMOTYPE_ECM_MINE) {
					ammotypesheld[ammotype] = true;
				}
			}
		}

		for (i = 0; i != ARRAYCOUNT(ammotypesheld); i++) {
			if (ammotypesheld[i] == false) {
				g_Vars.currentplayer->ammoheldarr[i] = 0;
			}
		}
	} else {
		invClear();

		for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->ammoheldarr); i++) {
			g_Vars.currentplayer->ammoheldarr[i] = 0;
		}
	}

	invGiveSingleWeapon(WEAPON_UNARMED);

	if (cmd) {
		if (cmd);
		if (cmd);
		if (cmd);
		if (cmd);
		if (cmd);
		if (cmd);

		if (g_Vars.antiplayernum < 0 || g_Vars.currentplayer != g_Vars.anti) {
			while (cmd[0] != INTROCMD_END) {
				switch (cmd[0]) {
				case INTROCMD_SPAWN:
					cmd += 3;
					break;
				case INTROCMD_CASE:
				case INTROCMD_CASERESPAWN:
					cmd += 3;
					break;
				case INTROCMD_HILL:
					cmd += 2;
					break;
				case INTROCMD_WEAPON:
					if (cmd[3] == 0) {
						if (cmd[2] >= 0) {
							invGiveDoubleWeapon(cmd[1], cmd[2]);
						} else {
							invGiveSingleWeapon(cmd[1]);
						}
					}
					cmd += 4;
					break;
				case INTROCMD_AMMO:
					if (cmd[3] == 0) {
						bgunSetAmmoQuantity(cmd[1], cmd[2]);
					}
					cmd += 4;
					break;
				case INTROCMD_3:
					cmd += 8;
					break;
				case INTROCMD_4:
					cmd += 2;
					break;
				case INTROCMD_OUTFIT:
					cmd += 2;
					break;
				case INTROCMD_6:
					cmd += 10;
					break;
				default:
					cmd++;
					break;
				}
			}
		}
	}

	if (g_Vars.coopplayernum >= 0 && g_Vars.currentplayer->stealhealth > 0) {
		g_Vars.currentplayer->bondhealth = g_Vars.currentplayer->stealhealth;
		g_Vars.currentplayer->oldhealth = 0;
		g_Vars.currentplayer->oldarmour = 0;
		g_Vars.currentplayer->apparenthealth = 0;
		g_Vars.currentplayer->apparentarmour = 0;
	}

	bmoveUpdateRooms(g_Vars.currentplayer);
	playerSpawn();

	if (g_Vars.normmplayerisrunning) {
		playerStartChrFade(120, 1);
	} else {
		playerStartChrFade(0, 1);
	}

	if (g_Vars.currentplayer->prop->chr) {
		g_Vars.currentplayer->prop->chr->chrflags &= ~CHRCFLAG_HIDDEN;
	}
}

void playerLoadDefaults(void)
{
	if (!g_Vars.mplayerisrunning || g_Vars.currentplayer->model00d4 == NULL) {
		g_Vars.currentplayer->vv_eyeheight = 159;
		g_Vars.currentplayer->vv_headheight = 172;
	}

	g_Vars.currentplayer->globaldrawworldoffset.x = 0;
	g_Vars.currentplayer->globaldrawworldoffset.y = 0;
	g_Vars.currentplayer->globaldrawworldoffset.z = 0;
	g_Vars.currentplayer->globaldrawcameraoffset.x = 0;
	g_Vars.currentplayer->globaldrawcameraoffset.y = 0;
	g_Vars.currentplayer->globaldrawcameraoffset.z = 0;
	g_Vars.currentplayer->globaldrawworldbgoffset.x = 0;
	g_Vars.currentplayer->globaldrawworldbgoffset.y = 0;
	g_Vars.currentplayer->globaldrawworldbgoffset.z = 0;

	g_Vars.currentplayer->cameramode = CAMERAMODE_DEFAULT;
	g_Vars.currentplayer->memcampos.x = 0;
	g_Vars.currentplayer->memcampos.y = 0;
	g_Vars.currentplayer->memcampos.z = 0;
	g_Vars.currentplayer->memcamroom = -1;

	g_Vars.currentplayer->bondmovemode = -1;
	g_Vars.currentplayer->walkinitmove = 0;

	bmoveSetMode(MOVEMODE_WALK);

	g_Vars.currentplayer->bondperimenabled = true;
	g_Vars.currentplayer->periminfo.header.type = GEOTYPE_CYL;
	g_Vars.currentplayer->periminfo.header.flags = GEOFLAG_WALL | GEOFLAG_BLOCK_SHOOT;
	g_Vars.currentplayer->periminfo.ymax = 0;
	g_Vars.currentplayer->periminfo.ymin = 0;
	g_Vars.currentplayer->periminfo.x = 0;
	g_Vars.currentplayer->periminfo.z = 0;
	g_Vars.currentplayer->periminfo.radius = 0;
	g_Vars.currentplayer->bondactivateorreload = false;
	g_Vars.currentplayer->isdead = false;

	if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_DUEL) {
		g_Vars.currentplayer->bondhealth = 0.01f;
	} else if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MAIANSOS) {
		g_Vars.currentplayer->bondhealth = 0.5f;
	} else {
		g_Vars.currentplayer->bondhealth = 1;
	}

	g_Vars.currentplayer->oldhealth = 1;
	g_Vars.currentplayer->oldarmour = 0;
	g_Vars.currentplayer->apparenthealth = 1;
	g_Vars.currentplayer->apparentarmour = 0;
	g_Vars.currentplayer->damageshowtime = -1;
	g_Vars.currentplayer->healthshowtime = -1;
	g_Vars.currentplayer->shieldshowtime = -1;
	g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_HIDDEN;
	g_Vars.currentplayer->bondbreathing = 0;
	g_Vars.currentplayer->speedtheta = 0;
	g_Vars.currentplayer->speedthetacontrol = 0;
	g_Vars.currentplayer->vv_costheta = 1;
	g_Vars.currentplayer->vv_sintheta = 0;
	g_Vars.currentplayer->vv_verta = -4;
	g_Vars.currentplayer->vv_verta360 = g_Vars.currentplayer->vv_verta;

	if (g_Vars.currentplayer->vv_verta360 < 0) {
		g_Vars.currentplayer->vv_verta360 += 360;
	}

	g_Vars.currentplayer->speedverta = 0;
	g_Vars.currentplayer->vv_cosverta = 1;
	g_Vars.currentplayer->vv_sinverta = 0;
	g_Vars.currentplayer->bondshotspeed.x = 0;
	g_Vars.currentplayer->bondshotspeed.y = 0;
	g_Vars.currentplayer->bondshotspeed.z = 0;

	g_Vars.currentplayer->docentreupdown = 0;
	g_Vars.currentplayer->lastupdown60 = 0;
	g_Vars.currentplayer->prevupdown = 0;
	g_Vars.currentplayer->movecentrerelease = 0;
	g_Vars.currentplayer->lookaheadcentreenabled = true;
	g_Vars.currentplayer->automovecentreenabled = true;
	g_Vars.currentplayer->fastmovecentreenabled = false;
	g_Vars.currentplayer->automovecentre = true;
	g_Vars.currentplayer->insightaimmode = false;

	g_Vars.currentplayer->autoyaimenabled = true;
	g_Vars.currentplayer->autoaimy = 0;
	g_Vars.currentplayer->autoyaimprop = NULL;
	g_Vars.currentplayer->autoyaimtime60 = -1;

	g_Vars.currentplayer->autoxaimenabled = true;
	g_Vars.currentplayer->autoaimx = 0;
	g_Vars.currentplayer->autoxaimprop = NULL;
	g_Vars.currentplayer->autoxaimtime60 = -1;

	g_Vars.currentplayer->autoaimdamp = (PAL ? 0.974f : 0.979f);

	g_Vars.currentplayer->colourscreenred = 0xff;
	g_Vars.currentplayer->colourscreengreen = 0xff;
	g_Vars.currentplayer->colourscreenblue = 0xff;
	g_Vars.currentplayer->colourscreenfrac = 0;
	g_Vars.currentplayer->colourfadetime60 = -1;
	g_Vars.currentplayer->colourfadetimemax60 = -1;
	g_Vars.currentplayer->colourfaderedold = 0xff;
	g_Vars.currentplayer->colourfaderednew = 0xff;
	g_Vars.currentplayer->colourfadegreenold = 0xff;
	g_Vars.currentplayer->colourfadegreennew = 0xff;
	g_Vars.currentplayer->colourfadeblueold = 0xff;
	g_Vars.currentplayer->colourfadebluenew = 0xff;
	g_Vars.currentplayer->colourfadefracold = 0;
	g_Vars.currentplayer->colourfadefracnew = 0;

	g_Vars.currentplayer->bondfadetime60 = -1;
	g_Vars.currentplayer->bondfadetimemax60 = -1;
	g_Vars.currentplayer->bondfadefracold = 0;
	g_Vars.currentplayer->bondfadefracnew = 0;

	g_Vars.currentplayer->controldef = 2;
	g_Vars.currentplayer->bondleandown = 15;
	g_Vars.currentplayer->shootrotx = 0;
	g_Vars.currentplayer->shootroty = 0;
	g_Vars.currentplayer->inlift = false;
	g_Vars.currentplayer->lift = NULL;
	g_Vars.currentplayer->onladder = false;

	g_Vars.currentplayer->eyesshut = false;
	g_Vars.currentplayer->eyesshutfrac = 0;

	g_Vars.currentplayer->waitforzrelease = false;
	g_Vars.currentplayer->devicesactive = 0;
	g_Vars.currentplayer->commandingaibot = NULL;
	g_Vars.currentplayer->deadtimer = -1;
	g_Vars.currentplayer->coopcanrestart = false;

	g_Vars.currentplayer->usinggoggles = false;
	g_Vars.currentplayer->nvhum = NULL;
	g_Vars.currentplayer->nvoverload = NULL;
	g_Vars.currentplayer->overexposurered = 0;
	g_Vars.currentplayer->overexposuregreen = 0;
	g_Vars.currentplayer->overexposureblue = 0;
	g_Vars.currentplayer->prevoverexposurered = 0;
	g_Vars.currentplayer->prevoverexposuregreen = 0;
	g_Vars.currentplayer->prevoverexposureblue = 0;
	g_Vars.currentplayer->amdowntime = 0;
	g_Vars.currentplayer->altdowntime = 0;
}

bool playerSpawnAnti(struct chrdata *hostchr, bool force)
{
	struct prop *hostprop;
	union modelrwdata *chrrootrwdata;
	struct chrdata *playerchr = g_Vars.currentplayer->prop->chr;
	union modelrwdata *playerrootrwdata;

	hostprop = hostchr->prop;

	hostchr->chrflags |= CHRCFLAG_PERIMDISABLEDTMP;
	playerchr->hidden |= CHRHFLAG_WARPONSCREEN;
	playerchr->radius = hostchr->radius;

	if (chrMoveToPos(playerchr, &hostchr->prop->pos, hostchr->prop->rooms, chrGetInverseTheta(hostchr), false) || force) {
		if (hostchr->weapons_held[0] && hostchr->weapons_held[1]) {
			// Dual wielding
			struct weaponobj *weapon1 = hostchr->weapons_held[0]->weapon;
			struct weaponobj *weapon2 = hostchr->weapons_held[1]->weapon;

#if VERSION >= VERSION_NTSC_1_0
			invGiveSingleWeapon(weapon1->weaponnum);
			invGiveDoubleWeapon(weapon1->weaponnum, weapon1->weaponnum);
			bgunEquipWeapon2(HAND_RIGHT, weapon1->weaponnum);
			bgunEquipWeapon2(HAND_LEFT, weapon1->weaponnum);
#else
			invGiveDoubleWeapon(weapon1->weaponnum, weapon2->weaponnum);
			bgunEquipWeapon2(HAND_RIGHT, weapon1->weaponnum);
			bgunEquipWeapon2(HAND_LEFT, weapon2->weaponnum);
#endif
		} else if (hostchr->weapons_held[0]) {
			// Right hand only
			struct weaponobj *weapon = hostchr->weapons_held[0]->weapon;

			if (weapon->weaponnum == WEAPON_SUPERDRAGON) {
				invGiveSingleWeapon(WEAPON_DRAGON);
				bgunEquipWeapon2(HAND_RIGHT, WEAPON_DRAGON);
			} else {
				invGiveSingleWeapon(weapon->weaponnum);
				bgunEquipWeapon2(HAND_RIGHT, weapon->weaponnum);
			}
		} else if (hostchr->weapons_held[1]) {
			// Left hand only
			struct weaponobj *weapon = hostchr->weapons_held[1]->weapon;

			if (weapon->weaponnum == WEAPON_SUPERDRAGON) {
				invGiveSingleWeapon(WEAPON_DRAGON);
				bgunEquipWeapon2(HAND_RIGHT, WEAPON_DRAGON);
			} else {
				invGiveSingleWeapon(weapon->weaponnum);
				bgunEquipWeapon2(HAND_RIGHT, weapon->weaponnum);
			}
		} else {
			// Unarmed
			invGiveSingleWeapon(WEAPON_UNARMED);
			bgunEquipWeapon2(HAND_RIGHT, WEAPON_UNARMED);
		}

		g_Vars.currentplayer->invdowntime = TICKS(-40);
		g_Vars.currentplayer->usedowntime = TICKS(-40);

		bgunGiveMaxAmmo(true);

		g_Vars.currentplayer->bondhealth = (chrGetMaxDamage(hostchr) - hostchr->damage) * 0.125f;

		if (g_Vars.currentplayer->bondhealth > 1) {
			g_Vars.currentplayer->bondhealth = 1;
		}

		chrSetShield(playerchr, chrGetShield(hostchr));

		g_Vars.currentplayer->haschrbody = false;
		g_Vars.currentplayer->model00d4 = NULL;

		chrRemove(g_Vars.currentplayer->prop, false);

		if (hostchr->bodynum == BODY_SKEDAR) {
			g_Vars.antiheadnum = HEAD_MRBLONDE;
			g_Vars.antibodynum = BODY_MRBLONDE;
		} else {
			g_Vars.antiheadnum = hostchr->headnum;
			g_Vars.antibodynum = hostchr->bodynum;
		}

		playerTickChrBody();
		modelCopyAnimData(hostchr->model, playerchr->model);
		func0f02e9a0(playerchr, 12);

		chrrootrwdata = modelGetNodeRwData(hostchr->model, hostchr->model->definition->rootnode);
		playerrootrwdata = modelGetNodeRwData(playerchr->model, playerchr->model->definition->rootnode);

		playerrootrwdata->chrinfo = chrrootrwdata->chrinfo;

		if (playerrootrwdata->chrinfo.unk34.y < 10) {
			playerrootrwdata->chrinfo.unk34.y = 10;
		}

		if (playerrootrwdata->chrinfo.unk24.y < 10) {
			playerrootrwdata->chrinfo.unk24.y = 10;
		}

		playerchr->radius = hostchr->radius;
		g_Vars.currentplayer->bond2.radius = hostchr->radius;

		chrRemove(hostprop, true);
		propDeregisterRooms(hostprop);
		propDelist(hostprop);
		propDisable(hostprop);
		propFree(hostprop);

		return true;
	}

	hostchr->chrflags &= ~CHRCFLAG_PERIMDISABLEDTMP;

	return false;
}

void playerSpawn(void)
{
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;
	f32 sqdist;
	struct chrdata *sortedchrs[10];
	f32 sorteddists[10];
	struct chrdata *tmpchr;
	s32 i;
	s32 j;
	s32 k;
	bool force;
	s32 numsqdists;
	struct coord sp9c;
	struct coord sp90;
	struct coord sp84;
	struct coord sp78;

	g_Vars.currentplayer->deathanimfinished = false;
	g_Vars.currentplayer->redbloodfinished = false;
	g_Vars.currentplayer->startnewbonddie = true;
	g_Vars.currentplayer->killsthislife = 0;

	g_Vars.currentplayer->lifestarttime60 = playerGetMissionTime();
	g_Vars.currentplayer->healthdisplaytime60 = 0;

	invGiveSingleWeapon(WEAPON_UNARMED);
	playerSetShieldFrac(0);

	if (cheatIsActive(CHEAT_JOSHIELD)) {
		playerSetShieldFrac(1);
	}

	if (cheatIsActive(CHEAT_SUPERSHIELD)) {
		playerSetShieldFrac(1);
		g_Vars.currentplayer->armourscale = 2;
	}

	if (g_Vars.mplayerisrunning) {
		if (g_Vars.antiplayernum >= 0 && g_Vars.currentplayer == g_Vars.anti) {
			numsqdists = 0;
			force = false;

			invGiveSingleWeapon(WEAPON_SUICIDEPILL);
			bgunEquipWeapon2(HAND_LEFT, WEAPON_NONE);
			bgunEquipWeapon2(HAND_RIGHT, WEAPON_UNARMED);

			if (g_Vars.lvframenum > 0) {
				s32 prevplayernum = g_Vars.currentplayernum;
				setCurrentPlayerNum(g_Vars.bondplayernum);
				bgun0f0a0c08(&sp84, &sp9c);
				mtx4RotateVec(camGetProjectionMtxF(), &sp9c, &sp90);
				mtx4TransformVec(camGetProjectionMtxF(), &sp84, &sp78);
				setCurrentPlayerNum(prevplayernum);
			}

			if (g_Vars.currentplayer->model00d4 == NULL) {
				playerTickChrBody();
			}

			for (i = 0; i < chrsGetNumSlots(); i++) {
				if (g_ChrSlots[i].model
						&& g_ChrSlots[i].prop
						&& (g_ChrSlots[i].hidden & CHRHFLAG_BASICGUARD)
						&& (g_ChrSlots[i].chrflags & CHRCFLAG_HIDDEN) == 0
						&& g_ChrSlots[i].prop->type == PROPTYPE_CHR
						&& !chrIsDead(&g_ChrSlots[i])
						&& (g_ChrSlots[i].prop->flags & PROPFLAG_ENABLED)) {
					if (g_Vars.bond->prop) {
						xdiff = g_ChrSlots[i].prop->pos.x - g_Vars.bond->prop->pos.x;
						ydiff = g_ChrSlots[i].prop->pos.y - g_Vars.bond->prop->pos.y;
						zdiff = g_ChrSlots[i].prop->pos.z - g_Vars.bond->prop->pos.z;
					} else {
						xdiff = g_ChrSlots[i].prop->pos.x - g_Vars.currentplayer->prop->pos.x;
						ydiff = g_ChrSlots[i].prop->pos.y - g_Vars.currentplayer->prop->pos.y;
						zdiff = g_ChrSlots[i].prop->pos.z - g_Vars.currentplayer->prop->pos.z;
					}

					sqdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

					if (g_Vars.lvframenum > 0
							&& (g_ChrSlots[i].hidden & CHRHFLAG_ONBONDSSCREEN)
							&& func0f06b39c(&sp78, &sp90, &g_ChrSlots[i].prop->pos, modelGetEffectiveScale(g_ChrSlots[i].model))
							&& (rngRandom() % 8)) {
						sqdist += 1000 * 1000;
					}

					// Insert sqdist to sorteddists, maintaining sort order,
					// and mirror the changes into the sortedchrs array.

					// Move j to the first sqdist that is further than the new one
					for (j = 0; j < numsqdists; j++) {
						if (sqdist < sorteddists[j]) {
							break;
						}
					}

					if (j < 10) {
						// Move the higher sorteddists forward, removing the highest item
						for (k = numsqdists; k > j; k--) {
							if (k < 10) {
								sortedchrs[k] = sortedchrs[k - 1];
								sorteddists[k] = sorteddists[k - 1];
							}
						}

						// Write new sqdist
						sortedchrs[j] = &g_ChrSlots[i];
						sorteddists[j] = sqdist;

						if (numsqdists < 9) {
							numsqdists++;
						}
					}
				}
			}

			// Randomly swap some of the earlier elements so the player
			// doesn't always spawn into the closest
			if (numsqdists > 1 && (rngRandom() % 2) == 0) {
				tmpchr = sortedchrs[0];
				sqdist = sorteddists[0];
				sortedchrs[0] = sortedchrs[1];
				sorteddists[0] = sorteddists[1];
				sortedchrs[1] = tmpchr;
				sorteddists[1] = sqdist;
			}

			if (numsqdists > 2 && (rngRandom() % 4) == 0) {
				tmpchr = sortedchrs[0];
				sqdist = sorteddists[0];
				sortedchrs[0] = sortedchrs[2];
				sorteddists[0] = sorteddists[2];
				sortedchrs[2] = tmpchr;
				sorteddists[2] = sqdist;
			}

			// Iterate sortedchrs in order and try to spawn into any of them.
			// The spawn may fail if the chr is on-screen, and potentially in
			// some other conditions such as the chr being too close to a wall.
			// If no chrs can be spawned into, iterate the list again but this
			// time allowing the spawn to happen on-screen (force = true).
			for (i = 0; i < numsqdists; i++) {
				if (playerSpawnAnti(sortedchrs[i], force)) {
					break;
				}

				if (i == numsqdists - 1) {
					i = 0;

					if (force) {
						break;
					}

					force = true;
				}
			}

			if (g_Vars.currentplayer->prop->chr) {
				g_Vars.currentplayer->prop->chr->blurdrugamount = 0;
				g_Vars.currentplayer->prop->chr->blurnumtimesdied = 0;
			}
		} else {
#ifndef PLATFORM_N64
			if (cheatIsActive(CHEAT_CLOAKINGDEVICE)) {
				invGiveSingleWeapon(WEAPON_CLOAKINGDEVICE);
#if VERSION >= VERSION_PAL_FINAL
				bgunSetAmmoQuantity(AMMOTYPE_CLOAK, TICKS(7200));
#else
				bgunSetAmmoQuantity(AMMOTYPE_CLOAK, 7200);
#endif
			}

			if (cheatIsActive(CHEAT_PERFECTDARKNESS)) {
				invGiveSingleWeapon(WEAPON_NIGHTVISION);
			}

			if (g_Vars.normmplayerisrunning
					&& (g_MpSetup.options & MPOPTION_SPAWNWITHWEAPON)
					&& g_MpSetup.weapons[0] != MPWEAPON_NONE
					&& g_MpSetup.weapons[0] != MPWEAPON_DISABLED
					&& g_MpSetup.weapons[0] != MPWEAPON_SHIELD) {
				struct mpweapon *mpweapon = &g_MpWeapons[g_MpSetup.weapons[0]];
				invGiveSingleWeapon(mpweapon->weaponnum);
				const s32 ammotype = (g_MpSetup.weapons[0] == MPWEAPON_COMBATBOOST) ? AMMOTYPE_BOOST : mpweapon->priammotype;
				if (ammotype) {
					s32 startammo = mpweapon->priammoqty / 2;
					if (startammo == 0) {
						startammo = 1;
					}
					bgunSetAmmoQuantity(ammotype, startammo);
				}
				bgunEquipWeapon2(HAND_LEFT, WEAPON_NONE);
				bgunEquipWeapon2(HAND_RIGHT, mpweapon->weaponnum);
			} else
#endif
			{
				bgunEquipWeapon2(HAND_LEFT, g_DefaultWeapons[HAND_LEFT]);
				bgunEquipWeapon2(HAND_RIGHT, g_DefaultWeapons[HAND_RIGHT]);
			}

#if VERSION >= VERSION_NTSC_1_0
			if (g_Vars.currentplayer->model00d4 == NULL
					&& (IS8MB() || g_Vars.fourmeg2player || g_MpAllChrPtrs[g_Vars.currentplayernum] == NULL)) {
				playerTickChrBody();
			}
#else
			if (g_Vars.currentplayer->model00d4 == NULL) {
				playerTickChrBody();
			}
#endif
		}
	}

	playerUpdatePerimInfo();
}

void playerResetBond(struct playerbond *pb, struct coord *pos)
{
	pb->unk10.x = pos->x;
	pb->unk10.y = pos->y;
	pb->unk10.z = pos->z;

	pb->unk1c.x = 1;
	pb->unk1c.y = 0;
	pb->unk1c.z = 0;

	pb->unk28.x = 0;
	pb->unk28.y = 1;
	pb->unk28.z = 0;

	pb->unk00.x = 0;
	pb->unk00.y = 0;
	pb->unk00.z = 1;

	pb->radius = 30;
}

void playersTickAllChrBodies(void)
{
	s32 prevplayernum = g_Vars.currentplayernum;
	s32 i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);
		playerTickChrBody();
	}

	setCurrentPlayerNum(prevplayernum);
}

void playerChooseBodyAndHead(s32 *bodynum, s32 *headnum, s32 *arg2)
{
	s32 outfit;
	bool solo;

	if (g_Vars.antiplayernum >= 0
			&& g_Vars.currentplayer == g_Vars.anti
			&& g_Vars.antiheadnum >= 0
			&& g_Vars.antibodynum >= 0) {
		*headnum = g_Vars.antiheadnum;
		*bodynum = g_Vars.antibodynum;
		return;
	}

	if (g_Vars.normmplayerisrunning) {
		if (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.mpheadnum < mpGetNumHeads2()) {
			*headnum = mpGetHeadId(g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.mpheadnum);
		} else {
			*headnum = g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.mpheadnum - mpGetNumHeads2();

			if (arg2) {
				*arg2 = true;
			}
		}

		*bodynum = mpGetBodyId(g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.mpbodynum);
		return;
	}

	outfit = g_Vars.currentplayer->bondtype;
	solo = !(g_Vars.coopplayernum >= 0) || (g_Vars.currentplayer != g_Vars.coop);

	if (cheatIsActive(CHEAT_PLAYASELVIS)) {
		*bodynum = BODY_THEKING;
		*headnum = HEAD_ELVIS;
		return;
	}

	if (g_Vars.stagenum == STAGE_VILLA && lvGetDifficulty() >= DIFF_PA) {
		outfit = OUTFIT_NEGOTIATOR;
	}

	if (g_Vars.currentplayer->disguised) {
		switch (g_Vars.stagenum) {
		case STAGE_RESCUE:  outfit = OUTFIT_LAB; break;
		case STAGE_AIRBASE: outfit = OUTFIT_STEWARDESS; break;
		}
	}

	switch (outfit) {
	default:
	case OUTFIT_DEFAULT:
		*bodynum = BODY_DARK_COMBAT;
		*headnum = solo ? HEAD_DARK_COMBAT : HEAD_VD;
		break;
	case OUTFIT_ELVIS:
		*bodynum = BODY_THEKING;
		*headnum = solo ? HEAD_ELVIS : HEAD_ELVIS;
		break;
	case OUTFIT_TRENT:
		*bodynum = BODY_TRENT;
		*headnum = solo ? HEAD_TRENT : HEAD_TRENT;
		break;
	case OUTFIT_TRENCH:
		*bodynum = BODY_DARK_TRENCH;
		*headnum = solo ? HEAD_DARK_COMBAT : HEAD_VD;
		break;
	case OUTFIT_FROCK_RIPPED:
		*bodynum = BODY_DARK_RIPPED;
		*headnum = solo ? HEAD_DARK_FROCK : HEAD_VD;
		break;
	case OUTFIT_FROCK:
		*bodynum = BODY_DARK_FROCK;
		*headnum = solo ? HEAD_DARK_FROCK : HEAD_VD;
		break;
	case OUTFIT_LEATHER:
		*bodynum = BODY_DARK_LEATHER;
		*headnum = solo ? HEAD_DARK_COMBAT : HEAD_VD;
		break;
	case OUTFIT_DEEPSEA:
		*bodynum = BODY_DARKWET;
		*headnum = solo ? HEAD_DARK_COMBAT : HEAD_VD;
		break;
	case OUTFIT_WETSUIT:
		*bodynum = BODY_DARKAQUALUNG;
		*headnum = solo ? HEAD_DARKAQUA : HEAD_VD;
		break;
	case OUTFIT_SNOW:
		*bodynum = BODY_DARKSNOW;
		*headnum = solo ? HEAD_DARK_SNOW : HEAD_VD;
		break;
	case OUTFIT_LAB:
		*bodynum = BODY_DARKLAB;
		*headnum = solo ? HEAD_DARK_COMBAT : HEAD_VD;
		break;
	case OUTFIT_STEWARDESS:
		*bodynum = BODY_DARK_AF1;
		*headnum = solo ? HEAD_DARK_FROCK : HEAD_VD;
		break;
	case OUTFIT_NEGOTIATOR:
		*bodynum = BODY_DARK_NEGOTIATOR;
		*headnum = solo ? HEAD_DARK_FROCK : HEAD_VD;
		break;
	case OUTFIT_MRBLONDE:
		*bodynum = BODY_MRBLONDE;
		*headnum = solo ? HEAD_MRBLONDE : HEAD_MRBLONDE;
		break;
	case OUTFIT_MAIAN:
		*bodynum = BODY_ELVIS1;
		*headnum = solo ? HEAD_MAIAN_S : HEAD_MAIAN_S;
		break;
	}
}

/**
 * Ensure the chr's "chrbody" is set up, then tick it.
 *
 * The majority of this function is code that sets up the chrbody. The chrbody
 * is their model, weapon and animation as seen from a third person perspective.
 * It's needed for cutscenes, and in some cases during gameplay such as when
 * using the eyespy or flying a Slayer rocket.
 *
 * When using 1 player, these allocations are made out of "gunmem", which is a
 * single allocation that's assumed to be big enough.
 *
 * When using 2-4 players, gunmem is not used here. This might be because all
 * these structures are already allocated elsewhere in memory due to the two
 * players being able to see each other at any time.
 */
void playerTickChrBody(void)
{
	f32 turnangle = (360.0f - g_Vars.currentplayer->vv_theta) * M_BADTAU / 360.0f;

	if (g_Vars.currentplayer->haschrbody == false) {
		struct chrdata *chr;
		struct texpool texpool;
		struct modeldef *bodymodeldef;
		struct modeldef *headmodeldef = NULL;
		struct modeldef *weaponmodeldef;
		s32 offset1 = 0;
		u8 *allocation;
		void *spe8;
		s32 offset2;
		u32 stack2;
		struct weaponobj *weaponobj;

		// Unused
		struct weaponobj template = {
			256,                    // extrascale
			0,                      // hidden2
			OBJTYPE_WEAPON,         // type
			MODEL_CHRFALCON2,       // modelnum
			-1,                     // pad
			OBJFLAG_ASSIGNEDTOCHR,  // flags
			0,                      // flags2
			0,                      // flags3
			NULL,                   // prop
			NULL,                   // model
			1, 0, 0,                // realrot
			0, 1, 0,
			0, 0, 1,
			0,                      // hidden
			NULL,                   // geo
			NULL,                   // projectile
			0,                      // damage
			1000,                   // maxdamage
			0xff, 0xff, 0xff, 0x00, // shadecol
			0xff, 0xff, 0xff, 0x00, // nextcol
			0x0fff,                 // floorcol
			0,                      // tiles
			WEAPON_FALCON2,         // weaponnum
			0,                      // unk5d
			0,                      // unk5e
			FUNC_PRIMARY,           // gunfunc
			0,                      // fadeouttimer60
			-1,                     // dualweaponnum
			-1,                     // timer240
			NULL,                   // dualweapon
		};

		s32 weaponmodelnum;
		s32 weaponnum = bgunGetWeaponNum2(HAND_RIGHT);
		s32 bodynum = BODY_DARK_COMBAT;
		s32 headnum = HEAD_DARK_COMBAT;
		bool sp60 = false;
		struct model *model = NULL;
		u32 *rwdatas;
		u32 stack3[2];

		g_Vars.currentplayer->haschrbody = true;
		playerChooseBodyAndHead(&bodynum, &headnum, &sp60);

		if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
			weaponnum = g_DefaultWeapons[0];
		}

		weaponmodelnum = playermgrGetModelOfWeapon(weaponnum);

		if (IS4MB()) {
			bodynum = BODY_DARK_COMBAT;
			headnum = HEAD_DARK_COMBAT;
		}

		if (!g_Vars.mplayerisrunning || (IS4MB() && PLAYERCOUNT() == 1)) {
			// 1 player
			if (g_Vars.currentplayer->gunmem2 == NULL) {
				if (!var8009dfc0 && bgunChangeGunMem(GUNMEMOWNER_CHRBODY)) {
					g_Vars.currentplayer->gunmem2 = bgunGetGunMem();
				} else {
					if (var8009dfc0);

					g_Vars.currentplayer->haschrbody = false;

					if (!var8009dfc0) {
						g_Vars.lockscreen = true;
					}
					return;
				}
			}

			offset1 = 0;
			var8007fc0c = 8;
			osSyncPrintf("Gunmem: 0x%08x\n", bgunGetGunMem());

			allocation = g_Vars.currentplayer->gunmem2;
			model = (struct model *)(allocation + offset1);
			osSyncPrintf("Gunmem: bondsub 0x%08x\n", (uintptr_t)model);
			offset1 += ALIGN64(sizeof(struct model));

			model->anim = (struct anim *)(allocation + offset1);
			osSyncPrintf("Gunmem: bondsub->anim 0x%08x\n", model->anim);
			offset1 += sizeof(struct anim);
			offset1 = ALIGN64(offset1);

			rwdatas = (u32 *)(allocation + offset1);
			osSyncPrintf("Gunmem: savedata 0x%08x\n", (uintptr_t)rwdatas);
			offset1 += 0x400;
#ifdef PLATFORM_64BIT
			offset1 += 0x200;
#endif
			offset1 = ALIGN64(offset1);

			weaponobj = (struct weaponobj *)(allocation + offset1);
			osSyncPrintf("Gunmem: wo 0x%08x\n", (uintptr_t)weaponobj);
			offset1 += sizeof(struct weaponobj);
			offset1 = ALIGN64(offset1);

			offset2 = offset1 + ALIGN64(fileGetInflatedSize(g_HeadsAndBodies[bodynum].filenum, LOADTYPE_MODEL));

			if (headnum >= 0) {
				offset2 += ALIGN64(fileGetInflatedSize(g_HeadsAndBodies[headnum].filenum, LOADTYPE_MODEL));
			}

			if (weaponmodelnum >= 0) {
				offset2 += ALIGN64(fileGetInflatedSize(g_ModelStates[weaponmodelnum].fileid, LOADTYPE_MODEL));
			}

			offset2 += 0x4000;
#ifdef PLATFORM_64BIT
			offset2 += 0x2000;
#endif
			bgunCalculateGunMemCapacity();
			spe8 = g_Vars.currentplayer->gunmem2 + offset2;
			texInitPool(&texpool, spe8, bgunCalculateGunMemCapacity() - offset2);
			bodymodeldef = modeldefLoad(g_HeadsAndBodies[bodynum].filenum, allocation + offset1, offset2 - offset1, &texpool);
			offset1 = ALIGN64(fileGetLoadedSize(g_HeadsAndBodies[bodynum].filenum) + offset1);

			if (headnum >= 0) {
				headmodeldef = modeldefLoad(g_HeadsAndBodies[headnum].filenum, allocation + offset1, offset2 - offset1, &texpool);
				offset1 = ALIGN64(fileGetLoadedSize(g_HeadsAndBodies[headnum].filenum) + offset1);
			}

			modelAllocateRwData(bodymodeldef);

			if (headmodeldef != NULL) {
				modelAllocateRwData(headmodeldef);
			}

			modelInit(model, bodymodeldef, rwdatas, false);
			animInit(model->anim);

			model->rwdatalen = 256;

#ifdef PLATFORM_64BIT
			model->rwdatalen += 128;
#endif

			texGetPoolLeftPos(&texpool);

			// @TODO: Figure out these arguments
			osSyncPrintf("Jo using %d bytes gunmem (gunmemsize %d)\n");
			osSyncPrintf("Gunmem: bondmeml 0x%08x size 0x%08x\n", bgunGetGunMem(), bgunCalculateGunMemCapacity());
			osSyncPrintf("Gunmem: tex block free 0x%08x\n");
			osSyncPrintf("Gunmem: Free at end %d\n");

			texGetPoolLeftPos(&texpool);
		} else {
			// 2-4 players
			if (g_HeadsAndBodies[bodynum].modeldef == NULL) {
				g_HeadsAndBodies[bodynum].modeldef = modeldefLoadToNew(g_HeadsAndBodies[bodynum].filenum);
			}

			bodymodeldef = g_HeadsAndBodies[bodynum].modeldef;

			if (g_HeadsAndBodies[bodynum].unk00_01) {
				headnum = -1;
			} else if (sp60) {
				headmodeldef = func0f18e57c(headnum, &headnum);
			} else if (g_Vars.normmplayerisrunning && IS8MB()) {
				g_HeadsAndBodies[headnum].modeldef = modeldefLoadToNew(g_HeadsAndBodies[headnum].filenum);
				headmodeldef = g_HeadsAndBodies[headnum].modeldef;
				g_FileInfo[g_HeadsAndBodies[headnum].filenum].loadedsize = 0;
				bodyCalculateHeadOffset(headmodeldef, headnum, bodynum);
			} else {
				if (g_HeadsAndBodies[headnum].modeldef == NULL) {
					g_HeadsAndBodies[headnum].modeldef = modeldefLoadToNew(g_HeadsAndBodies[headnum].filenum);
				}

				headmodeldef = g_HeadsAndBodies[headnum].modeldef;
			}
		}

		g_Vars.currentplayer->model00d4 = body0f02ce8c(bodynum, headnum, bodymodeldef, headmodeldef, false, model, true, true);

		chr0f020b14(g_Vars.currentplayer->prop, g_Vars.currentplayer->model00d4, &g_Vars.currentplayer->prop->pos,
				g_Vars.currentplayer->prop->rooms, turnangle, 0);
		g_Vars.currentplayer->prop->type = PROPTYPE_PLAYER;
		chr = g_Vars.currentplayer->prop->chr;

		if (g_Vars.mplayerisrunning) {
			g_MpAllChrPtrs[g_Vars.currentplayernum] = chr;
			g_MpAllChrConfigPtrs[g_Vars.currentplayernum] = &g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base;
		}

		chr->chrflags |= CHRCFLAG_FORCETOGROUND;

		modelSetRootPosition(g_Vars.currentplayer->model00d4, &g_Vars.currentplayer->prop->pos);
		chrSetLookAngle(g_Vars.currentplayer->prop->chr, turnangle);

		chr->headnum = headnum;
		chr->bodynum = bodynum;
		chr->race = bodyGetRace(chr->bodynum);
		chr->radius = g_Vars.currentplayer->bond2.radius;

		g_Vars.currentplayer->vv_eyeheight = (s32)g_HeadsAndBodies[bodynum].height;

#if VERSION >= VERSION_NTSC_1_0
		if (g_Vars.antiplayernum >= 0
				&& g_Vars.currentplayer == g_Vars.anti
				&& g_Vars.currentplayer->vv_eyeheight > 159) {
			g_Vars.currentplayer->vv_eyeheight = 159;
		}
#endif

		g_Vars.currentplayer->vv_headheight = g_Vars.currentplayer->vv_eyeheight;

		if (headnum >= 0) {
			g_Vars.currentplayer->vv_headheight += (s32)g_HeadsAndBodies[headnum].height;
		} else {
			g_Vars.currentplayer->vv_headheight += 13;
		}

		if (g_Vars.currentplayer->vv_headheight > g_HeadsAndBodies[BODY_MRBLONDE].height + g_HeadsAndBodies[HEAD_MRBLONDE].height) {
			g_Vars.currentplayer->vv_headheight = g_HeadsAndBodies[BODY_MRBLONDE].height + g_HeadsAndBodies[HEAD_MRBLONDE].height;
		}

		g_Vars.currentplayer->vv_height = g_Vars.currentplayer->vv_eyeheight;

		if (weaponmodelnum >= 0) {
			if (g_Vars.mplayerisrunning == false) {
				weaponmodeldef = modeldefLoad(g_ModelStates[weaponmodelnum].fileid, allocation + offset1, offset2 - offset1, &texpool);
				fileGetLoadedSize(g_ModelStates[weaponmodelnum].fileid);
				modelAllocateRwData(weaponmodeldef);
			} else {
				weaponobj = NULL;
				weaponmodeldef = NULL;
			}

			weaponCreateForChr(chr, weaponmodelnum, weaponnum, 0, weaponobj, weaponmodeldef);
		}

		chr->fireslots[0] = bgunAllocateFireslot();
		func0f02e9a0(chr, 0);
		bmoveUpdateRooms(g_Vars.currentplayer);
	} else {
		struct chrdata *chr = g_Vars.currentplayer->prop->chr;

		if (chr->model->anim == NULL) {
			chr->chrflags |= CHRCFLAG_FORCETOGROUND;
			func0f02e9a0(chr, 0);
			modelSetRootPosition(g_Vars.currentplayer->model00d4, &g_Vars.currentplayer->prop->pos);
			chrSetLookAngle(g_Vars.currentplayer->prop->chr, turnangle);
			bmoveUpdateRooms(g_Vars.currentplayer);
		}
	}
}

void playerRemoveChrBody(void)
{
	if (g_Vars.currentplayer->haschrbody) {
		if (!g_Vars.mplayerisrunning || (IS4MB() && PLAYERCOUNT() == 1)) {
			g_Vars.currentplayer->haschrbody = false;
			chrRemove(g_Vars.currentplayer->prop, false);
			g_Vars.currentplayer->model00d4 = NULL;
			bmoveUpdateRooms(g_Vars.currentplayer);
			bgunFreeGunMem();
			g_Vars.currentplayer->gunmem2 = NULL;
		}
	}
}

void playerSetTickMode(s32 tickmode)
{
	g_Vars.tickmode = tickmode;
	g_Vars.in_cutscene = false;
}

void playerBeginGeFadeIn(void)
{
	playerSetTickMode(TICKMODE_GE_FADEIN);
	g_PlayerTriggerGeFadeIn = false;
}

void playersBeginMpSwirl(void)
{
	playerSetTickMode(TICKMODE_MPSWIRL);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetMode(MOVEMODE_WALK);

	g_MpSwirlRotateSpeed = 0;
	g_MpSwirlAngleDegrees = -90;
	g_MpSwirlForwardSpeed = 0;
	g_MpSwirlDistance = 80;

	envChooseAndApply(mainGetStageNum(), false);
}

void playerTickMpSwirl(void)
{
	f32 angle;
	struct coord pos = {0, 0, 0};
	struct coord look = {0, 0, 1};
	struct coord up = {0, 1, 0};
	s32 i;

	playerSetCameraMode(CAMERAMODE_THIRDPERSON);

	// This function is called once for each player per frame,
	// but the swirl position should only be updated once per frame,
	// so it's only updated for the player at index 0.
	if (g_Vars.currentplayerindex == 0) {
		for (i = 0; i < g_Vars.lvupdate60; i++) {
			// Calculate rotation
			if (g_MpSwirlAngleDegrees < 179.5f) {
				if (g_MpSwirlAngleDegrees < -20) {
					g_MpSwirlRotateSpeed += 0.1f;
				}

				if (g_MpSwirlAngleDegrees > 110) {
					g_MpSwirlRotateSpeed -= 0.1f;
				}

				g_MpSwirlAngleDegrees += g_MpSwirlRotateSpeed;
			}

			if (g_MpSwirlAngleDegrees >= 179.5f) {
				g_MpSwirlAngleDegrees = 180.0f;
			}

			// Calculate distance
			if (g_MpSwirlAngleDegrees > 80) {
				if (g_MpSwirlDistance > 60) {
					g_MpSwirlForwardSpeed -= 0.1f;
				} else {
					g_MpSwirlForwardSpeed += 0.015f;
				}

				g_MpSwirlDistance += g_MpSwirlForwardSpeed;

				if (g_MpSwirlDistance < 1) {
					g_MpSwirlDistance = 1;
				}
			}
		}
	}

	angle = (g_MpSwirlAngleDegrees - g_Vars.currentplayer->vv_theta) * M_PI / 180.0f;

	pos.x = sinf(angle) * g_MpSwirlDistance + g_Vars.currentplayer->bond2.unk10.x;
	pos.y = g_Vars.currentplayer->bond2.unk10.y + g_MpSwirlDistance * 0.08f;
	pos.z = cosf(angle) * g_MpSwirlDistance + g_Vars.currentplayer->bond2.unk10.z;

	look.x = g_Vars.currentplayer->bond2.unk10.x - pos.x;
	look.y = g_Vars.currentplayer->bond2.unk10.y - pos.y;
	look.z = g_Vars.currentplayer->bond2.unk10.z - pos.z;

	player0f0c1840(&pos, &up, &look, &g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms);

	if (g_MpSwirlDistance < 5.0f) {
		playerEndCutscene();
	}
}

void player0f0b9a20(void)
{
	playerSetTickMode(TICKMODE_NORMAL);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetMode(MOVEMODE_WALK);

	if (mainGetStageNum() == STAGE_TEST_LEN) {
		playerSetFadeColour(0, 0, 0, 1);
		playerSetFadeFrac(0, 1);
	} else if (var80070748 != 0) {
		playerSetFadeColour(0, 0, 0, 1);
		playerSetFadeFrac(60, 0);
	}

	envChooseAndApply(mainGetStageNum(), false);
	bgunEquipWeapon2(HAND_LEFT, g_DefaultWeapons[HAND_LEFT]);
	bgunEquipWeapon2(HAND_RIGHT, g_DefaultWeapons[HAND_RIGHT]);
	var8007074c = 0;
}

void playerEndCutscene(void)
{
	if (g_IsTitleDemo) {
		mainChangeToStage(STAGE_TITLE);
	} else if (g_Vars.autocutplaying) {
		g_Vars.autocutfinished = true;
	} else {
		playerSetTickMode(TICKMODE_NORMAL);
		g_PlayerTriggerGeFadeIn = false;
		bmoveSetModeForAllPlayers(MOVEMODE_WALK);
	}
}

void playerPrepareWarpType1(s16 pad)
{
	playerSetTickMode(TICKMODE_WARP);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetModeForAllPlayers(MOVEMODE_CUTSCENE);
	playersClearMemCamRoom();

	g_WarpType1Pad = pad;
}

void playerPrepareWarpType2(struct warpparams *cmd, bool hasdir, s32 arg2)
{
	playerSetTickMode(TICKMODE_WARP);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetModeForAllPlayers(MOVEMODE_CUTSCENE);
	playersClearMemCamRoom();

	g_WarpType1Pad = -1;

	g_WarpType2Params = cmd;
	g_WarpType2HasDirection = hasdir;
	g_WarpType2Arg2 = arg2;
}

void playerPrepareWarpType3(f32 posangle, f32 rotangle, f32 range, f32 height1, f32 height2, s32 padnum)
{
	playerSetTickMode(TICKMODE_WARP);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetModeForAllPlayers(MOVEMODE_CUTSCENE);
	playersClearMemCamRoom();

	g_WarpType1Pad = -1;

	g_WarpType2Params = NULL;

	g_WarpType3PosAngle = posangle;
	g_WarpType3RotAngle = rotangle;
	g_WarpType3Range = range;
	g_WarpType3Height = height1;
	g_WarpType3MoreHeight = height2;
	g_WarpType3Pad = padnum;
}

void playerExecutePreparedWarp(void)
{
	struct pad pad;
	struct coord pos = {0, 0, 0};
	struct coord look = {0, 0, 1};
	struct coord up = {0, 1, 0};
	s32 room;
	struct coord memcampos;

	playerSetCameraMode(CAMERAMODE_THIRDPERSON);

	if (g_WarpType1Pad >= 0) {
		// Warp to an exact position with a static direction of 0, 0, 1.
		// Used by device and holo training to warp player back to room,
		// and Deep Sea teleports
		padUnpack(g_WarpType1Pad, PADFIELD_POS | PADFIELD_ROOM, &pad);

		memcampos.x = pad.pos.x;
		memcampos.y = pad.pos.y;
		memcampos.z = pad.pos.z;

		pos.x = memcampos.f[0];
		pos.y = memcampos.f[1];
		pos.z = memcampos.f[2];

		room = pad.room;
	} else if (g_WarpType2Params) {
		// Warp to an exact position with an optional direction.
		// Used by AI command 00df, but that command is not used.
		pos.x = g_WarpType2Params->pos.x;
		pos.y = g_WarpType2Params->pos.y;
		pos.z = g_WarpType2Params->pos.z;

		padUnpack(g_WarpType2Params->pad, PADFIELD_POS | PADFIELD_ROOM, &pad);

		room = pad.room;

		memcampos.x = pad.pos.x;
		memcampos.y = pad.pos.y;
		memcampos.z = pad.pos.z;

		if (1);

		if (g_WarpType2HasDirection != 1) {
			look.x = cosf(g_WarpType2Params->look[1]) * sinf(g_WarpType2Params->look[0]);
			look.y = sinf(g_WarpType2Params->look[1]);
			look.z = cosf(g_WarpType2Params->look[1]) * cosf(g_WarpType2Params->look[0]);
		}
	} else {
		// Warp to a location within a specified range and angle of the pad,
		// with options for the direction and height offset from the pad.
		// Used by AI command 00f4, but that command is not used.
		padUnpack(g_WarpType3Pad, PADFIELD_POS | PADFIELD_ROOM, &pad);

		room = pad.room;

		memcampos.x = pad.pos.x;
		memcampos.y = pad.pos.y;
		memcampos.z = pad.pos.z;

		pos.x = memcampos.x + sinf(g_WarpType3PosAngle) * g_WarpType3Range + cosf(g_WarpType3PosAngle) * 0.0f;
		pos.y = memcampos.y + g_WarpType3MoreHeight + g_WarpType3Height;
		pos.z = memcampos.z + cosf(g_WarpType3PosAngle) * g_WarpType3Range + sinf(g_WarpType3PosAngle) * 0.0f;

		look.x = memcampos.x + cosf(g_WarpType3PosAngle) * 0.0f - pos.f[0];
		look.y = memcampos.y + g_WarpType3MoreHeight - pos.f[1];
		look.z = memcampos.z + sinf(g_WarpType3PosAngle) * 0.0f - pos.f[2];

		g_WarpType3PosAngle += g_WarpType3RotAngle * g_Vars.lvupdate60freal;

		while (g_WarpType3PosAngle >= M_BADTAU) {
			g_WarpType3PosAngle -= M_BADTAU;
		}

		while (g_WarpType3PosAngle < 0) {
			g_WarpType3PosAngle += M_BADTAU;
		}
	}

	player0f0c1ba4(&pos, &up, &look, &memcampos, room);
}

void playerStartCutscene2(void)
{
	playerSetTickMode(TICKMODE_CUTSCENE);
	g_PlayerTriggerGeFadeIn = false;
	bmoveSetModeForAllPlayers(MOVEMODE_CUTSCENE);
	playersClearMemCamRoom();

#if PAL
	g_CutsceneCurAnimFrame240 = var8009e388pf;
	g_CutsceneCurAnimFrame60 = floorf(g_CutsceneCurAnimFrame240 + 0.01f);
#else
	g_CutsceneCurAnimFrame240 = g_CutsceneFrameOverrun240;
	g_CutsceneCurAnimFrame60 = g_CutsceneFrameOverrun240 >> 2;
#endif

	g_CutsceneBlurFrac = 0;
	g_CutsceneTweenDuration60 = -1;
	g_InCutscene = 1;

	paksStop(true);
	g_Vars.in_cutscene = g_Vars.tickmode == TICKMODE_CUTSCENE && g_CutsceneCurAnimFrame60 < animGetNumFrames(g_CutsceneAnimNum) - 1;
	g_Vars.cutsceneskip60ths = 0;
}

void playerStartCutscene(s16 animnum)
{
	if ((!g_IsTitleDemo && !g_Vars.autocutplaying)
			|| !g_Vars.in_cutscene
			|| !g_CutsceneSkipRequested) {
		joyDisableTemporarily();

		if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
			g_CutsceneSkipRequested = false;
			g_CutsceneCurTotalFrame60f = 0;
		}

		if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
			playersTickAllChrBodies();
		}

		g_CutsceneAnimNum = animnum;

		if (g_Vars.currentplayer->haschrbody) {
			playerStartCutscene2();
		}
	}
}

void playerReorientForCutsceneStop(s32 tweenduration60)
{
	struct coord rot;
	struct coord translate;
	struct coord scale;
	u8 frameslot;
	Mtxf rotmtx;
	s32 lastframe;
	f32 theta;
	u32 stack;

	g_CutsceneTweenDuration60 = tweenduration60;
	lastframe = animGetNumFrames(g_CutsceneAnimNum) - 1;
	animLoadHeader(g_CutsceneAnimNum);
	frameslot = animLoadFrame(g_CutsceneAnimNum, lastframe);
	animForgetFrameBirths();
	animGetRotTranslateScale(0, 0, &g_Skel20, g_CutsceneAnimNum, frameslot, &rot, &translate, &scale);
	mtx4LoadRotation(&rot, &rotmtx);

	theta = atan2f(-rotmtx.m[2][0], -rotmtx.m[2][2]);
	theta = (M_BADTAU - theta) * 57.304901123047f;
	g_Vars.bond->vv_theta = theta;

	chrSetLookAngle(g_Vars.bond->prop->chr, (360 - theta) * 0.017450513318181f);
}

void playerTickCutscene(bool arg0)
{
	struct coord pos;
	struct coord up;
	struct coord look;
	struct coord rot;
	struct coord translate;
	struct coord scale;
	u8 frameslot;
	Mtxf rotmtx;
	f32 translatescale = bgGetStageTranslationThing();
	f32 fovy;
	s32 endframe;
	s8 contpadnum = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
	u32 buttons;
#if PAL
	u8 stack3[0x2c];
#endif
	f32 tweenfrac;
	f32 sp104;
	Mtxf spc4;
	Mtxf sp84;
	f32 sp74[4];
	f32 sp64[4];
	f32 sp54[4];

	if (arg0) {
		buttons = joyGetButtons(contpadnum, 0xffffffff);
	} else {
		buttons = 0;
	}

	animLoadHeader(g_CutsceneAnimNum);

	endframe = animGetNumFrames(g_CutsceneAnimNum) - 1;

	if (g_Vars.currentplayerindex == 0) {
		g_Vars.cutsceneskip60ths = 0;

		if (g_CutsceneCurAnimFrame60 < endframe) {
#if PAL
			g_CutsceneCurAnimFrame240 += g_Vars.lvupdate60freal;
			g_CutsceneCurAnimFrame60 = floorf(g_CutsceneCurAnimFrame240 + 0.01f);
#else
			g_CutsceneCurAnimFrame240 += g_Vars.lvupdate240;
			g_CutsceneCurAnimFrame60 = g_CutsceneCurAnimFrame240 >> 2;
#endif

			if (g_Anims[g_CutsceneAnimNum].flags & ANIMFLAG_HASCUTSKIPFRAMES) {
				while (g_CutsceneCurAnimFrame60 < endframe
						&& animIsFrameCutSkipped(g_CutsceneAnimNum, g_CutsceneCurAnimFrame60)) {
#if PAL
					g_CutsceneCurAnimFrame240 += 1.2f;
					g_CutsceneCurAnimFrame60 = floorf(g_CutsceneCurAnimFrame240 + 0.01f);
#else
					g_CutsceneCurAnimFrame60++;
					g_CutsceneCurAnimFrame240 += 4;
#endif

					g_Vars.cutsceneskip60ths++;
				}
			}

			if (g_CutsceneCurAnimFrame60 >= endframe) {
#if PAL
				var8009e388pf = g_CutsceneCurAnimFrame240 - endframe;
#else
				g_CutsceneFrameOverrun240 = g_CutsceneCurAnimFrame240 - endframe * 4;
#endif
			}

			if (g_CutsceneCurAnimFrame60 > endframe) {
				g_CutsceneCurAnimFrame60 = endframe;
			}
		}
	}

	g_Vars.in_cutscene = (g_Vars.tickmode == TICKMODE_CUTSCENE && g_CutsceneCurAnimFrame60 < endframe);
	frameslot = animLoadFrame(g_CutsceneAnimNum, g_CutsceneCurAnimFrame60);
	animForgetFrameBirths();
	animGetRotTranslateScale(0, 0, &g_Skel20, g_CutsceneAnimNum, frameslot, &rot, &translate, &scale);

	pos.x = translate.x * translatescale;
	pos.y = translate.y * translatescale;
	pos.z = translate.z * translatescale;

	mtx4LoadRotation(&rot, &rotmtx);

	up.x = rotmtx.m[1][0];
	up.y = rotmtx.m[1][1];
	up.z = rotmtx.m[1][2];

	look.x = -rotmtx.m[2][0];
	look.y = -rotmtx.m[2][1];
	look.z = -rotmtx.m[2][2];

	fovy = animGetCameraValue(1, g_CutsceneAnimNum, frameslot);
	g_CutsceneBlurFrac = animGetCameraValue(2, g_CutsceneAnimNum, frameslot);
	g_CutsceneTweenFrac = 0;

	if (g_CutsceneTweenDuration60 > 0 && endframe - g_CutsceneCurAnimFrame60 <= g_CutsceneTweenDuration60) {
		// Cutscene is almost done
		// Camera is tweening to head and top/bottom bars are shrinking
		tweenfrac = 1 - (f32)(endframe - g_CutsceneCurAnimFrame60) / (f32)g_CutsceneTweenDuration60;

		g_CutsceneTweenFrac = tweenfrac;
		sp104 = 1 - cosf(1.5705462694168f * tweenfrac);

		bmoveSetMode(MOVEMODE_WALK);

		pos.x += sp104 * (g_Vars.bond->bond2.unk10.x - pos.x);
		pos.y += sp104 * (g_Vars.bond->bond2.unk10.y - pos.y);
		pos.z += sp104 * (g_Vars.bond->bond2.unk10.z - pos.z);

		mtx00016d58(&spc4, 0, 0, 0, -look.x, -look.y, -look.z, up.x, up.y, up.z);
		mtx00016d58(&sp84, 0, 0, 0,
				-g_Vars.bond->bond2.unk1c.x, -g_Vars.bond->bond2.unk1c.y, -g_Vars.bond->bond2.unk1c.z,
				g_Vars.bond->bond2.unk28.x, g_Vars.bond->bond2.unk28.y, g_Vars.bond->bond2.unk28.z);
		quaternion0f097044(&spc4, sp74);
		quaternion0f097044(&sp84, sp64);
		quaternion0f0976c0(sp64, sp74);
		quaternionSlerp(sp74, sp64, sp104, sp54);
		quaternionToMtx(sp54, &rotmtx);

		up.x = rotmtx.m[1][0];
		up.y = rotmtx.m[1][1];
		up.z = rotmtx.m[1][2];

		look.x = rotmtx.m[2][0];
		look.y = rotmtx.m[2][1];
		look.z = rotmtx.m[2][2];

		g_CutsceneBlurFrac += tweenfrac * (0 - g_CutsceneBlurFrac);
		fovy += tweenfrac * (60 - fovy);
	}

	playerSetCameraMode(CAMERAMODE_THIRDPERSON);
	player0f0c1bd8(&pos, &up, &look);
	playermgrSetFovY(fovy);
	viSetFovY(fovy);

	if (g_Vars.currentplayerindex == 0) {
		g_CutsceneCurTotalFrame60f += g_Vars.lvupdate60freal;
	}

#ifndef PLATFORM_N64
	if (arg0 && inputKeyJustPressed(VK_ESCAPE)) {
		buttons |= START_BUTTON;
	}
#endif

#if VERSION >= VERSION_NTSC_1_0
	if (g_CutsceneCurTotalFrame60f > 30 && (buttons & 0xffffffff)) {
		g_CutsceneSkipRequested = true;

		if (g_Vars.autocutplaying) {
			if (buttons & (B_BUTTON | START_BUTTON)) {
				g_Vars.autocutgroupskip = true;
			} else {
				g_Vars.autocutfinished = true;
			}
		}
	}
#else
	if (g_CutsceneCurTotalFrame60f > 30) {
		if (buttons & 0xffffffff) {
			g_CutsceneSkipRequested = true;
		}

		if ((buttons & (B_BUTTON | START_BUTTON)) && g_Vars.autocutplaying) {
			g_Vars.autocutgroupskip = true;
		}
	}
#endif
}

f32 playerGetCutsceneBlurFrac(void)
{
	return g_CutsceneBlurFrac;
}

void playerClampGunZoomFovY(s32 playernum)
{
	struct player *player = g_Vars.players[playernum];
	if (!player) {
		return;
	}

	for (s32 index = 0; index < ARRAYCOUNT(player->gunzoomfovs); ++index) {
		if (player->gunzoomfovs[index] < ADJUST_ZOOM_FOV(2)) {
			player->gunzoomfovs[index] = ADJUST_ZOOM_FOV(2);
		} else if (player->gunzoomfovs[index] > ADJUST_ZOOM_FOV(60)) {
			player->gunzoomfovs[index] = ADJUST_ZOOM_FOV(60);
		}
	}
}

void playerSetZoomFovY(f32 fovy, f32 timemax)
{
	g_Vars.currentplayer->zoomintime = 0;
	g_Vars.currentplayer->zoomintimemax = timemax;
	g_Vars.currentplayer->zoominfovyold = g_Vars.currentplayer->zoominfovy;
	g_Vars.currentplayer->zoominfovynew = fovy;
}

f32 playerGetZoomFovY(void)
{
	if (g_Vars.currentplayer->zoomintimemax > g_Vars.currentplayer->zoomintime) {
		return g_Vars.currentplayer->zoominfovynew;
	}

	return g_Vars.currentplayer->zoominfovy;
}

void playerTweenFovY(f32 targetfovy)
{
	f32 speed = 15.0f / 30.0f;

#ifndef PLATFORM_N64
	if (PLAYER_DEFAULT_FOV > 60.0f) { // adjust zoom speed depending on non-default fov setting (higher fov == faster zoom)
		speed /= PLAYER_DEFAULT_FOV / 60.0f;
	}
#endif

	if (playerGetZoomFovY() != targetfovy) {
		if (g_Vars.currentplayer->zoominfovy > targetfovy) {
			playerSetZoomFovY(targetfovy, (g_Vars.currentplayer->zoominfovy - targetfovy) * speed);
		} else {
			playerSetZoomFovY(targetfovy, (targetfovy - g_Vars.currentplayer->zoominfovy) * speed);
		}
	}
}

f32 playerGetTeleportFovY(void)
{
	f32 time;
	u32 fovyoffset;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER) {
		return 60.0f;
	}

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_EXITING) {
		time = 47 - g_Vars.currentplayer->teleporttime;
	} else {
		time = g_Vars.currentplayer->teleporttime;
	}

	time = time / 48.0f;
	time = 1.0f - cosf(time * M_PI * 0.5f);
	fovyoffset = 117.0f * time;

	return fovyoffset + 60.0f;
}

void playerUpdateZoom(void)
{
	f32 scale;
	f32 fovy;
	struct stagetableentry *stage;

	if (g_Vars.currentplayer->zoomintime < g_Vars.currentplayer->zoomintimemax) {
		g_Vars.currentplayer->zoomintime += g_Vars.lvupdate60freal;

		if (g_Vars.currentplayer->zoomintime > g_Vars.currentplayer->zoomintimemax) {
			g_Vars.currentplayer->zoomintime = g_Vars.currentplayer->zoomintimemax;
		}

		g_Vars.currentplayer->zoominfovy = g_Vars.currentplayer->zoominfovyold +
			(g_Vars.currentplayer->zoomintime *
			 (g_Vars.currentplayer->zoominfovynew - g_Vars.currentplayer->zoominfovyold))
			/ g_Vars.currentplayer->zoomintimemax;
	} else {
		g_Vars.currentplayer->zoomintime = g_Vars.currentplayer->zoomintimemax;
		g_Vars.currentplayer->zoominfovy = g_Vars.currentplayer->zoominfovynew;
	}

	playermgrSetFovY(g_Vars.currentplayer->zoominfovy);
	viSetFovY(g_Vars.currentplayer->zoominfovy);

	if (g_Vars.currentplayer->teleportstate != TELEPORTSTATE_INACTIVE) {
		fovy = playerGetTeleportFovY();
		playermgrSetFovY(fovy);
		viSetFovY(fovy);
	}

	if (g_Vars.currentplayer->zoominfovy >= 15) {
		scale = 1;
	} else if (g_Vars.currentplayer->zoominfovy >= 7) {
		scale = (g_Vars.currentplayer->zoominfovy - 7) * 0.0875f + 0.3f;
	} else if (g_Vars.currentplayer->zoominfovy >= 4) {
		scale = (g_Vars.currentplayer->zoominfovy - 4) * (1.0f / 30.0f) + 0.2f;
	} else if (g_Vars.currentplayer->zoominfovy >= 2) {
		scale = (g_Vars.currentplayer->zoominfovy - 2) * (1.0f / 20.0f) + 0.1f;
	} else {
		scale = 0.1;
	}

	stage = stageGetCurrent();
	bgSetScaleBg2Gfx((1 - (1 - stage->unk34) * (1 - scale) * (10.f / 9.0f)) * scale);
}

void playerStopAudioForPause(void)
{
	struct hand *hand;
	s32 i;

	alarmStopAudio();
	gasStopAudio();

	for (i = 0; i < 2; i++) {
		hand = &g_Vars.currentplayer->hands[i];

		if (hand->audiohandle2 && sndGetState(hand->audiohandle2) != AL_STOPPED) {
			audioStop(hand->audiohandle2);
		}
	}
}

u32 var8007083c = 0;
u32 g_GlobalMenuRoot = 0;

void playerTickPauseMenu(void)
{
	bool opened = false;

	switch (g_Vars.currentplayer->pausemode) {
	case PAUSEMODE_UNPAUSED:
		break;
	case PAUSEMODE_PAUSING:
		// Pause menu is opening
		switch (g_GlobalMenuRoot) {
		case MENUROOT_TRAINING:
		case MENUROOT_MAINMENU:
			opened = soloChoosePauseDialog();
			break;
		case MENUROOT_FILEMGR:
			opened = filemgrConsiderPushingFileSelectDialog();
			break;
		case MENUROOT_4MBMAINMENU:
		case MENUROOT_MPSETUP:
			opened = true;
			break;
		}

		if (opened) {
			struct trainingdata *data = dtGetData();
			lvSetPaused(true);
			g_Vars.currentplayer->pausemode = PAUSEMODE_PAUSED;

			if ((g_GlobalMenuRoot == MENUROOT_MAINMENU || g_GlobalMenuRoot == MENUROOT_TRAINING)
					&& g_Vars.stagenum == STAGE_CITRAINING) {
				s32 room = g_Vars.currentplayer->prop->rooms[0];

				if ((room >= ROOM_DISH_HOLO1 && room <= ROOM_DISH_HOLO4)
						|| room == ROOM_DISH_FIRINGRANGE
						|| room == ROOM_DISH_DEVICELAB
						|| (data && data->intraining)) {
					return;
				}
			}

			musicStartMenu();
		}
		break;
	case PAUSEMODE_PAUSED:
		// Pause menu is fully open
		break;
	case PAUSEMODE_UNPAUSING:
		// Pause menu is closing
		g_Vars.currentplayer->pausetime60 += g_Vars.diffframe60;

		if (g_Vars.currentplayer->pausetime60 >= 20) {
			lvSetPaused(false);
			g_Vars.currentplayer->pausemode = PAUSEMODE_UNPAUSED;
			musicEndMenu();
		}
		break;
	}
}

void playerPause(s32 root)
{
	g_GlobalMenuRoot = root;

	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		g_Vars.currentplayer->pausemode = PAUSEMODE_PAUSING;
	}
}

void playerUnpause(void)
{
	if (g_Vars.currentplayer->pausemode == PAUSEMODE_PAUSED) {
		lvSetPaused(false);
		musicEndMenu();
		g_Vars.currentplayer->pausemode = PAUSEMODE_UNPAUSED;
	}
}

Gfx *player0f0baf84(Gfx *gdl)
{
	if (g_Vars.currentplayer->pausemode != PAUSEMODE_UNPAUSED) {
		Mtx *a = gfxAllocateMatrix();
		u16 b;

		guPerspective(a, &b, g_Vars.currentplayer->zoominfovy,
				PAL ? 1.7316017150879f : 1.4545454978943f, 10, 300, 1);

		gSPMatrix(gdl++, OS_PHYSICAL_TO_K0(a), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
		gSPPerspNormalize(gdl++, b);
	}

	return gdl;
}

Gfx *playerDrawFade(Gfx *gdl, u32 r, u32 g, u32 b, f32 frac)
{
	if (frac > 0) {
		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);
		gDPSetTextureConvert(gdl++, G_TC_FILT);
		gDPSetTextureLUT(gdl++, G_TT_NONE);
		gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
		gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
		gDPSetPrimColor(gdl++, 0, 0, r, g, b, (s32)(frac * 255));
		gDPFillRectangle(gdl++, viGetViewLeft(), viGetViewTop(),
				viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());
		gDPPipeSync(gdl++);
		gDPSetColorDither(gdl++, G_CD_BAYER);
		gDPSetTexturePersp(gdl++, G_TP_PERSP);
		gDPSetTextureLOD(gdl++, G_TL_LOD);
	}

	return gdl;
}

Gfx *playerDrawStoredFade(Gfx *gdl)
{
	return playerDrawFade(gdl,
			g_Vars.currentplayer->colourscreenred,
			g_Vars.currentplayer->colourscreengreen,
			g_Vars.currentplayer->colourscreenblue,
			g_Vars.currentplayer->colourscreenfrac);
}

void playerSetFadeColour(s32 r, s32 g, s32 b, f32 frac)
{
	g_Vars.currentplayer->colourscreenred = r;
	g_Vars.currentplayer->colourscreengreen = g;
	g_Vars.currentplayer->colourscreenblue = b;
	g_Vars.currentplayer->colourscreenfrac = frac;
}

void playerAdjustFade(f32 maxfadetime, s32 r, s32 g, s32 b, f32 frac)
{
	g_Vars.currentplayer->colourfadetime60 = 0;
	g_Vars.currentplayer->colourfadetimemax60 = maxfadetime;
	g_Vars.currentplayer->colourfaderedold = g_Vars.currentplayer->colourscreenred;
	g_Vars.currentplayer->colourfaderednew = r;
	g_Vars.currentplayer->colourfadegreenold = g_Vars.currentplayer->colourscreengreen;
	g_Vars.currentplayer->colourfadegreennew = g;
	g_Vars.currentplayer->colourfadeblueold = g_Vars.currentplayer->colourscreenblue;
	g_Vars.currentplayer->colourfadebluenew = b;
	g_Vars.currentplayer->colourfadefracold = g_Vars.currentplayer->colourscreenfrac;
	g_Vars.currentplayer->colourfadefracnew = frac;
}

void playerSetFadeFrac(f32 maxfadetime, f32 frac)
{
	playerAdjustFade(maxfadetime,
			g_Vars.currentplayer->colourscreenred,
			g_Vars.currentplayer->colourscreengreen,
			g_Vars.currentplayer->colourscreenblue,
			frac);
}

bool playerIsFadeComplete(void)
{
	return g_Vars.currentplayer->colourfadetimemax60 < 0;
}

void playerUpdateColourScreenProperties(void)
{
	if (g_Vars.currentplayer->colourfadetimemax60 >= 0) {
		g_Vars.currentplayer->colourfadetime60 += g_Vars.lvupdate60freal;

		if (g_Vars.currentplayer->colourfadetime60 < g_Vars.currentplayer->colourfadetimemax60) {
			f32 mult = g_Vars.currentplayer->colourfadetime60 / g_Vars.currentplayer->colourfadetimemax60;
			g_Vars.currentplayer->colourscreenfrac = g_Vars.currentplayer->colourfadefracold + (g_Vars.currentplayer->colourfadefracnew - g_Vars.currentplayer->colourfadefracold) * mult;
			g_Vars.currentplayer->colourscreenred = g_Vars.currentplayer->colourfaderedold + (s32)((g_Vars.currentplayer->colourfaderednew - g_Vars.currentplayer->colourfaderedold) * mult);
			g_Vars.currentplayer->colourscreengreen = g_Vars.currentplayer->colourfadegreenold + (s32)((g_Vars.currentplayer->colourfadegreennew - g_Vars.currentplayer->colourfadegreenold) * mult);
			g_Vars.currentplayer->colourscreenblue = g_Vars.currentplayer->colourfadeblueold + (s32)((g_Vars.currentplayer->colourfadebluenew - g_Vars.currentplayer->colourfadeblueold) * mult);
			return;
		}

		g_Vars.currentplayer->colourscreenfrac = g_Vars.currentplayer->colourfadefracnew;
		g_Vars.currentplayer->colourscreenred = g_Vars.currentplayer->colourfaderednew;
		g_Vars.currentplayer->colourscreengreen = g_Vars.currentplayer->colourfadegreennew;
		g_Vars.currentplayer->colourscreenblue = g_Vars.currentplayer->colourfadebluenew;
		g_Vars.currentplayer->colourfadetimemax60 = -1;
	}
}

void playerStartChrFade(f32 duration60, f32 targetfrac)
{
	struct chrdata *chr = g_Vars.currentplayer->prop->chr;

	if (chr) {
		g_Vars.currentplayer->bondfadetime60 = 0;
		g_Vars.currentplayer->bondfadetimemax60 = duration60;
		g_Vars.currentplayer->bondfadefracold = chr->fadealpha / 255.0f;
		g_Vars.currentplayer->bondfadefracnew = targetfrac;
	}
}

void playerTickChrFade(void)
{
	if (g_Vars.currentplayer->bondfadetimemax60 >= 0) {
		struct chrdata *chr = g_Vars.currentplayer->prop->chr;
		f32 frac;

		g_Vars.currentplayer->bondfadetime60 += g_Vars.lvupdate60freal;

		if (g_Vars.currentplayer->bondfadetime60 < g_Vars.currentplayer->bondfadetimemax60) {
			frac = g_Vars.currentplayer->bondfadefracold
				+ (g_Vars.currentplayer->bondfadefracnew - g_Vars.currentplayer->bondfadefracold)
				* g_Vars.currentplayer->bondfadetime60
				/ g_Vars.currentplayer->bondfadetimemax60;
		} else {
			frac = g_Vars.currentplayer->bondfadefracnew;
			g_Vars.currentplayer->bondfadetimemax60 = -1;
		}

		if (chr) {
			chr->fadealpha = (s8)(frac * 255);
		}
	}
}

struct damagetype g_DamageTypes[] = {
	// flashstartframe
	// |  flashfullframe
	// |  |  flashendframe
	// |  |  |   maxalpha
	// |  |  |   |     red
	// |  |  |   |     |     green
	// |  |  |   |     |     |     blue
	// |  |  |   |     |     |     |
	{  0, 5, 40, 0.7,  0x96, 0x00, 0x00 },
	{  0, 5, 40, 0.7,  0x96, 0x00, 0x00 },
	{  0, 5, 30, 0.65, 0x96, 0x00, 0x00 },
	{  0, 5, 25, 0.6,  0x96, 0x00, 0x00 },
	{  0, 5, 22, 0.55, 0x96, 0x00, 0x00 },
	{  0, 5, 19, 0.5,  0x96, 0x00, 0x00 },
	{  0, 5, 17, 0.45, 0x96, 0x00, 0x00 },
	{  0, 5, 15, 0.4,  0x96, 0x00, 0x00 },
};

struct healthdamagetype g_HealthDamageTypes[] = {
	// openendframe
	// |  updatestartframe
	// |  |   updateendframe
	// |  |   |   closestartframe
	// |  |   |   |    closeendframe
	// |  |   |   |    |
	{ 20, 34, 46, 270, 285 },
	{ 20, 37, 52, 250, 265 },
	{ 20, 40, 58, 230, 245 },
	{ 20, 43, 64, 210, 225 },
	{ 20, 46, 70, 190, 205 },
	{ 20, 49, 76, 170, 185 },
	{ 20, 52, 82, 150, 165 },
	{ 20, 55, 88, 130, 145 },
};

/**
 * Make the health bar appear. If called while the health bar is already open,
 * the health displayed will be updated and the show timer will be reset.
 */
void playerDisplayHealth(void)
{
	switch (g_Vars.currentplayer->healthshowmode) {
	case HEALTHSHOWMODE_HIDDEN:
		g_Vars.currentplayer->oldhealth = g_Vars.currentplayer->bondhealth;
		g_Vars.currentplayer->oldarmour = playerGetShieldFrac();
		break;
	case HEALTHSHOWMODE_OPENING:
	case HEALTHSHOWMODE_PREVIOUS:
		break;
	case HEALTHSHOWMODE_UPDATING:
	case HEALTHSHOWMODE_CURRENT:
		g_Vars.currentplayer->oldhealth = g_Vars.currentplayer->apparenthealth;
		g_Vars.currentplayer->oldarmour = g_Vars.currentplayer->apparentarmour;
		break;
	case HEALTHSHOWMODE_CLOSING:
		g_Vars.currentplayer->oldhealth = g_Vars.currentplayer->bondhealth;
		g_Vars.currentplayer->oldarmour = playerGetShieldFrac();
		break;
	}

	switch (g_Vars.currentplayer->healthshowmode) {
	case HEALTHSHOWMODE_HIDDEN:
		g_Vars.currentplayer->healthshowtime = 0;
		g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_OPENING;
		break;
	case HEALTHSHOWMODE_OPENING:
	case HEALTHSHOWMODE_PREVIOUS:
		break;
	case HEALTHSHOWMODE_UPDATING:
	case HEALTHSHOWMODE_CURRENT:
		g_Vars.currentplayer->healthshowtime = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updatestartframe;
		g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_UPDATING;
		break;
	case HEALTHSHOWMODE_CLOSING:
		g_Vars.currentplayer->healthshowtime = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].openendframe * playerGetHealthBarHeightFrac();
		g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_OPENING;
		break;
	}
}

/**
 * Update properties relating to the damage flash and health bar updating.
 */
void playerTickDamageAndHealth(void)
{
	/**
	 * Handle flash of red when the player is damaged.
	 *
	 * damageshowtime is an incrementing timer. It's set to a negative value
	 * normally, 0 when damaged, then ticks up while the flash of red is
	 * visible.
	 *
	 * The player's health is split into 8 equally-sized parts, and the selected
	 * part determines which damage type is used. At lower health, the red flash
	 * and health bar animate faster.
	 */
	if (g_Vars.currentplayer->damageshowtime >= 0.0f) {
		if (g_Vars.currentplayer->damageshowtime == 0) {
			// This is the first frame of damage
			bgunSetSightVisible(GUNSIGHTREASON_DAMAGE, false);
			g_Vars.currentplayer->damagetype = (s32)(playerGetHealthFrac() * 8.0f);

			if (g_Vars.currentplayer->damagetype > DAMAGETYPE_7) {
				g_Vars.currentplayer->damagetype = DAMAGETYPE_7;
			}

			if (g_Vars.currentplayer->damagetype < DAMAGETYPE_0) {
				g_Vars.currentplayer->damagetype = DAMAGETYPE_0;
			}
		}

		if (!g_Vars.currentplayer->isdead
				&& g_Vars.currentplayer->damageshowtime <= g_DamageTypes[g_Vars.currentplayer->damagetype].flashendframe) {
			f32 inc;

			if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
				inc = g_Vars.lvupdate60freal;
			} else {
				inc = g_Vars.diffframe240freal;
			}

			if (inc > 5) {
				inc = 5;
			}

			g_Vars.currentplayer->damageshowtime += inc;

			if (g_Vars.currentplayer->damageshowtime >= g_DamageTypes[g_Vars.currentplayer->damagetype].flashstartframe
					&& g_Vars.currentplayer->damageshowtime <= g_DamageTypes[g_Vars.currentplayer->damagetype].flashendframe) {
				f32 alpha;
				f32 flashdoneframes = g_Vars.currentplayer->damageshowtime - g_DamageTypes[g_Vars.currentplayer->damagetype].flashstartframe;
				f32 flashfullframe = g_DamageTypes[g_Vars.currentplayer->damagetype].flashfullframe;
				f32 totalframes = g_DamageTypes[g_Vars.currentplayer->damagetype].flashendframe - g_DamageTypes[g_Vars.currentplayer->damagetype].flashstartframe;

				if (flashdoneframes < flashfullframe) {
					alpha = g_DamageTypes[g_Vars.currentplayer->damagetype].maxalpha * flashdoneframes / flashfullframe;
				} else {
					alpha = g_DamageTypes[g_Vars.currentplayer->damagetype].maxalpha * (totalframes - flashdoneframes) / (totalframes - flashfullframe);
				}

				playerSetFadeColour(
						g_DamageTypes[g_Vars.currentplayer->damagetype].red,
						g_DamageTypes[g_Vars.currentplayer->damagetype].green,
						g_DamageTypes[g_Vars.currentplayer->damagetype].blue, alpha);
			}
		} else {
			g_Vars.currentplayer->damageshowtime = -1;
			playerSetFadeColour(0xff, 0xff, 0xff, 0);

			if (!g_Vars.currentplayer->isdead) {
				bgunSetSightVisible(GUNSIGHTREASON_DAMAGE, true);
			}
		}
	}

	/**
	 * Handle updating the health bar.
	 *
	 * This works similarly to the damage code above, in that the health bar is
	 * split into 8 parts and the current part is used to look up settings.
	 */
	if (playerIsHealthVisible()) {
		if (g_Vars.currentplayer->healthshowmode == HEALTHSHOWMODE_OPENING) {
			g_Vars.currentplayer->healthdamagetype = (s32)((playerGetHealthFrac() + playerGetShieldFrac()) * 8.0f);

			if (g_Vars.currentplayer->healthdamagetype > DAMAGETYPE_7) {
				g_Vars.currentplayer->healthdamagetype = DAMAGETYPE_7;
			}

			if (g_Vars.currentplayer->healthdamagetype < DAMAGETYPE_0) {
				g_Vars.currentplayer->healthdamagetype = DAMAGETYPE_0;
			}
		}

		if (!g_Vars.currentplayer->isdead) {
			f32 updatedoneframes;
			f32 updateduration;
			f32 frac;
			f32 healthdiff;
			f32 armourdiff;

			switch (g_Vars.currentplayer->healthshowmode) {
			case HEALTHSHOWMODE_OPENING:
				g_Vars.currentplayer->apparenthealth = g_Vars.currentplayer->oldhealth;
				g_Vars.currentplayer->apparentarmour = g_Vars.currentplayer->oldarmour;
				g_Vars.currentplayer->healthshowtime += g_Vars.diffframe60freal;

				if (g_Vars.currentplayer->healthshowtime >= g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].openendframe) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_PREVIOUS;
				}
				break;
			case HEALTHSHOWMODE_PREVIOUS:
				g_Vars.currentplayer->apparenthealth = g_Vars.currentplayer->oldhealth;
				g_Vars.currentplayer->apparentarmour = g_Vars.currentplayer->oldarmour;
				g_Vars.currentplayer->healthshowtime += g_Vars.diffframe60freal;

				if (currentPlayerIsMenuOpenInSoloOrMp()) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_CURRENT;
				}

				if (g_Vars.currentplayer->healthshowtime >= g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updatestartframe) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_UPDATING;
				}
				break;
			case HEALTHSHOWMODE_UPDATING:
				g_Vars.currentplayer->healthshowtime += g_Vars.diffframe60freal;

				updatedoneframes = g_Vars.currentplayer->healthshowtime - g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updatestartframe;
				updateduration = (f32)g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updateendframe - (f32)g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updatestartframe;
				frac = updatedoneframes / updateduration;

				if (frac < 0) {
					frac = 0;
				}

				if (frac > 1) {
					frac = 1;
				}

				if (currentPlayerIsMenuOpenInSoloOrMp()) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_CURRENT;
				}

				healthdiff = g_Vars.currentplayer->oldhealth - g_Vars.currentplayer->bondhealth;
				armourdiff = g_Vars.currentplayer->oldarmour - playerGetShieldFrac();

				g_Vars.currentplayer->apparenthealth = g_Vars.currentplayer->oldhealth - frac * healthdiff;
				g_Vars.currentplayer->apparentarmour = g_Vars.currentplayer->oldarmour - frac * armourdiff;

				if (g_Vars.currentplayer->healthshowtime >= g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].updateendframe) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_CURRENT;
				}
				break;
			case HEALTHSHOWMODE_CURRENT:
				g_Vars.currentplayer->apparenthealth = g_Vars.currentplayer->bondhealth;
				g_Vars.currentplayer->apparentarmour = playerGetShieldFrac();
				g_Vars.currentplayer->healthshowtime += g_Vars.diffframe60freal;

				if (currentPlayerIsMenuOpenInSoloOrMp()) {
					g_Vars.currentplayer->healthshowtime = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closestartframe;
				}

				if (g_Vars.currentplayer->healthshowtime >= g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closestartframe
						&& !currentPlayerIsMenuOpenInSoloOrMp()) {
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_CLOSING;
					g_Vars.currentplayer->healthshowtime = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closestartframe;
				}
				break;
			case HEALTHSHOWMODE_CLOSING:
				g_Vars.currentplayer->healthshowtime += g_Vars.diffframe60freal;

				if (g_Vars.currentplayer->healthshowtime >= g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closeendframe) {
					g_Vars.currentplayer->healthshowtime = -1;
					g_Vars.currentplayer->healthshowmode = HEALTHSHOWMODE_HIDDEN;
				}
				break;
			}
		} else {
			g_Vars.currentplayer->healthshowtime = -1;
			g_Vars.currentplayer->healthshowmode = 0;
		}
	}
}

bool playerIsDamageVisible(void)
{
	return g_Vars.currentplayer->damageshowtime >= 0;
}

/**
 * Trigger the red flash when the player is damaged.
 *
 * May be called while the red flash is already happening, which may result in
 * the fade being reset to the full alpha point.
 */
void playerDisplayDamage(void)
{
	/**
	 * @bug: This should be using damagetype (not healthdamagetype) as the array
	 * index. These are usually the same value, but I beleive they may be
	 * different if the player has low health with a shield.
	 */
	if (g_Vars.currentplayer->damageshowtime >= g_DamageTypes[g_Vars.currentplayer->healthdamagetype].flashfullframe) {
		g_Vars.currentplayer->damageshowtime = g_DamageTypes[g_Vars.currentplayer->healthdamagetype].flashfullframe;
		return;
	}

	if (g_Vars.currentplayer->damageshowtime < 0) {
		g_Vars.currentplayer->damageshowtime = 0;
	}
}

Gfx *playerRenderHealthBar(Gfx *gdl)
{
	Mtxf matrix;
	Mtxf *addr = gfxAllocateMatrix();

#ifdef PLATFORM_N64
	mtx00016ae4(&matrix, 0, 370, 0, 0, 0, 0, 0, 0, -1);
#else
	f32 fovsc = 60.f / PLAYER_DEFAULT_FOV;
	if (fovsc > 1.01f) {
		fovsc *= 1.1f;
	}
	mtx00016ae4(&matrix, 0, 370.f * fovsc, 0, 0, 0, 0, 0, 0, -1);
#endif
	mtxF2L(&matrix, addr);

	gSPMatrix(gdl++, osVirtualToPhysical((void *)addr), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
	gDPSetPrimColorViaWord(gdl++, 0, 0, 0xe6e6e600);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
#ifndef PLATFORM_N64
	// bug?
	gSPClearGeometryMode(gdl++, G_ZBUFFER);
#endif

	gdl = healthbarDraw(gdl, NULL, 0, 0);

	gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

void playerSurroundWithExplosions(s32 arg0)
{
	g_Vars.currentplayer->bondexploding = true;
	g_Vars.currentplayer->bondnextexplode = arg0 + g_Vars.lvframe60;
	g_Vars.currentplayer->bondcurexplode = 0;
}

void playerTickExplode(void)
{
	g_Vars.currentplayer->bondcurexplode++;

	if (g_Vars.currentplayer->bondexploding && !g_PlayerInvincible
			&& g_Vars.lvframe60 > g_Vars.currentplayer->bondnextexplode) {
		struct coord pos;

		pos.x = g_Vars.currentplayer->prop->pos.x;
		pos.y = g_Vars.currentplayer->prop->pos.y;
		pos.z = g_Vars.currentplayer->prop->pos.z;

		switch (g_Vars.currentplayer->bondcurexplode % 4) {
		case 0: pos.x += 250.0f + 150.0f * RANDOMFRAC(); break;
		case 1: pos.x -= 250.0f + 150.0f * RANDOMFRAC(); break;
		case 2: pos.z += 250.0f + 150.0f * RANDOMFRAC(); break;
		case 3: pos.z -= 250.0f + 150.0f * RANDOMFRAC(); break;
		}

		pos.y += 200.0f * RANDOMFRAC() - 100.0f;

		explosionCreateSimple(NULL, &pos, g_Vars.currentplayer->prop->rooms, EXPLOSIONTYPE_BONDEXPLODE, g_Vars.currentplayernum);

		g_Vars.currentplayer->bondnextexplode = g_Vars.lvframe60 + TICKS(15) + (rngRandom() % TICKS(15));
	}
}

void playerResetLoResIf4Mb(void)
{
	if (IS4MB()) {
#if VERSION >= VERSION_PAL_BETA
		g_ViModes[VIRES_LO].fbwidth = FBALLOC_WIDTH_LO;
		g_ViModes[VIRES_LO].fbheight = FBALLOC_HEIGHT_LO;
		g_ViModes[VIRES_LO].width = FBALLOC_WIDTH_LO;
		g_ViModes[VIRES_LO].yscale = 1;
		g_ViModes[VIRES_LO].xscale = VIMODE_LO;
		g_ViModes[VIRES_LO].fullheight = FBALLOC_HEIGHT_LO;
		g_ViModes[VIRES_LO].fulltop = 0;
#else
		g_ViModes[VIRES_LO].fbheight = FBALLOC_HEIGHT_LO;
		g_ViModes[VIRES_LO].fulltop = 0;
		g_ViModes[VIRES_LO].fullheight = FBALLOC_HEIGHT_LO;
#endif

		g_ViModes[VIRES_LO].wideheight = 180;
		g_ViModes[VIRES_LO].widetop = 20;
		g_ViModes[VIRES_LO].cinemaheight = 136;
		g_ViModes[VIRES_LO].cinematop = 42;
	}
}

void playerSetHiResEnabled(bool enable)
{
#ifdef PLATFORM_N64
	g_HiResEnabled = enable;
#else
	g_HiResEnabled = false;
#endif
}

s16 playerGetFbWidth(void)
{
	s16 width = g_ViModes[g_ViRes].fbwidth;
	return width;
}

s16 playerGetFbHeight(void)
{
	s16 height = g_ViModes[g_ViRes].fbheight;

	if (g_Vars.fourmeg2player) {
		height = height >> 1;
	}

	return height;
}

#if VERSION >= VERSION_NTSC_1_0
bool playerHasSharedViewport(void)
{
	if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
			&& menuGetRoot() == MENUROOT_MPENDSCREEN
			&& var8009dfc0 == 0) {
		return true;
	}

	return (g_InCutscene && !g_MainIsEndscreen) || menuGetRoot() == MENUROOT_COOPCONTINUE;
}
#endif

s16 playerGetViewportWidth(void)
{
	s16 width;

#if VERSION >= VERSION_NTSC_1_0
	if (!playerHasSharedViewport())
#else
	if ((!g_InCutscene || g_MainIsEndscreen) && menuGetRoot() != MENUROOT_COOPCONTINUE)
#endif
	{
		if (PLAYERCOUNT() >= 3) {
			// 3/4 players
			width = g_ViModes[g_ViRes].width / 2;

			if (g_Vars.currentplayernum == 0 || g_Vars.currentplayernum == 2) {
				width--;
			}
		} else if (PLAYERCOUNT() == 2) {
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || g_Vars.fourmeg2player) {
				// 2 players vsplit
				width = g_ViModes[g_ViRes].width / 2;

				if (g_Vars.currentplayernum == 0) {
					width--;
				}
			} else {
				// 2 players full width
				width = g_ViModes[g_ViRes].width;
			}
		} else {
			// 1 player
			width = g_ViModes[g_ViRes].width;
		}
	} else {
		// Probably cutscene
		width = g_ViModes[g_ViRes].width;
	}

	return width;
}

s16 playerGetViewportLeft(void)
{
#if VERSION >= VERSION_NTSC_1_0
	s32 something = !playerHasSharedViewport();
#else
	s32 something = !((g_InCutscene && !g_MainIsEndscreen) || menuGetRoot() == MENUROOT_COOPCONTINUE);
#endif
	s16 left;

	if (PLAYERCOUNT() >= 3 && something != 0) {
		if (g_Vars.currentplayernum == 1 || g_Vars.currentplayernum == 3) {
			// 3/4 players - right side
			left = g_ViModes[g_ViRes].width / 2 + g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
		} else {
			// 3/4 players - left side
			left = g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
		}
	} else if (PLAYERCOUNT() == 2 && something != 0) {
		if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || g_Vars.fourmeg2player) {
			if (g_Vars.currentplayernum == 1) {
				// 2 players vsplit - right side
				left = (g_ViModes[g_ViRes].width / 2) + g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
			} else {
				// 2 players vsplit - left side
				left = g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
			}
		} else {
			// 2 players - full width
			left = g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
		}
	} else {
		// Full screen
		left = g_ViModes[g_ViRes].fbwidth - g_ViModes[g_ViRes].width;
	}

	return left;
}

s16 playerGetViewportHeight(void)
{
	s16 height;

	if (PLAYERCOUNT() >= 2
#if VERSION >= VERSION_NTSC_1_0
			&& !playerHasSharedViewport()
#else
			&& !((g_InCutscene && !g_MainIsEndscreen) || menuGetRoot() == MENUROOT_COOPCONTINUE)
#endif
			) {
		s16 tmp = g_ViModes[g_ViRes].fullheight;

		if (IS4MB() && !g_Vars.fourmeg2player) {
			height = tmp;
		} else {
			height = tmp / 2;
		}

		if (PLAYERCOUNT() == 2) {
			if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
				height = tmp;
			} else if (g_Vars.currentplayernum == 0 && IS8MB()) {
				height--;
			}
		} else if (g_Vars.currentplayernum == 0 || g_Vars.currentplayernum == 1) {
			height--;
		}
	} else {
		if (optionsGetEffectiveScreenSize() == SCREENSIZE_WIDE) {
			height = g_ViModes[g_ViRes].wideheight;
		} else if (optionsGetEffectiveScreenSize() == SCREENSIZE_CINEMA) {
			height = g_ViModes[g_ViRes].cinemaheight;
		} else if (g_InCutscene && !var8009dfc0) {
			if (g_CutsceneTweenDuration60 >= 1) {
				f32 a = g_ViModes[g_ViRes].wideheight;
				f32 b = g_ViModes[g_ViRes].fullheight;
				a = a * (1.0f - g_CutsceneTweenFrac);
				b = b * g_CutsceneTweenFrac;
				height = a + b;
			} else {
				height = g_ViModes[g_ViRes].wideheight;
			}
		} else {
			height = g_ViModes[g_ViRes].fullheight;
		}
	}

	return height;
}

s16 playerGetViewportTop(void)
{
	s16 top;

	if (PLAYERCOUNT() >= 2
#if VERSION >= VERSION_NTSC_1_0
			&& !playerHasSharedViewport()
#else
			&& (!g_InCutscene || g_MainIsEndscreen)
			&& menuGetRoot() != MENUROOT_COOPCONTINUE
#endif
			) {
		top = g_ViModes[g_ViRes].fulltop;

#if VERSION >= VERSION_NTSC_1_0
		if (optionsGetScreenSplit() != SCREENSPLIT_VERTICAL || PLAYERCOUNT() != 2)
#else
		if (optionsGetScreenSplit() != SCREENSPLIT_VERTICAL)
#endif
		{
			if (PLAYERCOUNT() == 2
					&& g_Vars.currentplayernum == 1
					&& optionsGetScreenSplit() != SCREENSPLIT_VERTICAL
					&& !g_Vars.fourmeg2player) {
				// 2 players hsplit - bottom side
				top = g_ViModes[g_ViRes].fulltop + g_ViModes[g_ViRes].fullheight / 2;
			} else if (g_Vars.currentplayernum == 2 || g_Vars.currentplayernum == 3) {
				// 3/4 players - bottom side
				top = g_ViModes[g_ViRes].fulltop + g_ViModes[g_ViRes].fullheight / 2;
			}
		}
	} else {
		if (optionsGetEffectiveScreenSize() == SCREENSIZE_WIDE) {
			if (g_InCutscene && optionsGetCutsceneSubtitles() && g_Vars.stagenum != STAGE_CITRAINING) {
				if (g_CutsceneTweenDuration60 >= 1) {
					f32 a = g_ViModes[g_ViRes].fulltop;
					f32 b = g_ViModes[g_ViRes].widetop;
					a = a * (1.0f - g_CutsceneTweenFrac);
					b = b * g_CutsceneTweenFrac;
					top = a + b;
				} else {
					top = g_ViModes[g_ViRes].fulltop;
				}
			} else {
				top = g_ViModes[g_ViRes].widetop;
			}
		} else if (optionsGetEffectiveScreenSize() == SCREENSIZE_CINEMA) {
			top = g_ViModes[g_ViRes].cinematop;
		} else {
			if (g_InCutscene && !var8009dfc0
					&& (!optionsGetCutsceneSubtitles() || g_Vars.stagenum == STAGE_CITRAINING)) {
				if (g_CutsceneTweenDuration60 >= 1) {
					f32 a = g_ViModes[g_ViRes].widetop;
					f32 b = g_ViModes[g_ViRes].fulltop;
					a = a * (1.0f - g_CutsceneTweenFrac);
					b = b * g_CutsceneTweenFrac;
					top = a + b;
				} else {
					top = g_ViModes[g_ViRes].widetop;
				}
			} else {
				return g_ViModes[g_ViRes].fulltop;
			}
		}
	}

	return top;
}

f32 player0f0bd358(void)
{
	f32 result;
	s16 stack;
	s16 height = playerGetViewportHeight();
	s16 width = playerGetViewportWidth();

	result = (f32)width / (f32)height;
	result = g_ViModes[g_ViRes].yscale * result;

#ifdef PLATFORM_N64
	return result;
#else
	return result * (videoGetAspect() / ((f32)SCREEN_WIDTH_LO / (f32)SCREEN_HEIGHT_LO));
#endif
}

void playerUpdateShake(void)
{
	struct coord coord = {0, 0, 0};

	if (g_Vars.currentplayer->isdead == false) {
		explosionsUpdateShake(&g_Vars.currentplayer->bond2.unk10, &g_Vars.currentplayer->bond2.unk1c, &coord);
	} else {
		viShake(0);
	}
}

void playerAutoWalk(s16 aimpad, u8 walkspeed, u8 turnspeed, u8 lookup, u8 dist)
{
	playerSetTickMode(TICKMODE_AUTOWALK);

	// Prevents momentum from being preserved. Fixes potential softlock during The Duel.
	g_Vars.currentplayer->resetheadpos = true;

	g_Vars.currentplayer->autocontrol_aimpad = aimpad;
	g_Vars.currentplayer->autocontrol_walkspeed = walkspeed;
	g_Vars.currentplayer->autocontrol_turnspeed = turnspeed;
	g_Vars.currentplayer->autocontrol_lookup = lookup;
	g_Vars.currentplayer->autocontrol_dist = dist;
}

void playerLaunchSlayerRocket(struct weaponobj *rocket)
{
	g_Vars.currentplayer->slayerrocket = rocket;
	g_Vars.currentplayer->visionmode = VISIONMODE_SLAYERROCKET;

	// Turn off these devices
	g_Vars.currentplayer->devicesactive &= ~(
			DEVICE_NIGHTVISION |
			DEVICE_XRAYSCANNER |
			DEVICE_EYESPY |
			DEVICE_IRSCANNER);

	g_Vars.currentplayer->badrockettime = 0;
}

void playerTickTeleport(f32 *aspectratio)
{
	if (g_Vars.currentplayer->teleportstate) {
		// empty
	}

	// State 1: TELEPORTSTATE_PREENTER
	// Wait in this state for 24 ticks
	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER) {
		u32 time = g_Vars.currentplayer->teleporttime + g_Vars.lvupdate60;

		if (time >= 24) {
			g_Vars.currentplayer->teleporttime = 0;
			g_Vars.currentplayer->teleportstate = TELEPORTSTATE_ENTERING;
		} else {
			g_Vars.currentplayer->teleporttime = time;
		}
	}

	// State 2: TELEPORTSTATE_ENTERING
	// Adjust aspect ratio over 48 ticks
	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		u32 time = g_Vars.currentplayer->teleporttime + g_Vars.lvupdate60;

		if (g_Vars.currentplayer->teleporttime == 48) {
			g_Vars.currentplayer->teleportstate = TELEPORTSTATE_WHITE;
			g_Vars.currentplayer->teleporttime = 0;
		} else if (time >= 48) {
			g_Vars.currentplayer->teleporttime = 48;
		} else {
			f32 tmp = 1 - cosf((time / 48.0f) * M_PI * 0.5f);
			g_Vars.currentplayer->teleporttime = time;
			*aspectratio = *aspectratio / (1.0f + 4.0f * tmp);
		}
	}

	// State 3: TELEPORTSTATE_WHITE
	// Wait indefinitely for AI scripting to progress it to state 4

	// State 4: TELEPORTSTATE_EXITING
	// Adjust aspect ratio over 48 ticks, but with slightly faster
	// time progression in the first several ticks.
	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_EXITING) {
		u32 time = g_Vars.currentplayer->teleporttime + g_Vars.lvupdate60;

		if (g_Vars.currentplayer->teleporttime < 7) {
			time = g_Vars.currentplayer->teleporttime + 1;
		}

		if (time >= 48) {
			g_Vars.currentplayer->teleporttime = 0;
			g_Vars.currentplayer->teleportstate = TELEPORTSTATE_INACTIVE;
		} else {
			f32 tmp = 1 - cosf(((47 - time) / 48.0f) * M_PI * 0.5f);
			g_Vars.currentplayer->teleporttime = time;
			*aspectratio = *aspectratio * (1.0f + 4.0f * tmp);
		}
	}

	if (g_Vars.currentplayer->teleportstate != TELEPORTSTATE_INACTIVE) {
		f32 fovy = playerGetTeleportFovY();
		playermgrSetFovY(fovy);
		viSetFovY(fovy);
	}
}

void playerConfigureVi(void)
{
	f32 ratio = player0f0bd358();
	g_ViRes = VIRES_LO;

	text0f1531dc(false);

#if VERSION >= VERSION_JPN_FINAL
	var800800f0jf = 0;
#endif

	playermgrSetFovY(PLAYER_DEFAULT_FOV);
	playermgrSetAspectRatio(ratio);
	playermgrSetViewSize(playerGetViewportWidth(), playerGetViewportHeight());
	playermgrSetViewPosition(playerGetViewportLeft(), playerGetViewportTop());

	viSetMode(g_ViModes[g_ViRes].xscale);

	viSetFovAspectAndSize(PLAYER_DEFAULT_FOV, ratio, playerGetViewportWidth(), playerGetViewportHeight());

	viSetViewPosition(playerGetViewportLeft(), playerGetViewportTop());
	viSetSize(playerGetFbWidth(), playerGetFbHeight());
	viSetBufSize(playerGetFbWidth(), playerGetFbHeight());
}

void playerTick(bool arg0)
{
	f32 aspectratio;
	f32 f20;

	g_ViRes = g_HiResEnabled;

	if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && PLAYERCOUNT() > 1) {
		g_ViRes = VIRES_LO;
	}

#if PAL
	text0f1531dc(false);
#else
	if (g_ViRes == VIRES_HI) {
		text0f1531dc(true);
	} else {
		text0f1531dc(false);
	}
#endif

#if VERSION >= VERSION_JPN_FINAL
	var800800f0jf = 0;
#endif

#ifdef PLATFORM_N64
	if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
		aspectratio = player0f0bd358() * 1.33333333f;
	} else {
		aspectratio = player0f0bd358();
	}
#else
	aspectratio = player0f0bd358();
#endif

#if PAL
	aspectratio *= 1.1904761791229f;
#endif

	mainOverrideVariable("tps", &var8007083c);

	if (var8007083c != TELEPORTSTATE_INACTIVE) {
		var8007083c = TELEPORTSTATE_INACTIVE;
		g_Vars.currentplayer->teleporttime = 0;
		g_Vars.currentplayer->teleportstate = TELEPORTSTATE_PREENTER;
	}

	if (g_Vars.currentplayer->teleportstate != TELEPORTSTATE_INACTIVE) {
		playerTickTeleport(&aspectratio);
	}

	if (g_Vars.stagenum == STAGE_TEST_OLD && func0f01ad5c()) {
		func0f01adb8();
		return;
	}

	playermgrSetFovY(PLAYER_DEFAULT_FOV);
	playermgrSetAspectRatio(aspectratio);
	playermgrSetViewSize(playerGetViewportWidth(), playerGetViewportHeight());
	playermgrSetViewPosition(playerGetViewportLeft(), playerGetViewportTop());

	viSetMode(g_ViModes[g_ViRes].xscale);
	viSetFovAspectAndSize(PLAYER_DEFAULT_FOV, aspectratio, playerGetViewportWidth(), playerGetViewportHeight());
	viSetViewPosition(playerGetViewportLeft(), playerGetViewportTop());
	viSetSize(playerGetFbWidth(), playerGetFbHeight());
	viSetBufSize(playerGetFbWidth(), playerGetFbHeight());

	playerUpdateColourScreenProperties();
	playerTickChrFade();

	bmoveSetAutoAimY(optionsGetAutoAim(g_Vars.currentplayerstats->mpindex));
	bmoveSetAutoAimX(optionsGetAutoAim(g_Vars.currentplayerstats->mpindex));
	bmoveSetAutoMoveCentreEnabled(optionsGetLookAhead(g_Vars.currentplayerstats->mpindex));
	bgunSetGunAmmoVisible(GUNAMMOREASON_OPTION, optionsGetAmmoOnScreen(g_Vars.currentplayerstats->mpindex));
	bgunSetSightVisible(GUNSIGHTREASON_1, true);

	if ((g_Vars.tickmode == TICKMODE_GE_FADEIN || g_Vars.tickmode == TICKMODE_NORMAL) && !g_InCutscene && !g_MainIsEndscreen) {
		g_Vars.currentplayer->bondviewlevtime60 += g_Vars.lvupdate60;
	}

	if (g_Vars.currentplayer->devicesactive & DEVICE_SUICIDEPILL) {
		playerDieByShooter(g_Vars.currentplayernum, true);
	}

	playerTickDamageAndHealth();
	playerTickExplode();

	if (g_Vars.currentplayer->eyespy) {
		// The stage uses an eyespy
		struct eyespy *eyespy = g_Vars.currentplayer->eyespy;
		u32 playernum = g_Vars.currentplayernum;

		if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
			// Turn off the eyespy if active
			struct chrdata *chr = eyespy->prop->chr;
			eyespy->deployed = false;
			eyespy->held = true;
			eyespy->active = false;
			psStopSound(eyespy->prop, PSTYPE_GENERAL, 0xffff);
			chr->chrflags |= CHRCFLAG_HIDDEN;
			chr->chrflags |= CHRCFLAG_INVINCIBLE;
			g_Vars.currentplayer->devicesactive &= ~DEVICE_EYESPY;
		} else {
			if (eyespy->held == false) {
				// Eyespy is deployed
#if VERSION >= VERSION_NTSC_1_0
				if (g_Vars.currentplayer->eyespy->active) {
					// And is being controlled
					s8 contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
					u32 buttons = arg0 ? joyGetButtons(contpad1, 0xffffffff) : 0;

#ifndef PLATFORM_N64
					if (arg0 && inputKeyJustPressed(VK_ESCAPE)) {
						buttons |= START_BUTTON;
					}
#endif

					if (g_Vars.currentplayer->isdead == false
							&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED
							&& (buttons & START_BUTTON)) {
						if (g_Vars.mplayerisrunning == false) {
							playerPause(MENUROOT_MAINMENU);
						} else {
							mpPushPauseDialog();
						}
					}
				}
#endif

				if (g_Vars.lvupdate240) {
					eyespyProcessInput(arg0);
				}
			} else {
				// Eyespy is held
				// If eyespy is activated, launch it
				if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)
						&& g_PlayersWithControl[playernum]
						&& !eyespyTryLaunch()) {
					// Launch failed
					eyespy->held = true;
					eyespy->active = false;
					g_Vars.currentplayer->devicesactive &= ~DEVICE_EYESPY;
				}
			}

			if (eyespy->deployed
					&& g_PlayersWithControl[playernum]
					&& (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)) {
				// Eyespy is being controlled
				if (eyespy->active == false) {
					// Eyespy is being turned off
					eyespy->active = true;
					eyespy->buttonheld = eyespy->camerabuttonheld = false;
					eyespy->camerashuttertime = 0;
					eyespy->startuptimer60 = 0;
					eyespy->prop->chr->soundtimer = TICKS(10);
					sndStart(var80095200, SFX_DETONATE, NULL, -1, -1, -1, -1, -1);
				}

				g_Vars.currentplayer->invdowntime = TICKS(-40);
			}
		}
	}

	if (lvIsPaused()) {
		playerStopAudioForPause();
	}

	if (g_Vars.currentplayer->pausemode != PAUSEMODE_UNPAUSED) {
		playerTickPauseMenu();
	}

	if (g_Vars.currentplayer->visionmode == VISIONMODE_SLAYERROCKET) {
		if (g_Vars.currentplayer->slayerrocket == NULL || g_Vars.currentplayer->isdead) {
			g_Vars.currentplayer->slayerrocket = NULL;
#if VERSION >= VERSION_NTSC_1_0
			g_Vars.currentplayer->visionmode = VISIONMODE_SLAYERROCKETSTATIC;
#else
			g_Vars.currentplayer->visionmode = VISIONMODE_NORMAL;
#endif
		}
	}

	if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
		g_InCutscene = false;
	}

	if (g_Vars.tickmode == (u32)TICKMODE_CUTSCENE) {
		// In a cutscene
		s32 i;

		playerTickChrBody();

		if (g_Vars.currentplayer->haschrbody) {
			g_Vars.currentplayer->invdowntime = TICKS(-40);
			bmoveTick(0, 0, 0, 1);
			playerTickCutscene(arg0);
			g_Vars.currentplayer->invdowntime = TICKS(-40);
		}

		for (i = 0; i < PLAYERCOUNT(); i++) {
			g_Vars.players[i]->joybutinhibit = 0xffffffff;
		}
	} else if (g_Vars.currentplayer->eyespy
			&& (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)
			&& g_Vars.currentplayer->eyespy->active) {
		// Controlling an eyespy
		struct coord sp308;
		playermgrSetFovY(120);
		viSetFovY(120);
		sp308.x = g_Vars.currentplayer->eyespy->prop->pos.x;
		sp308.y = g_Vars.currentplayer->eyespy->prop->pos.y;
		sp308.z = g_Vars.currentplayer->eyespy->prop->pos.z;
		playerTickChrBody();
		bmoveTick(0, 0, 0, 1);
		playerSetCameraMode(CAMERAMODE_EYESPY);
#if VERSION >= VERSION_JPN_FINAL
		player0f0c1840(&sp308, &g_Vars.currentplayer->eyespy->up, &g_Vars.currentplayer->eyespy->look,
				&g_Vars.currentplayer->eyespy->prop->pos, g_Vars.currentplayer->eyespy->prop->rooms);
#else
		player0f0c1bd8(&sp308, &g_Vars.currentplayer->eyespy->up, &g_Vars.currentplayer->eyespy->look);
#endif
	} else if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_WHITE) {
		// Deep Sea teleport
		playerTickChrBody();
		g_WarpType1Pad = g_Vars.currentplayer->teleportcamerapad;
		bmoveTick(0, 0, 0, 1);
		playerExecutePreparedWarp();
	} else if (g_Vars.currentplayer->visionmode == (u32)VISIONMODE_SLAYERROCKET) {
		// Controlling a Slayer rocket
		struct coord rocketpos = {0, 0, 0};
		struct coord sp2f0 = {0, 0, 1};
		struct coord sp2e4 = {0, 1, 0};

		bool rocketok = false;
		struct weaponobj *rocket = g_Vars.currentplayer->slayerrocket;

		playerSetCameraMode(CAMERAMODE_THIRDPERSON);
		playerTickChrBody();
		bmoveTick(0, 0, 0, 1);
		playerUpdateShake();

		if (rocket && rocket->base.prop) {
			f32 sp2b8[3][3];
			struct coord sp2ac;
			f32 sp2a8 = sqrtf(
					rocket->base.realrot[0][0] * rocket->base.realrot[0][0] +
					rocket->base.realrot[1][0] * rocket->base.realrot[1][0] +
					rocket->base.realrot[2][0] * rocket->base.realrot[2][0]);
			RoomNum inrooms[21];
			RoomNum aboverooms[21];
			RoomNum bestroom;
			s16 outofbounds = false;

			sp2b8[0][0] = rocket->base.realrot[0][0] / sp2a8;
			sp2b8[0][1] = rocket->base.realrot[0][1] / sp2a8;
			sp2b8[0][2] = rocket->base.realrot[0][2] / sp2a8;
			sp2b8[1][0] = rocket->base.realrot[1][0] / sp2a8;
			sp2b8[1][1] = rocket->base.realrot[1][1] / sp2a8;
			sp2b8[1][2] = rocket->base.realrot[1][2] / sp2a8;
			sp2b8[2][0] = rocket->base.realrot[2][0] / sp2a8;
			sp2b8[2][1] = rocket->base.realrot[2][1] / sp2a8;
			sp2b8[2][2] = rocket->base.realrot[2][2] / sp2a8;

			rocketpos.x = rocket->base.prop->pos.x;
			rocketpos.y = rocket->base.prop->pos.y;
			rocketpos.z = rocket->base.prop->pos.z;

			bgFindRoomsByPos(&rocketpos, inrooms, aboverooms, 20, &bestroom);

			if (inrooms[0] == -1) {
				outofbounds = true;
			}

			if (outofbounds) {
				// Slayer rocket has flown out of bounds
				// Allow 2 seconds of this, then blow up rocket
				g_Vars.currentplayer->badrockettime += g_Vars.lvupdate60;

				if (g_Vars.currentplayer->badrockettime > TICKS(120)) {
#if VERSION >= VERSION_NTSC_1_0
					g_Vars.currentplayer->visionmode = VISIONMODE_SLAYERROCKETSTATIC;
#else
					g_Vars.currentplayer->visionmode = VISIONMODE_NORMAL;
#endif
				}
			} else if (g_Vars.currentplayer->badrockettime > 0) {
				// Slayer rocket is in bounds, but was recently out
				g_Vars.currentplayer->badrockettime -= g_Vars.lvupdate60;

				if (g_Vars.currentplayer->badrockettime < 0) {
					g_Vars.currentplayer->badrockettime = 0;
				}
			}

			mtx00016208(sp2b8, &sp2f0);
			mtx00016208(sp2b8, &sp2e4);

			if (rocket->base.hidden & OBJHFLAG_PROJECTILE) {
				struct projectile *projectile = rocket->base.projectile;
				u32 mode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);
				f32 targetspeed;
				s8 contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
				s8 contpad2 = optionsGetContpadNum2(g_Vars.currentplayerstats->mpindex);
				s8 stickx = 0;
				s8 sticky = 0;
#ifndef PLATFORM_N64
				s8 rsticky = joyGetRStickY(contpad1);
#endif
				Mtxf sp1fc;
				Mtxf sp1bc;
				Mtxf sp17c;
				f32 sp178;
				f32 sp174;
				f32 sp15c[6];
				f32 sp14c[4];
				f32 sp13c[4];
				f32 sp12c[4];
				f32 prevspeed;
#ifdef AVOID_UB
				f32 sp11c[4];
#else
				f32 sp11c[3];
#endif
				bool explode = false;
				// NOTE: slayer handling
				bool slow = false;
				bool pause = false;
				f32 newspeed;

				if (mode == CONTROLMODE_23
						|| mode == CONTROLMODE_24
						|| mode == CONTROLMODE_22
						|| mode == CONTROLMODE_21) {
					if (g_PlayersWithControl[g_Vars.currentplayernum]) {
						if (mode == CONTROLMODE_21 || mode == CONTROLMODE_22) {
							if (joyGetButtons(contpad1, A_BUTTON | B_BUTTON)
									|| joyGetButtons(contpad2, A_BUTTON | B_BUTTON)
									|| joyGetButtons(contpad2, Z_TRIG)) {
								slow = true;
							}

							if (joyGetButtonsPressedThisFrame(contpad1, Z_TRIG)) {
								explode = true;
							}
						} else {
							if (joyGetButtons(contpad1, A_BUTTON | B_BUTTON)
									|| joyGetButtons(contpad2, A_BUTTON | B_BUTTON)
									|| joyGetButtons(contpad1, Z_TRIG)) {
								slow = true;
							}

							if (joyGetButtonsPressedThisFrame(contpad2, Z_TRIG)) {
								explode = true;
							}
						}

						stickx = joyGetStickX(contpad1);
						sticky = joyGetStickY(contpad1);
					} else {
						slow = true;
					}

					if (joyGetButtons(contpad1, START_BUTTON) || joyGetButtons(contpad2, START_BUTTON)) {
						pause = true;
					}
				} else {
					if (g_PlayersWithControl[g_Vars.currentplayernum]) {
						if (mode == CONTROLMODE_13 || mode == CONTROLMODE_14) {
							if (joyGetButtonsPressedThisFrame(contpad1, A_BUTTON)) {
								explode = true;
							}

							if (joyGetButtons(contpad1, B_BUTTON | Z_TRIG | R_TRIG)) {
								slow = true;
							}
						} else {
							if (joyGetButtonsPressedThisFrame(contpad1, Z_TRIG)) {
								explode = true;
							}

							if (joyGetButtons(contpad1, A_BUTTON | B_BUTTON | R_TRIG)) {
								slow = true;
							}
						}

						stickx = joyGetStickX(contpad1);
						sticky = joyGetStickY(contpad1);
					} else {
						slow = true;
					}

					if (joyGetButtons(contpad1, START_BUTTON)) {
						pause = true;
					}
				}

#ifndef PLATFORM_N64
				if (g_PlayersWithControl[g_Vars.currentplayernum] && inputKeyJustPressed(VK_ESCAPE)) {
					pause = true;
				}
#endif

				if (pause) {
					if (g_Vars.mplayerisrunning == false) {
						playerPause(MENUROOT_MAINMENU);
					} else {
						mpPushPauseDialog();
					}
				}

				rocketok = true;
				sp2ac.x = sp2b8[0][0];
				sp2ac.z = sp2b8[0][2];

				sp178 = sticky * LVUPDATE60FREAL() * 0.00025f;
				sp174 = -stickx * LVUPDATE60FREAL() * 0.00025f;

#ifndef PLATFORM_N64
				// respect the invert pitch setting
				if (optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex)) {
					sp178 = -sp178;
				}
				// mouse control
				if (g_Vars.currentplayernum == 0) {
					f32 mdx, mdy;
					inputMouseGetScaledDelta(&mdx, &mdy);
					if (mdx || mdy) {
						mdx *= 0.022f;
						mdy *= 0.022f;
						mdx = (mdx < -128.f) ? -128.f : (mdx > 127.f) ? 127.f : mdx;
						mdy = (mdy < -128.f) ? -128.f : (mdy > 127.f) ? 127.f : mdy;
						if (g_Vars.currentplayerstats && !optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex)) {
							mdy = -mdy;
						}
						sp178 += mdy;
						sp174 -= mdx;
					}
				}

				// Gyro control
				{
					f32 gdx_cam = 0.f, gdy_cam = 0.f, gdz_cam = 0.f;
					f32 gdx_crosshair = 0.f, gdy_crosshair = 0.f;
					s32 cidx = g_Vars.currentplayernum;

					// Apply gyro movement based on mode
					if (inputGetGyroAimMode(cidx) == GYRO_AIM_CAMERA || inputGetGyroAimMode(cidx) == GYRO_AIM_BOTH) {
						inputGyroGetScaledDelta(cidx, &gdx_cam, &gdy_cam, &gdz_cam); // Camera movement
					}

					if (inputGetGyroAimMode(cidx) == GYRO_AIM_CROSSHAIR || inputGetGyroAimMode(cidx) == GYRO_AIM_BOTH) {
						inputGyroGetScaledDeltaCrosshair(cidx, &gdx_crosshair, &gdy_crosshair); // Crosshair movement
					}

					// Apply gyro movement only when detected
					if (gdx_cam || gdy_cam || gdx_crosshair || gdy_crosshair) {
						gdx_cam *= 0.022f;
						gdy_cam *= 0.022f;
						gdx_crosshair *= 0.022f;
						gdy_crosshair *= 0.022f;

						// Clamping to match mouse sensitivity limits
						gdx_cam = (gdx_cam < -128.f) ? -128.f : (gdx_cam > 127.f) ? 127.f : gdx_cam;
						gdy_cam = (gdy_cam < -128.f) ? -128.f : (gdy_cam > 127.f) ? 127.f : gdy_cam;
						gdx_crosshair = (gdx_crosshair < -128.f) ? -128.f : (gdx_crosshair > 127.f) ? 127.f : gdx_crosshair;
						gdy_crosshair = (gdy_crosshair < -128.f) ? -128.f : (gdy_crosshair > 127.f) ? 127.f : gdy_crosshair;

						// Respect forward pitch setting
						if (g_Vars.currentplayerstats && !optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex)) {
							gdy_cam = -gdy_cam;
							gdy_crosshair = -gdy_crosshair;
						}

						// Apply movement separately for each mode
						sp178 += gdy_cam;
						sp174 -= gdx_cam;

						g_Vars.currentplayer->swivelpos[0] += gdx_crosshair; // Crosshair movement
						g_Vars.currentplayer->swivelpos[1] += gdy_crosshair;
					}
				}
#endif

				f20 = sqrtf(sp2ac.f[0] * sp2ac.f[0] + sp2ac.f[2] * sp2ac.f[2]);

				sp2ac.x /= f20;
				sp2ac.z /= f20;

				f20 = sinf(sp178);

				sp14c[0] = cosf(sp178);
				sp14c[1] = sp2ac.f[0] * f20;
				sp14c[2] = 0;
				sp14c[3] = sp2ac.f[2] * f20;

				f20 = sinf(sp174);

				sp15c[0] = cosf(sp174);
				sp15c[1] = 0;
				sp15c[2] = sp2b8[1][1] >= 0 ? f20 : -f20;
				sp15c[3] = 0;

				quaternionMultQuaternion(sp15c, sp14c, sp13c);
				quaternionToMtx(sp13c, &sp1fc);
				mtx4RotateVecInPlace(&sp1fc, &projectile->speed);

				projectile->powerlimit240 = -1;
				projectile->flags |= PROJECTILEFLAG_NOTIMELIMIT;
				projectile->unk018 = 0;
				projectile->unk014 = 0;
				projectile->unk010 = 0;

				if ((projectile->flags & PROJECTILEFLAG_LAUNCHING) == 0) {
					projectile->ownerprop = NULL;
				}

				if (explode) {
					rocket->team = TEAM_00;
				}

				prevspeed = sqrtf(
						projectile->speed.f[0] * projectile->speed.f[0] +
						projectile->speed.f[1] * projectile->speed.f[1] +
						projectile->speed.f[2] * projectile->speed.f[2]);

				if (slow) {
					targetspeed = 1;
				} else {
					targetspeed = 12;
				}

#ifndef PLATFORM_N64
				targetspeed += rsticky / 127.f * 12.f;
				if (targetspeed > 12) {
					targetspeed = 12;
				}
				if (targetspeed < 1) {
					targetspeed = 1;
				}
#endif

				newspeed = prevspeed;

				if (prevspeed < targetspeed) {
					newspeed = prevspeed + 0.05f * LVUPDATE60FREAL();

					if (newspeed > targetspeed) {
						newspeed = targetspeed;
					}
				} else if (prevspeed > targetspeed) {
					newspeed = prevspeed - 0.05f * LVUPDATE60FREAL();

					if (newspeed < targetspeed) {
						newspeed = targetspeed;
					}
				}

				projectile->speed.x = (projectile->speed.x * newspeed) / prevspeed;
				projectile->speed.y = (projectile->speed.y * newspeed) / prevspeed;
				projectile->speed.z = (projectile->speed.z * newspeed) / prevspeed;

				mtx3ToMtx4(sp2b8, &sp1bc);
				quaternion0f097044(&sp1bc, sp12c);
				quaternionMultQuaternion(sp13c, sp12c, sp11c);
				quaternionToMtx(sp11c, &sp17c);
				mtx4ToMtx3(&sp17c, sp2b8);

				rocket->base.realrot[0][0] = sp2b8[0][0] * sp2a8;
				rocket->base.realrot[0][1] = sp2b8[0][1] * sp2a8;
				rocket->base.realrot[0][2] = sp2b8[0][2] * sp2a8;
				rocket->base.realrot[1][0] = sp2b8[1][0] * sp2a8;
				rocket->base.realrot[1][1] = sp2b8[1][1] * sp2a8;
				rocket->base.realrot[1][2] = sp2b8[1][2] * sp2a8;
				rocket->base.realrot[2][0] = sp2b8[2][0] * sp2a8;
				rocket->base.realrot[2][1] = sp2b8[2][1] * sp2a8;
				rocket->base.realrot[2][2] = sp2b8[2][2] * sp2a8;
			}
		}

		if (!rocketok) {
			g_Vars.currentplayer->slayerrocket = NULL;
#if VERSION >= VERSION_NTSC_1_0
			g_Vars.currentplayer->visionmode = VISIONMODE_SLAYERROCKETSTATIC;
#else
			g_Vars.currentplayer->visionmode = VISIONMODE_NORMAL;
#endif
		}

		g_Vars.currentplayer->waitforzrelease = true;

		if (rocket && rocket->base.prop) {
			player0f0c1840(&rocketpos, &sp2e4, &sp2f0, &rocket->base.prop->pos, rocket->base.prop->rooms);
		} else {
			player0f0c1840(&rocketpos, &sp2e4, &sp2f0, NULL, NULL);
		}
	} else if (g_Vars.tickmode == TICKMODE_NORMAL) {
		// Normal movement
		f32 a = 0;
		f32 b = 0;
		f32 c = 0;
		struct coord spf4;
		struct prop *prop;
		struct chrdata *chr;
		s32 i;

		playerRemoveChrBody();

		if (g_PlayersWithControl[g_Vars.currentplayernum]) {
			bmoveTick(1, 1, arg0, 0);
		} else {
			bmoveTick(0, 0, 0, 1);
		}

		playerUpdateShake();
		playerSetCameraMode(CAMERAMODE_DEFAULT);

		spf4.x = g_Vars.currentplayer->bond2.unk10.x;
		spf4.y = g_Vars.currentplayer->bond2.unk10.y;
		spf4.z = g_Vars.currentplayer->bond2.unk10.z;

		spf4.x = a + spf4.x;
		spf4.y = b + spf4.y;
		spf4.z = c + spf4.z;

		player0f0c1840(&spf4,
				&g_Vars.currentplayer->bond2.unk28,
				&g_Vars.currentplayer->bond2.unk1c,
				&g_Vars.currentplayer->prop->pos,
				g_Vars.currentplayer->prop->rooms);

		if (g_Vars.normmplayerisrunning == false
				&& g_MissionConfig.iscoop
				&& g_Vars.numaibuddies > 0
				&& !g_Vars.aibuddiesspawned
				&& g_Vars.stagenum != STAGE_CITRAINING
				&& g_Vars.lvframenum > 20) {
			g_Vars.aibuddiesspawned = true;

			// Spawn coop bots
			for (i = 0; i < g_Vars.numaibuddies; i++) {
				prop = NULL;

				// If no buddy cheats are active, spawn Velvet
				if ((g_CheatsActiveBank0 & (
								1 << CHEAT_PUGILIST
								| 1 << CHEAT_HOTSHOT
								| 1 << CHEAT_HITANDRUN
								| 1 << CHEAT_ALIEN)) == 0) {
					if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
						prop = chrSpawnAtCoord(BODY_DARK_COMBAT, HEAD_VD,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta / 2),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MRBLONDE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else {
						prop = chrSpawnAtCoord(BODY_DARK_COMBAT, HEAD_VD,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta / 2),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					}

					if (prop) {
						chr = prop->chr;
						chr->flags |= CHRFLAG0_SKIPSAFETYCHECKS;
						chr->flags2 |= CHRFLAG1_IGNORECOVER | CHRFLAG1_NOOP_00200000 | CHRFLAG1_AIVSAI_ADVANTAGED;
						chr->team = TEAM_ALLY;
						chr->squadron = SQUADRON_01;
						chr->hidden |= CHRHFLAG_DETECTED;
						chr->voicebox = VOICEBOX_FEMALE;
						chr->teamscandist = 50;
						chr->accuracyrating = 100;
						chr->speedrating = 100;

						if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
							chrAddHealth(chr, 40);
						} else {
							chrAddHealth(chr, 20);
						}

						chrSetMaxDamage(chr, 4);

						chr->chrflags |= CHRCFLAG_NEVERSLEEP;
						chr->hidden |= CHRHFLAG_CLOAKED;
						chr->cloakfadefinished = true;
						chr->cloakfadefrac = 0;

						chrGiveWeapon(chr, MODEL_CHRFALCON2, WEAPON_FALCON2, 0);
					}
				}

				if (cheatIsActive(CHEAT_PUGILIST)) {
					if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MRBLONDE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else {
						prop = chrSpawnAtCoord(BODY_CARRINGTON, HEAD_JAMIE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_PUGILIST_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					}

					if (prop) {
						chr = prop->chr;
						chr->flags |= CHRFLAG0_SKIPSAFETYCHECKS | CHRFLAG0_CHUCKNORRIS;
						chr->flags2 |= CHRFLAG1_IGNORECOVER | CHRFLAG1_NOOP_00200000 | CHRFLAG1_AIVSAI_ADVANTAGED | CHRFLAG1_ADJUSTPUNCHSPEED | CHRFLAG1_HANDCOMBATONLY;
						chr->team = TEAM_ALLY;
						chr->squadron = SQUADRON_01;
						chr->teamscandist = 100;
						chr->hidden |= CHRHFLAG_DETECTED;
						chr->voicebox = VOICEBOX_MALE1;
						chr->accuracyrating = 100;
						chr->speedrating = 100;

						if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
							chrAddHealth(chr, 40);
						} else {
							chrAddHealth(chr, 20);
						}

						chr->chrflags |= CHRCFLAG_NEVERSLEEP;
						chr->hidden |= CHRHFLAG_CLOAKED;
						chr->cloakfadefinished = true;
						chr->cloakfadefrac = 0;

						chrSetMaxDamage(chr, 20);
					}
				}

				if (cheatIsActive(CHEAT_HITANDRUN)) {
					if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MRBLONDE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MARK2,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					}

					if (prop) {
						chr = prop->chr;
						chr->flags |= CHRFLAG0_SKIPSAFETYCHECKS;
						chr->flags2 |= CHRFLAG1_PUNCHHARDER | CHRFLAG1_NOOP_00200000 | CHRFLAG1_AIVSAI_ADVANTAGED;
						chr->team = TEAM_ALLY;
						chr->squadron = SQUADRON_01;
						chr->hidden |= CHRHFLAG_DETECTED;
						chr->voicebox = VOICEBOX_MALE2;
						chr->teamscandist = 50;
						chr->accuracyrating = 50;
						chr->speedrating = 100;

						if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
							chrAddHealth(chr, 20);
						} else {
							chrAddHealth(chr, 10);
						}

						chrSetMaxDamage(chr, 10);

						chr->chrflags |= CHRCFLAG_NEVERSLEEP;
						chr->hidden |= CHRHFLAG_CLOAKED;
						chr->cloakfadefinished = true;
						chr->cloakfadefrac = 0;

						chrGiveWeapon(chr, MODEL_CHRAVENGER, WEAPON_K7AVENGER, 0);
					}
				}

				if (cheatIsActive(CHEAT_HOTSHOT)) {
					if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MRBLONDE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else {
						prop = chrSpawnAtCoord(BODY_CISOLDIER, HEAD_CHRIST,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					}

					if (prop) {
						chr = prop->chr;
						chr->flags |= CHRFLAG0_SKIPSAFETYCHECKS;
						chr->flags2 |= CHRFLAG1_IGNORECOVER | CHRFLAG1_NOOP_00200000 | CHRFLAG1_AIVSAI_ADVANTAGED;
						chr->team = TEAM_ALLY;
						chr->squadron = SQUADRON_01;
						chr->hidden |= CHRHFLAG_DETECTED;
						chr->voicebox = VOICEBOX_MALE0;
						chr->teamscandist = 100;
						chr->accuracyrating = 50;
						chr->speedrating = 100;

						if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
							chrAddHealth(chr, 40);
						} else {
							chrAddHealth(chr, 20);
						}

						chrSetMaxDamage(chr, 10);

						chr->chrflags |= CHRCFLAG_NEVERSLEEP;
						chr->hidden |= CHRHFLAG_CLOAKED;
						chr->cloakfadefinished = true;
						chr->cloakfadefrac = 0;

						chrGiveWeapon(chr, MODEL_CHRDY357TRENT, WEAPON_DY357LX, 0);
						chrGiveWeapon(chr, MODEL_CHRDY357, WEAPON_DY357MAGNUM, OBJFLAG_WEAPON_LEFTHANDED);
					}
				}

				if (cheatIsActive(CHEAT_ALIEN)) {
					if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
						prop = chrSpawnAtCoord(BODY_MRBLONDE, HEAD_MRBLONDE,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					} else {
						prop = chrSpawnAtCoord(BODY_ELVIS1, HEAD_MAIAN_S,
								&g_Vars.currentplayer->prop->pos,
								g_Vars.currentplayer->prop->rooms,
								BADDEG2RAD(g_Vars.currentplayer->vv_theta),
								ailistFindById(GAILIST_INIT_DEFAULT_BUDDY),
								SPAWNFLAG_ALLOWONSCREEN);
					}

					if (prop) {
						chr = prop->chr;
						chr->flags |= CHRFLAG0_SKIPSAFETYCHECKS;
						chr->flags2 |= CHRFLAG1_PUNCHHARDER | CHRFLAG1_IGNORECOVER | CHRFLAG1_NOOP_00200000 | CHRFLAG1_AIVSAI_ADVANTAGED;
						chr->team = TEAM_ALLY;
						chr->squadron = SQUADRON_01;
						chr->hidden |= CHRHFLAG_DETECTED;
						chr->voicebox = VOICEBOX_MALE0;
						chr->teamscandist = 150;
						chr->accuracyrating = 100;
						chr->speedrating = 100;

						if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
							chrAddHealth(chr, 40);
						} else {
							chrAddHealth(chr, 20);
						}

						chrSetMaxDamage(chr, 10);

						chr->chrflags |= CHRCFLAG_NEVERSLEEP;
						chr->hidden |= CHRHFLAG_CLOAKED;
						chr->cloakfadefinished = true;
						chr->cloakfadefrac = 0;

						chrGiveWeapon(chr, MODEL_CHRRCP120, WEAPON_RCP120, 0);
					}
				}

				g_Vars.aibuddies[i] = prop;
			}
		}
	} else if (g_Vars.tickmode == TICKMODE_GE_FADEIN || g_Vars.tickmode == TICKMODE_GE_FADEOUT) {
		playerRemoveChrBody();
		bmoveTick(1, 1, arg0, 0);
		playerUpdateShake();
		playerSetCameraMode(CAMERAMODE_DEFAULT);
		player0f0c1840(&g_Vars.currentplayer->bond2.unk10,
				&g_Vars.currentplayer->bond2.unk28,
				&g_Vars.currentplayer->bond2.unk1c,
				&g_Vars.currentplayer->prop->pos,
				g_Vars.currentplayer->prop->rooms);
	} else if (g_Vars.tickmode == TICKMODE_MPSWIRL) {
		// Start of an MP match where the camera circles around the player
		playerTickChrBody();
		bmoveTick(0, 0, 0, 1);
		playerTickMpSwirl();
	} else if (g_Vars.tickmode == TICKMODE_WARP) {
		// Eg. In CI training, warping from device hallways
		// to device room at the end of a training session
		playerTickChrBody();
		bmoveTick(0, 0, 0, 1);
		playerExecutePreparedWarp();
	} else if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
		// Extraction bodyguard room and Duel
		f32 targetangle;
		f32 autodist;
		f32 oldangle;
		f32 xdist;
		f32 zdist;
		f32 diffangle;
		f32 direction;
		struct pad pad;
		f32 speedfrac;

		playerRemoveChrBody();
		padUnpack(g_Vars.currentplayer->autocontrol_aimpad, PADFIELD_POS, &pad);

		if (mainGetStageNum() == g_Stages[STAGEINDEX_EXTRACTION].id
				&& g_Vars.currentplayer->autocontrol_aimpad == 0x19) {
			pad.pos.x -= 100;
		}

		xdist = pad.pos.x - g_Vars.currentplayer->bond2.unk10.x;
		zdist = pad.pos.z - g_Vars.currentplayer->bond2.unk10.z;
		targetangle = atan2f(xdist, zdist);

		if (targetangle > M_BADTAU) {
			targetangle -= M_BADTAU;
		}

		if (targetangle < 0) {
			targetangle += M_BADTAU;
		}

		oldangle = atan2f(g_Vars.currentplayer->bond2.unk00.x, g_Vars.currentplayer->bond2.unk00.z);

		if (oldangle > M_BADTAU) {
			oldangle -= M_BADTAU;
		}

		if (oldangle < 0) {
			oldangle += M_BADTAU;
		}

		diffangle = oldangle - targetangle;

		if (diffangle > M_PI) {
			diffangle -= M_BADTAU;
		}

		if (diffangle < -M_PI) {
			diffangle += M_BADTAU;
		}

		direction = (diffangle / M_PI < 0) ? -1 : 1;

		g_Vars.currentplayer->autocontrol_x = (f32)direction * g_Vars.currentplayer->autocontrol_turnspeed;

		if (!(diffangle < -0.09f || diffangle > 0.09f)) {
			// Facing the target
			g_Vars.currentplayer->autocontrol_x = 0;

			if (g_Vars.currentplayer->autocontrol_walkspeed == 0) {
				g_Vars.currentplayer->autocontrol_turnspeed = 0;
			}
		}

		if (g_Vars.currentplayer->vv_verta <= 30) {
			g_Vars.currentplayer->vv_verta += g_Vars.currentplayer->autocontrol_lookup / 360.0f * M_BADTAU;
		}

		if (g_Vars.currentplayer->autocontrol_walkspeed) {
			xdist = sqrtf(xdist * xdist + zdist * zdist);

			if (xdist < g_Vars.currentplayer->autocontrol_dist) {
				playerSetTickMode(TICKMODE_NORMAL);
			}
		} else {
			if (diffangle >= -0.2f && diffangle <= 0.2f) {
				playerSetTickMode(TICKMODE_NORMAL);
			}
		}

		autodist = g_Vars.currentplayer->autocontrol_dist;

		speedfrac = 1;

		if (xdist < autodist + autodist) {
			if (xdist < autodist) {
				speedfrac = 0;
			} else {
				speedfrac = 0.5f + (xdist - autodist) / autodist * 0.5f;
			}
		}

		g_Vars.currentplayer->autocontrol_y = g_Vars.currentplayer->autocontrol_walkspeed * speedfrac;
		bmoveTick(1, 1, 0, 1);
		playerUpdateShake();
		playerSetCameraMode(CAMERAMODE_DEFAULT);
		player0f0c1840(&g_Vars.currentplayer->bond2.unk10,
				&g_Vars.currentplayer->bond2.unk28,
				&g_Vars.currentplayer->bond2.unk1c,
				&g_Vars.currentplayer->prop->pos,
				g_Vars.currentplayer->prop->rooms);
	}

#ifdef DEBUG
	if (debug0f11ed88()) {
		debug0f119a14nb();
	}
#endif

	// Increment the time on Bond's watch (leftover from GE)
	g_Vars.currentplayer->bondwatchtime60 += g_Vars.diffframe60freal;

	// Also a leftover from GE? Maybe cancelling fade in mission intros?
	if (var8007074c) {
		s8 contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);

		if (!lvIsPaused()
				&& arg0
				&& joyGetButtonsPressedThisFrame(contpad1, A_BUTTON | B_BUTTON | Z_TRIG | START_BUTTON | R_TRIG)) {
			var8007074c = 2;

			if (playerIsFadeComplete()) {
				if (g_Vars.currentplayer->colourscreenfrac == 0) {
					playerSetFadeColour(0, 0, 0, 0);
					playerSetFadeFrac(60, 1);
				}
			} else {
				if (g_Vars.currentplayer->colourfadefracnew == 0) {
					playerSetFadeFrac(g_Vars.currentplayer->colourfadetime60, 1);
				}
			}
		}

		if (var8007074c == 2
				&& playerIsFadeComplete()
				&& g_Vars.currentplayer->colourscreenfrac == 1) {
			func0000e990();
		}
	}

	if (g_PlayerTriggerGeFadeIn) {
		playerBeginGeFadeIn();
	}

	// Handle mission exit on death
	if (g_Vars.currentplayer->isdead) {
		if (g_Vars.currentplayer->redbloodfinished == false) {
			bgunHandlePlayerDead();
		}

		if (g_Vars.currentplayer->redbloodfinished && g_Vars.currentplayer->deathanimfinished) {
			if (g_Vars.mplayerisrunning == false) {
				mainEndStage();
			} else if (g_Vars.coopplayernum >= 0) {
				if (g_Vars.currentplayer == g_Vars.bond
						&& g_Vars.coop->isdead
						&& g_Vars.coop->redbloodfinished
						&& g_Vars.coop->deathanimfinished) {
					mainEndStage();
				} else {
					chrsClearRefsToPlayer(g_Vars.currentplayernum);
				}
			} else if (g_Vars.antiplayernum >= 0 && g_Vars.currentplayer == g_Vars.bond) {
				mainEndStage();
			}
		}
	}

	if (g_Vars.tickmode == TICKMODE_GE_FADEOUT && playerIsFadeComplete()) {
		mainEndStage();
	}
}

#define WIELDMODE_PISTOL   0
#define WIELDMODE_HEAVY    1
#define WIELDMODE_UNARMED  2
#define WIELDMODE_DUALGUNS 3

#define TURNMODE_STAND_NOTURN   0
#define TURNMODE_STAND_SOFTTURN 1
#define TURNMODE_STAND_HARDTURN 2
#define TURNMODE_DUCK_NOTURN    3
#define TURNMODE_DUCK_TURN      4
#define TURNMODE_SQUAT_NOTURN   5
#define TURNMODE_SQUAT_TURN     6

struct attackanimconfig var800709f4 = { ANIM_0281, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.34901028871536, -1.5705462694168, 1.5705462694168, -1.5705462694168, 0,   0   };
struct attackanimconfig var80070a3c = { ANIM_0285, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.34901028871536, -1.5705462694168, 1.5705462694168, -1.5705462694168, 0,   0   };
struct attackanimconfig var80070a84 = { ANIM_0282, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.34901028871536, -1.5705462694168, 1.5705462694168, -1.5705462694168, 1.6, 1.6 };
struct attackanimconfig var80070acc = { ANIM_0286, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.17450514435768, -1.5705462694168, 1.5705462694168, -1.5705462694168, 1.6, 1.6 };
struct attackanimconfig var80070b14 = { ANIM_0283, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.34901028871536, -1.5705462694168, 1.5705462694168, -1.5705462694168, 0,   0   };
struct attackanimconfig var80070b5c = { ANIM_0287, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.17450514435768, -1.5705462694168, 1.5705462694168, -1.5705462694168, 0,   0   };

struct var80070ba4 {
	struct attackanimconfig *animcfg;
	s16 animnum;
	f32 speed;
	f32 startframe;
	f32 endframe;
	f32 unk14;
};

struct var80070ba4 var80070ba4[4][7] = { // [wieldmode][turnmode]
	{
		{ var80065be0,           0,                       0.1,   79, 87,  1.0470308065414  },
		{ &g_WalkAttackAnims[2], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &g_WalkAttackAnims[3], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &var800709f4,          0,                       0.001, 0,  0.1, 1.0470308065414  },
		{ &var800709f4,          0,                       0.503, -1, -1,  1.0470308065414  },
		{ &var80070a3c,          0,                       0.001, 0,  0.1, 0.52351540327072 },
		{ &var80070a3c,          0,                       0.45,  -1, -1,  0.52351540327072 },
	}, {
		{ var800656c0,           0,                       0.05,  35, 40,  1.0470308065414  },
		{ &g_WalkAttackAnims[0], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &g_WalkAttackAnims[1], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &var80070a84,          0,                       0.001, 0,  0.1, 1.0470308065414  },
		{ &var80070a84,          0,                       0.503, -1, -1,  1.0470308065414  },
		{ &var80070acc,          0,                       0.001, 0,  0.1, 0.52351540327072 },
		{ &var80070acc,          0,                       0.45,  -1, -1,  0.52351540327072 },
	}, {
		{ NULL,                  ANIM_006A,               0.25,  0,  -1,  1.0470308065414  },
		{ NULL,                  ANIM_006B,               0.5,   -1, -1,  1.0470308065414  },
		{ NULL,                  ANIM_RUNNING_ONEHANDGUN, 0.5,   -1, -1,  1.0470308065414  },
		{ NULL,                  ANIM_0280,               0.001, 0,  0.1, 1.0470308065414  },
		{ NULL,                  ANIM_0280,               0.503, -1, -1,  1.0470308065414  },
		{ NULL,                  ANIM_0284,               0.001, 0,  0.1, 0.52351540327072 },
		{ NULL,                  ANIM_0284,               0.45,  -1, -1,  0.52351540327072 },
	}, {
		{ var800663d8,           0,                       0.1,   32, 42,  1.0470308065414  },
		{ &g_WalkAttackAnims[4], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &g_WalkAttackAnims[5], 0,                       0.5,   -1, -1,  1.0470308065414  },
		{ &var80070b14,          0,                       0.001, 0,  0.1, 1.0470308065414  },
		{ &var80070b14,          0,                       0.503, -1, -1,  1.0470308065414  },
		{ &var80070b5c,          0,                       0.001, 0,  0.1, 0.52351540327072 },
		{ &var80070b5c,          0,                       0.45,  -1, -1,  0.52351540327072 },
	},
};

void playerSetGlobalDrawWorldOffset(s32 room)
{
	roomGetPos(room, &g_Vars.currentplayer->globaldrawworldoffset);

	g_Vars.currentplayer->globaldrawworldbgoffset.x = g_Vars.currentplayer->globaldrawworldoffset.x;
	g_Vars.currentplayer->globaldrawworldbgoffset.y = g_Vars.currentplayer->globaldrawworldoffset.y;
	g_Vars.currentplayer->globaldrawworldbgoffset.z = g_Vars.currentplayer->globaldrawworldoffset.z;

	roomSetLastForOffset(room);
}

void playerSetGlobalDrawCameraOffset(void)
{
	g_Vars.currentplayer->globaldrawcameraoffset.x = g_Vars.currentplayer->globaldrawworldoffset.x;
	g_Vars.currentplayer->globaldrawcameraoffset.y = g_Vars.currentplayer->globaldrawworldoffset.y;
	g_Vars.currentplayer->globaldrawcameraoffset.z = g_Vars.currentplayer->globaldrawworldoffset.z;

	mtx4RotateVecInPlace(camGetWorldToScreenMtxf(), &g_Vars.currentplayer->globaldrawcameraoffset);
}

void playerAllocateMatrices(struct coord *cam_pos, struct coord *cam_look, struct coord *cam_up)
{
	Mtx spd0;
	LookAt *lookat;
	Mtxf sp8c;
	struct coord sp80;
	struct coord sp74;
	f32 scale;
	Mtxf *s0;
	Mtx *s1;
	s32 i;
	s32 j;

	scale = bgGetScaleBg2Gfx();
	playerSetGlobalDrawWorldOffset(g_Vars.currentplayer->cam_room);

	g_Vars.currentplayer->mtxl005c = gfxAllocateMatrix();
	g_Vars.currentplayer->mtxl0060 = gfxAllocateMatrix();
	g_Vars.currentplayer->mtxf0064 = gfxAllocateMatrix();
	g_Vars.currentplayer->mtxf0068 = gfxAllocateMatrix();

	lookat = gfxAllocateLookAt(2);

	sp74.x = (cam_pos->x - g_Vars.currentplayer->globaldrawworldoffset.x) * scale;
	sp74.y = (cam_pos->y - g_Vars.currentplayer->globaldrawworldoffset.y) * scale;
	sp74.z = (cam_pos->z - g_Vars.currentplayer->globaldrawworldoffset.z) * scale;

	sp80.f[0] = sp74.f[0] + cam_look->f[0];
	sp80.f[1] = sp74.f[1] + cam_look->f[1];
	sp80.f[2] = sp74.f[2] + cam_look->f[2];

	mtx00016874(&sp8c,
			sp74.x, sp74.y, sp74.z,
			cam_look->x, cam_look->y, cam_look->z,
			cam_up->x, cam_up->y, cam_up->z);

	guLookAtReflect(&spd0, lookat,
			sp74.x, sp74.y, sp74.z,
			sp80.x, sp80.y, sp80.z,
			cam_up->x, cam_up->y, cam_up->z);

	mtx00016874(g_Vars.currentplayer->mtxf0064,
			cam_pos->x, cam_pos->y, cam_pos->z,
			cam_look->x, cam_look->y, cam_look->z,
			cam_up->x, cam_up->y, cam_up->z);

	mtx00016b58(g_Vars.currentplayer->mtxf0068,
			cam_pos->x, cam_pos->y, cam_pos->z,
			cam_look->x, cam_look->y, cam_look->z,
			cam_up->x, cam_up->y, cam_up->z);

	s1 = gfxAllocateMatrix();
	s0 = gfxAllocateMatrix();
	mtx4MultMtx4(camGetMtxF1754(), &sp8c, s0);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (s0->m[i][j] > 32000.0f) {
				s0->m[i][j] = 32000.0f;
			} else if (s0->m[i][j] < -32000.0f) {
				s0->m[i][j] = -32000.0f;
			}
		}
	}

	camSetMtxF006c(s0);
	guMtxF2L(s0->m, s1);
	camSetOrthogonalMtxL(s1);
	mtx00015f04(scale, &sp8c);
	guMtxF2L(sp8c.m, g_Vars.currentplayer->mtxl005c);
	mtx00016820(g_Vars.currentplayer->mtxl005c, g_Vars.currentplayer->mtxl0060);
	camSetMtxL173c(g_Vars.currentplayer->mtxl005c);
	camSetMtxL1738(g_Vars.currentplayer->mtxl0060);
	camSetWorldToScreenMtxf(g_Vars.currentplayer->mtxf0064);
	camSetProjectionMtxF(g_Vars.currentplayer->mtxf0068);
	camSetLookAt(lookat);
	cam0f0b5838();
	playerSetGlobalDrawCameraOffset();
}

Gfx *playerUpdateShootRot(Gfx *gdl)
{
	struct coord sp3c;
	struct coord sp30;
	f32 y;
	f32 value;
	f32 rotx;
	f32 roty;

	playerAllocateMatrices(&g_Vars.currentplayer->cam_pos,
			&g_Vars.currentplayer->cam_look,
			&g_Vars.currentplayer->cam_up);
	bgun0f0a0c08(&sp30, &sp3c);
	y = sp3c.y;

	value = sqrtf(sp3c.z * sp3c.z + sp3c.x * sp3c.x);

	rotx = atan2f(y, value);
	rotx += (g_Vars.currentplayer->vv_verta * M_BADTAU) / 360.0f;

	if (rotx >= M_PI) {
		rotx -= M_BADTAU;
	}

	g_Vars.currentplayer->shootrotx = rotx;

	roty = atan2f(-sp3c.x, -sp3c.z);

	if (roty >= M_PI) {
		roty -= M_BADTAU;
	}

	g_Vars.currentplayer->shootroty = roty;

	return gdl;
}

/**
 * Trigger the shield display when the player is damaged while using a shield.
 *
 * May be called while the shield is already being displayed, in which case the
 * effect is restarted.
 */
void playerDisplayShield(void)
{
	if (g_Vars.currentplayer->shieldshowtime < 0) {
		s32 rand = ((g_Vars.currentplayer->shieldshowrnd >> 16) % 200) * 4 + 800;

		g_Vars.currentplayer->shieldshowrnd = rngRandom();
		g_Vars.currentplayer->shieldshowrot = g_Vars.thisframestart240 % rand;
	}

	g_Vars.currentplayer->shieldshowtime = 0;
}

/**
 * Render the current player's shield from the first person perspective.
 */
Gfx *playerRenderShield(Gfx *gdl)
{
	f32 sp90[2];
	f32 sp88[2];
	f32 shield;
	s32 red;
	s32 green;
	s32 blue;
	s32 maxrot;
	f32 maxrotf;
	f32 f20;
	s32 add;

	if (g_Vars.currentplayer->shieldshowtime >= 0) {
		shield = playerGetShieldFrac() * 8;
		maxrot = ((g_Vars.currentplayer->shieldshowrnd >> 16) % 200) * 4 + 800;
		maxrotf = maxrot;
		f20 = (60 - g_Vars.currentplayer->shieldshowtime) * (1.0f / 60.0f);

		g_Vars.currentplayer->shieldshowrot += g_Vars.lvupdate60freal * (0.8f + 2.0f * f20 * f20);

		if (g_Vars.currentplayer->shieldshowrot >= maxrotf) {
			g_Vars.currentplayer->shieldshowrot -= maxrotf;
		}

		f20 = (sinf(g_Vars.currentplayer->shieldshowrot * (M_BADTAU / maxrotf)) + 1) * 0.5f;
		sp90[0] = camGetScreenLeft() + camGetScreenWidth() * f20;

		f20 = (cosf(g_Vars.currentplayer->shieldshowrot * (M_BADTAU / maxrotf)) + 1) * 0.5f;
		sp90[1] = camGetScreenTop() + camGetScreenHeight() * f20;

		sp88[0] = camGetScreenWidth() * (1.0f + 0.002f * ((g_Vars.currentplayer->shieldshowrnd >> 20) % 100) + (g_Vars.currentplayer->shieldshowtime * (0.2f + 0.002f * (g_Vars.currentplayer->shieldshowrnd % 100)) * (1.0f / 60.0f)));
		sp88[1] = camGetScreenHeight() * (1.0f + 0.002f * ((g_Vars.currentplayer->shieldshowrnd >> 24) % 100) + (g_Vars.currentplayer->shieldshowtime * (0.2f + 0.002f * ((g_Vars.currentplayer->shieldshowrnd >> 8) % 100)) * (1.0f / 60.0f)));

		chr0f0295f8(shield, &red, &green, &blue);

		if (g_Vars.currentplayer->shieldshowtime < 30) {
			f20 = 1 - g_Vars.currentplayer->shieldshowtime * (1.0f / 120.0f);
			f20 = 50 * f20 * f20 * f20;
		} else if (g_Vars.currentplayer->shieldshowtime < 60) {
			f20 = (g_Vars.currentplayer->shieldshowtime - (1.0f / 120.0f)) * (1.0f / 120.0f);
			f20 = -30 * f20;
		}

		add = f20;

		red += add;

		if (red > 255) {
			red = 255;
		} else if (red < 0) {
			red = 0;
		}

		green += add;

		if (green > 255) {
			green = 255;
		} else if (green < 0) {
			green = 0;
		}

		blue += add;

		if (blue > 255) {
			blue = 255;
		} else if (blue < 0) {
			blue = 0;
		}

		f20 = 1 - g_Vars.currentplayer->shieldshowtime * (1.0f / 60.0f);
		texSelect(&gdl, &g_TexShieldConfigs[0], 4, 1, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_2CYCLE);
		gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_CLD_SURF2);
		gDPSetEnvColor(gdl++, red, green, blue, (s32)(200 * f20));
		gDPSetPrimColor(gdl++, 0, 0, 0xff, 0xff, 0xff, (s32)(175 * f20 * f20));
		gDPSetCombineMode(gdl++, G_CC_CUSTOM_00, G_CC_CUSTOM_01);

		func0f0b2740(&gdl, sp90, sp88, g_TexShieldConfigs->width, g_TexShieldConfigs->height,
				(g_Vars.currentplayer->shieldshowrnd & 1) != 0,
				(g_Vars.currentplayer->shieldshowrnd & 2) != 0,
				(g_Vars.currentplayer->shieldshowrnd & 4) != 0,
				0);

		g_Vars.currentplayer->shieldshowtime += g_Vars.lvupdate60freal;

		if (g_Vars.currentplayer->shieldshowtime > 60) {
			g_Vars.currentplayer->shieldshowtime = -1;
		}
	}

	return gdl;
}

Gfx *playerRenderHud(Gfx *gdl)
{
	if (g_Vars.currentplayer->cameramode == CAMERAMODE_THIRDPERSON) {
		gdl = boltbeamsRender(gdl);
		gdl = bgRenderArtifacts(gdl);
		gdl = hudmsgsRender(gdl);

		if (g_Vars.currentplayer->isdead == false) {
			gdl = playerDrawStoredFade(gdl);
		}

		if (g_Vars.stagenum == STAGE_ESCAPE) {
			gdl = gasRender(gdl);
		}

		return gdl;
	}

	if (g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY) {
		bgunTickGameplay2();
		gdl = boltbeamsRender(gdl);
		bgunRender(&gdl);
		gdl = lasersightRenderDot(gdl);

		if (g_Vars.currentplayer->visionmode != VISIONMODE_XRAY) {
			gdl = bgRenderArtifacts(gdl);
		}

		if (g_NbombsActive) {
			gdl = nbombRenderOverlay(gdl);
		}

		if (g_Vars.stagenum == STAGE_ESCAPE) {
			gdl = gasRender(gdl);
		}

		gdl = playerRenderShield(gdl);

		// Adjust eyes shutting
		if (g_Vars.currentplayer->eyesshut) {
			if (g_Vars.currentplayer->eyesshutfrac < 0.95f) {
				g_Vars.currentplayer->eyesshutfrac += g_Vars.lvupdate60freal * 0.12f;

				if (g_Vars.currentplayer->eyesshutfrac > 0.95f) {
					g_Vars.currentplayer->eyesshutfrac = 0.95f;
				}
			}
		} else {
			if (g_Vars.currentplayer->eyesshutfrac > 0) {
				g_Vars.currentplayer->eyesshutfrac -= g_Vars.lvupdate60freal * 0.12f;

				if (g_Vars.currentplayer->eyesshutfrac < 0) {
					g_Vars.currentplayer->eyesshutfrac = 0;
				}
			}
		}

		if (g_Vars.currentplayer->isdead == false
				&& g_InCutscene == 0
				&& (!g_Vars.currentplayer->eyespy || (g_Vars.currentplayer->eyespy && !g_Vars.currentplayer->eyespy->active))
				&& ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit) & DEVICE_NIGHTVISION)) {
			gdl = bviewDrawNvLens(gdl);
			gdl = bviewDrawNvBinoculars(gdl);
		} else if (g_Vars.currentplayer->isdead == false
				&& g_InCutscene == 0
				&& (!g_Vars.currentplayer->eyespy || (g_Vars.currentplayer->eyespy && !g_Vars.currentplayer->eyespy->active))
				&& ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit) & DEVICE_IRSCANNER)) {
			gdl = bviewDrawIrLens(gdl);
			gdl = bviewDrawIrBinoculars(gdl);
		}

		if (g_Vars.currentplayer->eyesshutfrac > 0) {
			gdl = playerDrawFade(gdl, 0, 0, 0, g_Vars.currentplayer->eyesshutfrac);
		}
	}

	gdl = player0f0baf84(gdl);

	// Draw menu
	if (g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY && g_Vars.currentplayer->mpmenuon) {
		s32 a = viGetViewLeft();
		s32 b = viGetViewTop();
		s32 c = viGetViewLeft() + viGetViewWidth();
		s32 d = viGetViewTop() + viGetViewHeight();

		gdl = text0f153628(gdl);
		gdl = text0f153a34(gdl, a, b, c, d, 0x000000a0);
		gdl = text0f153780(gdl);
	}

	if (g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY
			&& playerIsHealthVisible()
			&& func0f0f0c68()) {
		gdl = playerRenderHealthBar(gdl);
	}

	if (g_Vars.normmplayerisrunning == false) {
		objectivesCheckAll();
	}

	if (g_Vars.currentplayer->isdead) {
		g_Vars.currentplayer->coopcanrestart = false;

		if (g_Vars.currentplayer->deathanimfinished == false) {
			bool pass = false;

			if (g_Vars.currentplayer->isdead == 1) {
				pakDisableRumbleForPlayer(g_Vars.currentplayernum);
				g_Vars.currentplayer->isdead = 2;
				pass = true;
			}

			if (pass) {
				if (g_Vars.mplayerisrunning == false) {
					musicStartSoloDeath();
				} else {
					musicStartMpDeath();
				}
			} else {
				if (g_Vars.currentplayer->redbloodfinished) {
					playerSetFadeColour(0x96, 0, 0, 0.70588237f);
				} else {
					g_Vars.currentplayer->redbloodfinished = true;
				}
			}
		}

		if (modelGetCurAnimFrame(&g_Vars.currentplayer->model) >= modelGetAnimEndFrame(&g_Vars.currentplayer->model)
				&& g_Vars.currentplayer->redbloodfinished) {
			if (g_Vars.currentplayer->deathanimfinished == false) {
				g_Vars.currentplayer->deathanimfinished = true;
				playerAdjustFade(60, 0, 0, 0, 1);
				playerStartChrFade(120, 0);
			}

			if (playerIsFadeComplete()) {
				bool canrestart = false;

				if (g_Vars.mplayerisrunning) {
					if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
						// Coop or anti
						struct chrdata *chr = g_Vars.currentplayer->prop->chr;

						if (chr) {
							chr->chrflags |= CHRCFLAG_HIDDEN;
						}

						if (g_Vars.antiplayernum >= 0 && g_Vars.currentplayer == g_Vars.anti) {
							// Anti
							if (joyGetButtons(optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex), 0xb000) && !mpIsPaused()) {
								g_Vars.currentplayer->dostartnewlife = true;
							}
						} else {
							// Coop
							if (g_Vars.coopplayernum >= 0 &&
									(!g_Vars.bond->isdead || !g_Vars.coop->isdead)) {
								f32 totalhealth;
								u32 buddyplayernum = g_Vars.bondplayernum;
								u32 prevplayernum = g_Vars.currentplayernum;
								f32 stealhealth;
								f32 shield;

								canrestart = joyGetButtons(optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex), 0xb000)
									&& !mpIsPaused();

								// Get ready to respawn.
								// The other player's health will be halved.
								buddyplayernum = g_Vars.currentplayer == g_Vars.coop ? g_Vars.bondplayernum : g_Vars.coopplayernum;

								setCurrentPlayerNum(buddyplayernum);
								shield = chrGetShield(g_Vars.currentplayer->prop->chr) * 0.125f;
								totalhealth = g_Vars.currentplayer->bondhealth + shield;

#if VERSION >= VERSION_NTSC_FINAL
								// NTSC final prevents coop from being able to respawn
								// in Deep Sea after the mid cutscene. Without this condition,
								// the player could respawn on the other side of the exit trigger.
								// Additionally, the logic for coopcanrestart is different.
								if (totalhealth > 0.125f
										&& !(mainGetStageNum() == STAGE_DEEPSEA && chrHasStageFlag(NULL, 0x00000200))) {
									if (canrestart) {
										playerDisplayHealth();

										stealhealth = totalhealth * 0.5f;

										if (stealhealth < shield) {
											chrSetShield(g_Vars.currentplayer->prop->chr, (shield - stealhealth) * 8.0f);
										} else {
											chrSetShield(g_Vars.currentplayer->prop->chr, 0);
											g_Vars.currentplayer->bondhealth -= stealhealth - shield;
										}

										// Back to the player who died
										setCurrentPlayerNum(prevplayernum);
										g_Vars.currentplayer->dostartnewlife = true;
										g_Vars.currentplayer->oldhealth = 0;
										g_Vars.currentplayer->oldarmour = 0;
										g_Vars.currentplayer->apparenthealth = 0;
										g_Vars.currentplayer->apparentarmour = 0;
										g_Vars.currentplayer->stealhealth = stealhealth;
									} else {
										setCurrentPlayerNum(prevplayernum);
									}

									g_Vars.currentplayer->coopcanrestart = true;
								} else {
									// Can't respawn
									setCurrentPlayerNum(prevplayernum);
								}
#else
								if (totalhealth > 0.125f && canrestart) {
									playerDisplayHealth();

									stealhealth = totalhealth * 0.5f;

									if (stealhealth < shield) {
										chrSetShield(g_Vars.currentplayer->prop->chr, (shield - stealhealth) * 8.0f);
									} else {
										chrSetShield(g_Vars.currentplayer->prop->chr, 0);
										g_Vars.currentplayer->bondhealth -= stealhealth - shield;
									}

									// Back to the player who died
									setCurrentPlayerNum(prevplayernum);
									g_Vars.currentplayer->dostartnewlife = true;
									g_Vars.currentplayer->oldhealth = 0;
									g_Vars.currentplayer->oldarmour = 0;
									g_Vars.currentplayer->apparenthealth = 0;
									g_Vars.currentplayer->apparentarmour = 0;
									g_Vars.currentplayer->stealhealth = stealhealth;
								} else {
									setCurrentPlayerNum(prevplayernum);
								}

								if (totalhealth > 0.125f) {
									g_Vars.currentplayer->coopcanrestart = true;
								}
#endif
							}
						}
					} else {
						u32 playernum = g_Vars.currentplayernum;
						s32 playercount = PLAYERCOUNT();
						struct chrdata *chr = g_Vars.currentplayer->prop->chr;
						s32 numdeaths = 0;
						s32 i;

						if (chr) {
							chr->chrflags |= CHRCFLAG_HIDDEN;
						}

						for (i = 0; i < playercount; i++) {
							numdeaths += g_Vars.playerstats[i].kills[playernum];
						}

						if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
							if (g_Vars.currentplayer->deadtimer < 0) {
								g_Vars.currentplayer->deadtimer = TICKS(600);
							}

							if (g_Vars.currentplayer->deadtimer >= 0) {
								g_Vars.currentplayer->deadtimer -= g_Vars.lvupdate60;

								if (g_Vars.currentplayer->deadtimer < 0) {
									canrestart = true;
								}
							}
						}

						if (joyGetButtons(optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex), 0xb000)
								&& !mpIsPaused()
								&& g_NumReasonsToEndMpMatch == 0) {
							canrestart = true;
						}

						if (canrestart) {
							g_Vars.currentplayer->dostartnewlife = true;
						}
					}
				}
			}
		}
	}

	if (g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY) {
		gdl = bgunDrawSight(gdl);

		if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
			gdl = bviewDrawHorizonScanner(gdl);
		}

		if (optionsGetAmmoOnScreen(g_Vars.currentplayerstats->mpindex)) {
			gdl = bgunDrawHud(gdl);
		}

#if VERSION >= VERSION_NTSC_1_0
		gdl = radarRender(gdl);
		gdl = hudmsgsRender(gdl);
#else
		gdl = hudmsgsRender(gdl);
		gdl = radarRender(gdl);
#endif

		gdl = playerDrawStoredFade(gdl);
	} else {
		gdl = bgRenderArtifacts(gdl);

		if (g_Vars.currentplayer->eyespy) {
			if (g_Vars.currentplayer->eyespy->startuptimer60 < TICKS(50)) {
				gdl = bviewDrawFisheye(gdl, 0xffffffff, 255, 0, g_Vars.currentplayer->eyespy->startuptimer60, g_Vars.currentplayer->eyespy->hit);
			} else {
				s32 time = g_Vars.currentplayer->eyespy->camerashuttertime;

				if (time > 0) {
					if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
						gdl = bviewDrawFisheye(gdl, 0xffffffff, 255, time, TICKS(50), g_Vars.currentplayer->eyespy->hit);
					} else {
						gdl = bviewDrawFisheye(gdl, 0xffffffff, 255, 0, TICKS(50), g_Vars.currentplayer->eyespy->hit);
					}

					g_Vars.currentplayer->eyespy->camerashuttertime -= g_Vars.lvupdate60;
				} else {
					gdl = bviewDrawFisheye(gdl, 0xffffffff, 255, 0, TICKS(50), g_Vars.currentplayer->eyespy->hit);
				}
			}

			gdl = bviewDrawEyespyMetrics(gdl);
		}

		if (g_Vars.currentplayer->mpmenuon) {
			s32 a = viGetViewLeft();
			s32 b = viGetViewTop();
			s32 c = viGetViewLeft() + viGetViewWidth();
			s32 d = viGetViewTop() + viGetViewHeight();

			gdl = text0f153628(gdl);
			gdl = text0f153a34(gdl, a, b, c, d, 0x000000a0);
			gdl = text0f153780(gdl);
		}

		gdl = hudmsgsRender(gdl);
		gdl = playerDrawStoredFade(gdl);
	}

	return gdl;
}

void playerDie(bool force)
{
	struct chrdata *chr = g_Vars.currentplayer->prop->chr;
	s32 shooter;

	if (chr->lastshooter >= 0 && chr->timeshooter > 0) {
		shooter = chr->lastshooter;
	} else {
		shooter = g_Vars.currentplayernum;
	}

	playerDieByShooter(shooter, force);
}

void playerDieByShooter(u32 shooter, bool force)
{
#if VERSION >= VERSION_NTSC_1_0
	if (!g_Vars.currentplayer->isdead && (force || !g_Vars.currentplayer->invincible))
#else
	if (!g_Vars.currentplayer->isdead && (force || !g_Vars.currentplayer->invincible || !g_Vars.currentplayer->training))
#endif
	{
		u32 prevplayernum = g_MpPlayerNum;
		g_MpPlayerNum = g_Vars.currentplayerstats->mpindex;
		func0f0f8120();
		g_MpPlayerNum = prevplayernum;

		hudmsgsRemoveForDeadPlayer(g_Vars.currentplayernum);

		if (g_Vars.mplayerisrunning) {
			mpstatsRecordDeath(shooter, g_Vars.currentplayernum);
		}

		chrUncloak(g_Vars.currentplayer->prop->chr, true);

		if (g_Vars.mplayerisrunning &&
				(g_Vars.antiplayernum < 0
				 || g_Vars.currentplayernum != g_Vars.antiplayernum
				 || shooter != g_Vars.antiplayernum)) {
			currentPlayerDropAllItems();
		}

		g_Vars.currentplayer->isdead = true;
		g_Vars.currentplayer->bonddie = g_Vars.currentplayer->bond2;
		g_Vars.currentplayer->thetadie = g_Vars.currentplayer->vv_theta;
		g_Vars.currentplayer->vertadie = g_Vars.currentplayer->vv_verta;
		g_Vars.currentplayer->posdie.x = g_Vars.currentplayer->prop->pos.x;
		g_Vars.currentplayer->posdie.y = g_Vars.currentplayer->prop->pos.y;
		g_Vars.currentplayer->posdie.z = g_Vars.currentplayer->prop->pos.z;

		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
			if (g_Vars.currentplayer->unk1af0) {
				g_Vars.currentplayer->bondtankexplode = true;
			}
		} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
			g_Vars.currentplayer->bondtankexplode = true;
		}

		bmoveSetMode(MOVEMODE_WALK);
		bgunHandlePlayerDead();

		if (playerGetMissionTime() - g_Vars.currentplayer->lifestarttime60 < g_Vars.currentplayerstats->shortestlife) {
			g_Vars.currentplayerstats->shortestlife = playerGetMissionTime() - g_Vars.currentplayer->lifestarttime60;
		}

		g_Vars.currentplayer->lifestarttime60 = playerGetMissionTime();
	}
}

void playerCheckIfShotInBack(s32 attackerplayernum, f32 x, f32 z)
{
	if (g_Vars.normmplayerisrunning) {
		s32 victimplayernum = g_Vars.currentplayernum;
		f32 angle = atan2f(x, z);
		f32 finalangle = g_Vars.players[victimplayernum]->vv_theta - (360.0f - RAD2DEG2(angle));

		if (finalangle < 0) {
			finalangle = -finalangle;
		}

		if (finalangle < 90.0f || finalangle > 270.0f) {
			g_Vars.playerstats[attackerplayernum].backshotcount++;
		}
	}
}

/**
 * Determines what height the health bar should have. The function is called
 * while any menu is open and any time when the health bar should be shown.
 *
 * A return value of 0 means zero height, while 1 means full expanded height.
 */
f32 playerGetHealthBarHeightFrac(void)
{
	f32 done;
	f32 total;

	switch (g_Vars.currentplayer->healthshowmode) {
	case HEALTHSHOWMODE_HIDDEN:
		return 0;
	case HEALTHSHOWMODE_OPENING:
		total = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].openendframe;
		done = g_Vars.currentplayer->healthshowtime;
		return done / total;
	case HEALTHSHOWMODE_CLOSING:
		total = g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closeendframe - g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closestartframe;
		done = g_Vars.currentplayer->healthshowtime - g_HealthDamageTypes[g_Vars.currentplayer->healthdamagetype].closestartframe;
		return 1 - done / total;
	}

	return 1;
}

bool playerIsHealthVisible(void)
{
	return g_Vars.currentplayer->healthshowmode != HEALTHSHOWMODE_HIDDEN;
}

// Never called
void playerSetInvincible(bool enable)
{
	if (enable) {
		cheatActivate(CHEAT_INVINCIBLE);
	} else {
		cheatDeactivate(CHEAT_INVINCIBLE);
	}
}

void playerSetBondVisible(bool visible)
{
	g_Vars.bondvisible = visible;
}

void playerSetBondCollisionsEnabled(bool enabled)
{
	g_Vars.bondcollisions = enabled;
}

void playerSetCameraMode(s32 mode)
{
	g_Vars.currentplayer->cameramode = mode;
}

void player0f0c1840(struct coord *pos, struct coord *up, struct coord *look, struct coord *pos2, RoomNum *rooms2)
{
	bool done = false;
	RoomNum inrooms[21];
	RoomNum aboverooms[21];
	RoomNum sp54[8];
	RoomNum bestroom;
	RoomNum tmp;
	s32 i;
	s32 room;

	if (rooms2 != NULL && *rooms2 != -1) {
		portal00018148(pos2, pos, rooms2, sp54, NULL, 0);

		// Remove values from sp54 (room numbers) if that room doesn't contain
		// the coord, and shuffle the array back when removing values.
		for (i = 0; sp54[i] != -1; i++) {
			if (!bgRoomContainsCoord(pos, sp54[i])) {
				s32 j;

#if VERSION >= VERSION_NTSC_1_0
				for (j = i + 1; sp54[j] != -1; j++) {
					sp54[j - 1] = sp54[j];
				}

				sp54[j - 1] = -1;
				i--;
#else
				// ntsc-beta corrupts the array by overwriting the first shifted
				// value with -1, and leaving a duplicate at the end.
				for (j = i + 1; sp54[j] != -1; j++) {
					sp54[j - 1] = sp54[j];
				}

				sp54[i] = -1;
#endif
			}
		}

		if (sp54[0] != -1 && sp54[1] == -1) {
			playerSetCamPropertiesWithRoom(pos, up, look, sp54[0]);
			done = true;
		}

		if (!done) {
			for (i = 0; sp54[i] != -1; i++) {
				if ((g_Rooms[sp54[i]].flags & ROOMFLAG_COMPLICATEDPORTALS) == 0) {
					if (bgTestPosInRoom(pos, sp54[i])) {
						playerSetCamPropertiesWithRoom(pos, up, look, sp54[i]);
						done = true;
						break;
					}
				}
			}
		}

		// The same thing again but for rooms which have complicated portals
		if (!done) {
			for (i = 0; sp54[i] != -1; i++) {
				if (g_Rooms[sp54[i]].flags & ROOMFLAG_COMPLICATEDPORTALS) {
					if (bgTestPosInRoom(pos, sp54[i])) {
						playerSetCamPropertiesWithRoom(pos, up, look, sp54[i]);
						done = true;
						break;
					}
				}
			}
		}
	}

	if (!done) {
		bgFindRoomsByPos(pos, inrooms, aboverooms, 20, &bestroom);

		if (inrooms[0] != -1) {
			tmp = room = cdFindFloorRoomAtPos(pos, inrooms);

			if (room > 0) {
				playerSetCamPropertiesWithRoom(pos, up, look, tmp);
			} else {
				playerSetCamPropertiesWithRoom(pos, up, look, inrooms[0]);
			}
		} else if (aboverooms[0] != -1) {
			tmp = room = cdFindFloorRoomAtPos(pos, aboverooms);

			if (room > 0) {
				playerSetCamPropertiesWithoutRoom(pos, up, look, tmp);
			} else {
				playerSetCamPropertiesWithoutRoom(pos, up, look, aboverooms[0]);
			}
		} else {
			if (bestroom != -1) {
				playerSetCamPropertiesWithoutRoom(pos, up, look, bestroom);
			} else {
				playerSetCamPropertiesWithoutRoom(pos, up, look, 1);
			}
		}
	}
}

void player0f0c1ba4(struct coord *pos, struct coord *up, struct coord *look, struct coord *memcampos, s32 memcamroom)
{
	RoomNum rooms[2];
	rooms[0] = memcamroom;
	rooms[1] = -1;

	player0f0c1840(pos, up, look, memcampos, rooms);
}

void player0f0c1bd8(struct coord *pos, struct coord *up, struct coord *look)
{
	if (g_Vars.currentplayer->memcamroom >= 0) {
		player0f0c1ba4(pos, up, look, &g_Vars.currentplayer->memcampos, g_Vars.currentplayer->memcamroom);
	} else {
		player0f0c1840(pos, up, look, NULL, NULL);
	}
}

void playerSetCamPropertiesWithRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room)
{
	g_Vars.currentplayer->memcampos.x = pos->x;
	g_Vars.currentplayer->memcampos.y = pos->y;
	g_Vars.currentplayer->memcampos.z = pos->z;
	g_Vars.currentplayer->memcamroom = room;

	playerSetCamProperties(pos, up, look, room);
}

void playerSetCamPropertiesWithoutRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room)
{
	playerClearMemCamRoom();
	playerSetCamProperties(pos, up, look, room);
}

void playerSetCamProperties(struct coord *pos, struct coord *up, struct coord *look, s32 room)
{
	struct player *player = g_Vars.currentplayer;

	player->cam_pos.x = pos->x;
	player->cam_pos.y = pos->y;
	player->cam_pos.z = pos->z;
	player->cam_up.x = up->x;
	player->cam_up.y = up->y;
	player->cam_up.z = up->z;
	player->cam_look.x = look->x;
	player->cam_look.y = look->y;
	player->cam_look.z = look->z;
	player->cam_room = room;
}

void playerClearMemCamRoom(void)
{
	g_Vars.currentplayer->memcamroom = -1;
}

void playersClearMemCamRoom(void)
{
	s32 prevplayernum = g_Vars.currentplayernum;
	s32 i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);
		playerClearMemCamRoom();
	}

	setCurrentPlayerNum(prevplayernum);
}

void playerSetPerimEnabled(struct prop *prop, bool enable)
{
	u32 playernum = playermgrGetPlayerNumByProp(prop);

	if (g_Vars.players[playernum]->haschrbody) {
		chrSetPerimEnabled(prop->chr, enable);
	}

	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		if (g_Vars.currentplayer->unk1af0) {
			objSetPerimEnabled(g_Vars.currentplayer->unk1af0, enable);
		}
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		objSetPerimEnabled(g_Vars.currentplayer->hoverbike, enable);
	}

	g_Vars.players[playernum]->bondperimenabled = enable;
}

bool playerUpdateGeometry(struct prop *prop, u8 **start, u8 **end)
{
	s32 playernum = playermgrGetPlayerNumByProp(prop);

	if (g_Vars.players[playernum]->bondperimenabled
			&& (!g_Vars.mplayerisrunning || !g_Vars.players[playernum]->isdead)) {
		if (g_Vars.useperimshoot) {
			g_Vars.players[playernum]->perimshoot = g_Vars.players[playernum]->periminfo;
			g_Vars.players[playernum]->perimshoot.radius = 15;

			*start = (void *) &g_Vars.players[playernum]->perimshoot;
		} else {
			*start = (void *) &g_Vars.players[playernum]->periminfo;
		}

		*end = *start + sizeof(struct geocyl);

		return true;
	}

	*end = NULL;
	*start = NULL;

	return false;
}

void playerUpdatePerimInfo(void)
{
	g_Vars.currentplayer->periminfo.header.type = GEOTYPE_CYL;
	g_Vars.currentplayer->periminfo.header.flags = GEOFLAG_WALL | GEOFLAG_BLOCK_SHOOT;

	g_Vars.currentplayer->periminfo.ymin = g_Vars.currentplayer->vv_manground;
	g_Vars.currentplayer->periminfo.ymax = g_Vars.currentplayer->vv_manground + g_Vars.currentplayer->vv_headheight;

	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		// note: crouchoffsetrealsmall is negative
		f32 minsane;
		g_Vars.currentplayer->periminfo.ymax += g_Vars.currentplayer->crouchoffsetrealsmall;
		minsane = g_Vars.currentplayer->vv_manground + 80;

		if (g_Vars.currentplayer->periminfo.ymax < minsane) {
			g_Vars.currentplayer->periminfo.ymax = minsane;
		}
	}

	g_Vars.currentplayer->periminfo.x = g_Vars.currentplayer->prop->pos.x;
	g_Vars.currentplayer->periminfo.z = g_Vars.currentplayer->prop->pos.z;
	g_Vars.currentplayer->periminfo.radius = g_Vars.currentplayer->bond2.radius;
}

/**
 * Populates the width, ymax and ymin arguments with absolute coordinates.
 *
 * ymin is set to 30 units above the player's feet. This allows them to go up
 * steps or ledges that are 30 units or smaller.
 *
 * ymax is the top of the head, minus some if crouching, and always at least 80
 * units above the feet.
 */
void playerGetBbox(struct prop *prop, f32 *radius, f32 *ymax, f32 *ymin)
{
	s32 playernum = playermgrGetPlayerNumByProp(prop);

	*radius = g_Vars.players[playernum]->bond2.radius;
	*ymin = g_Vars.currentplayer->vv_manground + 30;
	*ymax = g_Vars.currentplayer->vv_manground + g_Vars.players[playernum]->vv_headheight;

	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		// note: crouchoffsetrealsmall is negative
		f32 minsane;
		*ymax += g_Vars.players[playernum]->crouchoffsetrealsmall;
		minsane = g_Vars.currentplayer->vv_manground + 80;

		if (*ymax < minsane) {
			*ymax = minsane;
		}
	}
}

f32 playerGetHealthFrac(void)
{
	return g_Vars.currentplayer->bondhealth;
}

f32 playerGetShieldFrac(void)
{
	f32 frac = chrGetShield(g_Vars.currentplayer->prop->chr) * 0.125f;

	if (frac < 0) {
		frac = 0;
	}

	if (frac > 1) {
		frac = 1;
	}

	return frac;
}

void playerSetShieldFrac(f32 frac)
{
	if (frac < 0) {
		frac = 0;
	}

	if (frac > 1) {
		frac = 1;
	}

	chrSetShield(g_Vars.currentplayer->prop->chr, frac * 8);
}

s32 playerGetMissionTime(void)
{
#if PAL
	return g_Vars.currentplayer->bondviewlevtime60 * 60 / 50;
#else
	return g_Vars.currentplayer->bondviewlevtime60;
#endif
}

s32 playerTickBeams(struct prop *prop)
{
	beamTick(&g_Vars.players[playermgrGetPlayerNumByProp(prop)]->hands[0].beam);
	beamTick(&g_Vars.players[playermgrGetPlayerNumByProp(prop)]->hands[1].beam);

	if (prop->chr && g_Vars.mplayerisrunning) {
		struct chrdata *chr = prop->chr;

		if (chr->fireslots[0] >= 0) {
			beamTick(&g_Fireslots[chr->fireslots[0]].beam);
		}

		if (chr->fireslots[1] >= 0) {
			beamTick(&g_Fireslots[chr->fireslots[1]].beam);
		}
	}

	return 0;
}

s32 playerTickThirdPerson(struct prop *prop)
{
	s32 playernum = playermgrGetPlayerNumByProp(prop);
	struct player *player = g_Vars.players[playernum];
	struct chrdata *chr = prop->chr;
	s32 i;
	s32 tickop1;
	Mtxf *spe8;
	Mtxf spa8;
	struct coord sp9c;
	s32 tickop2;
	struct coord sp8c;
	struct coord sp80;
	f32 angle;
	s32 animnum;
	f32 shootrotx;
	f32 shootroty;
	struct prop *leftprop;
	struct prop *rightprop;
	struct coord sp5c;

	if (g_Vars.currentplayerindex == 0 && player->haschrbody) {
		chr->hidden &= ~CHRHFLAG_00000800;
	}

	if (player->haschrbody && player->model00d4) {
		if (var80075d60 == 0
				|| var80075d60 == 1
				|| (player->cameramode == CAMERAMODE_THIRDPERSON && player->visionmode != VISIONMODE_SLAYERROCKET)) {
			chr->chrflags |= CHRCFLAG_FORCETOGROUND;

			player->bondperimenabled = false;
			tickop1 = chrTick(prop);
			player->bondperimenabled = true;

			player->vv_ground = chr->ground;
			player->vv_manground = chr->ground;

			chr0f0220ac(prop->chr);

			if (prop->flags & PROPFLAG_ONTHISSCREENTHISTICK) {
				if (player->model00d4->definition->skel == &g_SkelChr) {
					spe8 = player->model00d4->matrices;
				} else {
					spe8 = player->model00d4->matrices;
				}

				mtx00015be4(camGetProjectionMtxF(), spe8, &spa8);

				sp9c.x = spa8.m[3][0] + spa8.m[1][0] * 7;
				sp9c.y = spa8.m[3][1] + spa8.m[1][1] * 7;
				sp9c.z = spa8.m[3][2] + spa8.m[1][2] * 7;

				player->vv_theta = (M_BADTAU - chrGetInverseTheta(chr)) * 360.0f / M_BADTAU;
				player->vv_verta = 0;
			} else {
				sp9c.x = player->prop->pos.x;
				sp9c.y = player->prop->pos.y;
				sp9c.z = player->prop->pos.z;

				player->vv_theta = (M_BADTAU - chrGetInverseTheta(chr)) * 360.0f / M_BADTAU;
				player->vv_verta = 0;
			}

			bmoveUpdateVerta();
			bmove0f0cc19c(&sp9c);

			return tickop1;
		}
	}

	if (player->haschrbody
			&& player->model00d4
			&& ((g_Vars.mplayerisrunning && g_Vars.currentplayernum != playernum)
				|| player->cameramode == CAMERAMODE_EYESPY
				|| (player->cameramode == CAMERAMODE_THIRDPERSON && player->visionmode == VISIONMODE_SLAYERROCKET))) {
		chr->actiontype = ACT_BONDMULTI;

		if ((chr->hidden & CHRHFLAG_00000800) == 0) {
			leftprop = chrGetHeldProp(chr, HAND_LEFT);
			rightprop = chrGetHeldProp(chr, HAND_RIGHT);
			animnum = modelGetAnimNum(chr->model);

			playerChooseThirdPersonAnimation(chr, bmoveGetCrouchPosByPlayer(playernum), player->speedsideways, player->speedforwards, player->speedtheta, &player->angleoffset, &chr->act_bondmulti.animcfg);

			if (chrIsDead(chr)) {
				shootrotx = 0;
				shootroty = 0;
			} else {
				shootrotx = player->shootrotx;
				shootroty = player->shootroty;
			}

			if (modelGetAnimNum(chr->model) == animnum) {
				if (chr->act_bondmulti.animcfg) {
					chr->hidden2 &= ~CHRH2FLAG_AUTOANIM;
					chrCalculateAimEndProperties(chr, chr->act_bondmulti.animcfg, leftprop != NULL, rightprop != NULL, shootrotx);
				} else {
					chr->hidden2 |= CHRH2FLAG_AUTOANIM;
					chr->aimendback = shootrotx;
					chr->aimendrshoulder = 0;
					chr->aimendlshoulder = 0;
				}
			}

			chr->aimendsideback = shootroty;
			chr->aimendcount = 10;

			chrSetFiring(chr, HAND_RIGHT, player->hands[HAND_RIGHT].flashon);
			chrSetFiring(chr, HAND_LEFT, player->hands[HAND_LEFT].flashon);
		}

		sp80.x = prop->pos.x;
		sp80.y = prop->pos.y;
		sp80.z = prop->pos.z;

		modelGetRootPosition(chr->model, &sp8c);

		sp8c.x = prop->pos.x;
		sp8c.z = prop->pos.z;

		modelSetRootPosition(chr->model, &sp8c);

		angle = (360.0f - player->vv_theta) * 0.017450513318181f - player->angleoffset;

		if (angle >= M_BADTAU) {
			angle -= M_BADTAU;
		} else if (angle < 0) {
			angle += M_BADTAU;
		}

		chrSetLookAngle(chr, angle);

		chr->chrflags |= CHRHFLAG_DROPPINGITEM;

		tickop2 = chrTick(prop);

		prop->pos.x = sp80.x;
		prop->pos.y = sp80.y;
		prop->pos.z = sp80.z;

		if ((chr->hidden & CHRHFLAG_00000800) == 0) {
			for (i = 0; i < 2; i++) {
				if (chrGetGunPos(chr, i, &player->chrmuzzlelastpos[i])) {
					player->chrmuzzlelast[i] = g_Vars.lvframenum;
				} else if (player->chrmuzzlelast[i] < g_Vars.lvframenum - 1) {
					player->chrmuzzlelastpos[i].x = player->hands[i].muzzlepos.x;
					player->chrmuzzlelastpos[i].y = player->hands[i].muzzlepos.y;
					player->chrmuzzlelastpos[i].z = player->hands[i].muzzlepos.z;
				}
			}

			chr->hidden |= CHRHFLAG_00000800;
		}

		return tickop2;
	}

	if (PLAYERCOUNT() == 1) {
		chrUpdateCloak(chr);
	}

	if (player->haschrbody && chr->model) {
		modelGetRootPosition(chr->model, &sp5c);

		sp5c.x = prop->pos.x;
		sp5c.z = prop->pos.z;

		modelSetRootPosition(chr->model, &sp5c);
	}

	chr->ground = player->vv_ground;
	chr->manground = player->vv_manground;
	chr->sumground = chr->manground * (PAL ? 8.417509f : 9.999998f);

	if (g_Vars.mplayerisrunning) {
		if (chr->weapons_held[0] && (chr->weapons_held[0]->obj->hidden & OBJHFLAG_DELETING)) {
			objFree(chr->weapons_held[0]->obj, true, false);
		}

		if (chr->weapons_held[1] && (chr->weapons_held[1]->obj->hidden & OBJHFLAG_DELETING)) {
			objFree(chr->weapons_held[1]->obj, true, false);
		}
	}

	prop->flags &= ~PROPFLAG_ONTHISSCREENTHISTICK;

	return TICKOP_NONE;
}

/**
 * Choose and apply an animation for a multiplayer player from a third person
 * perspective, based on their crouch state, equipped weapons and how quickly
 * they are turning.
 *
 * Despite the function name, this is also used for bots.
 */
void playerChooseThirdPersonAnimation(struct chrdata *chr, s32 crouchpos, f32 speedsideways, f32 speedforwards, f32 speedtheta, f32 *angleoffset, struct attackanimconfig **animcfgptr)
{
	struct prop *leftprop = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightprop = chrGetHeldProp(chr, HAND_RIGHT);
	struct weaponobj *leftgun = NULL;
	struct weaponobj *rightgun = NULL;
	s16 animnum = 0;
	struct attackanimconfig *animcfg;
	f32 speed;
	s32 prevanimnum;
	bool reconfigure = false;
	s32 wieldmode;
	s32 i;
	f32 startframe = -1;
	f32 endframe = -1;
	struct var80070ba4 *row;
	f32 angle;
	f32 turnspeed;
	s32 turnmode;
	f32 limit;

	if (leftprop != NULL) {
		leftgun = leftprop->weapon;
	}

	if (rightprop != NULL) {
		rightgun = rightprop->weapon;
	}

	prevanimnum = modelGetAnimNum(chr->model);

	if (chrIsDead(chr)) {
		// Choose a death animation
		bool found = false;

		for (i = 0; i < g_NumDeathAnimations; i++) {
			if (g_DeathAnimations[i] == prevanimnum) {
				found = true;
				break;
			}
		}

		if (found) {
			animnum = prevanimnum;
			speed = 0.5f;
		} else {
			animnum = g_DeathAnimations[rngRandom() % g_NumDeathAnimations];
			speed = 0.5f;
		}

		animcfg = NULL;
	} else {
		struct prop *chrprop = chr->prop;

		if (chrprop->type == PROPTYPE_PLAYER
				&& g_Vars.players[playermgrGetPlayerNumByProp(chrprop)]->bondmovemode == MOVEMODE_BIKE) {
			// Player on a hoverbike
			if (leftprop && rightprop) {
				wieldmode = WIELDMODE_DUALGUNS;
			} else if (!leftprop && !rightprop) {
				wieldmode = WIELDMODE_UNARMED;
			} else if (leftgun && weaponHasFlag(leftgun->weaponnum, WEAPONFLAG_ONEHANDED)) {
				wieldmode = WIELDMODE_PISTOL;
			} else if (rightgun && weaponHasFlag(rightgun->weaponnum, WEAPONFLAG_ONEHANDED)) {
				wieldmode = WIELDMODE_PISTOL;
			} else {
				wieldmode = WIELDMODE_HEAVY;
			}

			if (wieldmode == WIELDMODE_PISTOL) {
				animnum = ANIM_ONBIKE_PISTOL;
			} else if (wieldmode == WIELDMODE_DUALGUNS) {
				animnum = ANIM_ONBIKE_DUALGUNS;
			} else if (wieldmode == WIELDMODE_HEAVY) {
				animnum = ANIM_ONBIKE_HEAVYGUN;
			} else {
				animnum = ANIM_ONBIKE_UNARMED;
			}

			speed = 0.5f;
			animcfg = NULL;
		} else {
			// Player or bot on foot
			if (leftprop && rightprop) {
				wieldmode = WIELDMODE_DUALGUNS;
			} else if (!leftprop && !rightprop) {
				wieldmode = WIELDMODE_UNARMED;
			} else if (leftgun && !weaponHasFlag(leftgun->weaponnum, WEAPONFLAG_AICANUSE)) {
				wieldmode = WIELDMODE_UNARMED;
			} else if (rightgun && !weaponHasFlag(rightgun->weaponnum, WEAPONFLAG_AICANUSE)) {
				wieldmode = WIELDMODE_UNARMED;
			} else if (leftgun && weaponHasFlag(leftgun->weaponnum, WEAPONFLAG_ONEHANDED)) {
				wieldmode = WIELDMODE_PISTOL;
			} else if (rightgun && weaponHasFlag(rightgun->weaponnum, WEAPONFLAG_ONEHANDED)) {
				wieldmode = WIELDMODE_PISTOL;
			} else {
				wieldmode = WIELDMODE_HEAVY;
			}

			turnspeed = sqrtf(speedsideways * speedsideways + speedforwards * speedforwards);

			if (speedtheta < 0) {
				speedtheta = -speedtheta;
			}

			if (turnspeed < speedtheta) {
				turnspeed = speedtheta;
			}

			if (turnspeed < 0.05f) {
				if (crouchpos == CROUCHPOS_SQUAT) {
					turnmode = TURNMODE_SQUAT_NOTURN;
				} else if (crouchpos == CROUCHPOS_DUCK) {
					turnmode = TURNMODE_DUCK_NOTURN;
				} else {
					turnmode = TURNMODE_STAND_NOTURN;
				}

				row = &var80070ba4[wieldmode][turnmode];
				speed = 1.0f;
				angle = 0.0f;
			} else {
				angle = atan2f(speedsideways, speedforwards);

				if (angle >= M_BADPI) {
					angle -= M_BADTAU;
				}

				if (crouchpos == CROUCHPOS_SQUAT) {
					turnmode = TURNMODE_SQUAT_TURN;
					speed = turnspeed * 2.8571429252625f;

					if (speed > 1.2f) {
						speed = 1.2f;
					}
				} else if (crouchpos == CROUCHPOS_DUCK) {
					turnmode = TURNMODE_DUCK_TURN;
					speed = turnspeed * 2;

					if (speed > 1.2f) {
						speed = 1.2f;
					}
				} else if (turnspeed < 0.4f
						|| (chr->prop->type == PROPTYPE_PLAYER
							&& g_Vars.players[playermgrGetPlayerNumByProp(chr->prop)]->headanim == HEADANIM_RESTING)) {
					turnmode = TURNMODE_STAND_SOFTTURN;
					speed = 2.0f * turnspeed;

					if (speed > 1.2f) {
						speed = 1.2f;
					}
				} else {
					turnmode = TURNMODE_STAND_HARDTURN;
					speed = turnspeed;

					if (speed > 1.2f) {
						speed = 1.2f;
					}
				}

				if (angle < -1.6333680152893f) {
					angle += M_BADPI;
					speed = -speed;
				} else if (angle > 1.6333680152893f) {
					angle -= M_BADPI;
					speed = -speed;
				}

				row = &var80070ba4[wieldmode][turnmode];

				if (angle < -row->unk14) {
					angle = -row->unk14;
				} else if (angle > row->unk14) {
					angle = row->unk14;
				}
			}

			limit = g_Vars.lvupdate60freal * 0.10470308363438f;

			if (angle - *angleoffset > limit) {
				*angleoffset += limit;
			} else if (angle - *angleoffset < -limit) {
				*angleoffset -= limit;
			} else {
				*angleoffset = angle;
			}

			animcfg = row->animcfg;

			if (row->animnum) {
				animnum = row->animnum;
			}

			speed *= row->speed;
			startframe = row->startframe;
			endframe = row->endframe;
		}
	}

	if (animcfg != NULL && animnum == 0) {
		animnum = animcfg->animnum;
	}

	if (animnum != prevanimnum) {
		reconfigure = true;
	}

	if (startframe >= 0 && (!chr->model->anim->looping || startframe != chr->model->anim->loopframe)) {
		reconfigure = true;
	}

	if (startframe < 0 && chr->model->anim->looping) {
		reconfigure = true;
	}

	if (reconfigure) {
		if (chr->model->anim->animnum2 == 0) {
			modelSetAnimation(chr->model, animnum, false, startframe >= 0 ? startframe : 0, speed, 16);

			if (startframe >= 0) {
				modelSetAnimLooping(chr->model, startframe, 16);
			}

			if (endframe >= 0) {
				modelSetAnimEndFrame(chr->model, endframe);
			}
		}
	} else {
		if (speed != modelGetAnimSpeed(chr->model)) {
			modelSetAnimSpeed(chr->model, speed, 1);
		}
	}

	*animcfgptr = animcfg;
}

Gfx *playerRender(struct prop *prop, Gfx *gdl, bool xlupass)
{
	if (g_Vars.players[playermgrGetPlayerNumByProp(prop)]->haschrbody) {
		gdl = chrRender(prop, gdl, xlupass);
	}

	return gdl;
}

Gfx *playerLoadMatrix(Gfx *gdl)
{
	gSPMatrix(gdl++, g_Vars.currentplayer->mtxl005c, G_MTX_LOAD);
	return gdl;
}

void player0f0c3320(Mtxf *matrices, s32 count)
{
	Mtxf sp40;
	s32 i;
	s32 j;

	for (i = 0, j = 0; i < count; i++, j += sizeof(Mtxf)) {
		mtx00015be4(camGetProjectionMtxF(), (Mtxf *)((uintptr_t)matrices + j), &sp40);

		sp40.m[3][0] -= g_Vars.currentplayer->globaldrawworldoffset.x;
		sp40.m[3][1] -= g_Vars.currentplayer->globaldrawworldoffset.y;
		sp40.m[3][2] -= g_Vars.currentplayer->globaldrawworldoffset.z;

		mtxF2L(&sp40, matrices + i);
	}
}
