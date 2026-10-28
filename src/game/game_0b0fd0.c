#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/player.h"
#include "game/hudmsg.h"
#include "game/playermgr.h"
#include "game/mplayer/setup.h"
#include "game/botcmd.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "data.h"
#include "types.h"

struct weapon *weaponFindById(s32 itemid)
{
	if (itemid < 0) {
		return NULL;
	}

	if (itemid >= ARRAYCOUNT(g_Weapons)) {
		return NULL;
	}

	return g_Weapons[itemid];
}

struct weaponfunc *weaponGetFunctionById(u32 weaponnum, u32 which)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (weapon) {
		return weapon->functions[which];
	}

	return NULL;
}

struct weaponfunc *gsetGetWeaponFunction2(struct gset *gset)
{
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (weapon) {
		return weapon->functions[gset->weaponfunc];
	}

	return NULL;
}

struct weaponfunc *gsetGetWeaponFunction(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
#ifdef AVOID_UB
		// SuperDragon grenades have FUNC_2, so this can happen
		if (gset->weaponfunc > FUNC_SECONDARY) {
			return NULL;
		}
#endif
		return weapon->functions[gset->weaponfunc];
	}

	return NULL;
}

struct weaponfunc *weaponGetFunction(struct gset *gset, s32 which)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->functions[which];
	}

	return NULL;
}

struct weaponfunc *currentPlayerGetWeaponFunction(u32 hand)
{
	struct weapon *weapon = weaponFindById(g_Vars.currentplayer->hands[hand].gset.weaponnum);

	if (weapon) {
		return weapon->functions[g_Vars.currentplayer->hands[hand].gset.weaponfunc];
	}

	return NULL;
}

u32 weaponGetNumFunctions(u32 weaponnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return 0;
	}

	for (i = 0; i < 2; i++) {
		if (weapon->functions[i] == NULL) {
			return i;
		}
	}

	return 2;
}

struct invaimsettings *gsetGetAimSettings(struct gset *gset)
{
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (weapon) {
		return weapon->aimsettings;
	}

	return &invaimsettings_default;
}

struct inventory_ammo *weaponGetAmmoByFunction(u32 weaponnum, u32 funcnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	struct weaponfunc *func = weaponGetFunctionById(weaponnum, funcnum);

	if (func && weapon && func->ammoindex >= 0) {
		return weapon->ammos[func->ammoindex];
	}

	return NULL;
}

void currentPlayerGetWeaponPos(struct coord *pos)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));

	if (weapon) {
		pos->x = weapon->posx;
		pos->y = weapon->posy;
		pos->z = weapon->posz;
	}
}

void currentPlayerSetWeaponPos(struct coord *pos)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));

	if (weapon) {
		weapon->posx = pos->x;
		weapon->posy = pos->y;
		weapon->posz = pos->z;
	}
}

f32 handGetXShift(s32 handnum)
{
	return g_Vars.currentplayer->hands[handnum].xshift;
}

f32 func0f0b131c(s32 hand)
{
	f32 x;
	struct weapon *weapon;

	if (hand == 0) {
		weapon = weaponFindById(bgunGetWeaponNum2(0));
		x = weapon->posx;

		if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			x -= 3.5f;

			if (g_Vars.currentplayernum == 0) {
				x += 2.0f;
			}
		}
	} else {
		weapon = weaponFindById(bgunGetWeaponNum2(1));
		x = -weapon->posx;

		if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			x += 3.5f;

			if (g_Vars.currentplayernum == 0) {
				x += 2.0f;
			}
		}
	}

	return x;
}

f32 currentPlayerGetGunZoomFov(void)
{
	s32 index = -1;
	struct weapon *weapon;

	switch (bgunGetWeaponNum2(0)) {
	case WEAPON_SNIPERRIFLE:
		index = 0;
		break;
	case WEAPON_FARSIGHT:
		index = 1;
		break;
	case WEAPON_HORIZONSCANNER:
		index = 2;
		break;
	}

	if (index >= 0) {
		return g_Vars.currentplayer->gunzoomfovs[index];
	}

	weapon = weaponFindById(bgunGetWeaponNum2(0));

	if (weapon) {
		f32 fov = weapon->aimsettings->zoomfov;
		return ADJUST_ZOOM_FOV(fov);
	}

	return 0;
}

void currentPlayerZoomOut(f32 fovpersec)
{
	s32 index = -1;

	switch (bgunGetWeaponNum2(0)) {
	case WEAPON_SNIPERRIFLE:
		index = 0;
		break;
	case WEAPON_FARSIGHT:
		index = 1;
		break;
	case WEAPON_HORIZONSCANNER:
		index = 2;
		break;
	}

	if (index >= 0) {
		f32 amount = fovpersec * 0.25f * LVUPDATE60FREAL();

		if (bgunGetWeaponNum2(0) == WEAPON_FARSIGHT) {
			amount *= 0.5f;
		}

		g_Vars.currentplayer->gunzoomfovs[index] *= 1.0f + amount * 0.1f;

		if (g_Vars.currentplayer->gunzoomfovs[index] > ADJUST_ZOOM_FOV(60)) {
			g_Vars.currentplayer->gunzoomfovs[index] = ADJUST_ZOOM_FOV(60);
		}
	}
}

void currentPlayerZoomIn(f32 fovpersec)
{
	s32 index = -1;

	switch (bgunGetWeaponNum2(0)) {
	case WEAPON_SNIPERRIFLE:
		index = 0;
		break;
	case WEAPON_FARSIGHT:
		index = 1;
		break;
	case WEAPON_HORIZONSCANNER:
		index = 2;
		break;
	}

	if (index >= 0) {
		f32 amount = fovpersec * 0.25f * LVUPDATE60FREAL();

		if (bgunGetWeaponNum2(0) == WEAPON_FARSIGHT) {
			amount *= 0.5f;
		}

		g_Vars.currentplayer->gunzoomfovs[index] /= 1 + amount * 0.1f;

		if (g_Vars.currentplayer->gunzoomfovs[index] < ADJUST_ZOOM_FOV(2)) {
			g_Vars.currentplayer->gunzoomfovs[index] = ADJUST_ZOOM_FOV(2);
		}
	}
}

bool weaponHasFlag(s32 itemid, u32 flag)
{
	struct weapon *weapon = weaponFindById(itemid);

	if (!weapon) {
		return false;
	}

#ifndef PLATFORM_N64
	if (g_AllowAllGunsTwoHanded && flag == WEAPONFLAG_DUALWIELD) {
		return true;
	}
#endif

	return (weapon->flags & flag) != 0;
}

bool weaponHasAimFlag(s32 weaponnum, u32 flag)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (!weapon) {
		return false;
	}

	return (weapon->aimsettings->flags & flag) != 0;
}

bool weaponHasAmmoFlag(s32 weaponnum, s32 funcnum, u32 flag)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	struct inventory_ammo *ammo;

	if (weapon == NULL) {
		return false;
	}

	ammo = weapon->ammos[funcnum];

	if (ammo) {
		return (ammo->flags & flag) != 0;
	}

	return false;
}

void func0f0b18ac(s32 arg0)
{
	// empty
}

void func0f0b18b4(s32 arg0)
{
	// empty
}

s32 currentPlayerGetDeviceState(s32 weaponnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return DEVICESTATE_UNEQUIPPED;
	}

	for (i = 0; i < ARRAYCOUNT(weapon->functions); i++) {
		if (weapon->functions[i]) {
			struct weaponfunc_device *devicefunc = weapon->functions[i];

			if ((devicefunc->base.type & 0xff) == INVENTORYFUNCTYPE_DEVICE) {
				if ((g_Vars.currentplayer->devicesactive & devicefunc->device) == 0) {
					return DEVICESTATE_INACTIVE;
				}

				return DEVICESTATE_ACTIVE;
			}
		}
	}

	return DEVICESTATE_UNEQUIPPED;
}

void currentPlayerSetDeviceActive(s32 weaponnum, bool active)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return;
	}

	for (i = 0; i < ARRAYCOUNT(weapon->functions); i++) {
		if (weapon->functions[i]) {
			struct weaponfunc_device *devicefunc = weapon->functions[i];

			if ((devicefunc->base.type & 0xff) == INVENTORYFUNCTYPE_DEVICE) {
				if (active) {
					if (devicefunc->device & (DEVICE_NIGHTVISION | DEVICE_XRAYSCANNER | DEVICE_EYESPY | DEVICE_IRSCANNER)) {
						g_Vars.currentplayer->devicesactive &= ~(DEVICE_NIGHTVISION | DEVICE_XRAYSCANNER | DEVICE_EYESPY | DEVICE_IRSCANNER);
					}

					g_Vars.currentplayer->devicesactive |= devicefunc->device;
					return;
				}

				g_Vars.currentplayer->devicesactive &= ~devicefunc->device;
				return;
			}
		}
	}
}

u16 weaponGetFileNum(s32 weaponnum)
{
	struct weapon *weapon = NULL;

	if (weaponnum != -1) {
		weapon = g_Weapons[weaponnum];
	}

	if (weapon) {
#ifdef PLATFORM_N64
		if (PLAYERCOUNT() >= 2) {
			return weapon->lo_model;
		}
#endif

		return weapon->hi_model;
	}

	return 0;
}

u16 weaponGetFileNum2(s32 weaponnum)
{
	return weaponGetFileNum(weaponnum);
}

void gsetPopulateFromCurrentPlayer(s32 handnum, struct gset *gset)
{
	gset->weaponnum = g_Vars.currentplayer->gunctrl.weaponnum;
	gset->weaponfunc = g_Vars.currentplayer->hands[handnum].gset.weaponfunc;
	gset->unk063a = g_Vars.currentplayer->hands[handnum].gset.unk063a;
	gset->unk0639 = g_Vars.currentplayer->hands[handnum].gset.unk0639;

	if (gset->weaponnum == WEAPON_MAULER) {
		gset->unk063a = g_Vars.currentplayer->hands[handnum].matmot1 * 10.0f;
	}

	if (gset->weaponnum == WEAPON_LASER) {
		gset->unk063a = g_Vars.currentplayer->hands[handnum].burstbullets & 0xff;
	}
}

struct inventory_ammo *gsetGetAmmoDefinition(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (func && func->ammoindex >= 0) {
		return weapon->ammos[func->ammoindex];
	}

	return NULL;
}

u8 gsetGetSinglePenetration(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->penetration;
	}

	return 0;
}

s32 handGetCasingEject(struct gset *gset)
{
	s32 result = 0;
	struct inventory_ammo *ammo = gsetGetAmmoDefinition(gset);

	if (ammo) {
		result = ammo->casingeject;
	}

	return result;
}

f32 gsetGetImpactForce(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	f32 result = 0;

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		result = funcshoot->impactforce;
	}

	return result;
}

f32 gsetGetDamage(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	f32 damage = 0;

	if (func) {
		if ((func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
			struct weaponfunc_shoot *shootfunc = (struct weaponfunc_shoot *)func;
			damage = shootfunc->damage;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			struct weaponfunc_melee *meleefunc = (struct weaponfunc_melee *)func;
			damage = meleefunc->damage;

			if (gset->weaponnum == WEAPON_REAPER) {
				damage *= LVUPDATE60FREAL();
			}
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_THROW) {
			struct weaponfunc_throw *throwfunc = (struct weaponfunc_throw *)func;
			damage = throwfunc->damage;
		}
	}

	if (gset->weaponnum == WEAPON_MAULER) {
		damage = (gset->unk063a / 3.0f + 1.0f) * damage;
	}

	if (bgunIsFiring(HAND_LEFT) && bgunIsFiring(HAND_RIGHT)) {
		damage += damage;
	}

	return damage;
}

u8 gsetGetFireslotDuration(struct gset *gset)
{
#if VERSION >= VERSION_PAL_FINAL
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	u8 result = 0;

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		result = funcshoot->duration60;
	}

	if (result >= 4) {
		result = TICKS(result);
	}

	return result;
#else
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->duration60;
	}

	return 0;
#endif
}

u16 gsetGetSingleShootSound(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->shootsound;
	}

	return 0;
}

bool gsetHasFunctionFlags(struct gset *gset, u32 flags)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func) {
		return (func->flags & flags) == flags;
	}

	return false;
}

s8 weaponGetNumTicksPerShot(u32 weaponnum, u32 funcindex)
{
	u32 stack[2];
	s32 result = 0;
	struct weapon *weapon = weaponFindById(weaponnum);
	struct weaponfunc *func = weapon->functions[funcindex];

	if (func && func->type == INVENTORYFUNCTYPE_SHOOT_AUTOMATIC) {
		struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *)func;

		result = 3600.0f / autofunc->maxrpm;
	}

#if VERSION != VERSION_PAL_BETA
	// PAL beta removes this check, only for it to be added back in PAL final
	if (result > 3) {
		result = TICKS(result);
	}
#endif

	return result;
}

u32 currentPlayerGetSight(void)
{
	struct weaponfunc *func = weaponGetFunctionById(
			g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum,
			g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
		return SIGHT_NONE;
	}

	if (cheatIsActive(CHEAT_CLASSICSIGHT)) {
		return SIGHT_CLASSIC;
	}

	switch (g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum) {
	case WEAPON_HORIZONSCANNER:
		return SIGHT_NONE;
	case WEAPON_NONE:
	case WEAPON_UNARMED:
	case WEAPON_FALCON2:
	case WEAPON_FALCON2_SILENCER:
	case WEAPON_DY357MAGNUM:
	case WEAPON_DY357LX:
	case WEAPON_CMP150:
	case WEAPON_CYCLONE:
	case WEAPON_RCP120:
	case WEAPON_SHOTGUN:
	case WEAPON_DEVASTATOR:
	case WEAPON_ROCKETLAUNCHER:
	case WEAPON_SLAYER:
	case WEAPON_COMBATKNIFE:
	case WEAPON_CROSSBOW:
	case WEAPON_TRANQUILIZER:
	case WEAPON_LASER:
	case WEAPON_GRENADE:
	case WEAPON_NBOMB:
	case WEAPON_TIMEDMINE:
	case WEAPON_PROXIMITYMINE:
	case WEAPON_REMOTEMINE:
	case WEAPON_ECMMINE:
		return SIGHT_DEFAULT;
	case WEAPON_FALCON2_SCOPE:
	case WEAPON_MAGSEC4:
	case WEAPON_SNIPERRIFLE:
	case WEAPON_LAPTOPGUN:
	case WEAPON_DRAGON:
	case WEAPON_K7AVENGER:
	case WEAPON_AR34:
	case WEAPON_SUPERDRAGON:
		return SIGHT_ZOOM;
	case WEAPON_MAULER:
	case WEAPON_REAPER:
		return SIGHT_SKEDAR;
	case WEAPON_PHOENIX:
	case WEAPON_CALLISTO:
	case WEAPON_FARSIGHT:
		return SIGHT_MAIAN;
	case WEAPON_PP9I:
	case WEAPON_CC13:
	case WEAPON_KL01313:
	case WEAPON_KF7SPECIAL:
	case WEAPON_ZZT:
	case WEAPON_DMC:
	case WEAPON_AR53:
	case WEAPON_RCP45:
		return SIGHT_CLASSIC;
	}

	return SIGHT_DEFAULT;
}

void gsetGetNoiseSettings(struct gset *gset, struct noisesettings *dst)
{
	struct noisesettings *settings = NULL;
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func != NULL) {
		settings = func->noisesettings;
	}

	if (settings == NULL) {
		settings = &invnoisesettings_silent;
	}

	dst->minradius = settings->minradius;
	dst->maxradius = settings->maxradius;
	dst->incradius = settings->incradius;
	dst->decbasespeed = settings->decbasespeed;
	dst->decremspeed = settings->decremspeed;
}

struct guncmd *handGetEquipAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->equip_animation;
	}

	return NULL;
}

struct guncmd *handGetUnequipAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->unequip_animation;
	}

	return NULL;
}

struct guncmd *gsetGetPriToSecAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->pritosec_animation;
	}

	return NULL;
}

struct guncmd *gsetGetSecToPriAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->sectopri_animation;
	}

	return NULL;
}
