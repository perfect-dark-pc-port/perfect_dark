#ifndef _IN_CONSTANTS_H
#define _IN_CONSTANTS_H
#include "versions.h"
#include "math.h"
#include "animations.h"
#include "files.h"
#include "sequences.h"
#include "sfx.h"

#define FALSE 0
#define TRUE  1

#define false 0
#define true  1

#define MAX_ARTIFACTS          120
#define MAX_BOTS               8
#define MAX_CHRSPERSQUADRON    16
#define MAX_CHRSPERTEAM        32
#define MAX_CHRWAYPOINTS       6
#define MAX_EYESPYDARTS        8
#define MAX_MPCHRS             (MAX_PLAYERS + MAX_BOTS)
#define MAX_MPPLAYERCONFIGS    (MAX_PLAYERS + 2)
#define MAX_OBJECTIVES         10
#define MAX_PLAYERS            4
#define MAX_PROPSPERROOMCHUNK  7
#define MAX_ROOMPROPLISTCHUNKS 256
#define MAX_SQUADRONS          16
#define MAX_TEAMS              8
#define MAX_EXPLOSIONS_DEFAULT 6

#define NUM_BOTDIFFS          6
#define NUM_CYCLEABLE_WEAPONS 45
#define NUM_DEVICETESTS       10
#define NUM_FRAMEBUFFERS      2
#define NUM_GFXTASKS          2
#define NUM_HOLOTESTS         7
#define NUM_MPWEAPONSLOTS     6
#define NUM_SOLOSTAGES        21
#define NUM_TEXTURES          (VERSION == VERSION_JPN_FINAL ? 3511 : 3503)

#define osSyncPrintf

#define S32_MAX  2147483647
#define U32_MAX  4294967295
#define MINFLOAT ((float)-3.40282346638528860e+38)
#define MAXFLOAT ((float)3.40282346638528860e+38)

#define ABS(val)            ((val) > 0 ? (val) : -(val))
#define ABSF(val)           ((val) > 0.0f ? (val) : -(val))
#define ALIGN2(val)         (((val) | 1) ^ 0x1)
#define ALIGN4(val)         (((val) | 3) ^ 0x3)
#define ALIGN8(val)         ((((val) + 0x7) | 0x7) ^ 0x7)
#define ALIGN16(val)        ((((val) + 0xf) | 0xf) ^ 0xf)
#define ALIGN64(val)        (((((uintptr_t)(val)) + 0x3f) | 0x3f) ^ 0x3f)
#define ARRAYCOUNT(a)       (s32)(sizeof(a) / sizeof(a[0]))
#define CHRNAVSEED(chr)     ((g_Vars.lvframe60 >> 9) * 128 + chr->chrnum * 8)
#define CHRRACE(chr)        (chr ? chr->race : RACE_HUMAN)
#define CRASH()             *(u8 *)0 = 69
#define CYCLES_PER_FRAME    ((s32) OS_CPU_COUNTER / (PAL ? 50 : 60))
#define IS4MB()             (g_Is4Mb == true)
#define IS8MB()             (g_Is4Mb != true)
#define LINEHEIGHT          (VERSION == VERSION_JPN_FINAL ? 14 : 11)
#define MIXCOLOUR(dialog, property) dialog->transitionfrac < 0.0f ? g_MenuColours[dialog->type].property : colourBlend(g_MenuColours[dialog->type2].property, g_MenuColours[dialog->type].property, dialog->colourweight)
#define MPCHR(index)        ((index) < MAX_PLAYERS ? &g_PlayerConfigsArray[index].base : &g_BotConfigsArray[(index) - MAX_PLAYERS].base)
#define RANDOMFRAC()        (rngRandom() * (1.0f / U32_MAX))
#define SECSTOTIME240(secs) (secs * 240)
#define SECSTOTIME60(secs)  (secs * 60)
#define PFS(device)         (device == SAVEDEVICE_GAMEPAK ? NULL : &g_Pfses[device])

#if MAX_PLAYERS >= 4
#define PLAYERCOUNT()       ((g_Vars.players[0] ? 1 : 0) + (g_Vars.players[1] ? 1 : 0) + (g_Vars.players[2] ? 1 : 0) + (g_Vars.players[3] ? 1 : 0))
#elif MAX_PLAYERS >= 3
#define PLAYERCOUNT()       ((g_Vars.players[0] ? 1 : 0) + (g_Vars.players[1] ? 1 : 0) + (g_Vars.players[2] ? 1 : 0))
#elif MAX_PLAYERS >= 2
#define PLAYERCOUNT()       ((g_Vars.players[0] ? 1 : 0) + (g_Vars.players[1] ? 1 : 0))
#else
#define PLAYERCOUNT()       1
#endif

#define VALIDWEAPON()       (g_Vars.currentplayer->gunctrl.weaponnum >= WEAPON_UNARMED && g_Vars.currentplayer->gunctrl.weaponnum <= WEAPON_COMBATBOOST)
#define FUNCISSEC()         (VALIDWEAPON() && (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].gunfuncs[(g_Vars.currentplayer->gunctrl.weaponnum - 1) >> 3] & (1 << ((g_Vars.currentplayer->gunctrl.weaponnum - 1) & 7))))

#define USINGDEVICE(device) (!g_Vars.currentplayer->isdead \
		&& !g_InCutscene \
		&& EYESPYINACTIVE() \
		&& (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & (device)))

#if VERSION >= VERSION_PAL_BETA
#define LVUPDATE60FREAL()   g_Vars.lvupdate60freal
#else
#define LVUPDATE60FREAL()   g_Vars.lvupdate60f
#endif

#if VERSION >= VERSION_NTSC_1_0
#define VOLUME(volume)      (volume > 0x5000 ? 0x5000 : volume)
#else
#define VOLUME(volume)      (volume)
#endif

#define EYESPYINACTIVE()    (!g_Vars.currentplayer->eyespy || (g_Vars.currentplayer->eyespy && !g_Vars.currentplayer->eyespy->active))

#define ERASERSQDIST(p) ( \
		(p[0] - g_Vars.currentplayer->eraserpos.f[0]) * (p[0] - g_Vars.currentplayer->eraserpos.f[0]) + \
		(p[1] - g_Vars.currentplayer->eraserpos.f[1]) * (p[1] - g_Vars.currentplayer->eraserpos.f[1]) + \
		(p[2] - g_Vars.currentplayer->eraserpos.f[2]) * (p[2] - g_Vars.currentplayer->eraserpos.f[2]))

#if PAL
#define TICKS(val)    ((val) * 50 / 60)
#define PALUP(val)    ((val) * 60 / 50)
#define PALUPF(val)   ((val) * 1.2f)
#define FRAMEDURATION (1 / 50.0f)
#else
#define TICKS(val)    (val)
#define PALUP(val)    (val)
#define PALUPF(val)   (val)
#define FRAMEDURATION (1 / 60.0f)
#endif

// Macro to convert an ASCII character to N64 font code.
// N64 font code uses 0x0f for space, 0x10-0x19 for 0-9 and 0x1a-0x33 for A-Z.
#define N64CHAR(c)          (c == ' ' ? 0x0f : (c >= 'A' && c <= 'Z' ? c - 0x27 : c - 0x20))

#define PORTAL_IS_CLOSED(portalnum) ((g_BgPortals[portalnum].flags & PORTALFLAG_CLOSED) && (g_BgPortals[portalnum].flags & PORTALFLAG_FORCEOPEN) == 0)

#ifdef __sgi
#define ALIGNED16
#else
#define ALIGNED16 __attribute__ ((aligned (16)))
#endif

#if VERSION >= VERSION_NTSC_1_0
#define ROM_COMPANYCODE 0x3459
#else
#define ROM_COMPANYCODE 0x3031
#endif

#if VERSION == VERSION_JPN_FINAL
#define ROM_GAMECODE    'NPDJ'
#elif PAL
#define ROM_GAMECODE    'NPDP'
#else
#define ROM_GAMECODE    'NPDE'
#endif

#if VERSION == VERSION_JPN_FINAL
#define MAX_JPN_CACHE_ITEMS() g_JpnMaxCacheItems
#else
#define MAX_JPN_CACHE_ITEMS() 124
#endif

// These actions are assigned to chr->actiontype
#define ACT_INIT             0
#define ACT_STAND            1
#define ACT_KNEEL            2
#define ACT_ANIM             3
#define ACT_DIE              4
#define ACT_DEAD             5
#define ACT_ARGH             6
#define ACT_PREARGH          7
#define ACT_ATTACK           8
#define ACT_ATTACKWALK       9
#define ACT_ATTACKROLL       10
#define ACT_SIDESTEP         11
#define ACT_JUMPOUT          12
#define ACT_RUNPOS           13
#define ACT_PATROL           14
#define ACT_GOPOS            15
#define ACT_SURRENDER        16
#define ACT_LOOKATTARGET     17
#define ACT_SURPRISED        18
#define ACT_STARTALARM       19
#define ACT_THROWGRENADE     20
#define ACT_TURNDIR          21
#define ACT_TEST             22
#define ACT_BONDINTRO        23
#define ACT_BONDDIE          24
#define ACT_BONDMULTI        25
#define ACT_NULL             26
#define ACT_BOT_ATTACKSTAND  27
#define ACT_BOT_ATTACKKNEEL  28
#define ACT_BOT_ATTACKSTRAFE 29
#define ACT_DRUGGEDDROP      30
#define ACT_DRUGGEDKO        31
#define ACT_DRUGGEDCOMINGUP  32
#define ACT_ATTACKAMOUNT     33
#define ACT_ROBOTATTACK      34
#define ACT_SKJUMP           35
#define ACT_PUNCH            36
#define ACT_CUTFIRE          37

#define AIBOTCMD_FOLLOW   0
#define AIBOTCMD_ATTACK   1
#define AIBOTCMD_DEFEND   2
#define AIBOTCMD_HOLD     3
#define AIBOTCMD_NORMAL   4
#define AIBOTCMD_DOWNLOAD 5
#define AIBOTCMD_GETCASE  6
#define AIBOTCMD_TAGBOX   7
#define AIBOTCMD_SAVECASE 8
#define AIBOTCMD_DEFHILL  9
#define AIBOTCMD_HOLDHILL 10
#define AIBOTCMD_GETCASE2 11
#define AIBOTCMD_POPCAP   12
#define AIBOTCMD_PROTECT  13

#define AICMD_END           0x0004
#define AICMD_DROPITEM      0x001c
#define AICMD_SPAWNCHRATPAD 0x00c6
#define AICMD_SPAWNCHRATCHR 0x00c7
#define AICMD_EQUIPWEAPON   0x00c8
#define AICMD_EQUIPHAT      0x00c9

// AI environment commands
#define AIENVCMD_00                   0x00
#define AIENVCMD_01                   0x01
#define AIENVCMD_02                   0x02
#define AIENVCMD_03                   0x03
#define AIENVCMD_04                   0x04
#define AIENVCMD_ROOM_SETAMBIENT      0x05
#define AIENVCMD_ROOM_SETOUTDOORS     0x06
#define AIENVCMD_07                   0x07
#define AIENVCMD_08                   0x08
#define AIENVCMD_SETAMBIENT           0x09
#define AIENVCMD_PLAYNOSEDIVE         0x0a
#define AIENVCMD_TICKMUSICQUEUE       0x0b
#define AIENVCMD_ROOM_SETFAULTYLIGHTS 0x0c
#define AIENVCMD_STOPNOSEDIVE         0x0d
#define AIENVCMD_PLAYUFOHUM           0x0e
#define AIENVCMD_STOPUFOHUM           0x0f

#define AIMCONTROL_HOLD   0
#define AIMCONTROL_TOGGLE 1

#define AMMOFLAG_NORESERVE         1 // Ammo cannot be held outside of weapon (cloak and combat boost)
#define AMMOFLAG_EQUIPPEDISRESERVE 2 // For throwables: equipped ammo and reserve is kind of the same thing
#define AMMOFLAG_INCREMENTALRELOAD 4 // Shotgun and crossbow
#define AMMOFLAG_QTYAFFECTSPARTVIS 8 // Quantity of ammo in the clip affects model part visibility on the weapon (Devastator)

#define AMMOTYPE_PISTOL       0x01
#define AMMOTYPE_SMG          0x02
#define AMMOTYPE_CROSSBOW     0x03
#define AMMOTYPE_RIFLE        0x04
#define AMMOTYPE_SHOTGUN      0x05
#define AMMOTYPE_FARSIGHT     0x06
#define AMMOTYPE_GRENADE      0x07
#define AMMOTYPE_ROCKET       0x08
#define AMMOTYPE_KNIFE        0x09
#define AMMOTYPE_MAGNUM       0x0a
#define AMMOTYPE_DEVASTATOR   0x0b
#define AMMOTYPE_REMOTE_MINE  0x0c
#define AMMOTYPE_PROXY_MINE   0x0d
#define AMMOTYPE_TIMED_MINE   0x0e
#define AMMOTYPE_REAPER       0x0f
#define AMMOTYPE_HOMINGROCKET 0x10
#define AMMOTYPE_DART         0x11
#define AMMOTYPE_NBOMB        0x12
#define AMMOTYPE_SEDATIVE     0x13
#define AMMOTYPE_CLOAK        0x14
#define AMMOTYPE_BOOST        0x15
#define AMMOTYPE_PSYCHOSIS    0x16
#define AMMOTYPE_17           0x17
#define AMMOTYPE_BUG          0x18
#define AMMOTYPE_MICROCAMERA  0x19
#define AMMOTYPE_PLASTIQUE    0x1a
#define AMMOTYPE_1B           0x1b
#define AMMOTYPE_1C           0x1c
#define AMMOTYPE_1D           0x1d
#define AMMOTYPE_TOKEN        0x1e
#define AMMOTYPE_1F           0x1f
#define AMMOTYPE_ECM_MINE     0x20

// Active menu slots
#define AMSLOTFLAG_CURRENT 0x02 // black, eg. current equipped weapon
#define AMSLOTFLAG_ACTIVE  0x08 // pulsates slightly, eg. cloak/xray when active
#define AMSLOTFLAG_NOAMMO  0x10 // black BG, no border, orange text

#define AMSLOTMODE_DEFAULT 0
#define AMSLOTMODE_FOCUSED 1
#define AMSLOTMODE_CURRENT 2

// Active menu (weapon wheel) modes
#define AMMODE_CLOSED 0
#define AMMODE_VIEW   1
#define AMMODE_EDIT   2 // unused

#define ANIMFLAG_LOOP                0x01
#define ANIMFLAG_ABSOLUTETRANSLATION 0x02
#define ANIMFLAG_HASREPEATFRAMES     0x04
#define ANIMFLAG_HASCUTSKIPFRAMES    0x08

#define ANIMFIELD_S16_ROTATE    0x01
#define ANIMFIELD_S16_TRANSLATE 0x02
#define ANIMFIELD_08            0x08
#define ANIMFIELD_F32_ROTATE    0x10
#define ANIMFIELD_S32_TRANSLATE 0x20
#define ANIMFIELD_CAMERA        0x40
#define ANIMFIELD_F32_SCALE     0x80

#define ARTIFACTTYPE_FREE   0
#define ARTIFACTTYPE_CIRCLE 1 // from a sun lens flare or teleport
#define ARTIFACTTYPE_GLARE  2 // from a light

#define ATTACKFLAG_AIMATBOND      0x0001 // aim/shoot at Jo
#define ATTACKFLAG_AIMFORWARD     0x0002 // aim/shoot in front of self
#define ATTACKFLAG_AIMATCHR       0x0004 // aim/shoot at chr (ID should be given in entityid)
#define ATTACKFLAG_AIMATPAD       0x0008 // aim/shoot at pad (ID should be given in entityid)
#define ATTACKFLAG_AIMATDIRECTION 0x0010 // aim/shoot in compass direction (0000, 4000, 8000, c000)
#define ATTACKFLAG_AIMONLY        0x0020 // aim only - do not shoot
#define ATTACKFLAG_DONTTURN       0x0040 // don't do a shooting animation that would change the chr's direction
#define ATTACKFLAG_SINGLESHOT     0x0080 // single shot only (doesn't work correctly with dual weapons)
#define ATTACKFLAG_NOVERTICAL     0x0100 // don't aim up/down
#define ATTACKFLAG_AIMATTARGET    0x0200 // aim/shoot at whatever is in the chr's `target` field

#define AUDIOCONFIGFLAG_01           0x01
#define AUDIOCONFIGFLAG_RESPONDHELLO 0x04
#define AUDIOCONFIGFLAG_08           0x08
#define AUDIOCONFIGFLAG_OFFENSIVE    0x10
#define AUDIOCONFIGFLAG_20           0x20
#define AUDIOCONFIGFLAG_40           0x40

#define AWARD_MOSTSUICIDAL     0x00000001
#define AWARD_WHONEEDSAMMO     0x00000002
#define AWARD_LEASTSHIELDED    0x00000004
#define AWARD_BESTPROTECTED    0x00000008
#define AWARD_MARKSMANSHIP     0x00000010
#define AWARD_MOSTPROFESSIONAL 0x00000020
#define AWARD_MOSTDEADLY       0x00000040
#define AWARD_MOSTHARMLESS     0x00000080
#define AWARD_MOSTCOWARDLY     0x00000100
#define AWARD_MOSTFRANTIC      0x00000200
#define AWARD_MOSTHONORABLE    0x00000400
#define AWARD_MOSTDISHONORABLE 0x00000800
#define AWARD_SHORTESTLIFE     0x00001000
#define AWARD_LONGESTLIFE      0x00002000
#define AWARD_DOUBLEKILL       0x00004000
#define AWARD_TRIPLEKILL       0x00008000
#define AWARD_QUADKILL         0x00010000

#define BANK_0 0
#define BANK_1 1

#define BOTDIFF_MEAT     0
#define BOTDIFF_EASY     1
#define BOTDIFF_NORMAL   2
#define BOTDIFF_HARD     3
#define BOTDIFF_PERFECT  4
#define BOTDIFF_DARK     5
#define BOTDIFF_DISABLED 6

#define BOTDISTCFG_CLOSE          0
#define BOTDISTCFG_PISTOL         1
#define BOTDISTCFG_DEFAULT        2
#define BOTDISTCFG_SHOOTEXPLOSIVE 3
#define BOTDISTCFG_KAZE           4
#define BOTDISTCFG_FARSIGHT       5
#define BOTDISTCFG_FOLLOW         6
#define BOTDISTCFG_THROWEXPLOSIVE 7

// Bot distance modes
#define BOTDISTMODE_BACKUP  1
#define BOTDISTMODE_OK      2
#define BOTDISTMODE_ADVANCE 3
#define BOTDISTMODE_GOTO    4

#define BOTFLAG_UNLIMITEDAMMO         0x0001
#define BOTFLAG_AVOIDINGDANGEROUSPROP 0x0004
#define BOTFLAG_THREWREMOTEMINE       0x1000

#define BOTTYPE_GENERAL 0
#define BOTTYPE_PEACE   1  // Collects weapons but doesn't engage in combat
#define BOTTYPE_SHIELD  2  // Prioritises full shield before combat
#define BOTTYPE_ROCKET  3  // Prefers explosive weapons
#define BOTTYPE_KAZE    4  // Does not keep distance
#define BOTTYPE_FIST    5  // Uses fists only
#define BOTTYPE_PREY    6  // Target players who are newly spawned, have inferior weapons or are low on health
#define BOTTYPE_COWARD  7  // Runs away, only attacks if opponent's weapon is inferior
#define BOTTYPE_JUDGE   8  // Targets the winning player
#define BOTTYPE_FEUD    9  // Targets a single player for the whole match
#define BOTTYPE_SPEED   10 // Runs faster
#define BOTTYPE_TURTLE  11 // Moves slower and has double shield
#define BOTTYPE_VENGE   12 // Targets the last player who killed it

#define BRIEFINGTYPE_LOCATION 0
#define BRIEFINGTYPE_TEXT_PA  1
#define BRIEFINGTYPE_TEXT_SA  2
#define BRIEFINGTYPE_TEXT_A   3

#define CAMERAMODE_DEFAULT     0
#define CAMERAMODE_THIRDPERSON 1
#define CAMERAMODE_EYESPY      2

#define CASING_NONE     -1
#define CASING_STANDARD 0
#define CASING_REAPER   1
#define CASING_RIFLE    2
#define CASING_SHOTGUN  3

#define CHANNEL_0        0
#define CHANNEL_1        1
#define CHANNEL_2        2
#define CHANNEL_3        3
#define CHANNEL_4        4
#define CHANNEL_5        5
#define CHANNEL_6        6
#define CHANNEL_7        7
#define CHANNEL_8        8
#define CHANNEL_9        9
#define CHANNEL_CUTSCENE 10

#define CHEAT_HURRICANEFISTS         0
#define CHEAT_CLOAKINGDEVICE         1
#define CHEAT_INVINCIBLE             2
#define CHEAT_ALLGUNS                3
#define CHEAT_UNLIMITEDAMMO          4
#define CHEAT_UNLIMITEDAMMONORELOADS 5
#define CHEAT_SLOMO                  6
#define CHEAT_DKMODE                 7
#define CHEAT_TRENTSMAGNUM           8
#define CHEAT_FARSIGHT               9
#define CHEAT_SMALLJO                10
#define CHEAT_SMALLCHARACTERS        11
#define CHEAT_ENEMYSHIELDS           12
#define CHEAT_JOSHIELD               13
#define CHEAT_SUPERSHIELD            14
#define CHEAT_CLASSICSIGHT           15
#define CHEAT_TEAMHEADSONLY          16
#define CHEAT_PLAYASELVIS            17
#define CHEAT_ENEMYROCKETS           18
#define CHEAT_UNLIMITEDAMMOLAPTOP    19
#define CHEAT_MARQUIS                20
#define CHEAT_PERFECTDARKNESS        21
#define CHEAT_PUGILIST               22
#define CHEAT_HOTSHOT                23
#define CHEAT_HITANDRUN              24
#define CHEAT_ALIEN                  25
#define CHEAT_RTRACKER               26
#define CHEAT_ROCKETLAUNCHER         27
#define CHEAT_SNIPERRIFLE            28
#define CHEAT_XRAYSCANNER            29
#define CHEAT_SUPERDRAGON            30
#define CHEAT_LAPTOPGUN              31
#define CHEAT_PHOENIX                32
#define CHEAT_PSYCHOSISGUN           33
#define CHEAT_PP9I                   34
#define CHEAT_CC13                   35
#define CHEAT_KL01313                36
#define CHEAT_KF7SPECIAL             37
#define CHEAT_ZZT                    38
#define CHEAT_DMC                    39
#define CHEAT_AR53                   40
#define CHEAT_RCP45                  41
#define CHEAT_DUALWIELDALLGUNS       42

#define CHEATFLAG_TIMED       0
#define CHEATFLAG_ALWAYSON    1
#define CHEATFLAG_TRANSFERPAK 2
#define CHEATFLAG_COMPLETION  4
#define CHEATFLAG_FIRINGRANGE 8

#define CHECKSUM_PLACEHOLDER 0x99aabbcc

#define CHOKETYPE_NONE   0
#define CHOKETYPE_GURGLE 1
#define CHOKETYPE_COUGH  2

#define CHR_P1P2_OPPOSITE 0xf1
#define CHR_P1P2          0xf2
#define CHR_ANY           0xf3 // Only supported by if_chr_activated_object command
#define CHR_ANTI          0xf4
#define CHR_COOP          0xf5
#define CHR_TARGET        0xf6
#define CHR_BOND          0xf8
#define CHR_CLONE         0xf9
#define CHR_SEESHOT       0xfa
#define CHR_SEEDIE        0xfb
#define CHR_PRESET        0xfc
#define CHR_SELF          0xfd

#define CHRANIMFLAG_FLIP          0x01
#define CHRANIMFLAG_MOVEWHENINVIS 0x02
#define CHRANIMFLAG_PAUSEATEND    0x04
#define CHRANIMFLAG_COMPLETED     0x08
#define CHRANIMFLAG_SLOWUPDATE    0x10
#define CHRANIMFLAG_LOCKPOS       0x40
#define CHRANIMFLAG_REVERSE       0x80

// chr->flags
#define CHRFLAG0_CANT_ALERT_GROUP   0x00000001 // Don't set group alertness when becoming aware
#define CHRFLAG0_UNUSED_00000002    0x00000002
#define CHRFLAG0_GP1                0x00000004 // General purpose
#define CHRFLAG0_CAN_EXAMINE_BODY   0x00000008
#define CHRFLAG0_UNDERCOVER         0x00000010 // Chr is hiding behind cover
#define CHRFLAG0_SAID_INJURY_QUIP   0x00000020
#define CHRFLAG0_COVER_TYPE1        0x00000040 // Use a certain type of cover - never set though
#define CHRFLAG0_CHUCKNORRIS        0x00000080 // Makes punch animation faster, and damage is multiplied by the chr's morale then doubled. Must also have CHRFLAG1_ADJUSTPUNCHSPEED
#define CHRFLAG0_CAN_BACKOFF        0x00000100
#define CHRFLAG0_CAN_RETREAT        0x00000200
#define CHRFLAG0_SURPRISABLE        0x00000400
#define CHRFLAG0_CAN_THROW_GRENADES 0x00000800
#define CHRFLAG0_CAN_FLANK          0x00001000
#define CHRFLAG0_SKIPSAFETYCHECKS   0x00002000
#define CHRFLAG0_FORCESAFETYCHECKS  0x00004000 // Overrides CHRFLAG0_SKIPSAFETYCHECKS
#define CHRFLAG0_00008000           0x00008000 // Something to do with breaking cover or grenades?
#define CHRFLAG0_AIVSAI             0x00010000 // Allow chr to fight with other AI
#define CHRFLAG0_CAN_TRAP           0x00020000
#define CHRFLAG0_ACTIVATEALARM      0x00040000 // Chr will run for alarm
#define CHRFLAG0_SQUADALERTANYDIST  0x00080000
#define CHRFLAG0_ALERT_ON_HEARSPAWN 0x00100000 // Become alert when hearing player and never on screen
#define CHRFLAG0_CAN_FLEESURRENDER  0x00200000 // When unarmed, chr can run from player and surrender if caught up
#define CHRFLAG0_UNUSED_00400000    0x00400000
#define CHRFLAG0_CAN_HEAR_ALARMS    0x00800000
#define CHRFLAG0_NOACCURACYAFTERJAM 0x01000000
#define CHRFLAG0_SAID_AMBUSH_QUIP   0x02000000
#define CHRFLAG0_HOLD_POSITION      0x04000000 // Chr can't track (follow) player
#define CHRFLAG0_COVER_TYPE2        0x08000000
#define CHRFLAG0_CAMP               0x10000000 // Flag is turned off permanently if player gets too close
#define CHRFLAG0_CAN_HEARSPAWN      0x20000000
#define CHRFLAG0_HEAR_REQUIRE_LOS   0x40000000
#define CHRFLAG0_CANLOSEGUN         0x80000000

// chr->flags2
#define CHRFLAG1_HANDCOMBATONLY            0x00000001
#define CHRFLAG1_LOSTGUN                   0x00000002 // Chr has dropped their gun and can no longer recover it
#define CHRFLAG1_CAN_ATTACKAMOUNT          0x00000004 // Can use ACT_ATTACKAMOUNT as part of general attack anims
#define CHRFLAG1_WARFIRSTKING              0x00000008 // Specific to WAR only
#define CHRFLAG1_CAN_ATTACKAMOUNTRELOAD    0x00000010 // Same as CHRFLAG1_CAN_ATTACKAMOUNT, but reload afterwards
#define CHRFLAG1_CAN_RELOAD                0x00000020
#define CHRFLAG1_CAN_SNIPE                 0x00000040
#define CHRFLAG1_ADJUSTPUNCHSPEED          0x00000080 // Makes chr punch slower, unless chr also has CHRFLAG0_CHUCKNORRIS
#define CHRFLAG1_DOINGIDLEANIMATION        0x00000100
#define CHRFLAG1_PREWARNED                 0x00000200
#define CHRFLAG1_INDARKROOM                0x00000400 // Lights are out - chr has poor visibility
#define CHRFLAG1_PUNCHAIEXTRADAMAGE        0x00000800 // Punch does extra damage if target is AI - once only
#define CHRFLAG1_NOHANDCOMBAT              0x00001000 // Chr will not use hand combat
#define CHRFLAG1_THROWGRENADEFIRST         0x00002000 // Chr will throw grenade as first attack
#define CHRFLAG1_ALLOWSOFTCOVER            0x00004000
#define CHRFLAG1_LONG_CAMSPY_OBSERVATION   0x00008000
#define CHRFLAG1_HEARSPAWN_ALERT_IF_ALARM  0x00010000 // After hearing an alarm, if the chr spawns a clone due to hearing you the chr themself will become alert
#define CHRFLAG1_DODGED                    0x00020000 // Has no effect
#define CHRFLAG1_FLANKED                   0x00040000
#define CHRFLAG1_NOIDLEANIMS               0x00080000 // Don't yawn etc
#define CHRFLAG1_AIVSAI_ADVANTAGED         0x00100000 // Chr will attack other AI first
#define CHRFLAG1_NOOP_00200000             0x00200000
#define CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT 0x00400000 // If unset, return after punching. If set, assign general combat
#define CHRFLAG1_DONE_SEARCH_ANIM          0x00800000
#define CHRFLAG1_LOOKINGFORTARGET          0x01000000 // Chr is tracking/searching for their target
#define CHRFLAG1_STOPTRACKINGIFLOOKEDAT    0x02000000
#define CHRFLAG1_CAN_LOOK_AROUND           0x04000000
#define CHRFLAG1_TALKINGTODISGUISE         0x08000000
#define CHRFLAG1_SEARCHSAMEROOM            0x10000000 // Search for player in chr's current room rather than player's current room
#define CHRFLAG1_CAN_DRAW_PISTOL           0x20000000
#define CHRFLAG1_IGNORECOVER               0x40000000
#define CHRFLAG1_PUNCHHARDER               0x80000000 // Punches deal 6x the usual damage. Must not have CHRFLAG0_CHUCKNORRIS

// chr->hidden
#define CHRHFLAG_DROPPINGITEM         0x00000001 // Chr is currently dropping an item
#define CHRHFLAG_IS_HEARING_TARGET    0x00000002 // Chr is currently hearing their target
#define CHRHFLAG_FIRINGLEFT           0x00000004
#define CHRHFLAG_FIRINGRIGHT          0x00000008
#define CHRHFLAG_BLOCKINGDOOR         0x00000010
#define CHRHFLAG_DELETING             0x00000020
#define CHRHFLAG_TIMER_RUNNING        0x00000040
#define CHRHFLAG_FIRESOUNDDONE        0x00000080
#define CHRHFLAG_PERIMDISABLED        0x00000100
#define CHRHFLAG_CONSIDERPROXIES      0x00000200 // Used for optimisation
#define CHRHFLAG_SPAWNONLYSURROUNDING 0x00000200 // When spawning, only try nearby positions rather than the given position
#define CHRHFLAG_PASSIVE              0x00000400
#define CHRHFLAG_00000800             0x00000800
#define CHRHFLAG_UNTARGETABLE         0x00001000 // Player is disgused on Rescue, or just warped
#define CHRHFLAG_DONTLOSETARGET       0x00002000 // Don't lose target when target has CHRHFLAG_UNTARGETABLE
#define CHRHFLAG_TRIGGER_BUDDY_WARP   0x00004000
#define CHRHFLAG_PERFECTACCURACY      0x00008000 // Duel opponents
#define CHRHFLAG_DISGUISE_UNCOVERED   0x00010000
#define CHRHFLAG_00020000             0x00020000
#define CHRHFLAG_00040000             0x00040000 // Not used
#define CHRHFLAG_DISGUISED            0x00080000
#define CHRHFLAG_WARPONSCREEN         0x00100000 // Allow chr warp to occur onscreen
#define CHRHFLAG_NEEDANIM             0x00200000
#define CHRHFLAG_BASICGUARD           0x00400000 // Can be used for anti, have movement stopped during high lag, and warped out of lift doorways
#define CHRHFLAG_ONBONDSSCREEN        0x00800000 // Anti mode: chr is currently on Bond's screen so shouldn't be used for spawning anti
#define CHRHFLAG_ASKEDABOUTGUN        0x01000000
#define CHRHFLAG_ALMOSTUNCOVERED      0x02000000
#define CHRHFLAG_ASKEDTHENSHOT        0x04000000 // Chr asked player about their gun and then was shot
#define CHRHFLAG_DONTSHOOTME          0x08000000 // AI buddy must use melee attacks on this chr rather than shoot
#define CHRHFLAG_INFINITESHIELD       0x10000000
#define CHRHFLAG_CLOAKED              0x20000000
#define CHRHFLAG_ANTINONINTERACTABLE  0x40000000
#define CHRHFLAG_DETECTED             0x80000000 // If set on player
#define CHRHFLAG_PSYCHOSISED          0x80000000 // If set on AI

// chr->hidden2
#define CHRH2FLAG_HIDDENFORCUTSCENE 0x0001
#define CHRH2FLAG_SHIELDHIT         0x0002 // Turns off when shield no longer visible
#define CHRH2FLAG_AUTOANIM          0x0004 // Do automatic look up/down anims for multiplayer third person
#define CHRH2FLAG_BLUESIGHT         0x0008
#define CHRH2FLAG_TICKDURINGAUTOCUT 0x0010
#define CHRH2FLAG_FIRESOUNDDONE     0x0020
#define CHRH2FLAG_CONSIDERPROXIES   0x0040
#define CHRH2FLAG_SPAWNED           0x0080
#define CHRH2FLAG_HEADSHOTTED       0x1000

// chr->chrflags
#define CHRCFLAG_FORCETOGROUND               0x00000001
#define CHRCFLAG_CLONEABLE                   0x00000002
#define CHRCFLAG_NEAR_MISS                   0x00000004
#define CHRCFLAG_EVERONSCREEN                0x00000008
#define CHRCFLAG_INVINCIBLE                  0x00000010
#define CHRCFLAG_LOSEXTRAHEIGHT              0x00000020
#define CHRCFLAG_NOFRIENDLYFIRE              0x00000040 // Chr can only hurt enemies
#define CHRCFLAG_CANFACEWALL                 0x00000080 // Defection programmer, Rescue lab techs, globals when doing idle animation
#define CHRCFLAG_JUST_INJURED                0x00000100
#define CHRCFLAG_CANCHANGEACTDURINGARGH      0x00000200
#define CHRCFLAG_HIDDEN                      0x00000400
#define CHRCFLAG_NOAUTOAIM                   0x00000800
#define CHRCFLAG_UNEXPLODABLE                0x00001000
#define CHRCFLAG_NOSHADOW                    0x00002000
#define CHRCFLAG_HAS_SPECIAL_DEATH_ANIMATION 0x00004000
#define CHRCFLAG_DIEWITHFORCE                0x00008000 // When killed with a weak weapon, fling the chr as if it was a strong weapon
#define CHRCFLAG_PERIMDISABLEDTMP            0x00010000 // Collisions disabled temporarily
#define CHRCFLAG_FORCEAUTOAIM                0x00020000 // Allow auto aim even if chr is unarmed
#define CHRCFLAG_NEVERSLEEP                  0x00040000
#define CHRCFLAG_RUNFASTER                   0x00080000
#define CHRCFLAG_KILLCOUNTABLE               0x00100000
#define CHRCFLAG_TRIGGERSHOTLIST             0x00200000
#define CHRCFLAG_00400000                    0x00400000 // Not used
#define CHRCFLAG_UNPLAYABLE                  0x00800000
#define CHRCFLAG_PUSHABLE                    0x01000000 // Typically set on allies during init
#define CHRCFLAG_SHIELDDAMAGED               0x02000000
#define CHRCFLAG_KEEPCORPSEKO                0x04000000
#define CHRCFLAG_CONSIDER_DODGE              0x08000000
#define CHRCFLAG_AVOIDING                    0x10000000
#define CHRCFLAG_DELAYANIM                   0x20000000 // Never set, but would cause a chr to pause their animation for one frame
#define CHRCFLAG_40000000                    0x40000000 // Not used
#define CHRCFLAG_INJUREDTARGET               0x80000000

#define CIQUIP_GREETING 0
#define CIQUIP_MAIN     1
#define CIQUIP_ANNOYED  2
#define CIQUIP_THANKS   3

#define CMD_LABEL 0x0002
#define CMD_END   0x0004
#define CMD_PRINT 0x00b5

// Collision detection
#define CDRESULT_ERROR       -1
#define CDRESULT_COLLISION   0
#define CDRESULT_NOCOLLISION 1

#define CDTYPE_OBJS                   0x0001
#define CDTYPE_DOORS                  0x0002
#define CDTYPE_PLAYERS                0x0004
#define CDTYPE_CHRS                   0x0008
#define CDTYPE_PATHBLOCKER            0x0010 // glass and scenery with OBJFLAG_PATHBLOCKER
#define CDTYPE_BG                     0x0020
#define CDTYPE_OBJSIMMUNETOGUNFIRE    0x0080
#define CDTYPE_AIOPAQUE               0x0100 // exclude objects with OBJFLAG_AISEETHROUGH
#define CDTYPE_OBJSIMMUNETOEXPLOSIONS 0x0200
#define CDTYPE_OBJSNOTSAFEORHELI      0x0400
#define CDTYPE_DOORSWITHOUTFLAG       0x0800
#define CDTYPE_CLOSEDDOORS            0x1000
#define CDTYPE_OPENDOORS              0x2000
#define CDTYPE_AJARDOORS              0x4000
#define CDTYPE_DOORSLOCKEDTOAI        0x8000
#define CDTYPE_ALL                    0x003f

#define CHECKVERTICAL_NO  0
#define CHECKVERTICAL_YES 1

#define CHOPPERMODE_PATROL 0
#define CHOPPERMODE_COMBAT 1
#define CHOPPERMODE_FALL   2
#define CHOPPERMODE_DEAD   3

#define COLOR_00_GREEN  0x00
#define COLOR_02_WHITE  0x02
#define COLOR_03_RED    0x03
#define COLOR_04_ORANGE 0x04
#define COLOR_05_GREEN  0x05
#define COLOR_06_WHITE  0x06
#define COLOR_07_RED    0x07
#define COLOR_08_RED    0x08
#define COLOR_09_BLUE   0x09

#define COMPARE_ANY     0
#define COMPARE_FRIENDS 1
#define COMPARE_ENEMIES 2

// Control styles - 1.1, 1.2 etc
#define CONTROLMODE_11 0
#define CONTROLMODE_12 1
#define CONTROLMODE_13 2
#define CONTROLMODE_14 3
#define CONTROLMODE_21 4
#define CONTROLMODE_22 5
#define CONTROLMODE_23 6
#define CONTROLMODE_24 7
#define CONTROLMODE_PC 8 // "pc port" controls; enabled in the .ini file

#define COUNTDOWNTIMERREASON_AI        0x01
#define COUNTDOWNTIMERREASON_NOCONTROL 0x10

#define COVERCRITERIA_0001                   0x0001
#define COVERCRITERIA_FURTHEREST             0x0002
#define COVERCRITERIA_DISTTOME               0x0004
#define COVERCRITERIA_DISTTOTARGET           0x0008
#define COVERCRITERIA_FORCENEWCOVER          0x0010
#define COVERCRITERIA_0020                   0x0020
#define COVERCRITERIA_0040                   0x0040
#define COVERCRITERIA_ALLOWNEIGHBOURINGROOMS 0x0080
#define COVERCRITERIA_ONLYNEIGHBOURINGROOMS  0x0100
#define COVERCRITERIA_0200                   0x0200
#define COVERCRITERIA_ROOMSFROMTARGET        0x0400
#define COVERCRITERIA_ROOMSFROMME            0x0800
#define COVERCRITERIA_1000                   0x1000
#define COVERCRITERIA_2000                   0x2000
#define COVERCRITERIA_DISTTOFETCHPROP        0x4000
#define COVERCRITERIA_ALLOWSOFT                   0x8000

#define COVERFLAG_OUTOFSIGHT      0x0001
#define COVERFLAG_INUSE           0x0002
#define COVERFLAG_AIMSAMEROOM     0x0004 // look + 6 metres is in the same room
#define COVERFLAG_AIMDIFFROOM     0x0008 // look + 6 metres is not in the same room
#define COVERFLAG_OMNIDIRECTIONAL 0x0010
#define COVERFLAG_SPECIAL1        0x0020
#define COVERFLAG_SPECIAL2        0x0040
#define COVERFLAG_SPECIAL3        0x0080
#define COVERFLAG_AIBOTINUSE      0x0100

// L/C/R = left/centered/right
// SM/MD/LG = small/medium/large
#define CREDITSTYLE_CORETEAM    1  // LG C, MD R
#define CREDITSTYLE_HEADING1    2  // LG C, LG C
#define CREDITSTYLE_NAME1       3  // LG C, LG C
#define CREDITSTYLE_ASSISTANT   4  // MD C, MD C
#define CREDITSTYLE_COPYRIGHT   5  // SM C, SM C
#define CREDITSTYLE_UNUSED_06   6  // MD C, SM C
#define CREDITSTYLE_HEADING2    7  // LG L, LG L
#define CREDITSTYLE_UNUSED_08   8  // LG R, LG R
#define CREDITSTYLE_NAME2       9  // MD R, MD R
#define CREDITSTYLE_RAREDESIGNS 10 // LG C, LG C
#define CREDITSTYLE_SUPERVISOR  11 // MD L, MD R
#define CREDITSTYLE_EDITOR      12 // MD C, MD C
#define CREDITSTYLE_TERMINATOR  20

#define CROUCHPOS_SQUAT 0
#define CROUCHPOS_DUCK  1
#define CROUCHPOS_STAND 2

#define CULLMODE_NONE  1
#define CULLMODE_FRONT 2
#define CULLMODE_BACK  3

#define DAMAGETYPE_0 0
#define DAMAGETYPE_1 1
#define DAMAGETYPE_2 2
#define DAMAGETYPE_3 3
#define DAMAGETYPE_4 4
#define DAMAGETYPE_5 5
#define DAMAGETYPE_6 6
#define DAMAGETYPE_7 7

#define DESCRIPTION_BRIEFING       0x00
#define DESCRIPTION_MPCONFIG       0x01
#define DESCRIPTION_MPCHALLENGE    0x02
#define DESCRIPTION_CHRBIO         0x03
#define DESCRIPTION_MISCBIO        0x04
#define DESCRIPTION_DEVICETRAINING 0x05
#define DESCRIPTION_FRWEAPON       0x06
#define DESCRIPTION_HANGARBIO      0x07
#define DESCRIPTION_HOLOTRAINING   0x08
#define DESCRIPTION_HOLOTIP1       0x09
#define DESCRIPTION_HOLOTIP2       0x0a
#define DESCRIPTION_DEVICETIP1     0x0b
#define DESCRIPTION_DEVICETIP2     0x0c

#define DEVICE_NIGHTVISION 0x01
#define DEVICE_XRAYSCANNER 0x02
#define DEVICE_EYESPY      0x04
#define DEVICE_IRSCANNER   0x08
#define DEVICE_RTRACKER    0x10
#define DEVICE_SUICIDEPILL 0x20
#define DEVICE_CLOAKDEVICE 0x40
#define DEVICE_CLOAKRCP120 0x80

#define DEVICESTATE_UNEQUIPPED -1
#define DEVICESTATE_INACTIVE   0
#define DEVICESTATE_ACTIVE     1

#define DIFF_A  0x00
#define DIFF_SA 0x01
#define DIFF_PA 0x02
#define DIFF_PD 0x03

// These difficulties are for briefings, which can be bitwise ORed
#define DIFFBIT_A  0x01
#define DIFFBIT_SA 0x02
#define DIFFBIT_PA 0x04
#define DIFFBIT_PD 0x08

#define DOORFLAG_0001            0x0001
#define DOORFLAG_WINDOWED        0x0002
#define DOORFLAG_0004            0x0004
#define DOORFLAG_FLIP            0x0008
#define DOORFLAG_AUTOMATIC       0x0010
#define DOORFLAG_0020            0x0020
#define DOORFLAG_ROTATEDPAD      0x0040
#define DOORFLAG_0080            0x0080
#define DOORFLAG_0100            0x0100
#define DOORFLAG_LONGRANGE       0x0200
#define DOORFLAG_DAMAGEONCONTACT 0x0400 // Lasers
#define DOORFLAG_UNBLOCKABLEOPEN 0x0800 // Skip collision checks when opening
#define DOORFLAG_4000            0x4000 // Two Investigation vertical doors after lasers

#define DOORMODE_IDLE    0
#define DOORMODE_OPENING 1
#define DOORMODE_CLOSING 2
#define DOORMODE_WAITING 3 // Waiting for sibling door to close. Eg. Dam gates in GE

// For AI commands - bitwise ORable
#define DOORSTATE_CLOSED  1
#define DOORSTATE_OPEN    2
#define DOORSTATE_CLOSING 4
#define DOORSTATE_OPENING 8

#define DOORTYPE_SLIDING    0
#define DOORTYPE_FLEXI1     1  // GE only - Bunker flexi door
#define DOORTYPE_FLEXI2     2
#define DOORTYPE_FLEXI3     3
#define DOORTYPE_VERTICAL   4
#define DOORTYPE_SWINGING   5
#define DOORTYPE_EYE        6  // GE only - Caverns
#define DOORTYPE_IRIS       7  // GE only - Caverns
#define DOORTYPE_FALLAWAY   8  // GE only - Surface grate and Train floor panel
#define DOORTYPE_AZTECCHAIR 9  // GE only
#define DOORTYPE_HULL       10 // Attack Ship windows
#define DOORTYPE_LASER      11

#define DRCAROLLIMAGE_EYESDEFAULT  0
#define DRCAROLLIMAGE_HEARTRATE    1
#define DRCAROLLIMAGE_EYESFROWNING 2
#define DRCAROLLIMAGE_X            3
#define DRCAROLLIMAGE_STATIC       4
#define DRCAROLLIMAGE_BINARY       5
#define DRCAROLLIMAGE_EYESSLEEPY   6
#define DRCAROLLIMAGE_RANDOM25     7
#define DRCAROLLIMAGE_RANDOM       8

#define DROPTYPE_DEFAULT      1
#define DROPTYPE_SURRENDER    2
#define DROPTYPE_THROWGRENADE 3
#define DROPTYPE_HAT          4
#define DROPTYPE_5            5
#define DROPTYPE_OWNERREAP    6

#define DYNTEXTYPE_RIVER       1
#define DYNTEXTYPE_ARROWS      2
#define DYNTEXTYPE_TELEPORTAL  3
#define DYNTEXTYPE_MONITOR     4
#define DYNTEXTYPE_OCEAN       5
#define DYNTEXTYPE_POWERJUICE  6
#define DYNTEXTYPE_POWERRING   7

// Might be the same flags as PROJECTILEFLAG
#define EMBEDMENTFLAG_FREE 0x00000001

#define EJECTSTATE_INACTIVE 0
#define EJECTSTATE_INIT     1
#define EJECTSTATE_AIRBORNE 2
#define EJECTSTATE_FINISHED 3

#define EJECTTYPE_GUN        0
#define EJECTTYPE_GRENADEPIN 1
#define EJECTTYPE_TRANQCASE  2

#define EXPLOSIONTYPE_NONE          0
#define EXPLOSIONTYPE_BULLETHOLE    1
#define EXPLOSIONTYPE_EYESPY        2
#define EXPLOSIONTYPE_LAPTOP        3
#define EXPLOSIONTYPE_A51TABLE      4
#define EXPLOSIONTYPE_FRTARGET      5
#define EXPLOSIONTYPE_6             6
#define EXPLOSIONTYPE_7             7
#define EXPLOSIONTYPE_8             8
#define EXPLOSIONTYPE_9             9
#define EXPLOSIONTYPE_11            11
#define EXPLOSIONTYPE_12            12
#define EXPLOSIONTYPE_ROCKET        13
#define EXPLOSIONTYPE_GASBARREL     14
#define EXPLOSIONTYPE_16            16
#define EXPLOSIONTYPE_HUGE17        17
#define EXPLOSIONTYPE_BONDEXPLODE   18
#define EXPLOSIONTYPE_SDGRENADE     21
#define EXPLOSIONTYPE_PHOENIX       22
#define EXPLOSIONTYPE_DRAGONBOMBSPY 23 // Used by both Dragon and bombspy
#define EXPLOSIONTYPE_24            24
#define EXPLOSIONTYPE_HUGE25        25

#define EYESPYHIT_NONE   0
#define EYESPYHIT_BG     1
#define EYESPYHIT_CHR    2
#define EYESPYHIT_DOOR   3
#define EYESPYHIT_DAMAGE 4
#define EYESPYHIT_OBJ    5

#define EYESPYMODE_CAMSPY  0
#define EYESPYMODE_DRUGSPY 1
#define EYESPYMODE_BOMBSPY 2

#define FILEERROR_NOPAK            0
#define FILEERROR_SAVEFAILED       1
#define FILEERROR_LOADFAILED       2
#define FILEERROR_DELETEFAILED     3
#define FILEERROR_OUTOFMEMORY      4
#define FILEERROR_ALREADYLOADED    5
#define FILEERROR_PAKREMOVED       6
#define FILEERROR_PAKDAMAGED       7
#define FILEERROR_DELETENOTEFAILED 8

#define FILELOADMETHOD_EXTRAMEM 0x11
#define FILELOADMETHOD_DEFAULT  0x22

#define FILEOP_IS_LOAD(op) (op >= 100)
#define FILEOP_IS_SAVE(op) (op < 100)

#define FILEOP_SAVE_GAME_000  0 // from endscreens
#define FILEOP_SAVE_GAME_001  1 // from closing options dialog
#define FILEOP_SAVE_GAME_002  2
#define FILEOP_SAVE_MPPLAYER  3
#define FILEOP_SAVE_MPSETUP   4
#define FILEOP_005            5 // Perfect head?
#define FILEOP_WRITE_GAME     6
#define FILEOP_WRITE_MPSETUP  7
#define FILEOP_WRITE_MPPLAYER 8
#define FILEOP_LOAD_GAME      100
#define FILEOP_LOAD_MPPLAYER  101
#define FILEOP_LOAD_MPSETUP   102
#define FILEOP_103            103 // Perfect head?
#define FILEOP_READ_GAME      104
#define FILEOP_READ_MPSETUP   105
#define FILEOP_READ_MPPLAYER  106

#define FILESTATE_UNSELECTED    0
#define FILESTATE_SELECTED      1
#define FILESTATE_CHANGINGAGENT 2

#define FILETYPE_GAME     0
#define FILETYPE_MPSETUP  1
#define FILETYPE_MPPLAYER 2
#define FILETYPE_CAMERA   3

#define FINDPROPCONTEXT_QUERY 0
#define FINDPROPCONTEXT_SHOOT 2

#define FLOORTYPE_DEFAULT 0
#define FLOORTYPE_WOOD    1
#define FLOORTYPE_STONE   2
#define FLOORTYPE_CARPET  3
#define FLOORTYPE_METAL   4
#define FLOORTYPE_MUD     5
#define FLOORTYPE_WATER   6
#define FLOORTYPE_DIRT    7
#define FLOORTYPE_SNOW    8

#define FRCMD_ADDTARGET           0x00
#define FRCMD_SETMAXACTIVETARGETS 0x01
#define FRCMD_SETGOALSCORE        0x02
#define FRCMD_SETTIMELIMIT        0x03
#define FRCMD_SETAMMOLIMIT        0x04
#define FRCMD_SETEXTRASPEED       0x05
#define FRCMD_SETGOALACCURACY     0x06
#define FRCMD_SETGOALTARGETS      0x07
#define FRCMD_SETHELPSCRIPT       0x08
#define FRCMD_SETGRENADELIMIT     0x09
#define FRCMD_SETPADINDEXOFFSET   0x0a
#define FRCMD_SETSCOREMULTIPLIER  0x0b
#define FRCMD_GOTOPAD             0x0c
#define FRCMD_RESTART             0x0d
#define FRCMD_WAITSECONDS         0x0e
#define FRCMD_ROTATE              0x0f
#define FRCMD_HUDMSG              0x10
#define FRCMD_WAITUNTILSHOOT      0x11
#define FRCMD_HELPWAITSECONDS     0x12
#define FRCMD_END                 0x13
#define FRCMD_IFBRONZE            0xfb
#define FRCMD_IFSILVER            0xfc
#define FRCMD_IFGOLD              0xfd
#define FRCMD_START               0xfe

#define FRDIFFICULTY_BRONZE 0
#define FRDIFFICULTY_SILVER 1
#define FRDIFFICULTY_GOLD   2

#define FRFAILREASON_NOTFAILED         0
#define FRFAILREASON_OUTOFAMMO         1
#define FRFAILREASON_TIMEOVER          2
#define FRFAILREASON_SCOREUNATTAINABLE 3
#define FRFAILREASON_INACCURATE        4

#define FRMENUTYPE_WEAPONLIST 0
#define FRMENUTYPE_DETAILS    1
#define FRMENUTYPE_FAILED     2
#define FRMENUTYPE_COMPLETED  3

#define FRROTATE_90R  0
#define FRROTATE_180R 1
#define FRROTATE_90L  2
#define FRROTATE_180L 3

#define FRTARGETFLAG_BRONZE                 0x01
#define FRTARGETFLAG_SILVER                 0x02
#define FRTARGETFLAG_GOLD                   0x04
#define FRTARGETFLAG_ROTATEONCLOAK          0x08
#define FRTARGETFLAG_SPAWNFACINGAWAY        0x10
#define FRTARGETFLAG_ONEHITEXPLODE          0x20
#define FRTARGETFLAG_FARSIGHTAUTOTARGETABLE 0x40
#define FRTARGETFLAG_TMPINVINCIBLE          0x80

#define FRZONE_RING3    1
#define FRZONE_RING2    2
#define FRZONE_RING1    5
#define FRZONE_BULLSEYE 10
#define FRZONE_EXPLODE  11

// Weapon functions
#define FUNC_PRIMARY   0
#define FUNC_SECONDARY 1
#define FUNC_2         2
#define FUNC_POISON    3 // Internal function for delivering knife poison periodically

#define FUNCFLAG_00000001               0x00000001
#define FUNCFLAG_BURST3                 0x00000002
#define FUNCFLAG_BURST50                0x00000020 // automatics only
#define FUNCFLAG_NOAUTOAIM              0x00000040
#define FUNCFLAG_STICKTOWALL            0x00000100
#define FUNCFLAG_MAKEDIZZY              0x00000200
#define FUNCFLAG_DISARM                 0x00000400
#define FUNCFLAG_FLYBYWIRE              0x00000800
#define FUNCFLAG_BURST2                 0x00001000
#define FUNCFLAG_NOMUZZLEFLASH          0x00002000
#define FUNCFLAG_EXPLOSIVESHELLS        0x00004000
#define FUNCFLAG_BLUNTIMPACT            0x00008000
#define FUNCFLAG_NOSTUN                 0x00010000
#define FUNCFLAG_BURST5                 0x00020000
#define FUNCFLAG_DISCARDWEAPON          0x00040000 // Dragon and Laptop throw
#define FUNCFLAG_THREATDETECTOR         0x00080000
#define FUNCFLAG_AUTOSWITCHUNSELECTABLE 0x00100000
#define FUNCFLAG_PSYCHOSIS              0x00200000
#define FUNCFLAG_00400000               0x00400000 // punch, disarm and pistol whip
#define FUNCFLAG_CALCULATETRAJECTORY    0x00800000 // throwables will land on crosshair
#define FUNCFLAG_PROJECTILE_POWERED     0x08000000
#define FUNCFLAG_10000000               0x10000000 // grenade launchers
#define FUNCFLAG_20000000               0x20000000 // explosives related
#define FUNCFLAG_HOMINGROCKET           0x40000000
#define FUNCFLAG_PROJECTILE_LIGHTWEIGHT 0x80000000

#define GAILIST_IDLE                   0x0000
#define GAILIST_UNALERTED_0001         0x0001
#define GAILIST_UNALERTED_0002         0x0002
#define GAILIST_STOP_UNALERTED         0x0003 // unused
#define GAILIST_UNALERTED_0004         0x0004
#define GAILIST_END_CINEMA             0x0005
#define GAILIST_UNALERTED              0x0006
#define GAILIST_ALERTED                0x0007
#define GAILIST_WAKEUP                 0x0008
#define GAILIST_IDLE_0009              0x0009
#define GAILIST_BUSY                   0x000a
#define GAILIST_CHOOSE_TARGET          0x000b
#define GAILIST_COMBAT_WITH_TARGET     0x000c
#define GAILIST_INIT_COMBAT            0x000d // unused
#define GAILIST_SEE_THEN_ATTACK        0x000e // unused
#define GAILIST_HAND_COMBAT            0x000f
#define GAILIST_CIVILIAN_SAY_COMMENT   0x0010
#define GAILIST_FLEE_FROM_GRENADE      0x0011
#define GAILIST_INIT_DEFAULT_BUDDY     0x0012
#define GAILIST_INIT_PUGILIST_BUDDY    0x0013
#define GAILIST_BUDDY_MAIN             0x0014
#define GAILIST_BUDDY_STEALTH          0x0015
#define GAILIST_SHOW_OBJ_FAILED_MSG    (VERSION >= VERSION_NTSC_1_0 ? 0x0016 : 0x0015)
#define GAILIST_REBUILD_GROUPS         (VERSION >= VERSION_NTSC_1_0 ? 0x0017 : 0x0016)
#define GAILIST_DO_BORED_ANIMATION     (VERSION >= VERSION_NTSC_1_0 ? 0x0018 : 0x0017)
#define GAILIST_DO_SITTING_ANIMATION   (VERSION >= VERSION_NTSC_1_0 ? 0x0019 : 0x0018)
#define GAILIST_PATROLLER_DIS_TALKING  (VERSION >= VERSION_NTSC_1_0 ? 0x001a : 0x0019)
#define GAILIST_OBSERVE_CAMSPY         (VERSION >= VERSION_NTSC_1_0 ? 0x001b : 0x001a)
#define GAILIST_SURPRISED              (VERSION >= VERSION_NTSC_1_0 ? 0x001c : 0x001b)
#define GAILIST_SEARCH_FOR_PLAYER      (VERSION >= VERSION_NTSC_1_0 ? 0x001d : 0x001c)
#define GAILIST_LOOK_AROUND            (VERSION >= VERSION_NTSC_1_0 ? 0x001e : 0x001d)
#define GAILIST_RELATED_TO_SPAWNING    (VERSION >= VERSION_NTSC_1_0 ? 0x001f : 0x001e)
#define GAILIST_BUDDY_WARP             (VERSION >= VERSION_NTSC_1_0 ? 0x0020 : 0x001f)
#define GAILIST_STOP_AND_IDLE          (VERSION >= VERSION_NTSC_1_0 ? 0x0021 : 0x0020)
#define GAILIST_COMMENT_ON_PLAYER_DEAD (VERSION >= VERSION_NTSC_1_0 ? 0x0022 : 0x0021) // unused
#define GAILIST_DODGE                  (VERSION >= VERSION_NTSC_1_0 ? 0x0023 : 0x0022)
#define GAILIST_FOLLOW_BOND            (VERSION >= VERSION_NTSC_1_0 ? 0x0024 : 0x0023)
#define GAILIST_POINTLESS              (VERSION >= VERSION_NTSC_1_0 ? 0x0025 : 0x0024) // unused
#define GAILIST_INIT_PSYCHOSIS         (VERSION >= VERSION_NTSC_1_0 ? 0x0026 : 0x0025)
#define GAILIST_PSYCHOSISED            (VERSION >= VERSION_NTSC_1_0 ? 0x0027 : 0x0026)
#define GAILIST_AIBOT_DEAD             (VERSION >= VERSION_NTSC_1_0 ? 0x0028 : 0x0027) // unused
#define GAILIST_AIBOT_INIT             (VERSION >= VERSION_NTSC_1_0 ? 0x0029 : 0x0028) // unused
#define GAILIST_AIBOT_MAIN             (VERSION >= VERSION_NTSC_1_0 ? 0x002a : 0x0029) // unused
#define GAILIST_AVOID                  (VERSION >= VERSION_NTSC_1_0 ? 0x002b : 0x002a) // unused
#define GAILIST_INIT_SEARCH            (VERSION >= VERSION_NTSC_1_0 ? 0x002c : 0x002b) // unused
#define GAILIST_INVINCIBLE_AND_IDLE    (VERSION >= VERSION_NTSC_1_0 ? 0x002d : 0x002c)

#define GAMEFILEFLAG_P1_FORWARDPITCH      0x00
#define GAMEFILEFLAG_P1_AUTOAIM           0x01
#define GAMEFILEFLAG_P1_AIMCONTROL        0x02
#define GAMEFILEFLAG_P1_SIGHTONSCREEN     0x03
#define GAMEFILEFLAG_P1_LOOKAHEAD         0x04
#define GAMEFILEFLAG_P1_AMMOONSCREEN      0x05
#define GAMEFILEFLAG_SCREENSIZE_WIDE      0x06
#define GAMEFILEFLAG_SCREENRATIO          0x07
#define GAMEFILEFLAG_SCREENSIZE_CINEMA    0x08
#define GAMEFILEFLAG_P1_HEADROLL          0x09
#define GAMEFILEFLAG_P1_SHOWGUNFUNCTION   0x0a
#define GAMEFILEFLAG_INGAMESUBTITLES      0x0b
#define GAMEFILEFLAG_P2_FORWARDPITCH      0x0c
#define GAMEFILEFLAG_P2_AUTOAIM           0x0d
#define GAMEFILEFLAG_P2_AIMCONTROL        0x0e
#define GAMEFILEFLAG_P2_SIGHTONSCREEN     0x0f
#define GAMEFILEFLAG_P2_LOOKAHEAD         0x10
#define GAMEFILEFLAG_P2_AMMOONSCREEN      0x11
#define GAMEFILEFLAG_P2_HEADROLL          0x12
#define GAMEFILEFLAG_P2_SHOWGUNFUNCTION   0x13
#define GAMEFILEFLAG_CUTSCENESUBTITLES    0x14
#define GAMEFILEFLAG_P1_ALWAYSSHOWTARGET  0x15
#define GAMEFILEFLAG_P2_ALWAYSSHOWTARGET  0x16
#define GAMEFILEFLAG_P1_SHOWZOOMRANGE     0x17
#define GAMEFILEFLAG_P2_SHOWZOOMRANGE     0x18
#define GAMEFILEFLAG_SCREENSPLIT          0x19
#define GAMEFILEFLAG_P1_SHOWMISSIONTIME   0x1a
#define GAMEFILEFLAG_P2_SHOWMISSIONTIME   0x1b
#define GAMEFILEFLAG_COOPRADARON          0x1c
#define GAMEFILEFLAG_COOPFRIENDLYFIRE     0x1d
#define GAMEFILEFLAG_ANTIRADARON          0x1e
#define GAMEFILEFLAG_ANTIPLAYERNUM        0x1f
#define GAMEFILEFLAG_P1_PAINTBALL         0x20
#define GAMEFILEFLAG_P2_PAINTBALL         0x21
#define GAMEFILEFLAG_HIRES                0x22
#define GAMEFILEFLAG_USED_TRANSFERPAK     0x23
#define GAMEFILEFLAG_CI_TOUR_DONE         0x24
#define GAMEFILEFLAG_CI_HOLO7_DONE        0x29
#define GAMEFILEFLAG_CI_HOLO6_DONE        0x2a
#define GAMEFILEFLAG_CI_HOLO5_DONE        0x2b
#define GAMEFILEFLAG_CI_HOLO4_DONE        0x2c
#define GAMEFILEFLAG_CI_HOLO3_DONE        0x2d
#define GAMEFILEFLAG_CI_HOLO2_DONE        0x2e
#define GAMEFILEFLAG_CI_HOLO1_DONE        0x2f
#define GAMEFILEFLAG_CI_CLOAK_DONE        0x30
#define GAMEFILEFLAG_CI_DISGUISE_DONE     0x31
#define GAMEFILEFLAG_CI_XRAY_DONE         0x32
#define GAMEFILEFLAG_CI_IR_DONE           0x33
#define GAMEFILEFLAG_CI_RTRACKER_DONE     0x34
#define GAMEFILEFLAG_CI_DOORDECODER_DONE  0x35
#define GAMEFILEFLAG_CI_NIGHTVISION_DONE  0x36
#define GAMEFILEFLAG_CI_CAMSPY_DONE       0x37
#define GAMEFILEFLAG_CI_ECMMINE_DONE      0x38
#define GAMEFILEFLAG_CI_UPLINK_DONE       0x39
#define GAMEFILEFLAG_CI_TOUR_STARTED      0x3a
#define GAMEFILEFLAG_CRASHSITE_BIKE       0x3b
#define GAMEFILEFLAG_DEFENSE_JON          0x3c
#define GAMEFILEFLAG_AF1_ENTRY            0x3d
#define GAMEFILEFLAG_RESCUE_MECHANIC_DEAD 0x3e
#define GAMEFILEFLAG_G5_MINE              0x3f // true if thrown on top door
#define GAMEFILEFLAG_LANGFILTERON         0x40
#define GAMEFILEFLAG_FOUNDTIMEDMINE       0x41
#define GAMEFILEFLAG_FOUNDPROXYMINE       0x42
#define GAMEFILEFLAG_FOUNDREMOTEMINE      0x43
#define GAMEFILEFLAG_LANGBIT1             0x44
#define GAMEFILEFLAG_LANGBIT2             0x45
#define GAMEFILEFLAG_LANGBIT3             0x46
#define GAMEFILEFLAG_HOWTO_HOVERCRATE     0x47
#define GAMEFILEFLAG_HOWTO_HOVERBIKE      0x48
#define GAMEFILEFLAG_HOWTO_DOORS          0x49
#define GAMEFILEFLAG_HOWTO_ELEVATORS      0x4a
#define GAMEFILEFLAG_HOWTO_TERMINALS      0x4b
#define GAMEFILEFLAG_4C                   0x4c
#define GAMEFILEFLAG_4D                   0x4d
#define GAMEFILEFLAG_4E                   0x4e

#define GBGAME_OTHER 0
#define GBGAME_PD    2

#define GEOFLAG_FLOOR1            0x0001
#define GEOFLAG_FLOOR2            0x0002
#define GEOFLAG_WALL              0x0004
#define GEOFLAG_BLOCK_SIGHT       0x0008
#define GEOFLAG_BLOCK_SHOOT       0x0010
#define GEOFLAG_LIFTFLOOR         0x0020
#define GEOFLAG_LADDER            0x0040
#define GEOFLAG_RAMPWALL          0x0080
#define GEOFLAG_SLOPE             0x0100
#define GEOFLAG_UNDERWATER        0x0200
#define GEOFLAG_0400              0x0400 // Unused
#define GEOFLAG_AIBOTCROUCH       0x0800
#define GEOFLAG_AIBOTDUCK         0x1000
#define GEOFLAG_STEP              0x2000 // Ascend regardless of steepness
#define GEOFLAG_DIE               0x4000
#define GEOFLAG_LADDER_PLAYERONLY 0x8000 // Used for most ledges in Chicago, but not near drain pickup

#define GEOTYPE_TILE_I 0 // Tiles with integer vertices - used for BG
#define GEOTYPE_TILE_F 1 // Tiles with float vertices - used for lifts
#define GEOTYPE_BLOCK  2 // Most objects - multiple x/z vertices, and a single ymin and ymax
#define GEOTYPE_CYL    3 // Cylinder - used by chrs and the hoverbike

#define GOPOSFLAG_WALK         0x00
#define GOPOSFLAG_JOG          0x01
#define GOPOSFLAG_RUN          0x02
#define GOPOSFLAG_INIT         0x04
#define GOPOSFLAG_FORPATHSTART 0x08
#define GOPOSFLAG_WAITING      0x20 // for a door to open, or at/in lift
#define GOPOSFLAG_CROUCH       0x40
#define GOPOSFLAG_DUCK         0x80

#define GOPOSMASK_SPEED        (GOPOSFLAG_WALK | GOPOSFLAG_JOG | GOPOSFLAG_RUN)

#define GUNAMMOREASON_OPTION     0x01
#define GUNAMMOREASON_NOCONTROL  0x02

#define GUNCMD_END               0
#define GUNCMD_SHOWPART          1
#define GUNCMD_HIDEPART          2
#define GUNCMD_WAITFORZRELEASED  3
#define GUNCMD_WAITTIME          4
#define GUNCMD_PLAYSOUND         5
#define GUNCMD_INCLUDE           6
#define GUNCMD_RANDOM            7
#define GUNCMD_REPEATUNTILFULL   8
#define GUNCMD_POPOUTSACKOFPILLS 9
#define GUNCMD_PLAYANIMATION     10
#define GUNCMD_SETSOUNDSPEED     11

#define GUNMEMOWNER_BONDGUN  0
#define GUNMEMOWNER_INVMENU  1
#define GUNMEMOWNER_CHRBODY  2
#define GUNMEMOWNER_3        3
#define GUNMEMOWNER_CHANGING 10
#define GUNMEMOWNER_FREE     11

#define GUNSIGHTREASON_1         0x01
#define GUNSIGHTREASON_NOTAIMING 0x02
#define GUNSIGHTREASON_NOCONTROL 0x04
#define GUNSIGHTREASON_DAMAGE    0x10

#define GUNVISCMD_END              0
#define GUNVISCMD_ALWAYSTRUE       1
#define GUNVISCMD_CHECKUPGRADE     4
#define GUNVISCMD_CHECKINLEFTHAND  5
#define GUNVISCMD_CHECKINRIGHTHAND 6

#define GUNVISOP_IFTRUE_SETVISIBLE 0
#define GUNVISOP_IFTRUE_SETHIDDEN  1
#define GUNVISOP_SETVISIBILITY     3

#define HAND_RIGHT 0
#define HAND_LEFT  1

#define HANDANIMMODE_IDLE 0
#define HANDANIMMODE_BUSY 2

#define HANDATTACKTYPE_SHOOT           1
#define HANDATTACKTYPE_SHOOTPROJECTILE 2
#define HANDATTACKTYPE_THROWPROJECTILE 3
#define HANDATTACKTYPE_MELEE           4
#define HANDATTACKTYPE_DETONATE        5
#define HANDATTACKTYPE_BOOST           6
#define HANDATTACKTYPE_REVERTBOOST     7
#define HANDATTACKTYPE_CROUCH          8
#define HANDATTACKTYPE_RCP120CLOAK     9
#define HANDATTACKTYPE_MELEENOUNCLOAK  10
#define HANDATTACKTYPE_UPLINK          12

#define HANDMODE_NONE   0
#define HANDMODE_ATTACK 1
#define HANDMODE_2      2
#define HANDMODE_6      6
#define HANDMODE_7      7
#define HANDMODE_EQUIP  8
#define HANDMODE_RELOAD 9
#define HANDMODE_11     11
#define HANDMODE_12     12
#define HANDMODE_13     13

#define HANDSTATE_IDLE        0
#define HANDSTATE_RELOAD      1
#define HANDSTATE_2           2
#define HANDSTATE_ATTACKEMPTY 3
#define HANDSTATE_ATTACK      4
#define HANDSTATE_CHANGEGUN   5
#define HANDSTATE_6           6
#define HANDSTATE_CHANGEFUNC  7
#define HANDSTATE_AUTOSWITCH  8

#define HANDSTATEFLAG_00000001 0x00000001
#define HANDSTATEFLAG_00000010 0x00000010
#define HANDSTATEFLAG_00000020 0x00000020
#define HANDSTATEFLAG_00000040 0x00000040
#define HANDSTATEFLAG_00000080 0x00000080

#define HANDSTATEMINOR_ATTACK_MELEE_0           0
#define HANDSTATEMINOR_ATTACK_MELEE_1           1
#define HANDSTATEMINOR_ATTACK_MELEE_2           2
#define HANDSTATEMINOR_ATTACK_MELEE_3           3
#define HANDSTATEMINOR_ATTACK_SHOOT_0           0
#define HANDSTATEMINOR_ATTACK_SHOOT_1           1
#define HANDSTATEMINOR_ATTACK_SHOOT_2           2
#define HANDSTATEMINOR_ATTACK_SPECIAL_START     0
#define HANDSTATEMINOR_ATTACK_SPECIAL_EXECUTE   1
#define HANDSTATEMINOR_ATTACK_SPECIAL_RECOVER   2
#define HANDSTATEMINOR_ATTACK_THROW_0           0
#define HANDSTATEMINOR_ATTACK_THROW_1           1
#define HANDSTATEMINOR_ATTACK_THROW_2           2
#define HANDSTATEMINOR_ATTACK_THROW_GRENADEWAIT 55
#define HANDSTATEMINOR_AUTOSWITCH_UNEQUIP       0
#define HANDSTATEMINOR_AUTOSWITCH_DELETE        1
#define HANDSTATEMINOR_AUTOSWITCH_2             2
#define HANDSTATEMINOR_CHANGEGUN_UNEQUIP        0
#define HANDSTATEMINOR_CHANGEGUN_LOWER          1
#define HANDSTATEMINOR_CHANGEGUN_LOAD           2
#define HANDSTATEMINOR_CHANGEGUN_RAISE          3
#define HANDSTATEMINOR_CHANGEGUN_EQUIP          4
#define HANDSTATEMINOR_RELOAD_MAIN              0
#define HANDSTATEMINOR_RELOAD_LOWER             1
#define HANDSTATEMINOR_RELOAD_SOUND             2
#define HANDSTATEMINOR_RELOAD_RAISE             3
#define HANDSTATEMINOR_RELOAD_WAIT              9

#define HANGARBIO_INSTITUTE      0
#define HANGARBIO_DDTOWER        1
#define HANGARBIO_LABBASEMENT    2
#define HANGARBIO_VILLA          3
#define HANGARBIO_CHICAGO        4
#define HANGARBIO_G5             5
#define HANGARBIO_AREA51         6
#define HANGARBIO_AIRBASE        7
#define HANGARBIO_AIRFORCEONE    8
#define HANGARBIO_CRASHSITE      9
#define HANGARBIO_PELAGIC        10
#define HANGARBIO_DEEPSEA        11
#define HANGARBIO_ATTACKSHIP     12
#define HANGARBIO_SKEDARRUINS    13
#define HANGARBIO_JUMPSHIP       14
#define HANGARBIO_HOVERCRATE     15
#define HANGARBIO_HOVERBIKE      16
#define HANGARBIO_HOVERBOT       17
#define HANGARBIO_HOVERCOPTER    18
#define HANGARBIO_G5ROBOT        19
#define HANGARBIO_A51INTERCEPTOR 20
#define HANGARBIO_MAIANVESSEL    21
#define HANGARBIO_SKEDARSHUTTLE  22

#define HATTYPE_2     2
#define HATTYPE_METAL 3
#define HATTYPE_CLOTH 5

#define HEADANIM_RESTING 0
#define HEADANIM_MOVING  1

#define HEADBODYTYPE_DEFAULT     0
#define HEADBODYTYPE_FEMALE      1
#define HEADBODYTYPE_FEMALEGUARD 2
#define HEADBODYTYPE_MAIAN       3
#define HEADBODYTYPE_CASS        4
#define HEADBODYTYPE_MRBLONDE    5

#define HEALTHSHOWMODE_HIDDEN   0 // health bar not visible
#define HEALTHSHOWMODE_OPENING  1 // height expanding
#define HEALTHSHOWMODE_PREVIOUS 2 // full height, showing previous health amount
#define HEALTHSHOWMODE_UPDATING 3 // full height, transitioning from previous amount to current amount
#define HEALTHSHOWMODE_CURRENT  4 // full height, showing current amount
#define HEALTHSHOWMODE_CLOSING  5 // height collapsing

#define HITPART_LFOOT       1
#define HITPART_LSHIN       2
#define HITPART_LTHIGH      3
#define HITPART_RFOOT       4
#define HITPART_RSHIN       5
#define HITPART_RTHIGH      6
#define HITPART_PELVIS      7
#define HITPART_HEAD        8
#define HITPART_LHAND       9
#define HITPART_LFOREARM    10
#define HITPART_LBICEP      11
#define HITPART_RHAND       12
#define HITPART_RFOREARM    13
#define HITPART_RBICEP      14
#define HITPART_TORSO       15
#define HITPART_TAIL        16
#define HITPART_GUN         100
#define HITPART_HAT         110
#define HITPART_GENERAL     200
#define HITPART_GENERALHALF 201

#define HOVFLAG_FIRSTTICK 1

#define HOVTYPE_BED   0
#define HOVTYPE_BIKE  1
#define HOVTYPE_CRATE 2
#define HOVTYPE_3     3 // unused
#define HOVTYPE_4     4 // unused

#define HUDHALIGN_RIGHT  0
#define HUDHALIGN_LEFT   1
#define HUDHALIGN_MIDDLE 2

#define HUDMSGALIGN_SCREENLEFT    0
#define HUDMSGALIGN_LEFT          1
#define HUDMSGALIGN_RIGHT         2
#define HUDMSGALIGN_XMIDDLE       3
#define HUDMSGALIGN_SCREENTOP     4
#define HUDMSGALIGN_TOP           5
#define HUDMSGALIGN_BOTTOM        6
#define HUDMSGALIGN_YMIDDLE       7
#define HUDMSGALIGN_BELOWVIEWPORT 8

#define HUDMSGFLAG_ONLYIFALIVE 0x01
#define HUDMSGFLAG_FORCEOFF    0x02 // for subtitles - turn off message immediately rather than when audio finishes
#define HUDMSGFLAG_NOCHANNEL   0x04 // not linked to audio
#define HUDMSGFLAG_ALLOWDUPES  0x08
#define HUDMSGFLAG_DELAY       0x10 // wait minimum 3 frames before showing
#define HUDMSGFLAG_NOWRAP      0x20

#define HUDMSGREASON_NOCONTROL 0x00000002

#define HUDMSGSTATE_FREE             0
#define HUDMSGSTATE_QUEUED           1
#define HUDMSGSTATE_CHOOSETRANSITION 2
#define HUDMSGSTATE_FADINGIN         3
#define HUDMSGSTATE_ONSCREEN         4
#define HUDMSGSTATE_FADINGOUT        5

#define HUDMSGTYPE_DEFAULT           0
#define HUDMSGTYPE_OBJECTIVECOMPLETE 1
#define HUDMSGTYPE_OBJECTIVEFAILED   2
#define HUDMSGTYPE_3                 3
#define HUDMSGTYPE_4                 4
#define HUDMSGTYPE_5                 5
#define HUDMSGTYPE_INGAMESUBTITLE    6
#define HUDMSGTYPE_7                 7
#define HUDMSGTYPE_8                 8
#define HUDMSGTYPE_MPSCENARIO        9
#define HUDMSGTYPE_TRAINING          10
#define HUDMSGTYPE_CUTSCENESUBTITLE  11

#define HUDVALIGN_BOTTOM 0
#define HUDVALIGN_TOP    1
#define HUDVALIGN_MIDDLE 2

#define IDLEACTION_STANDING        0x01
#define IDLEACTION_SITTING_TYPING  0x02
#define IDLEACTION_SITTING_DORMANT 0x03
#define IDLEACTION_OPERATING       0x04
#define IDLEACTION_OPERATING_PAD   0x05

#define INTROCMD_SPAWN        0
#define INTROCMD_WEAPON       1
#define INTROCMD_AMMO         2
#define INTROCMD_3            3
#define INTROCMD_4            4
#define INTROCMD_OUTFIT       5
#define INTROCMD_6            6
#define INTROCMD_WATCHTIME    7
#define INTROCMD_CREDITOFFSET 8
#define INTROCMD_CASE         9
#define INTROCMD_CASERESPAWN  10
#define INTROCMD_HILL         11
#define INTROCMD_END          12

#define INVENTORYFUNCTYPE_NONE             0x0000
#define INVENTORYFUNCTYPE_SHOOT            0x0001
#define INVENTORYFUNCTYPE_SHOOT_SINGLE     0x0001
#define INVENTORYFUNCTYPE_SHOOT_AUTOMATIC  0x0101
#define INVENTORYFUNCTYPE_SHOOT_PROJECTILE 0x0201
#define INVENTORYFUNCTYPE_0200             0x0200
#define INVENTORYFUNCTYPE_THROW            0x0002
#define INVENTORYFUNCTYPE_MELEE            0x0003
#define INVENTORYFUNCTYPE_SPECIAL          0x0004
#define INVENTORYFUNCTYPE_DEVICE           0x0005

#define INVITEMTYPE_WEAP 1
#define INVITEMTYPE_PROP 2
#define INVITEMTYPE_DUAL 3

#define LIFTACTION_NOTUSINGLIFT   0
#define LIFTACTION_WAITINGFORLIFT 1
#define LIFTACTION_ONLIFT         2
#define LIFTACTION_WAITINGONLIFT  3

#define LANGUAGE_NTSC_EN 0
#define LANGUAGE_NTSC_JP 1

#define LANGUAGE_PAL_EN  0
#define LANGUAGE_PAL_FR  1
#define LANGUAGE_PAL_DE  2
#define LANGUAGE_PAL_IT  3
#define LANGUAGE_PAL_ES  4

#define LIGHTOP_NONE       0
#define LIGHTOP_SET        1
#define LIGHTOP_SETRANDOM  2
#define LIGHTOP_TRANSITION 3
#define LIGHTOP_SINELOOP   4
#define LIGHTOP_HIGHLIGHT  5
#define LIGHTOP_TURNON     6
#define LIGHTOP_TURNOFF    7

#define LOADTYPE_NONE  0
#define LOADTYPE_BG    1
#define LOADTYPE_TILES 2
#define LOADTYPE_LANG  3
#define LOADTYPE_SETUP 4
#define LOADTYPE_PADS  5
#define LOADTYPE_MODEL 6
#define LOADTYPE_GUN   7

// These actions are assigned to chr->myaction
#define MA_NONE            0
#define MA_NORMAL          1
#define MA_COVERWAIT       2
#define MA_GRENADEWAIT     3
#define MA_WAITING         4
#define MA_COVERGOTO       5
#define MA_COVERBREAK      6
#define MA_COVERSEEN       7
#define MA_FLANKLEFT       8
#define MA_FLANKRIGHT      9
#define MA_DODGE           10
#define MA_GRENADE         11
#define MA_WAITSEEN        12
#define MA_WITHDRAW        13
#define MA_SHOOTING        14
#define MA_SYNCSHOOT       15
#define MA_WAITTIMEOUT     16
#define MA_COVERTIMEOUT    17
#define MA_TRACKING        18
#define MA_RETREAT         19
#define MA_SURRENDER       20
#define MA_TALKING         21
#define MA_LISTENING       22
#define MA_GOTOALARM       23
#define MA_BOTFRIENDFOLLOW 24
#define MA_BOTHIDE         25
#define MA_BOTPATH         26
#define MA_BOTINJURED      27
#define MA_BOTNORMAL       28
#define MA_BOTSHOOTING     29
#define MA_DRUGGED         30
#define MA_PANIC           31
#define MA_RUNFROMGRENADE  32
#define MA_UNARMEDATTACK   33
#define MA_SKJUMP_START    34
#define MA_SKJUMP_AIR      35
#define MA_SKJUMP_LAND     36
#define MA_SKJUMP_LANDLOOP 37
#define MA_SKJUMP_SHOT     38
#define MA_AIBOTDEADLIST   39
#define MA_AIBOTINIT       40
#define MA_AIBOTMAINLOOP   41
#define MA_AIBOTGETITEM    42
#define MA_AIBOTGOTOPOS    43
#define MA_AIBOTGOTOPROP   44
#define MA_AIBOTRUNAWAY    45
#define MA_AIBOTDOWNLOAD   46
#define MA_AIBOTATTACK     47
#define MA_UNUSED1         48
#define MA_UNUSED2         49
#define MA_AIBOTFOLLOW     50
#define MA_AIBOTDEFEND     51
#define MA_FLANKBEST       52
#define MA_FACING          53
#define MA_PUNCHING        54
#define MA_END             55

#define MEDAL_KILLMASTER 0x01
#define MEDAL_HEADSHOT   0x02
#define MEDAL_ACCURACY   0x04
#define MEDAL_SURVIVOR   0x08

#define MEMBANK_ONBOARD   0
#define MEMBANK_EXPANSION 1

#define MEMPOOL_0         0
#define MEMPOOL_STAGE     4 // Cleared on stage load
#define MEMPOOL_5         5
#define MEMPOOL_PERMANENT 6 // Never cleared
#define MEMPOOL_7         7
#define MEMPOOL_8         8

#define MENUBANNER_SEARCHINGFORCAMERA 0
#define MENUBANNER_CALIBRATINGCAMERA  1
#define MENUBANNER_DOWNLOADINGIMAGE   2
#define MENUBANNER_LOADINGIMAGE       3
#define MENUBANNER_SAVINGIMAGE        4
#define MENUBANNER_TRANSFERRINGIMAGE  5
#define MENUBANNER_UPLOADINGSEGMENT   6
#define MENUBANNER_CHECKINGPAK        7
#define MENUBANNER_GETTINGPERFECTHEAD 8
#define MENUBANNER_SAVINGPERFECTHEAD  9
#define MENUBANNER_AUTOADJUST         10

#define MENUBG_BLUR       1
#define MENUBG_BLACK      2 // solid black, unused?
#define MENUBG_FAILURE    3 // solo mission failure endscreen
#define MENUBG_CONEALPHA  4 // combat simulator
#define MENUBG_GRADIENT   5 // blue/red gradient (bootpakmgr)
#define MENUBG_6          6
#define MENUBG_SUCCESS    7 // solo mission success endscreen
#define MENUBG_8          8
#define MENUBG_CONEOPAQUE 9 // 4MB combat simulator

#define MENUDIALOGFLAG_CLOSEONSELECT     0x0001
#define MENUDIALOGFLAG_0002              0x0002
#define MENUDIALOGFLAG_STARTSELECTS      0x0004
#define MENUDIALOGFLAG_DISABLEITEMSCROLL 0x0008
#define MENUDIALOGFLAG_MPLOCKABLE        0x0010
#define MENUDIALOGFLAG_IGNOREBACK        0x0020
#define MENUDIALOGFLAG_SMOOTHSCROLLABLE  0x0040
#define MENUDIALOGFLAG_DISABLEBANNER     0x0080
#define MENUDIALOGFLAG_DISABLETITLEBAR   0x0100
#define MENUDIALOGFLAG_DISABLERESIZE     0x0200
#define MENUDIALOGFLAG_0400              0x0400
#define MENUDIALOGFLAG_DROPOUTONCLOSE    0x0800
#define MENUDIALOGFLAG_1000              0x1000
#ifndef PLATFORM_N64
#define MENUDIALOGFLAG_LITERAL_TEXT      0x2000
#endif

#define MENUDIALOGSTATE_PREOPEN    0
#define MENUDIALOGSTATE_OPENING    1
#define MENUDIALOGSTATE_POPULATING 2
#define MENUDIALOGSTATE_POPULATED  3

#define MENUDIALOGTYPE_DEFAULT 1
#define MENUDIALOGTYPE_DANGER  2
#define MENUDIALOGTYPE_SUCCESS 3
#define MENUDIALOGTYPE_4       4
#define MENUDIALOGTYPE_WHITE   5

#define MENUITEMFLAG_NEWCOLUMN               0x00000001
#define MENUITEMFLAG_00000002                0x00000002
#define MENUITEMFLAG_SELECTABLE_OPENSDIALOG  0x00000004
#define MENUITEMFLAG_SELECTABLE_CLOSESDIALOG 0x00000008
#define MENUITEMFLAG_LESSLEFTPADDING         0x00000010
#define MENUITEMFLAG_SELECTABLE_CENTRE       0x00000020
#define MENUITEMFLAG_LIST_WIDE               0x00000040
#define MENUITEMFLAG_DROPDOWN_BELOW          0x00000080
#define MENUITEMFLAG_LABEL_ALTCOLOUR         0x00000100
#define MENUITEMFLAG_SMALLFONT               0x00000200
#define MENUITEMFLAG_ALWAYSDISABLED          0x00000400
#define MENUITEMFLAG_MARQUEE_FADEBOTHSIDES   0x00000800
#define MENUITEMFLAG_SLIDER_FAST             0x00000800
#define MENUITEMFLAG_ADJUSTWIDTH             0x00001000
#define MENUITEMFLAG_SLIDER_HIDEVALUE        0x00002000
#define MENUITEMFLAG_DARKERBG                0x00004000
#define MENUITEMFLAG_LABEL_HASRIGHTTEXT      0x00008000
#define MENUITEMFLAG_00010000                0x00010000
#define MENUITEMFLAG_LOCKABLEMINOR           0x00020000
#define MENUITEMFLAG_LOCKABLEMAJOR           0x00040000
#define MENUITEMFLAG_MPWEAPONSLOT            0x00080000
#define MENUITEMFLAG_SLIDER_ALTSIZE          0x00100000
#define MENUITEMFLAG_LIST_CUSTOMRENDER       0x00200000
#define MENUITEMFLAG_BIGFONT                 0x00400000
#define MENUITEMFLAG_LIST_AUTOWIDTH          0x00800000
#define MENUITEMFLAG_LABEL_CUSTOMCOLOUR      0x01000000
#define MENUITEMFLAG_LESSHEIGHT              0x02000000
#define MENUITEMFLAG_CAROUSEL_04000000       0x04000000
#define MENUITEMFLAG_LITERAL_TEXT            0x08000000
#define MENUITEMFLAG_SLIDER_WIDE             0x10000000
#define MENUITEMFLAG_SLIDER_DEFERRED         0x20000000
#define MENUITEMFLAG_SLIDER_SLOW             0x80000000

#define MENUITEMTYPE_LABEL       0x01
#define MENUITEMTYPE_LIST        0x02
#define MENUITEMTYPE_03          0x03
#define MENUITEMTYPE_SELECTABLE  0x04
#define MENUITEMTYPE_SCROLLABLE  0x05
#define MENUITEMTYPE_OBJECTIVES  0x06
#define MENUITEMTYPE_07          0x07
#define MENUITEMTYPE_SLIDER      0x08
#define MENUITEMTYPE_CHECKBOX    0x09
#define MENUITEMTYPE_0A          0x0a
#define MENUITEMTYPE_SEPARATOR   0x0b
#define MENUITEMTYPE_DROPDOWN    0x0c
#define MENUITEMTYPE_KEYBOARD    0x0d
#define MENUITEMTYPE_RANKING     0x0e
#define MENUITEMTYPE_PLAYERSTATS 0x0f
#define MENUITEMTYPE_10          0x10
#define MENUITEMTYPE_CAROUSEL    0x11
#define MENUITEMTYPE_MODEL       0x12
#define MENUITEMTYPE_13          0x13
#define MENUITEMTYPE_14          0x14
#define MENUITEMTYPE_METER       0x15
#define MENUITEMTYPE_16          0x16
#define MENUITEMTYPE_MARQUEE     0x17
#define MENUITEMTYPE_18          0x18
#define MENUITEMTYPE_CONTROLLER  0x19
#define MENUITEMTYPE_END         0x1a

#ifndef PLATFORM_N64
#define MENUITEMTYPE_COLORBOX    0x1b
#endif

#define MENUMODELFLAG_HASSCALE    0x01
#define MENUMODELFLAG_HASPOSITION 0x02
#define MENUMODELFLAG_HASROTATION 0x04

#define MENUMODELPARAMS_HAS_MASTER_HEADBODY(params)           (((params) & 0x80000000) != 0)
#define MENUMODELPARAMS_GET_MASTER_HEADNUM(params)            ((params) & 0x3ff)
#define MENUMODELPARAMS_GET_MASTER_BODYNUM(params)            (((params) & 0xffc00) >> 10)
#define MENUMODELPARAMS_GET_MP_HEADNUM(params)                (((params) >> 16) & 0xff)
#define MENUMODELPARAMS_GET_MP_BODYNUM(params)                (((params) >> 24) & 0xff)
#define MENUMODELPARAMS_GET_FILENUM(params)                   ((params) & 0xffff)
#define MENUMODELPARAMS_SET_FILENUM(filenum)                  (filenum)
#define MENUMODELPARAMS_SET_MP_HEADBODY(mpheadnum, mpbodynum) (0xffff | ((mpheadnum) << 16) | ((mpbodynum) << 24))

#define MENUMODELTYPE_DEFAULT     0
#define MENUMODELTYPE_HUDPIECE    1
#define MENUMODELTYPE_2           2
#define MENUMODELTYPE_3           3
#define MENUMODELTYPE_CREDITSLOGO 4

#define MENUOP_GETOPTIONCOUNT      1
#define MENUOP_GETOPTGROUPCOUNT    2
#define MENUOP_GETOPTIONTEXT       3
#define MENUOP_GETOPTGROUPTEXT     4
#define MENUOP_GETGROUPSTARTINDEX  5
#define MENUOP_SET                 6
#define MENUOP_GETSELECTEDINDEX    7
#define MENUOP_GET                 8
#define MENUOP_GETSLIDER           9
#define MENUOP_GETSLIDERLABEL      10
#define MENUOP_11                  11
#define MENUOP_CHECKDISABLED       12
#define MENUOP_FOCUS               13
#define MENUOP_GETLISTITEMCHECKBOX 14
#define MENUOP_CHECKPREFOCUSED     15
#define MENUOP_LISTITEMFOCUS       16
#define MENUOP_GETTEXT             17
#define MENUOP_SETTEXT             18
#define MENUOP_RENDER              19
#define MENUOP_GETOPTIONHEIGHT     20
#define MENUOP_21                  21
#define MENUOP_GETCOLOUR           22
#define MENUOP_CHECKHIDDEN         24
#define MENUOP_25                  25
#define MENUOP_OPEN                100
#define MENUOP_CLOSE               101
#define MENUOP_TICK                102

#define MENUPLANE_00 0
#define MENUPLANE_01 1
#define MENUPLANE_02 2
#define MENUPLANE_03 3
#define MENUPLANE_04 4
#define MENUPLANE_05 5
#define MENUPLANE_06 6
#define MENUPLANE_07 7
#define MENUPLANE_08 8
#define MENUPLANE_09 9
#define MENUPLANE_10 10
#define MENUPLANE_11 11

#define MENUREPEATMODE_RELEASED -1
#define MENUREPEATMODE_SLOW     0
#define MENUREPEATMODE_FAST     1

#define MENUROOT_ENDSCREEN    1
#define MENUROOT_MAINMENU     2
#define MENUROOT_MPSETUP      3
#define MENUROOT_MPPAUSE      4
#define MENUROOT_MPENDSCREEN  5
#define MENUROOT_FILEMGR      6
#define MENUROOT_BOOTPAKMGR   7
#define MENUROOT_PICKTARGET   8
#define MENUROOT_COOPCONTINUE 9
#define MENUROOT_4MBFILEMGR   10
#define MENUROOT_4MBMAINMENU  11
#define MENUROOT_12           12
#define MENUROOT_TRAINING     13

#define MENUSOUND_SWIPE          0x00 // Navigating to left/right dialog
#define MENUSOUND_OPENDIALOG     0x01
#define MENUSOUND_FOCUS          0x02 // Focusing a different item
#define MENUSOUND_SELECT         0x03
#define MENUSOUND_ERROR          0x04
#define MENUSOUND_EXPLOSION      0x05
#define MENUSOUND_TOGGLEON       0x08 // Checking a checkbox
#define MENUSOUND_TOGGLEOFF      0x09 // Unchecking a checkbox, opening a dropdown, cancelling a dropdown
#define MENUSOUND_SUBFOCUS       0x0a // Changing focus within a list or dropdown
#define MENUSOUND_0B             0x0b
#define MENUSOUND_KEYBOARDFOCUS  0x0c // Changing focus within a keyboard item
#define MENUSOUND_KEYBOARDCANCEL 0x0d // Pressing A on a keyboard's cancel button
#define MENUSOUND_SUCCESS        0x0e

#define MENUTICKFLAG_DIALOGISCURRENT 0x01
#define MENUTICKFLAG_ITEMISFOCUSED   0x02
#define MENUTICKFLAG_DIALOGISDIMMED  0x04

#define MISCBIO_MAIANS     0
#define MISCBIO_SKEDAR     1
#define MISCBIO_BACKGROUND 2
#define MISCBIO_STORY      3

#define MISCSFX_BOOSTHEARTBEAT   0
#define MISCSFX_SLAYERROCKETHUM  1
#define MISCSFX_SLAYERROCKETBEEP 2

// Models
#define MODEL_ROOFGUN               0x0000
#define MODEL_GROUNDGUN             0x0001
#define MODEL_TVSCREEN              0x0002
#define MODEL_BORG_CRATE            0x0003
#define MODEL_WINDOW                0x0004
#define MODEL_A51_CRATE1_0005       0x0005
#define MODEL_A51_CRATE1_0006       0x0006
#define MODEL_A51_CRATE1_0007       0x0007
#define MODEL_A51_CRATE1_0008       0x0008
#define MODEL_A51_CRATE1_0009       0x0009
#define MODEL_A51_CRATE1_000a       0x000a
#define MODEL_A51_CRATE1_000b       0x000b
#define MODEL_A51_CRATE1_000c       0x000c
#define MODEL_A51_CRATE1_000d       0x000d
#define MODEL_A51_CRATE1_000e       0x000e
#define MODEL_A51_CRATE1_000f       0x000f
#define MODEL_CRYPTDOOR1B           0x0010
#define MODEL_CHRBRIEFCASE          0x0011
#define MODEL_CHRBUG                0x0012
#define MODEL_CHRDATATHIEF          0x0013
#define MODEL_NINTENDOLOGO          0x0014
#define MODEL_DOOR_ROLLERTRAIN      0x0015
#define MODEL_FLAG                  0x0016
#define MODEL_MODEMBOX              0x0017
#define MODEL_DOORCONSOLE           0x0018
#define MODEL_A51_HORIZ_DOOR_TOP    0x0019
#define MODEL_A51_HORIZ_DOOR_BOT    0x001a
#define MODEL_A51_VERT_DOOR_LEFT    0x001b
#define MODEL_A51_VERT_DOOR_RIGHT   0x001c
#define MODEL_A51_VERT_DOOR_ST      0x001d
#define MODEL_A51_HORIZ_DOOR_GL     0x001e
#define MODEL_A51_HORIZ_DOOR_SECRET 0x001f
#define MODEL_A51_CRATE1            0x0020
#define MODEL_A51_CRATE2            0x0021
#define MODEL_A51_CRATE3            0x0022
#define MODEL_A51_EXP1              0x0023
#define MODEL_A51_UNEXP1            0x0024
#define MODEL_A51_EXP2              0x0025
#define MODEL_A51_UNEXP2            0x0026
#define MODEL_A51_UNEXP3            0x0027
#define MODEL_AIVILLADOOR1          0x0028
#define MODEL_AIVILLADOOR2A         0x0029
#define MODEL_AIVILLADOOR4          0x002a
#define MODEL_A51_LIFT_HANGAR       0x002b
#define MODEL_A51_LIFT_CONTROL      0x002c
#define MODEL_A51_LIFT_STORE        0x002d
#define MODEL_A51_LIFT_THINWALL     0x002e
#define MODEL_AIVILLABOT1           0x002f
#define MODEL_AIVILLABOT2           0x0030
#define MODEL_AIVILLABOT3           0x0031
#define MODEL_AIVILLAWINDMILL       0x0032
#define MODEL_HOVERBED              0x0033
#define MODEL_MARKER                0x0034
#define MODEL_ALDOOR_R              0x0035
#define MODEL_ALDOOR_L              0x0036
#define MODEL_DD_LIFTR              0x0037
#define MODEL_DD_FANROOF            0x0038
#define MODEL_DD_FANWALL            0x0039
#define MODEL_HOVBIKE               0x003a
#define MODEL_DD_OFFICEDOOR         0x003b
#define MODEL_DD_PLANTRUBBER        0x003c
#define MODEL_DD_PLANTSPIKE         0x003d
#define MODEL_DD_PLANTSPIDER        0x003e
#define MODEL_DD_WINDOW             0x003f
#define MODEL_DD_REDSOFA            0x0040
#define MODEL_DD_REDARM             0x0041
#define MODEL_DD_SERVICEDOOR        0x0042
#define MODEL_DD_WINDDOOR           0x0043
#define MODEL_DD_LIFTDOOR           0x0044
#define MODEL_DD_VERTBLIND          0x0045
#define MODEL_DD_DESK               0x0046
#define MODEL_DD_CHAIR              0x0047
#define MODEL_NLOGO                 0x0048
#define MODEL_NLOGO2                0x0049
#define MODEL_NLOGO3                0x004a
#define MODEL_PERFECTDARK           0x004b
#define MODEL_PDONE                 0x004c
#define MODEL_PDTWO                 0x004d
#define MODEL_PDTHREE               0x004e
#define MODEL_PDFOUR                0x004f
#define MODEL_DD_HOVCOP             0x0050
#define MODEL_DD_HOVMOTO            0x0051
#define MODEL_DD_HOVTRUCK           0x0052
#define MODEL_DD_HOVCAR             0x0053
#define MODEL_DD_HOVCAB             0x0054
#define MODEL_DD_AC_UNEXP           0x0055
#define MODEL_DD_AC_EXP             0x0056
#define MODEL_DD_ACBOT_UNEXP        0x0057
#define MODEL_DD_ACBOT_EXP          0x0058
#define MODEL_PC1                   0x0059
#define MODEL_HOVERCRATE1           0x005a
#define MODEL_DROPSHIP              0x005b
#define MODEL_AL_AIRLOCK            0x005c
#define MODEL_AL_DOCKLIFT           0x005d
#define MODEL_CASE                  0x005e
#define MODEL_DD_STONEDESK          0x005f
#define MODEL_MEDLABWIN1            0x0060
#define MODEL_MEDLABWIN2            0x0061
#define MODEL_A51TABLE              0x0062
#define MODEL_A51CHAIR              0x0063
#define MODEL_A51SCREEN             0x0064
#define MODEL_A51WASTEBIN           0x0065
#define MODEL_A51DESKENT            0x0066
#define MODEL_A51TROLLEY            0x0067
#define MODEL_A51DIVIDE             0x0068
#define MODEL_A51BOARD              0x0069
#define MODEL_SKCREV_EXP1           0x006a
#define MODEL_SKCREV_UNEXP1         0x006b
#define MODEL_SKTNL_EXP1            0x006c
#define MODEL_SKTNL_UNEXP1          0x006d
#define MODEL_SK_DOOR1              0x006e
#define MODEL_SK_SHIP_DOOR1         0x006f
#define MODEL_SK_SHIP_HOLO1         0x0070
#define MODEL_SK_SHIP_HOLO2         0x0071
#define MODEL_SK_SHIP_HULLDOOR1     0x0072
#define MODEL_SK_SHIP_HULLDOOR2     0x0073
#define MODEL_SK_SHIP_HULLDOOR3     0x0074
#define MODEL_SK_SHIP_HULLDOOR4     0x0075
#define MODEL_SK_FIGHTER1           0x0076
#define MODEL_SK_CRYOPOD1_TOP       0x0077
#define MODEL_SK_CRYOPOD1_BOT       0x0078
#define MODEL_SK_UNDER_GENERATOR    0x0079
#define MODEL_SK_UNDER_TRANS        0x007a
#define MODEL_SK_HANGARDOOR_TOP     0x007b
#define MODEL_SK_HANGARDOOR_BOT     0x007c
#define MODEL_DOOR2_G5              0x007d
#define MODEL_DOOR1A_G5             0x007e
#define MODEL_DOOR1B_G5             0x007f
#define MODEL_DOOR1ATRI_G5          0x0080
#define MODEL_DOOR2A_G5             0x0081
#define MODEL_DD_DECODOOR           0x0082
#define MODEL_DD_SECRETDOOR         0x0083
#define MODEL_DD_SECRETDOOR2        0x0084
#define MODEL_DDJUMPSHIP            0x0085
#define MODEL_TAXICAB               0x0086
#define MODEL_POLICECAR             0x0087
#define MODEL_RAVINELIFT            0x0088
#define MODEL_DD_LAB_DOOR_BS        0x0089
#define MODEL_DD_LAB_DOOR_SEC       0x008a
#define MODEL_DD_LAB_DOOR_WIND      0x008b
#define MODEL_HOOVERBOT             0x008c
#define MODEL_TESTERBOT             0x008d
#define MODEL_DD_LAB_SECTOR2BOT     0x008e
#define MODEL_DD_LAB_SECTOR2TOP     0x008f
#define MODEL_DD_LAB_CAUTIONTOP     0x0090
#define MODEL_DD_LAB_HAZARD         0x0091
#define MODEL_DD_LAB_CAUTION        0x0092
#define MODEL_DR_CAROLL_DOOR        0x0093
#define MODEL_DD_LAB_SECTOR3TOP     0x0094
#define MODEL_DD_LAB_SECTOR3        0x0095
#define MODEL_DD_LAB_SECTOR3WIND    0x0096
#define MODEL_DD_HOVERCOPTER        0x0097
#define MODEL_DD_LAB_SECTOR4TOP     0x0098
#define MODEL_DD_LAB_RESTRICTED     0x0099
#define MODEL_DOOR4A_G5             0x009a
#define MODEL_DOOR4B_G5             0x009b
#define MODEL_LASDOOR               0x009c
#define MODEL_G5SAFEDOOR            0x009d
#define MODEL_ROPE                  0x009e
#define MODEL_G5_MAINFRAME          0x009f
#define MODEL_DR_CAROLL_DOOR_BASE   0x00a0
#define MODEL_DR_CAROLL_DOOR_MAIN   0x00a1
#define MODEL_DR_CAROLL_DOOR_LEFT   0x00a2
#define MODEL_DR_CAROLL_DOOR_RIGHT  0x00a3
#define MODEL_DR_CAROLL_DOOR_BMAIN  0x00a4
#define MODEL_DR_CAROLL_DOOR_BLEFT  0x00a5
#define MODEL_DR_CAROLL_DOOR_BRIGHT 0x00a6
#define MODEL_DD_BANNER             0x00a7
#define MODEL_G5_ESCDOORUP          0x00a8
#define MODEL_G5_ESCDOORUPBOOM      0x00a9
#define MODEL_G5_ESCDOORDOWN        0x00aa
#define MODEL_G5_ESCDOORDOWNBOOM    0x00ab
#define MODEL_DUMPSTER              0x00ac
#define MODEL_G5CARLIFTDOOR         0x00ad
#define MODEL_CH_SHUTTER1           0x00ae
#define MODEL_CCTV_PD               0x00af
#define MODEL_COMHUB                0x00b0
#define MODEL_QUADPOD               0x00b1
#define MODEL_PD_CONSOLE            0x00b2
#define MODEL_DD_GRATE              0x00b3
#define MODEL_LIFT_PLATFORM         0x00b4
#define MODEL_LIGHTSWITCH           0x00b5
#define MODEL_BLASTSHIELD           0x00b6
#define MODEL_LIGHTSWITCH2          0x00b7
#define MODEL_DD_ACCESSDOORUP       0x00b8
#define MODEL_DD_ACCESSDOORDN       0x00b9
#define MODEL_LAB_CONTAINER         0x00ba
#define MODEL_LAB_CHAIR             0x00bb
#define MODEL_LAB_TABLE             0x00bc
#define MODEL_LAB_MICROSCOPE        0x00bd
#define MODEL_LAB_MAINFRAME         0x00be
#define MODEL_DD_LABDOOR            0x00bf
#define MODEL_DD_LAB_DOORTOP        0x00c0
#define MODEL_MULTI_AMMO_CRATE      0x00c1
#define MODEL_CHRCHAIN              0x00c2
#define MODEL_TDOOR                 0x00c3
#define MODEL_CI_SOFA               0x00c4
#define MODEL_CI_LIFT               0x00c5
#define MODEL_CI_LIFTDOOR           0x00c6
#define MODEL_LASERCUT              0x00c7
#define MODEL_SK_SHUTTLE            0x00c8
#define MODEL_NEWVILLADOOR          0x00c9
#define MODEL_SK_PILLARLEFT         0x00ca
#define MODEL_SK_PILLARRIGHT        0x00cb
#define MODEL_SK_PLINTH_T           0x00cc
#define MODEL_SK_PLINTH_ML          0x00cd
#define MODEL_SK_PLINTH_MR          0x00ce
#define MODEL_SK_PLINTH_BL          0x00cf
#define MODEL_SK_PLINTH_BR          0x00d0
#define MODEL_SK_FL_SHAD_T          0x00d1
#define MODEL_SK_FL_SHAD_ML         0x00d2
#define MODEL_SK_FL_SHAD_MR         0x00d3
#define MODEL_SK_FL_SHAD_BL         0x00d4
#define MODEL_SK_FL_SHAD_BR         0x00d5
#define MODEL_SK_FL_NOSHAD_T        0x00d6
#define MODEL_SK_FL_NOSHAD_ML       0x00d7
#define MODEL_SK_FL_NOSHAD_MR       0x00d8
#define MODEL_SK_FL_NOSHAD_BL       0x00d9
#define MODEL_SK_FL_NOSHAD_BR       0x00da
#define MODEL_SK_TEMPLECOLUMN1      0x00db
#define MODEL_SK_TEMPLECOLUMN2      0x00dc
#define MODEL_SK_TEMPLECOLUMN3      0x00dd
#define MODEL_SK_SUNSHAD1           0x00de
#define MODEL_SK_SUNSHAD2           0x00df
#define MODEL_SK_SUNNOSHAD1         0x00e0
#define MODEL_SK_SUNNOSHAD2         0x00e1
#define MODEL_BARREL                0x00e2
#define MODEL_GLASS_FLOOR           0x00e3
#define MODEL_ESCA_STEP             0x00e4
#define MODEL_MATRIX_LIFT           0x00e5
#define MODEL_RUBBLE1               0x00e6
#define MODEL_RUBBLE2               0x00e7
#define MODEL_RUBBLE3               0x00e8
#define MODEL_RUBBLE4               0x00e9
#define MODEL_CABLE_CAR             0x00ea
#define MODEL_ELVIS_SAUCER          0x00eb
#define MODEL_STEWARDESS_TROLLEY    0x00ec
#define MODEL_AIRBASE_LIFT_ENCLOSED 0x00ed
#define MODEL_AIRBASE_LIFT_ANGLE    0x00ee
#define MODEL_AIRBASE_SAFEDOOR      0x00ef
#define MODEL_AF1_PILOTCHAIR        0x00f0
#define MODEL_AF1_PASSCHAIR         0x00f1
#define MODEL_TESTOBJ               0x00f2
#define MODEL_CHRNIGHTSIGHT         0x00f3
#define MODEL_CHRSHIELD             0x00f4
#define MODEL_CHRFALCON2            0x00f5
#define MODEL_CHRLEEGUN1            0x00f6
#define MODEL_CHRMAULER             0x00f7
#define MODEL_CHRDY357              0x00f8
#define MODEL_CHRDY357TRENT         0x00f9
#define MODEL_CHRMAIANPISTOL        0x00fa
#define MODEL_CHRFALCON2SIL         0x00fb
#define MODEL_CHRFALCON2SCOPE       0x00fc
#define MODEL_CHRCMP150             0x00fd
#define MODEL_CHRAR34               0x00fe
#define MODEL_CHRDRAGON             0x00ff
#define MODEL_CHRSUPERDRAGON        0x0100
#define MODEL_CHRAVENGER            0x0101
#define MODEL_CHRCYCLONE            0x0102
#define MODEL_CHRMAIANSMG           0x0103
#define MODEL_CHRRCP120             0x0104
#define MODEL_CHRPCGUN              0x0105
#define MODEL_CHRSHOTGUN            0x0106
#define MODEL_CHRSKMINIGUN          0x0107
#define MODEL_CHRDYROCKET           0x0108
#define MODEL_CHRDEVASTATOR         0x0109
#define MODEL_CHRSKROCKET           0x010a
#define MODEL_CHRZ2020              0x010b
#define MODEL_CHRSNIPERRIFLE        0x010c
#define MODEL_CHRCROSSBOW           0x010d
#define MODEL_CHRDRUGGUN            0x010e
#define MODEL_CHRKNIFE              0x010f
#define MODEL_CHRNBOMB              0x0110
#define MODEL_CHRFLASHBANG          0x0111
#define MODEL_CHRGRENADE            0x0112
#define MODEL_CHRTIMEDMINE          0x0113
#define MODEL_CHRPROXIMITYMINE      0x0114
#define MODEL_CHRREMOTEMINE         0x0115
#define MODEL_CHRECMMINE            0x0116
#define MODEL_CHRWPPK               0x0117
#define MODEL_CHRTT33               0x0118
#define MODEL_CHRSKORPION           0x0119
#define MODEL_CHRKALASH             0x011a
#define MODEL_CHRUZI                0x011b
#define MODEL_CHRMP5K               0x011c
#define MODEL_CHRM16                0x011d
#define MODEL_CHRFNP90              0x011e
#define MODEL_CHRDYROCKETMIS        0x011f
#define MODEL_CHRSKROCKETMIS        0x0120
#define MODEL_CHRCROSSBOLT          0x0121
#define MODEL_CHRDEVGRENADE         0x0122
#define MODEL_CHRDRAGGRENADE        0x0123
#define MODEL_A51_TURRET            0x0124
#define MODEL_PELAGICDOOR           0x0125
#define MODEL_AUTOSURGEON           0x0126
#define MODEL_LIMO                  0x0127
#define MODEL_A51INTERCEPTOR        0x0128
#define MODEL_A51DISH               0x0129
#define MODEL_A51RADARCONSOLE       0x012a
#define MODEL_A51LOCKERDOOR         0x012b
#define MODEL_G5GENERATOR           0x012c
#define MODEL_G5DUMPSTER            0x012d
#define MODEL_CHRCLOAKER            0x012e
#define MODEL_CHRSPEEDPILL          0x012f
#define MODEL_BIGPELAGICDOOR        0x0130
#define MODEL_SK_JONRUBBLE3         0x0131
#define MODEL_SK_JONRUBBLE4         0x0132
#define MODEL_SK_JONRUBBLE5         0x0133
#define MODEL_SK_JONRUBBLE6         0x0134
#define MODEL_BAGGAGECARRIER        0x0135
#define MODEL_MINESIGN              0x0136
#define MODEL_CHAMBER               0x0137
#define MODEL_ISOTOPEEXPERIMENT     0x0138
#define MODEL_ISOTOPE               0x0139
#define MODEL_REACTORDOOR           0x013a
#define MODEL_SAUCERINSIDE          0x013b
#define MODEL_VILLASTOOL            0x013c
#define MODEL_CETANWINDOW1          0x013d
#define MODEL_CETANWINDOW2          0x013e
#define MODEL_CETANWINDOW3          0x013f
#define MODEL_BINOCULARS            0x0140
#define MODEL_SUBMARINE             0x0141
#define MODEL_AIRFORCE1             0x0142
#define MODEL_ENGINEPART            0x0143
#define MODEL_CETROOFGUN            0x0144
#define MODEL_CETANSMALLDOOR        0x0145
#define MODEL_POWERNODE             0x0146
#define MODEL_CETANBLUEGREENL       0x0147
#define MODEL_CETANBLUEGREENR       0x0148
#define MODEL_SKEDARCONSOLE         0x0149
#define MODEL_SKEDARCONSOLEPANEL    0x014a
#define MODEL_WEAPONCDOOR           0x014b
#define MODEL_TARGET                0x014c
#define MODEL_DEVICESECRETDOOR      0x014d
#define MODEL_CARRINGTONSECRETDOOR  0x014e
#define MODEL_SINISTERPC            0x014f
#define MODEL_SINISTERSTATION       0x0150
#define MODEL_KEYPADLOCK            0x0151
#define MODEL_THUMBPRINTSCANNER     0x0152
#define MODEL_RETINALOCK            0x0153
#define MODEL_CARDLOCK              0x0154
#define MODEL_GOODSTATION           0x0155
#define MODEL_GOODPC                0x0156
#define MODEL_CHRAUTOGUN            0x0157
#define MODEL_G5BIGCHAIR            0x0158
#define MODEL_G5SMALLCHAIR          0x0159
#define MODEL_KINGSCEPTRE           0x015a
#define MODEL_LABCOAT               0x015b
#define MODEL_CIDOOR1               0x015c
#define MODEL_G5_CHAIR              0x015d
#define MODEL_G5_CHAIR2             0x015e
#define MODEL_DD_WINDOW_FOYER       0x015f
#define MODEL_CI_CABINET            0x0160
#define MODEL_CI_DESK               0x0161
#define MODEL_CI_CARR_DESK          0x0162
#define MODEL_CI_F_CHAIR            0x0163
#define MODEL_CI_LOUNGER            0x0164
#define MODEL_CI_F_SOFA             0x0165
#define MODEL_CI_TABLE              0x0166
#define MODEL_CV_COFFEE_TABLE       0x0167
#define MODEL_CV_CHAIR1             0x0168
#define MODEL_CV_CHAIR2             0x0169
#define MODEL_CV_SOFA               0x016a
#define MODEL_CV_CHAIR4             0x016b
#define MODEL_CV_LAMP               0x016c
#define MODEL_CV_CABINET            0x016d
#define MODEL_CV_F_BED              0x016e
#define MODEL_PEL_CHAIR1            0x016f
#define MODEL_SK_CONSOLE2           0x0170
#define MODEL_DD_EAR_TABLE          0x0171
#define MODEL_DD_EAR_CHAIR          0x0172
#define MODEL_AIRBASE_TABLE2        0x0173
#define MODEL_AIRBASE_CHAIR2        0x0174
#define MODEL_MISC_CRATE            0x0175
#define MODEL_A51_CRATE1_0175       0x0176
#define MODEL_MISC_IRSPECS          0x0177
#define MODEL_A51_ROOFGUN           0x0178
#define MODEL_SK_DRONE_GUN          0x0179
#define MODEL_CI_ROOFGUN            0x017a
#define MODEL_CV_TABLE              0x017b
#define MODEL_CIDOOR1_REF           0x017c
#define MODEL_ALASKADOOR_OUT        0x017d
#define MODEL_ALASKADOOR_IN         0x017e
#define MODEL_WIREFENCE             0x017f
#define MODEL_RARELOGO              0x0180
#define MODEL_KEYCARD               0x0181
#define MODEL_BODYARMOUR            0x0182
#define MODEL_A51GATE_R             0x0183
#define MODEL_A51GATE_L             0x0184
#define MODEL_AF1_LAMP              0x0185
#define MODEL_AF1_TOILET            0x0186
#define MODEL_AF1_DOORBIG2          0x0187
#define MODEL_AF1_PHONE             0x0188
#define MODEL_AF1_CARGODOOR         0x0189
#define MODEL_G5_ALARM              0x018a
#define MODEL_G5_LASER_SWITCH       0x018b
#define MODEL_SK_TEMPLECOLUMN4      0x018c
#define MODEL_COREHATCH             0x018d
#define MODEL_A51GRATE              0x018e
#define MODEL_AF1ESCAPEDOOR         0x018f
#define MODEL_PRESCAPSULE           0x0190
#define MODEL_SKEDARBRIDGE          0x0191
#define MODEL_PELAGICDOOR2          0x0192
#define MODEL_TTB_BOX               0x0193
#define MODEL_INSTFRONTDOOR         0x0194
#define MODEL_CHRLASER              0x0195
#define MODEL_BAFTA                 0x0196
#define MODEL_CHRSONICSCREWER       0x0197
#define MODEL_CHRLUMPHAMMER         0x0198
#define MODEL_EXPLOSIVEBRICK        0x0199
#define MODEL_SKEDARBOMB            0x019a
#define MODEL_ZIGGYCARD             0x019b
#define MODEL_SAFEITEM              0x019c
#define MODEL_RUSSDAR               0x019d
#define MODEL_XRAYSPECS             0x019e
#define MODEL_CHRLUMPHAMMER2        0x019f
#define MODEL_CHREYESPY             0x01a0
#define MODEL_CHRDOORDECODER        0x01a1
#define MODEL_AF1_TABLE             0x01a2
#define MODEL_SHUTTLEDOOR           0x01a3
#define MODEL_RUINBRIDGE            0x01a4
#define MODEL_SECRETINDOOR          0x01a5
#define MODEL_SENSITIVEINFO         0x01a6
#define MODEL_SUITCASE              0x01a7
#define MODEL_SKPUZZLEOBJECT        0x01a8
#define MODEL_A51LIFTDOOR           0x01a9
#define MODEL_CIHUB                 0x01aa
#define MODEL_SK_SHIP_DOOR2         0x01ab
#define MODEL_SK_WINDOW1            0x01ac
#define MODEL_SK_HANGARDOORB_TOP    0x01ad
#define MODEL_SK_HANGARDOORB_BOT    0x01ae
#define MODEL_AF1_INNERDOOR         0x01af
#define MODEL_LASER_POST            0x01b0
#define MODEL_TARGETAMP             0x01b1
#define MODEL_SK_LIFT               0x01b2
#define MODEL_KNOCKKNOCK            0x01b3
#define MODEL_CETANDOOR             0x01b4
#define MODEL_AF1RUBBLE             0x01b5
#define MODEL_DD_DR_NONREF          0x01b6
#define MODEL_CETANDOORSIDE         0x01b7
#define MODEL_BUDDYBRIDGE           0x01b8
#define MODEL_JPNLOGO               0x01b9
#define MODEL_JPNPD                 0x01ba
#define NUM_MODELS                  (VERSION == VERSION_JPN_FINAL ? 0x1bb : 0x1b9)

#define MODELNODETYPE_CHRINFO      0x01
#define MODELNODETYPE_POSITION     0x02
#define MODELNODETYPE_GUNDL        0x04
#define MODELNODETYPE_05           0x05
#define MODELNODETYPE_DISTANCE     0x08
#define MODELNODETYPE_REORDER      0x09
#define MODELNODETYPE_BBOX         0x0a
#define MODELNODETYPE_0B           0x0b
#define MODELNODETYPE_CHRGUNFIRE   0x0c
#define MODELNODETYPE_0D           0x0d
#define MODELNODETYPE_0E           0x0e
#define MODELNODETYPE_0F           0x0f
#define MODELNODETYPE_11           0x11
#define MODELNODETYPE_TOGGLE       0x12
#define MODELNODETYPE_POSITIONHELD 0x15
#define MODELNODETYPE_STARGUNFIRE  0x16
#define MODELNODETYPE_HEADSPOT     0x17
#define MODELNODETYPE_DL           0x18
#define MODELNODETYPE_0100         0x0100
#define MODELNODETYPE_0200         0x0200

/**
 * Models can contain "parts", which are a list of certain node addresses in the
 * model's header. It allows the game to quickly find a certain node in the
 * model if it's registered in the parts list.
 *
 * Parts are identified by a part number, and the mapping is usually unique to
 * each model type. For example, for "chrgun" model types, part number 0 is the
 * gunfire starburst. An exception to this rule is first-person gun models,
 * where some part numbers are consistent between all guns while other part
 * numbers differ between different guns.
 *
 * The game assumes that the part numbers refer to particular node types.
 * For example, it assumes that chrgun part 02 is a toggle node.
 *
 * The names below are in the format MODELPART_{modeltype}_{part}.
 * The comment is the node type.
 */
#define MODELPART_AR34_0042              0x0042 // toggle
#define MODELPART_AR34_0050              0x0050 // position
#define MODELPART_AR34_0051              0x0051 // position
#define MODELPART_AR34_MAGAZINE1         0x0029 // toggle
#define MODELPART_AR34_MAGAZINE2         0x002a // toggle
#define MODELPART_AUTOGUN_0000           0x0000 // position
#define MODELPART_AUTOGUN_0001           0x0001 // position
#define MODELPART_AUTOGUN_0002           0x0002 // position
#define MODELPART_AUTOGUN_0003           0x0003 // position
#define MODELPART_AUTOGUN_0004           0x0004 // position
#define MODELPART_AUTOGUN_0006           0x0006 // position
#define MODELPART_AUTOGUN_FLASHLEFT      0x0005 // gunfire
#define MODELPART_AUTOGUN_FLASHRIGHT     0x0007 // gunfire
#define MODELPART_BASIC_0064             0x0064 // type19
#define MODELPART_BASIC_0065             0x0065 // type19
#define MODELPART_BASIC_0066             0x0066 // type19
#define MODELPART_BASIC_0067             0x0067 // bbox
#define MODELPART_BASIC_00C8             0x00c8 // toggle
#define MODELPART_BASIC_00C9             0x00c9 // toggle
#define MODELPART_BASIC_00CA             0x00ca // toggle
#define MODELPART_BASIC_00CB             0x00cb // toggle
#define MODELPART_BASIC_00CC             0x00cc // toggle
#define MODELPART_BASIC_00CD             0x00cd // toggle
#define MODELPART_BASIC_00CE             0x00ce // toggle
#define MODELPART_BASIC_00CF             0x00cf // toggle
#define MODELPART_BASIC_00D0             0x00d0 // toggle
#define MODELPART_BASIC_00D1             0x00d1 // toggle
#define MODELPART_BASIC_00D2             0x00d2 // toggle
#define MODELPART_BASIC_00D3             0x00d3 // toggle
#define MODELPART_CALLISTO_ORB           0x0028 // toggle
#define MODELPART_CCTV_0002              0x0002 // bbox
#define MODELPART_CCTV_0003              0x0003 // toggle
#define MODELPART_CCTV_CASING            0x0000 // position
#define MODELPART_CCTV_LENS              0x0001 // dl
#define MODELPART_CHOPPER_0000           0x0000 // position
#define MODELPART_CHOPPER_0001           0x0001 // position
#define MODELPART_CHOPPER_0002           0x0002 // position
#define MODELPART_CHOPPER_0005           0x0005 // toggle
#define MODELPART_CHOPPER_GUNFLASH       0x0003 // gunfire
#define MODELPART_CHRGUN_0001            0x0001 // positionheld
#define MODELPART_CHRGUN_0002            0x0002 // toggle
#define MODELPART_CHRGUN_GUNFIRE         0x0000 // gunfire
#define MODELPART_CHR_0000               0x0000 // position
#define MODELPART_CHR_0001               0x0001 // position
#define MODELPART_CHR_0002               0x0002 // position
#define MODELPART_CHR_0006               0x0006 // position
#define MODELPART_CHR_HEADSPOT           0x0004 // headspot
#define MODELPART_CHR_LEFTHAND           0x0005 // position
#define MODELPART_CHR_NECKLACE           0x0007 // toggle
#define MODELPART_CHR_RIGHTHAND          0x0003 // position
#define MODELPART_CIHUB_0000             0x0000 // dl
#define MODELPART_CIHUB_0001             0x0001 // dl
#define MODELPART_CIHUB_0002             0x0002 // dl
#define MODELPART_CIHUB_0003             0x0003 // dl
#define MODELPART_CLASSICGUN_0050        0x0050 // position
#define MODELPART_CMP150_0042            0x0042 // toggle
#define MODELPART_CMP150_0050            0x0050 // position
#define MODELPART_CMP150_0051            0x0051 // position
#define MODELPART_CMP150_MAGAZINE1       0x002a // toggle
#define MODELPART_CMP150_MAGAZINE2       0x002b // toggle
#define MODELPART_CROSSBOW_0028          0x0028 // toggle
#define MODELPART_CROSSBOW_0042          0x0042 // toggle
#define MODELPART_CROSSBOW_BOLT          0x0029 // toggle
#define MODELPART_CROSSBOW_HANDLE        0x002a // toggle
#define MODELPART_CYCLONE_0050           0x0050 // position
#define MODELPART_CYCLONE_0051           0x0051 // position
#define MODELPART_CYCLONE_MAGAZINE       0x0028 // toggle
#define MODELPART_DEVASTATOR_0028        0x0028 // position
#define MODELPART_DEVASTATOR_CART1       0x0064 // toggle - innermost cartridge
#define MODELPART_DEVASTATOR_CART2       0x0065 // toggle
#define MODELPART_DEVASTATOR_CART3       0x0066 // toggle
#define MODELPART_DEVASTATOR_CART4       0x0067 // toggle
#define MODELPART_DEVASTATOR_CART5       0x0068 // toggle
#define MODELPART_DEVASTATOR_CART6       0x0069 // toggle
#define MODELPART_DEVASTATOR_CART7       0x006a // toggle
#define MODELPART_DEVASTATOR_CART8       0x006b // toggle - outermost cartridge
#define MODELPART_DEVASTATOR_MAGAZINE1   0x0029 // toggle
#define MODELPART_DEVASTATOR_MAGAZINE2   0x002a // toggle
#define MODELPART_DRAGON_0042            0x0042 // toggle
#define MODELPART_DRAGON_0050            0x0050 // position
#define MODELPART_DRAGON_0051            0x0051 // position
#define MODELPART_DRAGON_0052            0x0052 // position
#define MODELPART_DRAGON_MAGAZINE1       0x0028 // toggle
#define MODELPART_DRAGON_MAGAZINE2       0x002a // toggle
#define MODELPART_DRCAROLL_0000          0x0000 // toggle
#define MODELPART_DRCAROLL_0001          0x0001 // toggle
#define MODELPART_DRCAROLL_0002          0x0002 // toggle
#define MODELPART_DRCAROLL_0003          0x0003 // toggle
#define MODELPART_DRCAROLL_0004          0x0004 // toggle
#define MODELPART_DRCAROLL_0005          0x0005 // toggle
#define MODELPART_DRCAROLL_0006          0x0006 // toggle
#define MODELPART_DRCAROLL_0007          0x0007 // toggle
#define MODELPART_DRCAROLL_0008          0x0008 // toggle
#define MODELPART_DRCAROLL_0009          0x0009 // toggle
#define MODELPART_DRCAROLL_000A          0x000a // toggle
#define MODELPART_DRCAROLL_000B          0x000b // toggle
#define MODELPART_DROPSHIP_0064          0x0064 // type19
#define MODELPART_DROPSHIP_INTERIOR      0x006e // toggle
#define MODELPART_FALCON2_002E           0x002e // toggle
#define MODELPART_FALCON2_002F           0x002f // toggle
#define MODELPART_FALCON2_0042           0x0042 // toggle
#define MODELPART_FALCON2_0050           0x0050 // position
#define MODELPART_FALCON2_0051           0x0051 // position
#define MODELPART_FALCON2_MAGAZINE1      0x002a // toggle
#define MODELPART_FALCON2_MAGAZINE2      0x002b // toggle
#define MODELPART_FALCON2_SCOPE          0x002c // toggle
#define MODELPART_FALCON2_SILENCER       0x002d // toggle
#define MODELPART_FARSIGHT_ORB           0x0028 // toggle
#define MODELPART_GRENADE_002B           0x002b // position
#define MODELPART_GRENADE_0064           0x0064 // toggle
#define MODELPART_GUN_CARTEJECTPOS       0x003c // position
#define MODELPART_GUN_CARTFLAPCLOSED     0x0046 // toggle
#define MODELPART_GUN_CARTFLAPOPEN       0x0047 // toggle
#define MODELPART_GUN_HOLDPOS            0x0037 // position
#define MODELPART_GUN_LASERLIQUID        0x0041 // gundl
#define MODELPART_GUN_LASERSIGHT         0x0034 // position
#define MODELPART_GUN_MUZZLEFLASH1       0x005a // toggle
#define MODELPART_GUN_MUZZLEFLASH2       0x005b // toggle
#define MODELPART_GUN_MUZZLEFLASH3       0x005c // toggle
#define MODELPART_GUN_MUZZLEPOS          0x0032 // position
#define MODELPART_GUN_SLIDE              0x0033 // position
#define MODELPART_HAND_LEFT              0x0035 // toggle
#define MODELPART_HAND_RIGHT             0x0036 // toggle
#define MODELPART_HEAD_0190              0x0190 // dl
#define MODELPART_HEAD_0191              0x0191 // dl
#define MODELPART_HEAD_0192              0x0192 // dl
#define MODELPART_HEAD_EYESCLOSED        0x0003 // toggle
#define MODELPART_HEAD_EYESOPEN          0x0002 // toggle
#define MODELPART_HEAD_HAT               0x0001 // toggle
#define MODELPART_HEAD_HUDPIECE          0x0004 // toggle
#define MODELPART_HEAD_SUNGLASSES        0x0000 // toggle
#define MODELPART_HOVERBIKE_0064         0x0064 // type19
#define MODELPART_HUDPIECE_0000          0x0000 // gundl
#define MODELPART_HUDPIECE_0001          0x0001 // position
#define MODELPART_HUDPIECE_0002          0x0002 // position
#define MODELPART_JOYPAD_0000            0x0000 // position
#define MODELPART_JOYPAD_0001            0x0001 // position
#define MODELPART_JOYPAD_0002            0x0002 // position
#define MODELPART_JOYPAD_0003            0x0003 // position
#define MODELPART_JOYPAD_0004            0x0004 // position
#define MODELPART_JOYPAD_0005            0x0005 // position
#define MODELPART_JOYPAD_0006            0x0006 // position
#define MODELPART_JOYPAD_0007            0x0007 // position
#define MODELPART_JOYPAD_0008            0x0008 // position
#define MODELPART_JOYPAD_0009            0x0009 // position
#define MODELPART_JOYPAD_000A            0x000a // position
#define MODELPART_JOYPAD_000B            0x000b // position
#define MODELPART_JOYPAD_000C            0x000c // position
#define MODELPART_JOYPAD_000D            0x000d // toggle
#define MODELPART_K7AVENGER_0042         0x0042 // toggle
#define MODELPART_K7AVENGER_0050         0x0050 // position
#define MODELPART_K7AVENGER_0051         0x0051 // position
#define MODELPART_K7AVENGER_MAGAZINE1    0x0029 // toggle
#define MODELPART_K7AVENGER_MAGAZINE2    0x002a // toggle
#define MODELPART_KNIFE_0064             0x0064 // toggle
#define MODELPART_LAPTOPGUN_0050         0x0050 // position
#define MODELPART_LAPTOPGUN_0051         0x0051 // position
#define MODELPART_LAPTOPGUN_MAGAZINE1    0x0029 // toggle
#define MODELPART_LAPTOPGUN_MAGAZINE2    0x002a // toggle
#define MODELPART_LASER_0042             0x0042 // toggle
#define MODELPART_LIFT_DOORBLOCK         0x0004 // type19
#define MODELPART_LIFT_FLOORNONRECT1     0x0005 // type19
#define MODELPART_LIFT_FLOORNONRECT2     0x0006 // type19
#define MODELPART_LIFT_FLOORRECT         0x0000 // bbox
#define MODELPART_LIFT_WALL1             0x0001 // type19
#define MODELPART_LIFT_WALL2             0x0002 // type19
#define MODELPART_LIFT_WALL3             0x0003 // type19
#define MODELPART_LOGO_0000              0x0000 // toggle
#define MODELPART_LOGO_0001              0x0001 // toggle
#define MODELPART_LOGO_FRONTSIDE         0x0002 // dl
#define MODELPART_LOGO_0003              0x0003 // dl
#define MODELPART_LOGO_RIGHTSIDE         0x0004 // dl
#define MODELPART_LOGO_0005              0x0005 // dl
#define MODELPART_LOGO_BACKSIDE          0x0006 // dl
#define MODELPART_LOGO_0007              0x0007 // dl
#define MODELPART_LOGO_LEFTSIDE          0x0008 // dl
#define MODELPART_LOGO_0009              0x0009 // dl
#define MODELPART_LOGO_0110              0x0110 // toggle
#define MODELPART_LOGO_0500              0x0500 // toggle
#define MODELPART_LOGO_4040              0x4040 // dl
#define MODELPART_MAGNUM_000A            0x000a // position
#define MODELPART_MAGNUM_000B            0x000b // position
#define MODELPART_MAGNUM_000C            0x000c // position
#define MODELPART_MAGNUM_000D            0x000d // position
#define MODELPART_MAGNUM_000E            0x000e // position
#define MODELPART_MAGNUM_000F            0x000f // position
#define MODELPART_MAGNUM_0042            0x0042 // toggle
#define MODELPART_MAGNUM_0050            0x0050 // position
#define MODELPART_MAGNUM_0051            0x0051 // position
#define MODELPART_MAGNUM_CART1           0x0028 // toggle
#define MODELPART_MAGNUM_CART2           0x0029 // toggle
#define MODELPART_MAGNUM_CART3           0x002a // toggle
#define MODELPART_MAGNUM_CART4           0x002b // toggle
#define MODELPART_MAGNUM_CART5           0x002c // toggle
#define MODELPART_MAGNUM_CART6           0x002d // toggle
#define MODELPART_MAIANUFO_0064          0x0064 // type19
#define MODELPART_MAULER_0050            0x0050 // position
#define MODELPART_MAULER_MAGAZINE1       0x002a // toggle
#define MODELPART_MAULER_MAGAZINE2       0x002b // toggle
#define MODELPART_PHOENIX_0042           0x0042 // toggle
#define MODELPART_PHOENIX_ORB            0x0028 // toggle
#define MODELPART_PISTOL_0042            0x0042 // toggle
#define MODELPART_PISTOL_0050            0x0050 // position
#define MODELPART_PISTOL_MAGAZINE1       0x0029 // toggle
#define MODELPART_PISTOL_MAGAZINE2       0x002a // toggle
#define MODELPART_RARELOGO_000B          0x000b // toggle
#define MODELPART_RARELOGO_000C          0x000c // toggle
#define MODELPART_RARELOGO_000D          0x000d // toggle
#define MODELPART_RCP120_0042            0x0042 // toggle
#define MODELPART_RCP120_0050            0x0050 // position
#define MODELPART_RCP120_0051            0x0051 // position
#define MODELPART_RCP120_MAGAZINE1       0x0028 // toggle
#define MODELPART_RCP120_MAGAZINE2       0x0029 // toggle
#define MODELPART_REAPER_001E            0x001e // position
#define MODELPART_REAPER_001F            0x001f // position
#define MODELPART_REAPER_0020            0x0020 // position
#define MODELPART_REAPER_002C            0x002c // position
#define MODELPART_REAPER_002D            0x002d // position
#define MODELPART_REAPER_002E            0x002e // position
#define MODELPART_REAPER_002F            0x002f // position
#define MODELPART_REAPER_0042            0x0042 // toggle
#define MODELPART_REAPER_0050            0x0050 // position
#define MODELPART_REAPER_0051            0x0051 // position
#define MODELPART_REAPER_0052            0x0052 // position
#define MODELPART_REAPER_CARTEJECTPOS1   0x0030 // position
#define MODELPART_REAPER_CARTEJECTPOS2   0x0031 // position
#define MODELPART_REAPER_MAGAZINE1       0x002a // toggle
#define MODELPART_REAPER_MAGAZINE2       0x002b // toggle
#define MODELPART_REMOTEMINE_002A        0x002a // position
#define MODELPART_REMOTEMINE_DETONATOR   0x0029 // toggle
#define MODELPART_REMOTEMINE_MINE        0x0028 // toggle
#define MODELPART_ROBOT_0000             0x0000 // position
#define MODELPART_ROBOT_0001             0x0001 // position
#define MODELPART_ROBOT_0004             0x0004 // toggle
#define MODELPART_ROBOT_LGUNFIRE         0x0003 // gunfire
#define MODELPART_ROBOT_RGUNFIRE         0x0002 // gunfire
#define MODELPART_ROCKETLAUNCHER_0042    0x0042 // toggle
#define MODELPART_ROCKETLAUNCHER_ROCKET  0x0028 // toggle
#define MODELPART_SHOTGUN_0042           0x0042 // toggle
#define MODELPART_SHOTGUN_0050           0x0050 // position
#define MODELPART_SHOTGUN_CART           0x002b // toggle
#define MODELPART_SKEDAR_0000            0x0000 // position
#define MODELPART_SKEDAR_0001            0x0001 // position
#define MODELPART_SKEDAR_EYESCLOSED      0x0005 // toggle
#define MODELPART_SKEDAR_EYESOPEN        0x0004 // toggle
#define MODELPART_SKEDAR_LEFTHAND        0x0003 // position
#define MODELPART_SKEDAR_RIGHTHAND       0x0002 // position
#define MODELPART_SKSHUTTLE_GANGWAY      0x00b7 // toggle
#define MODELPART_SLAYERROCKET_0042      0x0042 // toggle
#define MODELPART_SNIPERRIFLE_0042       0x0042 // toggle
#define MODELPART_SNIPERRIFLE_MAGAZINE1  0x0028 // toggle
#define MODELPART_SNIPERRIFLE_MAGAZINE2  0x0029 // toggle
#define MODELPART_SNIPERRIFLE_SCOPE1     0x002a // position
#define MODELPART_SNIPERRIFLE_SCOPE2     0x002b // position
#define MODELPART_SNIPERRIFLE_SCOPE3     0x002c // position
#define MODELPART_SNIPERRIFLE_SCOPE4     0x002d // position
#define MODELPART_SUPERDRAGON_0042       0x0042 // toggle
#define MODELPART_SUPERDRAGON_0050       0x0050 // position
#define MODELPART_SUPERDRAGON_0051       0x0051 // position
#define MODELPART_SUPERDRAGON_0052       0x0052 // position
#define MODELPART_SUPERDRAGON_MAGAZINE1  0x0028 // toggle
#define MODELPART_SUPERDRAGON_MAGAZINE2  0x0029 // toggle
#define MODELPART_SUPERDRAGON_MAGAZINE3  0x002a // toggle
#define MODELPART_SUPERDRAGON_MAGAZINE4  0x002b // toggle
#define MODELPART_TERMINAL_0000          0x0000 // dl
#define MODELPART_TRANQUILIZER_002B      0x002b // position
#define MODELPART_TRANQUILIZER_0042      0x0042 // toggle
#define MODELPART_TRANQUILIZER_MAGAZINE1 0x0028 // toggle
#define MODELPART_TRANQUILIZER_MAGAZINE2 0x0029 // toggle
#define MODELPART_TRANQUILIZER_MAGAZINE3 0x002a // toggle
#define MODELPART_TRUCK_0005             0x0005 // ?
#define MODELPART_UZI_0050               0x0050 // position
#define MODELPART_UZI_0051               0x0051 // position
#define MODELPART_WINDOWEDDOOR_0000      0x0000 // bbox
#define MODELPART_WINDOWEDDOOR_0001      0x0001 // toggle
#define MODELPART_WINDOWEDDOOR_0002      0x0002 // bbox
#define MODELPART_WINDOWEDDOOR_0003      0x0003 // dl

// Unsure which model types these are for
#define MODELPART_0000 0x0000
#define MODELPART_0001 0x0001
#define MODELPART_0002 0x0002
#define MODELPART_0003 0x0003
#define MODELPART_0010 0x0010
#define MODELPART_0011 0x0011
#define MODELPART_0042 0x0042
#define MODELPART_0065 0x0065
#define MODELPART_0066 0x0066
#define MODELPART_0067 0x0067

#define MODELRENDERFLAG_OPA 1
#define MODELRENDERFLAG_XLU 2

#define MODFILE_GAME    1
#define MODFILE_MPSETUP 2
#define MODFILE_BOSS    4

#define MOVEMODE_WALK     0
#define MOVEMODE_BIKE     3
#define MOVEMODE_GRAB     4
#define MOVEMODE_CUTSCENE 5

#define MP3RESPONSETYPE_NONE        0
#define MP3RESPONSETYPE_ACKNOWLEDGE 1
#define MP3RESPONSETYPE_WHISPER     2
#define MP3RESPONSETYPE_GREETING    3

#define MPBODY_DARK_COMBAT      0x00
#define MPBODY_DARK_TRENCH      0x01
#define MPBODY_DARK_FROCK       0x02
#define MPBODY_DARK_RIPPED      0x03
#define MPBODY_DARK_AF1         0x04
#define MPBODY_DARK_LEATHER     0x05
#define MPBODY_DARK_NEGOTIATOR  0x06
#define MPBODY_DARKWET          0x07
#define MPBODY_DARKAQUALUNG     0x08
#define MPBODY_DARKSNOW         0x09
#define MPBODY_DARKLAB          0x0a
#define MPBODY_THEKING          0x0b
#define MPBODY_ELVIS1           0x0c
#define MPBODY_ELVISWAISTCOAT   0x0d
#define MPBODY_CARRINGTON       0x0e
#define MPBODY_CARREVENINGSUIT  0x0f
#define MPBODY_MRBLONDE         0x10
#define MPBODY_CASSANDRA        0x11
#define MPBODY_TRENT            0x12
#define MPBODY_JONATHAN         0x13
#define MPBODY_CILABTECH        0x14
#define MPBODY_CIFEMTECH        0x15
#define MPBODY_CISOLDIER        0x16
#define MPBODY_DDSHOCK          0x17
#define MPBODY_FEM_GUARD        0x18
#define MPBODY_DD_SECGUARD      0x19
#define MPBODY_DD_GUARD         0x1a
#define MPBODY_DD_SHOCK_INF     0x1b
#define MPBODY_SECRETARY        0x1c
#define MPBODY_OFFICEWORKER     0x1d
#define MPBODY_OFFICEWORKER2    0x1e
#define MPBODY_NEGOTIATOR       0x1f
#define MPBODY_DDSNIPER         0x20
#define MPBODY_G5_GUARD         0x21
#define MPBODY_G5_SWAT_GUARD    0x22
#define MPBODY_CIAGUY           0x23
#define MPBODY_FBIGUY           0x24
#define MPBODY_AREA51GUARD      0x25
#define MPBODY_A51TROOPER       0x26
#define MPBODY_A51AIRMAN        0x27
#define MPBODY_OVERALL          0x28
#define MPBODY_STRIPES          0x29
#define MPBODY_LABTECH          0x2a
#define MPBODY_FEMLABTECH       0x2b
#define MPBODY_DD_LABTECH       0x2c
#define MPBODY_BIOTECH          0x2d
#define MPBODY_ALASKAN_GUARD    0x2e
#define MPBODY_PILOTAF1         0x2f
#define MPBODY_STEWARD          0x30
#define MPBODY_STEWARDESS       0x31
#define MPBODY_STEWARDESS_COAT  0x32
#define MPBODY_PRESIDENT        0x33
#define MPBODY_NSA_LACKEY       0x34
#define MPBODY_PRES_SECURITY    0x35
#define MPBODY_PRESIDENT_CLONE2 0x36
#define MPBODY_PELAGIC_GUARD    0x37
#define MPBODY_MAIAN_SOLDIER    0x38
#define MPBODY_CONNERY          0x39
#define MPBODY_MOORE            0x3a
#define MPBODY_DALTON           0x3b
#define MPBODY_DJBOND           0x3c

#define MPCONFIG_TEMPLE      0x00
#define MPCONFIG_PISTOLS     0x01
#define MPCONFIG_FARSIGHT    0x02
#define MPCONFIG_DRUGGUN     0x03
#define MPCONFIG_SLAYER      0x04
#define MPCONFIG_CLOAKING    0x05
#define MPCONFIG_SLOWMO      0x06
#define MPCONFIG_NOSHIELD    0x07
#define MPCONFIG_BRIEFCASE   0x08
#define MPCONFIG_SKEDARKING  0x09
#define MPCONFIG_FLAGS       0x0a
#define MPCONFIG_SIMULANTS   0x0b
#define MPCONFIG_AUTOMATICS  0x0c
#define MPCONFIG_ROCKETS     0x0d
#define MPCONFIG_CHALLENGE01 0x0e
#define MPCONFIG_CHALLENGE02 0x0f
#define MPCONFIG_CHALLENGE03 0x10
#define MPCONFIG_CHALLENGE04 0x11
#define MPCONFIG_CHALLENGE05 0x12
#define MPCONFIG_CHALLENGE06 0x13
#define MPCONFIG_CHALLENGE07 0x14
#define MPCONFIG_CHALLENGE08 0x15
#define MPCONFIG_CHALLENGE09 0x16
#define MPCONFIG_CHALLENGE10 0x17
#define MPCONFIG_CHALLENGE11 0x18
#define MPCONFIG_CHALLENGE12 0x19
#define MPCONFIG_CHALLENGE13 0x1a
#define MPCONFIG_CHALLENGE14 0x1b
#define MPCONFIG_CHALLENGE15 0x1c
#define MPCONFIG_CHALLENGE16 0x1d
#define MPCONFIG_CHALLENGE17 0x1e
#define MPCONFIG_CHALLENGE18 0x1f
#define MPCONFIG_CHALLENGE19 0x20
#define MPCONFIG_CHALLENGE20 0x21
#define MPCONFIG_CHALLENGE21 0x22
#define MPCONFIG_CHALLENGE22 0x23
#define MPCONFIG_CHALLENGE23 0x24
#define MPCONFIG_CHALLENGE24 0x25
#define MPCONFIG_CHALLENGE25 0x26
#define MPCONFIG_CHALLENGE26 0x27
#define MPCONFIG_CHALLENGE27 0x28
#define MPCONFIG_CHALLENGE28 0x29
#define MPCONFIG_CHALLENGE29 0x2a
#define MPCONFIG_CHALLENGE30 0x2b

#define MPDISPLAYOPTION_HIGHLIGHTPLAYERS 0x01
#define MPDISPLAYOPTION_HIGHLIGHTPICKUPS 0x02
#define MPDISPLAYOPTION_RADAR            0x04
#define MPDISPLAYOPTION_HIGHLIGHTTEAMS   0x08

// Features are things that can be unlocked
#define MPFEATURE_WEAPON_FARSIGHT        0x01
#define MPFEATURE_WEAPON_TRANQUILIZER    0x02
#define MPFEATURE_WEAPON_SUPERDRAGON     0x03
#define MPFEATURE_WEAPON_SLAYER          0x04
#define MPFEATURE_WEAPON_FALCON2SILENCED 0x05
#define MPFEATURE_WEAPON_FALCON2SCOPE    0x06
#define MPFEATURE_WEAPON_MAULER          0x07
#define MPFEATURE_WEAPON_PHOENIX         0x08
#define MPFEATURE_WEAPON_DY357LX         0x09
#define MPFEATURE_WEAPON_CALLISTO        0x0a
#define MPFEATURE_WEAPON_LAPTOPGUN       0x0b
#define MPFEATURE_WEAPON_K7AVENGER       0x0c
#define MPFEATURE_WEAPON_RCP120          0x0d
#define MPFEATURE_WEAPON_SHOTGUN         0x0e
#define MPFEATURE_WEAPON_REAPER          0x0f
#define MPFEATURE_WEAPON_DEVASTATOR      0x10
#define MPFEATURE_WEAPON_CROSSBOW        0x11
#define MPFEATURE_WEAPON_NBOMB           0x12
#define MPFEATURE_WEAPON_PROXIMITYMINE   0x13
#define MPFEATURE_WEAPON_REMOTEMINE      0x14
#define MPFEATURE_WEAPON_XRAYSCANNER     0x15
#define MPFEATURE_WEAPON_SHIELD          0x16
#define MPFEATURE_WEAPON_CLOAKINGDEVICE  0x17
#define MPFEATURE_WEAPON_COMBATBOOST     0x18
#define MPFEATURE_BOTDIFF_HARD           0x19
#define MPFEATURE_BOTDIFF_PERFECT        0x1a
#define MPFEATURE_1B                     0x1b
#define MPFEATURE_BOTDIFF_DARK           0x1c
#define MPFEATURE_SLOWMOTION             0x1d
#define MPFEATURE_ONEHITKILLS            0x1e
#define MPFEATURE_SCENARIO_KOH           0x1f
#define MPFEATURE_SCENARIO_HTB           0x20
#define MPFEATURE_SCENARIO_CTC           0x21
#define MPFEATURE_22                     0x22
#define MPFEATURE_STAGE_CARPARK          0x23
#define MPFEATURE_STAGE_COMPLEX          0x24
#define MPFEATURE_STAGE_WAREHOUSE        0x25
#define MPFEATURE_STAGE_RAVINE           0x26
#define MPFEATURE_STAGE_TEMPLE           0x27
#define MPFEATURE_STAGE_G5BUILDING       0x28
#define MPFEATURE_STAGE_GRID             0x29
#define MPFEATURE_STAGE_FELICITY         0x2a
#define MPFEATURE_STAGE_VILLA            0x2b
#define MPFEATURE_STAGE_SEWERS           0x2c
#define MPFEATURE_STAGE_RUINS            0x2d
#define MPFEATURE_STAGE_BASE             0x2e
#define MPFEATURE_2F                     0x2f
#define MPFEATURE_STAGE_FORTRESS         0x30
#define MPFEATURE_31                     0x31
#define MPFEATURE_CHR_FEMGUARD           0x32
#define MPFEATURE_CHR_OFFICEWORKER       0x33
#define MPFEATURE_CHR_VILLACHRS          0x34
#define MPFEATURE_CHR_TRENT              0x35
#define MPFEATURE_CHR_NSALACKEY          0x36
#define MPFEATURE_CHR_G5                 0x37
#define MPFEATURE_CHR_MRBLONDE           0x38
#define MPFEATURE_CHR_CIAFBI             0x39
#define MPFEATURE_CHR_INFILTRATION       0x3a
#define MPFEATURE_CHR_LABTECH            0x3b
#define MPFEATURE_CHR_BIOTECH            0x3c
#define MPFEATURE_CHR_ELVIS              0x3d
#define MPFEATURE_CHR_ALASKANGUARD       0x3e
#define MPFEATURE_CHR_AF1                0x3f
#define MPFEATURE_8BOTS                  0x40 // also unlocks the 4 Bonds
#define MPFEATURE_CHR_CI                 0x41
#define MPFEATURE_CHR_PRESCLONE          0x42
#define MPFEATURE_CHR_PRESSECURITY       0x43
#define MPFEATURE_CHR_STRIPES            0x44
#define MPFEATURE_CHR_PELAGIC            0x45
#define MPFEATURE_CHR_JOTRENCH           0x46
#define MPFEATURE_CHR_JOSNOW             0x47
#define MPFEATURE_48                     0x48
#define MPFEATURE_49                     0x49
#define MPFEATURE_CHR_DARKSNOW           0x4a
#define MPFEATURE_4B                     0x4b
#define MPFEATURE_CHR_JONATHAN           0x4c
#define MPFEATURE_SCENARIO_PAC           0x4d
#define MPFEATURE_SCENARIO_HTM           0x4e
#define MPFEATURE_WEAPON_LASER           0x4f

#define MPHEAD_DARK_COMBAT  0x00
#define MPHEAD_DARK_FROCK   0x01
#define MPHEAD_DARKAQUA     0x02
#define MPHEAD_DARK_SNOW    0x03
#define MPHEAD_ELVIS        0x04
#define MPHEAD_ELVIS_GOGS   0x05
#define MPHEAD_CARRINGTON   0x06
#define MPHEAD_MRBLONDE     0x07
#define MPHEAD_CASSANDRA    0x08
#define MPHEAD_TRENT        0x09
#define MPHEAD_JONATHAN     0x0a
#define MPHEAD_VD           0x0b
#define MPHEAD_PRESIDENT    0x0c
#define MPHEAD_DDSHOCK      0x0d
#define MPHEAD_BIOTECH      0x0e
#define MPHEAD_DDSNIPER     0x0f
#define MPHEAD_A51FACEPLATE 0x10
#define MPHEAD_SECRETARY    0x11
#define MPHEAD_FEM_GUARD    0x12
#define MPHEAD_FEM_GUARD2   0x13
#define MPHEAD_MAIAN_S      0x14
#define MPHEAD_JON          0x15
#define MPHEAD_BEAU1        0x16
#define MPHEAD_ROSS         0x17
#define MPHEAD_MARK2        0x18
#define MPHEAD_CHRIST       0x19
#define MPHEAD_RUSS         0x1a
#define MPHEAD_DARLING      0x1b
#define MPHEAD_BRIAN        0x1c
#define MPHEAD_JAMIE        0x1d
#define MPHEAD_DUNCAN2      0x1e
#define MPHEAD_KEITH        0x1f
#define MPHEAD_STEVEM       0x20
#define MPHEAD_GRANT        0x21
#define MPHEAD_PENNY        0x22
#define MPHEAD_DAVEC        0x23
#define MPHEAD_JONES        0x24
#define MPHEAD_GRAHAM       0x25
#define MPHEAD_ROBERT       0x26
#define MPHEAD_NEIL2        0x27
#define MPHEAD_SHAUN        0x28
#define MPHEAD_ROBIN        0x29
#define MPHEAD_COOK         0x2a
#define MPHEAD_PRYCE        0x2b
#define MPHEAD_SILKE        0x2c
#define MPHEAD_SMITH        0x2d
#define MPHEAD_GARETH       0x2e
#define MPHEAD_MURCHIE      0x2f
#define MPHEAD_WONG         0x30
#define MPHEAD_CARTER       0x31
#define MPHEAD_TINTIN       0x32
#define MPHEAD_MUNTON       0x33
#define MPHEAD_STAMPER      0x34
#define MPHEAD_PHELPS       0x35
#define MPHEAD_ALEX         0x36
#define MPHEAD_JULIANNE     0x37
#define MPHEAD_LAURA        0x38
#define MPHEAD_EDMCG        0x39
#define MPHEAD_ANKA         0x3a
#define MPHEAD_LESLIE_S     0x3b
#define MPHEAD_MATT_C       0x3c
#define MPHEAD_PEER_S       0x3d
#define MPHEAD_EILEEN_T     0x3e
#define MPHEAD_ANDY_R       0x3f
#define MPHEAD_BEN_R        0x40
#define MPHEAD_STEVE_K      0x41
#define MPHEAD_SANCHEZ      0x42
#define MPHEAD_TIM          0x43
#define MPHEAD_KEN          0x44
#define MPHEAD_EILEEN_H     0x45
#define MPHEAD_SCOTT_H      0x46
#define MPHEAD_JOEL         0x47
#define MPHEAD_GRIFFEY      0x48
#define MPHEAD_MOTO         0x49
#define MPHEAD_WINNER       0x4a

#define MPLOCKTYPE_NONE       0
#define MPLOCKTYPE_LASTWINNER 1
#define MPLOCKTYPE_LASTLOSER  2
#define MPLOCKTYPE_RANDOM     3
#define MPLOCKTYPE_PLAYER     4
#define MPLOCKTYPE_CHALLENGE  5

#define MPOPTION_ONEHITKILLS            0x00000001
#define MPOPTION_TEAMSENABLED           0x00000002
#define MPOPTION_NORADAR                0x00000004
#define MPOPTION_NOAUTOAIM              0x00000008
#define MPOPTION_NOPLAYERHIGHLIGHT      0x00000010
#define MPOPTION_NOPICKUPHIGHLIGHT      0x00000020
#define MPOPTION_SLOWMOTION_ON          0x00000040
#define MPOPTION_SLOWMOTION_SMART       0x00000080
#define MPOPTION_FASTMOVEMENT           0x00000100
#define MPOPTION_DISPLAYTEAM            0x00000200
#define MPOPTION_KILLSSCORE             0x00000400
#define MPOPTION_HTB_HIGHLIGHTBRIEFCASE 0x00000800
#define MPOPTION_HTB_SHOWONRADAR        0x00001000
#define MPOPTION_CTC_SHOWONRADAR        0x00002000
#define MPOPTION_KOH_HILLONRADAR        0x00004000
#define MPOPTION_KOH_MOBILEHILL         0x00008000
#define MPOPTION_00010000               0x00010000
#define MPOPTION_HTM_HIGHLIGHTTERMINAL  0x00020000
#define MPOPTION_HTM_SHOWONRADAR        0x00040000
#define MPOPTION_PAC_HIGHLIGHTTARGET    0x00080000
#define MPOPTION_PAC_SHOWONRADAR        0x00100000
#define MPOPTION_SPAWNWITHWEAPON        0x00200000
#define MPOPTION_NODRUGBLUR             0x00400000
#define MPOPTION_AUTORANDOMWEAPON_START 0x00800000
#define MPOPTION_AUTORANDOMWEAPON_END   0x01000000
#define MPOPTION_FRIENDLYFIRE           0x02000000
#define MPOPTION_NOPLAYERONRADAR        0x04000000
#define MPOPTION_NODOORS                0x08000000

#define MPPAUSEMODE_UNPAUSED 0
#define MPPAUSEMODE_PAUSED   1
#define MPPAUSEMODE_GAMEOVER 2

#define MPPLAYERTITLE_BEGINNER     0
#define MPPLAYERTITLE_TRAINEE      1
#define MPPLAYERTITLE_AMATEUR      2
#define MPPLAYERTITLE_ROOKIE       3
#define MPPLAYERTITLE_NOVICE       4
#define MPPLAYERTITLE_TROOPER      5
#define MPPLAYERTITLE_AGENT        6
#define MPPLAYERTITLE_STARAGENT    7
#define MPPLAYERTITLE_SPECIALAGENT 8
#define MPPLAYERTITLE_EXPERT       9
#define MPPLAYERTITLE_VETERAN      10
#define MPPLAYERTITLE_PROFESSIONAL 11
#define MPPLAYERTITLE_DANGEROUS    12
#define MPPLAYERTITLE_DEADLY       13
#define MPPLAYERTITLE_KILLER       14
#define MPPLAYERTITLE_ASSASSIN     15
#define MPPLAYERTITLE_LETHAL       16
#define MPPLAYERTITLE_ELITE        17
#define MPPLAYERTITLE_INVINCIBLE   18
#define MPPLAYERTITLE_NEARPERFECT  19
#define MPPLAYERTITLE_PERFECT      20

#define MPQUICKTEAM_PLAYERSONLY    0
#define MPQUICKTEAM_PLAYERSANDSIMS 1
#define MPQUICKTEAM_PLAYERSTEAMS   2
#define MPQUICKTEAM_PLAYERSVSSIMS  3
#define MPQUICKTEAM_PLAYERSIMTEAMS 4
#define MPQUICKTEAM_NONE           5

#define MPSCENARIO_COMBAT           0
#define MPSCENARIO_HOLDTHEBRIEFCASE 1
#define MPSCENARIO_HACKERCENTRAL    2
#define MPSCENARIO_POPACAP          3
#define MPSCENARIO_KINGOFTHEHILL    4
#define MPSCENARIO_CAPTURETHECASE   5

#define MPSETUPMENU_ADVSETUP 1
#define MPSETUPMENU_GENERAL  2
#define MPSETUPMENU_QUICKGO  3

#define MPTEAM_0 0
#define MPTEAM_1 1
#define MPTEAM_2 2
#define MPTEAM_3 3
#define MPTEAM_4 4
#define MPTEAM_5 5
#define MPTEAM_6 6
#define MPTEAM_7 7

#define MPWEAPON_NONE             0x00
#define MPWEAPON_FALCON2          0x01
#define MPWEAPON_FALCON2_SILENCER 0x02
#define MPWEAPON_FALCON2_SCOPE    0x03
#define MPWEAPON_MAGSEC4          0x04
#define MPWEAPON_MAULER           0x05
#define MPWEAPON_PHOENIX          0x06
#define MPWEAPON_DY357MAGNUM      0x07
#define MPWEAPON_DY357LX          0x08
#define MPWEAPON_CMP150           0x09
#define MPWEAPON_CYCLONE          0x0a
#define MPWEAPON_CALLISTO         0x0b
#define MPWEAPON_RCP120           0x0c
#define MPWEAPON_LAPTOPGUN        0x0d
#define MPWEAPON_DRAGON           0x0e
#define MPWEAPON_K7AVENGER        0x0f
#define MPWEAPON_AR34             0x10
#define MPWEAPON_SUPERDRAGON      0x11
#define MPWEAPON_SHOTGUN          0x12
#define MPWEAPON_REAPER           0x13
#define MPWEAPON_SNIPERRIFLE      0x14
#define MPWEAPON_FARSIGHT         0x15
#define MPWEAPON_DEVASTATOR       0x16
#define MPWEAPON_ROCKETLAUNCHER   0x17
#define MPWEAPON_SLAYER           0x18
#ifdef PLATFORM_N64
#define MPWEAPON_COMBATKNIFE      (VERSION == VERSION_JPN_FINAL ?    0 : 0x19)
#define MPWEAPON_CROSSBOW         (VERSION == VERSION_JPN_FINAL ? 0x19 : 0x1a)
#define MPWEAPON_TRANQUILIZER     (VERSION == VERSION_JPN_FINAL ? 0x1a : 0x1b)
#define MPWEAPON_GRENADE          (VERSION == VERSION_JPN_FINAL ? 0x1b : 0x1c)
#define MPWEAPON_NBOMB            (VERSION == VERSION_JPN_FINAL ? 0x1c : 0x1d)
#define MPWEAPON_TIMEDMINE        (VERSION == VERSION_JPN_FINAL ? 0x1d : 0x1e)
#define MPWEAPON_PROXIMITYMINE    (VERSION == VERSION_JPN_FINAL ? 0x1e : 0x1f)
#define MPWEAPON_REMOTEMINE       (VERSION == VERSION_JPN_FINAL ? 0x1f : 0x20)
#define MPWEAPON_LASER            (VERSION == VERSION_JPN_FINAL ? 0x20 : 0x21)
#define MPWEAPON_XRAYSCANNER      (VERSION == VERSION_JPN_FINAL ? 0x21 : 0x22)
#define MPWEAPON_CLOAKINGDEVICE   (VERSION == VERSION_JPN_FINAL ? 0x22 : 0x23)
#define MPWEAPON_COMBATBOOST      (VERSION == VERSION_JPN_FINAL ? 0x23 : 0x24)
#define MPWEAPON_SHIELD           (VERSION == VERSION_JPN_FINAL ? 0x24 : 0x25)
#define MPWEAPON_DISABLED         (VERSION == VERSION_JPN_FINAL ? 0x25 : 0x26)
#define NUM_MPWEAPONS             (VERSION == VERSION_JPN_FINAL ? 0x26 : 0x27)
#else // add all classic weapons to multiplayer and allow combat knife in JPN and add IR Scanner and Night Vision
#define MPWEAPON_COMBATKNIFE      0x19
#define MPWEAPON_CROSSBOW         0x1a
#define MPWEAPON_TRANQUILIZER     0x1b
#define MPWEAPON_GRENADE          0x1c
#define MPWEAPON_NBOMB            0x1d
#define MPWEAPON_TIMEDMINE        0x1e
#define MPWEAPON_PROXIMITYMINE    0x1f
#define MPWEAPON_REMOTEMINE       0x20
#define MPWEAPON_LASER            0x21
#define MPWEAPON_XRAYSCANNER      0x22
#define MPWEAPON_NIGHTVISION      0x23
#define MPWEAPON_IRSCANNER        0x24
#define MPWEAPON_CLOAKINGDEVICE   0x25
#define MPWEAPON_COMBATBOOST      0x26
#define MPWEAPON_PP9I             0x27
#define MPWEAPON_CC13             0x28
#define MPWEAPON_KL01313          0x29
#define MPWEAPON_KF7SPECIAL       0x2a
#define MPWEAPON_ZZT              0x2b
#define MPWEAPON_DMC              0x2c
#define MPWEAPON_AR53             0x2d
#define MPWEAPON_RCP45            0x2e
#define MPWEAPON_SHIELD           0x2f
#define MPWEAPON_DISABLED         0x30
#define NUM_MPWEAPONS             0x31
#endif

#define MUSICEVENTTYPE_PLAY        1
#define MUSICEVENTTYPE_STOP        2
#define MUSICEVENTTYPE_FADE        3
#define MUSICEVENTTYPE_STOPALL     4
#define MUSICEVENTTYPE_SETINTERVAL 5

#define OBJECTIVE_INCOMPLETE 0
#define OBJECTIVE_COMPLETE   1
#define OBJECTIVE_FAILED     2

#define OBJECTIVEFLAG_AUTOCOMPLETE 0x01

// obj->flags
#define OBJFLAG_FALL                       0x00000001
#define OBJFLAG_00000002                   0x00000002 // Editor: "In Air Rotated 90 Deg Upside-Down"
#define OBJFLAG_UPSIDEDOWN                 0x00000004
#define OBJFLAG_00000008                   0x00000008 // Editor: "In Air"
#define OBJFLAG_00000010                   0x00000010 // Editor: "Scale to Pad Bounds"
#define OBJFLAG_XTOPADBOUNDS               0x00000020
#define OBJFLAG_YTOPADBOUNDS               0x00000040
#define OBJFLAG_ZTOPADBOUNDS               0x00000080
#define OBJFLAG_00000100                   0x00000100 // G5 mines, Air Base brown door, AF1 grate and escape door, Defense shuttle, Ruins mines, MBR lift door. Editor suggests "Force Collisions" but this seems wrong
#define OBJFLAG_ORTHOGONAL                 0x00000200
#define OBJFLAG_IGNOREFLOORCOLOUR          0x00000400
#define OBJFLAG_PATHBLOCKER                0x00000800 // Glass and explodable scenery which may be blocking a path segment
#define OBJFLAG_IGNOREROOMCOLOUR           0x00001000 // Editor: "Absolute Position"
#define OBJFLAG_AIUNDROPPABLE              0x00002000 // AI cannot drop item
#define OBJFLAG_ASSIGNEDTOCHR              0x00004000
#define OBJFLAG_INSIDEANOTHEROBJ           0x00008000 // Eg. gun inside a crate or suitcase inside a dumpster
#define OBJFLAG_FORCEMORTAL                0x00010000
#define OBJFLAG_INVINCIBLE                 0x00020000
#define OBJFLAG_COLLECTABLE                0x00040000
#define OBJFLAG_THROWNLAPTOP               0x00080000
#define OBJFLAG_UNCOLLECTABLE              0x00100000
#define OBJFLAG_BOUNCEIFSHOT               0x00200000 // Bounce or explode
#define OBJFLAG_FORCENOBOUNCE              0x00400000 // Override the above
#define OBJFLAG_HELDROCKET                 0x00800000 // Rocket obj that's currently in an equipped rocket launcher
#define OBJFLAG_01000000                   0x01000000 // Editor: "Embedded Object"
#define OBJFLAG_CANNOT_ACTIVATE            0x02000000 // Makes it do nothing if player presses B on object. Used mostly for doors.
#define OBJFLAG_AISEETHROUGH               0x04000000 // Glass, glass doors, small objects such as plant pots
#define OBJFLAG_AUTOGUN_3DRANGE            0x08000000 // Autogun uses 3 dimensions when checking if target is in range
#define OBJFLAG_DEACTIVATED                0x10000000
#define OBJFLAG_AMMOCRATE_EXPLODENOW       0x10000000
#define OBJFLAG_DOOR_HASPORTAL             0x10000000
#define OBJFLAG_GLASS_HASPORTAL            0x10000000
#define OBJFLAG_WEAPON_LEFTHANDED          0x10000000
#define OBJFLAG_ESCSTEP_ZALIGNED           0x10000000
#define OBJFLAG_AUTOGUN_SEENTARGET         0x20000000
#define OBJFLAG_CAMERA_DISABLED            0x20000000
#define OBJFLAG_CHOPPER_INIT               0x20000000
#define OBJFLAG_DOOR_OPENTOFRONT           0x20000000
#define OBJFLAG_HOVERCAR_INIT              0x20000000
#define OBJFLAG_HOVERPROP_20000000         0x20000000
#define OBJFLAG_LIFT_LATERALMOVEMENT       0x20000000
#define OBJFLAG_MONITOR_20000000           0x20000000
#define OBJFLAG_WEAPON_AICANNOTUSE         0x20000000
#define OBJFLAG_AUTOGUN_DAMAGED            0x40000000
#define OBJFLAG_CAMERA_BONDINVIEW          0x40000000
#define OBJFLAG_DOOR_KEEPOPEN              0x40000000
#define OBJFLAG_HOVERBIKE_MOVINGWHILEEMPTY 0x40000000
#define OBJFLAG_HOVERCAR_40000000          0x40000000 // Set but not read
#define OBJFLAG_LIFT_TRIGGERDISABLE        0x40000000
#define OBJFLAG_MONITOR_RENDERPOSTBG       0x40000000
#define OBJFLAG_WEAPON_NOAMMO              0x40000000 // Don't give ammo when picking up
#define OBJFLAG_80000000                   0x80000000
#define OBJFLAG_CHOPPER_INACTIVE           0x80000000
#define OBJFLAG_DOOR_TWOWAY                0x80000000 // Door swings in both directions
#define OBJFLAG_HOVERCAR_ISHOVERBOT        0x80000000
#define OBJFLAG_LIFT_CHECKCEILING          0x80000000
#define OBJFLAG_WEAPON_CANMIXDUAL          0x80000000

// obj->flags2
#define OBJFLAG2_IMMUNETOANTI            0x00000001 // Counter-op cannot damage this object
#define OBJFLAG2_BOUNCEIFSHOTWHENDEAD    0x00000002 // Ruins spikes
#define OBJFLAG2_SKIPDOORLOCKEDMSG       0x00000004
#define OBJFLAG2_DOOR_PENDINGACTIVATION  0x00000008 // Editor: "Don't load in Multiplayer"
#define OBJFLAG2_EXCLUDE_A               0x00000010
#define OBJFLAG2_EXCLUDE_SA              0x00000020
#define OBJFLAG2_EXCLUDE_PA              0x00000040
#define OBJFLAG2_EXCLUDE_PD              0x00000080
#define OBJFLAG2_NOFALL                  0x00000100
#define OBJFLAG2_FALLWITHOUTROTATION     0x00000200
#define OBJFLAG2_LINKEDTOSAFE            0x00000400 // Applied to safe door and item
#define OBJFLAG2_INTERACTCHECKLOS        0x00000800 // Check line of sight when attempting to interact with object
#define OBJFLAG2_PICKUPWITHOUTLOS        0x00001000 // Object can be picked up without having line of sight
#define OBJFLAG2_REMOVEWHENDESTROYED     0x00002000
#define OBJFLAG2_IMMUNETOGUNFIRE         0x00004000
#define OBJFLAG2_SHOOTTHROUGH            0x00008000
#define OBJFLAG2_DRAWONTOP               0x00010000
#define OBJFLAG2_DONTPAUSE               0x00020000 // Don't allow prop to pause
#define OBJFLAG2_INVHIDDEN               0x00040000 // Don't show in inventory menu
#define OBJFLAG2_INVISIBLE               0x00080000
#define OBJFLAG2_BULLETPROOF             0x00100000 // Only magnum and FarSight can shoot through it
#define OBJFLAG2_IMMUNETOEXPLOSIONS      0x00200000
#define OBJFLAG2_EXCLUDE_2P              0x00400000
#define OBJFLAG2_EXCLUDE_3P              0x00800000
#define OBJFLAG2_EXCLUDE_4P              0x01000000
#define OBJFLAG2_THROWTHROUGH            0x02000000 // Rockets/mines/grenades etc pass through object
#define OBJFLAG2_CANFILLVIEWPORT         0x04000000
#define OBJFLAG2_LOCKEDFRONT             0x08000000 // One-way door lock
#define OBJFLAG2_LOCKEDBACK              0x10000000 // One-way door lock
#define OBJFLAG2_AICANNOTUSE             0x20000000 // AI can't equip weapon
#define OBJFLAG2_AUTOGUN_MALFUNCTIONING2 0x20000000
#define OBJFLAG2_AIRLOCKDOOR             0x40000000 // Door waits for sibling to close before it can open
#define OBJFLAG2_AUTOGUN_ZEROTOROT       0x40000000 // set xzero/yzero to xrot/yrot when malfunctioning
#define OBJFLAG2_AUTOGUN_MALFUNCTIONING1 0x80000000
#define OBJFLAG2_DEBRIS_80000000         0x80000000
#define OBJFLAG2_DEFAULT_80000000        0x80000000
#define OBJFLAG2_DOOR_ALTCOORDSYSTEM     0x80000000 // Floor grates, but also Temple doors
#define OBJFLAG2_GLASS_80000000          0x80000000
#define OBJFLAG2_HOVERPROP_80000000      0x80000000
#define OBJFLAG2_WEAPON_HUGEEXP          0x80000000

// obj->flags3
#define OBJFLAG3_PUSHABLE           0x00000001
#define OBJFLAG3_GRABBABLE          0x00000002
#define OBJFLAG3_DOOR_STICKY        0x00000004 // eg. Skedar Ruins
#define OBJFLAG3_00000008           0x00000008 // Not used in scripts
#define OBJFLAG3_00000010           0x00000010 // Used heaps
#define OBJFLAG3_AUTOCUTSCENESOUNDS 0x00000020 // For doors and objs - play default open/close noises
#define OBJFLAG3_RTRACKED_YELLOW    0x00000040
#define OBJFLAG3_CANHARDFREE        0x00000080 // Can free prop while on screen (MP weapons only)
#define OBJFLAG3_HARDFREEING        0x00000100
#define OBJFLAG3_00000200           0x00000200 // Not used in scripts
#define OBJFLAG3_WALKTHROUGH        0x00000400
#define OBJFLAG3_RTRACKED_BLUE      0x00000800
#define OBJFLAG3_SHOWSHIELD         0x00001000 // Show shield effect around object (always)
#define OBJFLAG3_HTMTERMINAL        0x00002000 // Terminal for Hacker Central scenario (HTM = Hack That Mac)
#define OBJFLAG3_ISFETCHTARGET      0x00004000 // AI bot is fetching this obj
#define OBJFLAG3_REACTTOSIGHT       0x00008000 // Turn sight blue or red when targeted with R
#define OBJFLAG3_INTERACTABLE       0x00010000
#define OBJFLAG3_SHIELDHIT          0x00020000 // Turns off when shield no longer visible
#define OBJFLAG3_RENDERPOSTBG       0x00040000
#define OBJFLAG3_DRAWONTOP          0x00080000
#define OBJFLAG3_HOVERBEDSHIELD     0x00100000
#define OBJFLAG3_INTERACTSHORTRANGE 0x00200000
#define OBJFLAG3_PLAYERUNDROPPABLE  0x00400000 // Player does not drop item when dead
#define OBJFLAG3_LONGPUSHRANGE      0x00800000 // Not used in scripts
#define OBJFLAG3_PUSHFREELY         0x01000000 // Not used in scripts
#define OBJFLAG3_GEOCYL             0x02000000 // Use cylinder geometry rather than block
#define OBJFLAG3_04000000           0x04000000 // Not used in scripts
#define OBJFLAG3_08000000           0x08000000 // Not used in scripts
#define OBJFLAG3_10000000           0x10000000 // Editor: "Keep Collisions After Fully Destroyed"
#define OBJFLAG3_ONSHELF            0x20000000 // Obj is on a shelf - use bigger pickup range for Small Jo and Play as Elvis cheats and skip line of sight checks
#define OBJFLAG3_INFRARED           0x40000000 // Obj is highlighted on IR scanner
#define OBJFLAG3_80000000           0x80000000 // Not used in scripts

// obj->hidden
#define OBJHFLAG_LIFTDOOR           0x00000001
#define OBJHFLAG_00000002           0x00000002
#define OBJHFLAG_DELETING           0x00000004
#define OBJHFLAG_ISRETICK           0x00000008
#define OBJHFLAG_TAGGED             0x00000010
#define OBJHFLAG_THROWNKNIFE        0x00000020 // thrown knife
#define OBJHFLAG_EMBEDDED           0x00000040 // obj is embedded in a chr or other obj (eg. knife)
#define OBJHFLAG_PROJECTILE         0x00000080 // obj is airborne, sliding or falling
#define OBJHFLAG_DAMAGEFORBOUNCE    0x00000100
#define OBJHFLAG_DOOREVEROPENED     0x00000200
#define OBJHFLAG_HASTEXTOVERRIDE    0x00000400
#define OBJHFLAG_GONE               0x00000800 // Waiting to regen in MP, or grenade in chr's hand during throw anim before it becomes visible
#define OBJHFLAG_00001000           0x00001000
#define OBJHFLAG_PADLOCKEDDOOR      0x00002000
#define OBJHFLAG_ACTIVATED_BY_BOND  0x00004000
#define OBJHFLAG_00008000           0x00008000
#define OBJHFLAG_00010000           0x00010000
#define OBJHFLAG_00020000           0x00020000
#define OBJHFLAG_ACTIVATED_BY_COOP  0x00040000
#define OBJHFLAG_HASOWNER           0x00080000
#define OBJHFLAG_SUSPICIOUS         0x00100000
#define OBJHFLAG_OCCUPIEDCHAIR      0x00200000
#define OBJHFLAG_PERIMDISABLED      0x00400000
#define OBJHFLAG_DOORPERIMDISABLED  0x00800000
#define OBJHFLAG_CONDITIONALSCENERY 0x01000000 // Set on triggers, exp and unexp
#define OBJHFLAG_BLOCKEDPATH        0x02000000
#define OBJHFLAG_MOUNTED            0x04000000 // for hoverbike
#define OBJHFLAG_GRABBED            0x08000000

// obj->hidden2
#define OBJH2FLAG_HASOPA       0x01 // obj has opaque polygons (ie. most objects)
#define OBJH2FLAG_HASXLU       0x02 // obj has translucent polygons
#define OBJH2FLAG_CANREGEN     0x04
#define OBJH2FLAG_08           0x08
#define OBJH2FLAG_10           0x10
#define OBJH2FLAG_DESTROYED    0x40
#define OBJH2FLAG_80           0x80

#define OBJTYPE_DOOR               0x01
#define OBJTYPE_DOORSCALE          0x02
#define OBJTYPE_BASIC              0x03
#define OBJTYPE_KEY                0x04
#define OBJTYPE_ALARM              0x05
#define OBJTYPE_CCTV               0x06
#define OBJTYPE_AMMOCRATE          0x07
#define OBJTYPE_WEAPON             0x08
#define OBJTYPE_CHR                0x09
#define OBJTYPE_SINGLEMONITOR      0x0a
#define OBJTYPE_MULTIMONITOR       0x0b
#define OBJTYPE_HANGINGMONITORS    0x0c
#define OBJTYPE_AUTOGUN            0x0d
#define OBJTYPE_LINKGUNS           0x0e
#define OBJTYPE_DEBRIS             0x0f
#define OBJTYPE_10                 0x10
#define OBJTYPE_HAT                0x11
#define OBJTYPE_GRENADEPROB        0x12
#define OBJTYPE_LINKLIFTDOOR       0x13
#define OBJTYPE_MULTIAMMOCRATE     0x14
#define OBJTYPE_SHIELD             0x15
#define OBJTYPE_TAG                0x16
#define OBJTYPE_BEGINOBJECTIVE     0x17
#define OBJTYPE_ENDOBJECTIVE       0x18
#define OBJECTIVETYPE_DESTROYOBJ   0x19
#define OBJECTIVETYPE_COMPFLAGS    0x1a
#define OBJECTIVETYPE_FAILFLAGS    0x1b
#define OBJECTIVETYPE_COLLECTOBJ   0x1c
#define OBJECTIVETYPE_THROWOBJ     0x1d
#define OBJECTIVETYPE_HOLOGRAPH    0x1e
#define OBJECTIVETYPE_1F           0x1f
#define OBJECTIVETYPE_ENTERROOM    0x20
#define OBJECTIVETYPE_THROWINROOM  0x21
#define OBJTYPE_22                 0x22
#define OBJTYPE_BRIEFING           0x23
#define OBJTYPE_GASBOTTLE          0x24
#define OBJTYPE_RENAMEOBJ          0x25
#define OBJTYPE_PADLOCKEDDOOR      0x26
#define OBJTYPE_TRUCK              0x27
#define OBJTYPE_HELI               0x28
#define OBJTYPE_29                 0x29
#define OBJTYPE_GLASS              0x2a
#define OBJTYPE_SAFE               0x2b
#define OBJTYPE_SAFEITEM           0x2c
#define OBJTYPE_TANK               0x2d
#define OBJTYPE_CAMERAPOS          0x2e
#define OBJTYPE_TINTEDGLASS        0x2f
#define OBJTYPE_LIFT               0x30
#define OBJTYPE_CONDITIONALSCENERY 0x31
#define OBJTYPE_BLOCKEDPATH        0x32
#define OBJTYPE_HOVERBIKE          0x33
#define OBJTYPE_END                0x34
#define OBJTYPE_HOVERPROP          0x35 // Hovercrates and hoverbeds
#define OBJTYPE_FAN                0x36
#define OBJTYPE_HOVERCAR           0x37 // Datadyne cars, Chicago taxi and limo, and hoverbots
#define OBJTYPE_PADEFFECT          0x38
#define OBJTYPE_CHOPPER            0x39 // Extraction hovercopter and A51 interceptors
#define OBJTYPE_MINE               0x3a
#define OBJTYPE_ESCASTEP           0x3b

#define OPERATOR_LESS_THAN    0
#define OPERATOR_GREATER_THAN 1

#define OPTION_FORWARDPITCH     0x0001
#define OPTION_LOOKAHEAD        0x0002
#define OPTION_SIGHTONSCREEN    0x0004
#define OPTION_AUTOAIM          0x0008
#define OPTION_AIMCONTROL       0x0010
#define OPTION_AMMOONSCREEN     0x0020
#define OPTION_SHOWGUNFUNCTION  0x0040
#define OPTION_HEADROLL         0x0080
#define OPTION_0100             0x0100
#define OPTION_ALWAYSSHOWTARGET 0x0200
#define OPTION_SHOWZOOMRANGE    0x0400
#define OPTION_PAINTBALL        0x0800
#define OPTION_ASKEDSAVEPLAYER  0x4000
#define OPTION_SHOWMISSIONTIME  0x8000

#define OUTFIT_DEFAULT      0x00
#define OUTFIT_ELVIS        0x01
#define OUTFIT_TRENT        0x02 // unused
#define OUTFIT_TRENCH       0x03
#define OUTFIT_FROCK_RIPPED 0x04
#define OUTFIT_FROCK        0x05
#define OUTFIT_LEATHER      0x06
#define OUTFIT_DEEPSEA      0x07
#define OUTFIT_WETSUIT      0x08
#define OUTFIT_SNOW         0x09
#define OUTFIT_LAB          0x0a
#define OUTFIT_STEWARDESS   0x0b
#define OUTFIT_MRBLONDE     0x0c
#define OUTFIT_MAIAN        0x0d
#define OUTFIT_NEGOTIATOR   0x0e // unused by scripts

#define PAD_PRESET 9000

#define PADEFFECT_SPARKS     0
#define PADEFFECT_01         1 // Unimplemented
#define PADEFFECT_SPLASH     2
#define PADEFFECT_SPARKS2    3 // Same as PADEFFECT_SPARKS
#define PADEFFECT_SMOKE      4
#define PADEFFECT_OUTROSMOKE 5 // Like smoke, but only shows in cutscenes and only if all objectives are complete

#define PADFIELD_POS    0x0002
#define PADFIELD_LOOK   0x0004
#define PADFIELD_UP     0x0008
#define PADFIELD_NORMAL 0x0010
#define PADFIELD_BBOX   0x0020
#define PADFIELD_ROOM   0x0040
#define PADFIELD_FLAGS  0x0080
#define PADFIELD_LIFT   0x0100

#define PADFLAG_INTPOS          0x0001
#define PADFLAG_UPALIGNTOX      0x0002
#define PADFLAG_UPALIGNTOY      0x0004
#define PADFLAG_UPALIGNTOZ      0x0008
#define PADFLAG_UPALIGNINVERT   0x0010
#define PADFLAG_LOOKALIGNTOX    0x0020
#define PADFLAG_LOOKALIGNTOY    0x0040
#define PADFLAG_LOOKALIGNTOZ    0x0080
#define PADFLAG_LOOKALIGNINVERT 0x0100
#define PADFLAG_HASBBOXDATA     0x0200
#define PADFLAG_AIWAITLIFT      0x0400
#define PADFLAG_AIONLIFT        0x0800
#define PADFLAG_AIWALKDIRECT    0x1000 // walk directly to this pad even if this+1 is in sight (eg. for catwalks)
#define PADFLAG_AIDROP          0x2000
#define PADFLAG_AICROUCH        0x4000
#define PADFLAG_AIIGNOREY       0x8000 // for ledges/drops, AI will advance to next waypoint if they fall over or under it
#define PADFLAG_AIDUCK          0x10000
#define PADFLAG_AIBOTINUSE      0x20000

#define PAK008_00 0
#define PAK008_01 1
#define PAK008_02 2
#define PAK008_03 3
#define PAK008_04 4
#define PAK008_05 5
#define PAK008_06 6
#define PAK008_07 7
#define PAK008_08 8
#define PAK008_09 9
#define PAK008_11 11
#define PAK008_12 12

#define PAK00C_00 0
#define PAK00C_01 1
#define PAK00C_02 2
#define PAK00C_03 3

#define PAKERRORDIALOG_FULL          0
#define PAKERRORDIALOG_CORRUPT       1
#define PAKERRORDIALOG_DEVICEERROR   2
#define PAKERRORDIALOG_GB_UNREADABLE 3
#define PAKERRORDIALOG_DATALOST      4

#define PAKSTATE_NOPAK                 0
#define PAKSTATE_UNPLUGGING            1
#define PAKSTATE_PROBE                 2
#define PAKSTATE_MEM_DISPATCH          3
#define PAKSTATE_MEM_PRE_PREPARE       4
#define PAKSTATE_MEM_PREPARE           5
#define PAKSTATE_MEM_POST_PREPARE      6
#define PAKSTATE_07                    7
#define PAKSTATE_GB_PRE_PREPARE        8
#define PAKSTATE_GB_PREPARE            9
#define PAKSTATE_GB_POST_PREPARE1      10
#define PAKSTATE_READY                 11
#define PAKSTATE_12                    12
#define PAKSTATE_13                    13
#define PAKSTATE_MEM_ENTER_DEVICEERROR 14
#define PAKSTATE_MEM_ENTER_CORRUPT     15
#define PAKSTATE_MEM_ENTER_FULL        16
#define PAKSTATE_17                    17
#define PAKSTATE_18                    18
#define PAKSTATE_MEM_DEVICEERROR       19
#define PAKSTATE_MEM_CORRUPT           20
#define PAKSTATE_MEM_FULL              21
#define PAKSTATE_22                    22
#define PAKSTATE_GB_POST_PREPARE2      24
#define PAKSTATE_GB_POST_PREPARE3      25
#define PAKSTATE_GB_OPEN_UNREADABLE    26
#define PAKSTATE_GB_IDLE_UNREADABLE    27

#define PAK_ERR1_OK                0
#define PAK_ERR1_NOPAK             PFS_ERR_NOPACK        /* no memory card is plugged */
#define PAK_ERR1_NEWPAK            PFS_ERR_NEW_PACK      /* ram pack has been changed to a different one */
#define PAK_ERR1_INCONSISTENT      PFS_ERR_INCONSISTENT  /* need to run Pfschecker */
#define PAK_ERR1_CONTRFAIL         PFS_ERR_CONTRFAIL
#define PAK_ERR1_INVALID           PFS_ERR_INVALID       /* invalid parameter or file not exist */
#define PAK_ERR1_BADDATA           PFS_ERR_BAD_DATA      /* the data read from pack are bad */
#define PAK_ERR1_DATAFULL          PFS_DATA_FULL         /* no free pages on ram pack */
#define PAK_ERR1_DIRFULL           PFS_DIR_FULL          /* no free directories on ram pack */
#define PAK_ERR1_EXIST             PFS_ERR_EXIST         /* file exists */
#define PAK_ERR1_IDFATAL           PFS_ERR_ID_FATAL      /* dead ram pack */
#define PAK_ERR1_DEVICE            PFS_ERR_DEVICE        /* wrong device type */
#define PAK_ERR1_NOGBCART          PFS_ERR_NO_GBCART     /* no gb cartridge (64GB-PAK) */
#define PAK_ERR1_NEWGBCART         PFS_ERR_NEW_GBCART    /* gb cartridge may be changed */
#define PAK_ERR1_EEPROMMISSING     0x80
#define PAK_ERR1_EEPROMREADFAILED  0x81
#define PAK_ERR1_EEPROMWRITEFAILED 0x82
#define PAK_ERR1_EEPROMINVALIDOP   0x83
#define PAK_ERR1_EEPROMINVALIDARG  0x84

#define PAK_ERR2_OK         0  /* no error */
#define PAK_ERR2_NOPAK      1  /* pak is not inserted */
#define PAK_ERR2_BADOFFSET  4  /* offset given is out of range */
#define PAK_ERR2_CHECKSUM   7  /* checksum mismatch */
#define PAK_ERR2_VERSION    9  /* file is for a different version (0 or 1) */
#define PAK_ERR2_CORRUPT    11 /* cache error or file is zero len */
#define PAK_ERR2_INCOMPLETE 15 /* previous write did not complete */

#define PAKFILETYPE_001        0x001 // Unused
#define PAKFILETYPE_BLANK      0x002 // Blank space (ie. deleted file)
#define PAKFILETYPE_TERMINATOR 0x004 // Marks the end of the device's filesystem
#define PAKFILETYPE_CAMERA     0x008 // PerfectHead File
#define PAKFILETYPE_BOSS       0x010 // EEPROM only, one per cart (stores language, MP team names, selected MP soundtracks)
#define PAKFILETYPE_MPPLAYER   0x020 // Combat Simulator Player File
#define PAKFILETYPE_MPSETUP    0x040 // Combat Simulator Settings File
#define PAKFILETYPE_GAME       0x080 // Single Player Agent File
#define PAKFILETYPE_ALL        0x100 // Not really a file type

#define PAKTYPE_NONE          0
#define PAKTYPE_RUMBLE        1
#define PAKTYPE_MEMORY        2
#define PAKTYPE_GAMEBOY       3
#define PAKTYPE_GAMEBOY_ERROR 4

#define PATHFLAG_CIRCULAR 1
#define PATHFLAG_FLYING   2
#define PATHFLAG_INUSE    4

#define PAUSEMODE_UNPAUSED  0
#define PAUSEMODE_PAUSING   1
#define PAUSEMODE_PAUSED    3
#define PAUSEMODE_UNPAUSING 5

#define PDMODEPROP_REACTION 0
#define PDMODEPROP_HEALTH   1
#define PDMODEPROP_DAMAGE   2
#define PDMODEPROP_ACCURACY 3

#define PORTALFLAG_CLOSED    0x01 // Due to door being closed, or player is too far from glass
#define PORTALFLAG_02        0x02
#define PORTALFLAG_FORCEOPEN 0x04 // Glass is destroyed, or door is freed
#define PORTALFLAG_SKIP      0x08 // DD tower exterior - don't bother processing these

#define PRESETANIM_TALK0      0
#define PRESETANIM_TALK1      1
#define PRESETANIM_TALK2      2
#define PRESETANIM_TALK3      3
#define PRESETANIM_TALK4      4
#define PRESETANIM_TALK5      5
#define PRESETANIM_TALK6      6
#define PRESETANIM_TALK7      7
#define PRESETANIM_GUNJAM     254
#define PRESETANIM_TALKRANDOM 255

#define PROFILE_MAINTICK_END     0x10000
#define PROFILE_RSP_END          0x10001
#define PROFILE_RDP_END          0x10002
#define PROFILE_MAINTICK_START   0x20000
#define PROFILE_RDP_START2       0x20002
#define PROFILE_AUDIOFRAME_START 0x30000
#define PROFILE_RSP_START        0x30001
#define PROFILE_RDP_START1       0x40001
#define PROFILE_AUDIOFRAME_END   0x60000

#define PROJECTILEFLAG_AIRBORNE    0x00000001
#define PROJECTILEFLAG_00000002    0x00000002
#define PROJECTILEFLAG_STICKY      0x00000004
#define PROJECTILEFLAG_POWERED     0x00000010 // No gravity
#define PROJECTILEFLAG_00000020    0x00000020
#define PROJECTILEFLAG_LAUNCHING   0x00000080
#define PROJECTILEFLAG_00000100    0x00000100
#define PROJECTILEFLAG_FALLING     0x00000400
#define PROJECTILEFLAG_SLIDING     0x00000800
#define PROJECTILEFLAG_00001000    0x00001000
#define PROJECTILEFLAG_00002000    0x00002000
#define PROJECTILEFLAG_NOTIMELIMIT 0x00004000
#define PROJECTILEFLAG_INROOM      0x00008000
#define PROJECTILEFLAG_00010000    0x00010000
#define PROJECTILEFLAG_LIGHTWEIGHT 0x40000000
#define PROJECTILEFLAG_FREE        0x80000000

#define PROPFLAG_RENDERPOSTBG         0x01
#define PROPFLAG_ONTHISSCREENTHISTICK 0x02
#define PROPFLAG_ENABLED              0x04
#define PROPFLAG_NOTYETTICKED         0x08
#define PROPFLAG_DONTPAUSE            0x10
#define PROPFLAG_DRAWONTOP            0x20
#define PROPFLAG_ONANYSCREENTHISTICK  0x40
#define PROPFLAG_ONANYSCREENPREVTICK  0x80

#define PROPTYPE_OBJ       1
#define PROPTYPE_DOOR      2
#define PROPTYPE_CHR       3
#define PROPTYPE_WEAPON    4
#define PROPTYPE_EYESPY    5
#define PROPTYPE_PLAYER    6
#define PROPTYPE_EXPLOSION 7
#define PROPTYPE_SMOKE     8

#define PSFLAG_FREE         0x0001
#define PSFLAG_REPEATING    0x0002
#define PSFLAG_FORPROP      0x0008
#define PSFLAG_ISMP3        0x0010
#define PSFLAG_HASCONFIGPAN 0x0020
#define PSFLAG_HASCONFIG    0x0040
#define PSFLAG_CUTSCENE     0x0080
#define PSFLAG_FORHUDMSG    0x0200
#define PSFLAG_0400         0x0400
#define PSFLAG_0800         0x0800
#define PSFLAG_FIRSTTICK    0x1000
#define PSFLAG_OUTOFRANGE   0x2000
#define PSFLAG_CHANGINGPAN  0x4000
#define PSFLAG_IGNOREROOMS  0x8000

#define PSFLAG2_RESPONDHELLO 0x0001
#define PSFLAG2_MPPAUSABLE   0x0002
#define PSFLAG2_PRINTABLE    0x0004
#define PSFLAG2_0010         0x0010
#define PSFLAG2_OFFENSIVE    0x0020
#define PSFLAG2_0040         0x0040
#define PSFLAG2_STOPPED      0x0080

#define PSTYPE_NONE        0
#define PSTYPE_GENERAL     1
#define PSTYPE_CHOPPERHUM1 5
#define PSTYPE_CHOPPERHUM2 6
#define PSTYPE_CHOPPERGUN  7
#define PSTYPE_CHRTALK     9
#define PSTYPE_MARKER      11
#define PSTYPE_DOOR        12
#define PSTYPE_CHRCHOKE    13
#define PSTYPE_COMMHUB     15
#define PSTYPE_FOOTSTEP    16
#define PSTYPE_CHRSHOOT    17

#define QUADRANT_BACK           0x01
#define QUADRANT_SIDE1          0x02
#define QUADRANT_SIDE2          0x04
#define QUADRANT_FRONT          0x08
#define QUADRANT_TOWARDSTARGET  0x10
#define QUADRANT_AWAYFROMTARGET 0x20

#define QUIP_ATTACK1            0
#define QUIP_ATTACK2            1
#define QUIP_GOTOCOVER1         2
#define QUIP_GRENADE1           3
#define QUIP_RETREAT1           4
#define QUIP_GRENADE2           5
#define QUIP_RETREAT2           6
#define QUIP_FLANK              7
#define QUIP_SURRENDER          8
#define QUIP_HEARNOISE          9
#define QUIP_10                 10
#define QUIP_SEEPLAYER          11
#define QUIP_SHOTUNALERT        12
#define QUIP_INJURED1           13
#define QUIP_INJURED2           14
#define QUIP_KILLEDPLAYER1      15
#define QUIP_WARNFRIENDS        16
#define QUIP_17                 17
#define QUIP_SURPRISED          18
#define QUIP_INSPECTBODY        19
#define QUIP_20                 20
#define QUIP_HITPLAYER          21
#define QUIP_MISSEDPLAYER1      22
#define QUIP_MISSEDPLAYER2      23
#define QUIP_GOTOCOVER2         24
#define QUIP_DIE                25
#define QUIP_26                 26
#define QUIP_SEARCHSUCCESS      27
#define QUIP_SEEEYESPY          28
#define QUIP_GREETING           29
#define QUIP_ASKWEAPON1         30
#define QUIP_ASKWEAPON2         31
#define QUIP_UNCOVERDISGUISE1   32
#define QUIP_LOSTGUN            33
#define QUIP_GOFORALARM         34
#define QUIP_SEARCHFAIL         35
#define QUIP_ATTACK3            36
#define QUIP_ATTACK4            37
#define QUIP_KILLEDPLAYER2      38
#define QUIP_GUNJAMMED          39
#define QUIP_UNCOVEREDDISGUISE2 40

#define RACE_HUMAN    0
#define RACE_SKEDAR   1
#define RACE_DRCAROLL 2
#define RACE_EYESPY   3
#define RACE_ROBOT    4

#define RENDERPASS_OPA_PREBG  0
#define RENDERPASS_XLU        1
#define RENDERPASS_OPA_POSTBG 2

#define RESETTYPE_COLD 0
#define RESETTYPE_WARM 1

#define ROOMBLOCKTYPE_LEAF   0
#define ROOMBLOCKTYPE_PARENT 1

#define ROOMFLAG_DISABLEDBYSCRIPT      0x0001
#define ROOMFLAG_HASDYNTEX             0x0002 // Has dynamic textures such as water
#define ROOMFLAG_ONSCREEN              0x0004
#define ROOMFLAG_STANDBY               0x0008 // Neighbour of an onscreen room - usually loaded as well
#define ROOMFLAG_COMPLICATEDPORTALS    0x0010 // Room has portals that aren't on the room's bounding box
#define ROOMFLAG_LOADCANDIDATE         0x0020 // Room is a good candidate for loading on this tick
#define ROOMFLAG_BRIGHTNESS_CALCED     0x0040
#define ROOMFLAG_RENDERALWAYS          0x0080
#define ROOMFLAG_LIGHTS_DIRTY          0x0100
#define ROOMFLAG_BRIGHTNESS_DIRTY_PERM 0x0200
#define ROOMFLAG_BRIGHTNESS_DIRTY_TEMP 0x0400
#define ROOMFLAG_BBOXHACK              0x0800
#define ROOMFLAG_NEEDRESHADE           0x1000
#define ROOMFLAG_LIGHTSOFF             0x2000
#define ROOMFLAG_PLAYAMBIENTTRACK      0x4000
#define ROOMFLAG_OUTDOORS              0x8000

// flags for the extra_flags field
#define ROOMFLAG_EX_WEATHERPROOF       0x0001

#define RUMBLESTATE_1                 1
#define RUMBLESTATE_ENABLED_STOPPED   2
#define RUMBLESTATE_ENABLED_STARTING  3
#define RUMBLESTATE_ENABLED_RUMBLING  4
#define RUMBLESTATE_ENABLED_STOPPING  5
#define RUMBLESTATE_DISABLED_STOPPING 6
#define RUMBLESTATE_DISABLED_STOPPED  7
#define RUMBLESTATE_ENABLING          8

#define SAVEDEVICE_CONTROLLERPAK1 0
#define SAVEDEVICE_CONTROLLERPAK2 1
#define SAVEDEVICE_CONTROLLERPAK3 2
#define SAVEDEVICE_CONTROLLERPAK4 3
#define SAVEDEVICE_GAMEPAK        4
#define SAVEDEVICE_INVALID        5

// These constants are used when the game references 320 or 240 regardless of region.
// They are sometimes used for things like scissor dimensions, despite never rendering
// to these dimensions.
#define SCREEN_320 320
#define SCREEN_240 240

// The framebuffer allocation sizes are mostly the same between versions.
// Note that the height is the same regardless of lo/hi-res, but we use separate constants.
// FBALLOC_HEIGHT (without the LO/HI suffix) is used when the game references this value
// without knowing the resolution.
#define FBALLOC_WIDTH_LO  320
#define FBALLOC_HEIGHT_LO 220
#define FBALLOC_WIDTH_HI  640
#define FBALLOC_HEIGHT_HI 220
#define FBALLOC_HEIGHT    220

// Dimensions used for rendering
#if VERSION == VERSION_PAL_FINAL
#define SCREEN_WIDTH_LO  320
#define SCREEN_HEIGHT_LO 266
#define SCREEN_WIDTH_HI  448
#define SCREEN_HEIGHT_HI 266
#elif VERSION == VERSION_PAL_BETA
#define SCREEN_WIDTH_LO  320
#define SCREEN_HEIGHT_LO 252
#define SCREEN_WIDTH_HI  448
#define SCREEN_HEIGHT_HI 252
#else
#define SCREEN_WIDTH_LO  320
#define SCREEN_HEIGHT_LO 220
#define SCREEN_WIDTH_HI  640
#define SCREEN_HEIGHT_HI 220
#endif

#define SCREEN_ASPECT ((f32)SCREEN_WIDTH_LO / (f32)SCREEN_HEIGHT_LO)

#define SCREENRATIO_NORMAL 0
#define SCREENRATIO_16_9   1

#define SCREENSIZE_FULL   0
#define SCREENSIZE_WIDE   1
#define SCREENSIZE_CINEMA 2

#define SCREENSPLIT_HORIZONTAL 0
#define SCREENSPLIT_VERTICAL   1

#define SHADEMODE_XLU  0
#define SHADEMODE_FRAC 1
#define SHADEMODE_OPA  2

#define SHARDTYPE_GLASS  0
#define SHARDTYPE_BOTTLE 1
#define SHARDTYPE_WOOD   2

#define SHOTREGION_TOTAL  0
#define SHOTREGION_HEAD   1
#define SHOTREGION_BODY   2
#define SHOTREGION_LIMB   3
#define SHOTREGION_GUN    4
#define SHOTREGION_HAT    5
#define SHOTREGION_OBJECT 6

#define SIGHT_DEFAULT  0
#define SIGHT_CLASSIC  1
#define SIGHT_2        2
#define SIGHT_3        3
#define SIGHT_SKEDAR   4
#define SIGHT_ZOOM     5
#define SIGHT_MAIAN    6
#define SIGHT_NONE     7

#define SIGHTTRACKTYPE_NONE           0
#define SIGHTTRACKTYPE_DEFAULT        1
#define SIGHTTRACKTYPE_BETASCANNER    2
#define SIGHTTRACKTYPE_ROCKETLAUNCHER 3
#define SIGHTTRACKTYPE_FOLLOWLOCKON   4
#define SIGHTTRACKTYPE_THREATDETECTOR 5

#define SKEL_BASIC          0x02
#define SKEL_CHRGUN         0x03
#define SKEL_JOYPAD         0x04
#define SKEL_UZI            0x05
#define SKEL_06             0x06
#define SKEL_CLASSICGUN     0x07
#define SKEL_CASING         0x08
#define SKEL_CHR            0x09
#define SKEL_0A             0x0a
#define SKEL_0B             0x0b
#define SKEL_0C             0x0c
#define SKEL_HEAD           0x0d
#define SKEL_CCTV           0x0f
#define SKEL_WINDOWEDDOOR   0x10
#define SKEL_11             0x11
#define SKEL_12             0x12
#define SKEL_13             0x13
#define SKEL_TERMINAL       0x14
#define SKEL_CIHUB          0x15
#define SKEL_AUTOGUN        0x16
#define SKEL_17             0x17
#define SKEL_18             0x18
#define SKEL_19             0x19
#define SKEL_LIFT           0x1b
#define SKEL_SKEDAR         0x1c
#define SKEL_LOGO           0x1d
#define SKEL_PDLOGO         0x1e
#define SKEL_HOVERBIKE      0x1f
#define SKEL_20             0x20
#define SKEL_21             0x21
#define SKEL_22             0x22
#define SKEL_LAPTOPGUN      0x23
#define SKEL_K7AVENGER      0x24
#define SKEL_FALCON2        0x25
#define SKEL_KNIFE          0x26
#define SKEL_CHOPPER        0x27
#define SKEL_DRCAROLL       0x28
#define SKEL_ROPE           0x29
#define SKEL_HUDPIECE       0x2a
#define SKEL_RARELOGO       0x2c
#define SKEL_JUMPSHIP       0x32
#define SKEL_BANNER         0x33
#define SKEL_ROBOT          0x34
#define SKEL_CABLECAR       0x3c
#define SKEL_SUBMARINE      0x43
#define SKEL_MAIANUFO       0x44
#define SKEL_SKSHUTTLE      0x45
#define SKEL_CMP150         0x46
#define SKEL_DRAGON         0x47
#define SKEL_SUPERDRAGON    0x48
#define SKEL_ROCKET         0x49
#define SKEL_4A             0x4a
#define SKEL_SHOTGUN        0x4b
#define SKEL_FARSIGHT       0x4c
#define SKEL_4D             0x4d
#define SKEL_REAPER         0x4e
#define SKEL_DROPSHIP       0x4f
#define SKEL_MAULER         0x64
#define SKEL_DEVASTATOR     0x65
#define SKEL_PISTOL         0x66
#define SKEL_AR34           0x67
#define SKEL_MAGNUM         0x68
#define SKEL_SLAYERROCKET   0x69
#define SKEL_CYCLONE        0x6a
#define SKEL_SNIPERRIFLE    0x6b
#define SKEL_TRANQUILIZER   0x6c
#define SKEL_CROSSBOW       0x6d
#define SKEL_TIMEDPROXYMINE 0x6e
#define SKEL_PHOENIX        0x6f
#define SKEL_CALLISTO       0x70
#define SKEL_RCP120         0x71
#define SKEL_LASER          0x72
#define SKEL_GRENADE        0x73
#define SKEL_ECMMINE        0x74
#define SKEL_UPLINK         0x75
#define SKEL_REMOTEMINE     0x76
#define SKEL_TARGET         0x8c
#define SKEL_BB             0xbb
#define SKEL_WIREFENCE      0xc8
#define SKEL_HAND           0xfa

#define SKJUMPSTATE_TAKEOFF  0
#define SKJUMPSTATE_AIRBORNE 1

#define SLOWMOTION_OFF   0
#define SLOWMOTION_ON    1
#define SLOWMOTION_SMART 2

#define AUTORANDOMWEAPON_OFF   0
#define AUTORANDOMWEAPON_START 1
#define AUTORANDOMWEAPON_END   2

#define SMOKETYPE_NONE             0
#define SMOKETYPE_ELECTRICAL       1 // Dr Caroll, mainframes in Infiltration bunker
#define SMOKETYPE_MINI             2 // Phoenix, Laptop sentry
#define SMOKETYPE_3                3
#define SMOKETYPE_SMALL            4 // SuperDragon grenades, some barrels and autoguns
#define SMOKETYPE_MEDIUM           5 // Maian SOS hoverbed
#define SMOKETYPE_LARGE            6 // Rockets, mines etc
#define SMOKETYPE_BULLETIMPACT     7
#define SMOKETYPE_ROCKETTAIL       8
#define SMOKETYPE_GRENADETAIL      9
#define SMOKETYPE_HOMINGTAIL       11
#define SMOKETYPE_SKCORPSE         13
#define SMOKETYPE_14               14
#define SMOKETYPE_MUZZLE_PISTOL    15
#define SMOKETYPE_MUZZLE_REAPER    16
#define SMOKETYPE_MUZZLE_AUTOMATIC 17
#define SMOKETYPE_MUZZLE_SHOTGUN   18
#define SMOKETYPE_PINBALL          19 // Grenade in proxy pinball mode
#define SMOKETYPE_WATER            20 // Splash effect in Chicago
#define SMOKETYPE_DEBRIS           21 // Crash site cockpit, Skedar Ruins outro
#define SMOKETYPE_UFO              22 // AF1 outro

#define SNDSTATEFLAG_01 0x01
#define SNDSTATEFLAG_02 0x02
#define SNDSTATEFLAG_04 0x04
#define SNDSTATEFLAG_10 0x10
#define SNDSTATEFLAG_20 0x20

#define SOLOSTAGEINDEX_DEFECTION     0x00
#define SOLOSTAGEINDEX_INVESTIGATION 0x01
#define SOLOSTAGEINDEX_EXTRACTION    0x02
#define SOLOSTAGEINDEX_VILLA         0x03
#define SOLOSTAGEINDEX_CHICAGO       0x04
#define SOLOSTAGEINDEX_G5BUILDING    0x05
#define SOLOSTAGEINDEX_INFILTRATION  0x06
#define SOLOSTAGEINDEX_RESCUE        0x07
#define SOLOSTAGEINDEX_ESCAPE        0x08
#define SOLOSTAGEINDEX_AIRBASE       0x09
#define SOLOSTAGEINDEX_AIRFORCEONE   0x0a
#define SOLOSTAGEINDEX_CRASHSITE     0x0b
#define SOLOSTAGEINDEX_PELAGIC       0x0c
#define SOLOSTAGEINDEX_DEEPSEA       0x0d
#define SOLOSTAGEINDEX_DEFENSE       0x0e
#define SOLOSTAGEINDEX_ATTACKSHIP    0x0f
#define SOLOSTAGEINDEX_SKEDARRUINS   0x10
#define SOLOSTAGEINDEX_MBR           0x11
#define SOLOSTAGEINDEX_MAIANSOS      0x12
#define SOLOSTAGEINDEX_WAR           0x13
#define SOLOSTAGEINDEX_DUEL          0x14

#define SOUNDMODE_MONO      0
#define SOUNDMODE_STEREO    1
#define SOUNDMODE_HEADPHONE 2
#define SOUNDMODE_SURROUND  3

#define SPARKTYPE_DEFAULT           0x00
#define SPARKTYPE_ELECTRICAL        0x01
#define SPARKTYPE_BLOOD             0x02
#define SPARKTYPE_FLESH             0x03
#define SPARKTYPE_FLESH_LARGE       0x04
#define SPARKTYPE_ENVIRONMENTAL1    0x09
#define SPARKTYPE_ENVIRONMENTAL2    0x0a
#define SPARKTYPE_ENVIRONMENTAL3    0x0b
#define SPARKTYPE_ENVIRONMENTAL4    0x0c
#define SPARKTYPE_ENVIRONMENTAL5    0x0d
#define SPARKTYPE_SHALLOWWATER      0x0f
#define SPARKTYPE_PROJECTILE        0x10
#define SPARKTYPE_LIGHT1            0x11
#define SPARKTYPE_LIGHT2            0x12
#define SPARKTYPE_LIGHT3            0x13
#define SPARKTYPE_LIGHT4            0x14
#define SPARKTYPE_BGHIT_ORANGE      0x16
#define SPARKTYPE_BGHIT_GREEN       0x17
#define SPARKTYPE_BGHIT_TRANQULIZER 0x18
#define SPARKTYPE_PAINT             0x19
#define SPARKTYPE_DEEPWATER         0x1a

#define SPAWNFLAG_FORCESUNGLASSES     0x00000001 // 100% chance of wearing sunglasses if head model supports it
#define SPAWNFLAG_MAYBESUNGLASSES     0x00000002 // 50% chance of wearing sunglasses if head model supports it
#define SPAWNFLAG_INVINCIBLE          0x00000008
#define SPAWNFLAG_ALLOWONSCREEN       0x00000010
#define SPAWNFLAG_ONLYONA             0x00000020
#define SPAWNFLAG_ONLYONSA            0x00000040
#define SPAWNFLAG_ONLYONPA            0x00000080
#define SPAWNFLAG_IGNORECOLLISION     0x00000100 // For initial chr placement only
#define SPAWNFLAG_BASICGUARD          0x00000200 // Can be used for anti, have movement stopped during high lag, and warped out of lift doorways
#define SPAWNFLAG_ANTINONINTERACTABLE 0x00000400
#define SPAWNFLAG_DONTSHOOTME         0x00000800 // AI buddy must use melee attacks on this chr rather than shoot
#define SPAWNFLAG_HIDDEN              0x00001000
#define SPAWNFLAG_NOBLOOD             0x00002000
#define SPAWNFLAG_FIXEDHEIGHT         0x00004000 // By default, chr heights vary slightly. This disables it.
#define SPAWNFLAG_RTRACKED            0x00008000
#define SPAWNFLAG_BLUESIGHT           0x00010000

#define SPEAKERMODE_MONO      1
#define SPEAKERMODE_STEREO    2
#define SPEAKERMODE_HEADPHONE 3
#define SPEAKERMODE_SURROUND  4

#define SPECIALDIE_NONE        0
#define SPECIALDIE_FALLBACK    1
#define SPECIALDIE_ROLL1       2
#define SPECIALDIE_ROLL2       3
#define SPECIALDIE_ROLL3       4
#define SPECIALDIE_OVERRAILING 5
#define SPECIALDIE_ONCHAIR     6

#define SPSEGMENT_PHYSICAL   0
#define SPSEGMENT_GETITLE    2
#define SPSEGMENT_MODEL_MTX  3
#define SPSEGMENT_MODEL_VTX  4
#define SPSEGMENT_MODEL_COL1 5
#define SPSEGMENT_MODEL_COL2 6
#define SPSEGMENT_BG_COL     13
#define SPSEGMENT_BG_VTX     14
#define SPSEGMENT_BG_DL      15

#define SQUADRON_00 0x00
#define SQUADRON_01 0x01
#define SQUADRON_02 0x02
#define SQUADRON_03 0x03
#define SQUADRON_04 0x04
#define SQUADRON_05 0x05
#define SQUADRON_06 0x06
#define SQUADRON_07 0x07
#define SQUADRON_08 0x08
#define SQUADRON_09 0x09
#define SQUADRON_0A 0x0a
#define SQUADRON_0B 0x0b
#define SQUADRON_0C 0x0c
#define SQUADRON_0D 0x0d
#define SQUADRON_0E 0x0e
#define SQUADRON_0F 0x0f

#define STACKSIZE_RMON  0x300
#define STACKSIZE_IDLE  0x40
#define STACKSIZE_SCHED 0x400
#define STACKSIZE_MAIN  0x9800
#define STACKSIZE_AUDIO 0x1000
#define STACKSIZE_FAULT 0x2000
#define STACKSIZE_RESET 0x100

#define STACK_START (0x80400000 \
		- STACKSIZE_MAIN \
		- STACKSIZE_IDLE \
		- STACKSIZE_RMON \
		- STACKSIZE_SCHED \
		- STACKSIZE_AUDIO \
		- (VERSION >= VERSION_NTSC_1_0 ? 8 : 0))

#define STAGE_MP_RANDOM     0x01
#define STAGE_MAIANSOS      0x09
#define STAGE_TEST_SILO     0x14
#define STAGE_WAR           0x16
#define STAGE_MP_RAVINE     0x17
#define STAGE_TEST_ARCH     0x18
#define STAGE_ESCAPE        0x19
#define STAGE_TEST_DEST     0x1a
#define STAGE_RETAKING      0x1b
#define STAGE_CRASHSITE     0x1c
#define STAGE_CHICAGO       0x1d
#define STAGE_G5BUILDING    0x1e
#define STAGE_MP_COMPLEX    0x1f
#define STAGE_MP_G5BUILDING 0x20
#define STAGE_PELAGIC       0x21
#define STAGE_EXTRACTION    0x22
#define STAGE_TEST_RUN      0x23
#define STAGE_24            0x24
#define STAGE_MP_TEMPLE     0x25
#define STAGE_CITRAINING    0x26
#define STAGE_AIRBASE       0x27
#define STAGE_28            0x28
#define STAGE_MP_PIPES      0x29
#define STAGE_SKEDARRUINS   0x2a
#define STAGE_2B            0x2b
#define STAGE_VILLA         0x2c
#define STAGE_DEFENSE       0x2d
#define STAGE_TEST_ASH      0x2e
#define STAGE_INFILTRATION  0x2f
#define STAGE_DEFECTION     0x30
#define STAGE_AIRFORCEONE   0x31
#define STAGE_MP_SKEDAR     0x32
#define STAGE_INVESTIGATION 0x33
#define STAGE_ATTACKSHIP    0x34
#define STAGE_RESCUE        0x35
#define STAGE_TEST_LEN      0x36
#define STAGE_MBR           0x37
#define STAGE_DEEPSEA       0x38
#define STAGE_MP_BASE       0x39
#define STAGE_TEST_MP2      0x3a
#define STAGE_MP_AREA52     0x3b
#define STAGE_MP_WAREHOUSE  0x3c
#define STAGE_MP_CARPARK    0x3d
#define STAGE_TEST_MP6      0x3e
#define STAGE_TEST_MP7      0x3f
#define STAGE_TEST_MP8      0x40
#define STAGE_MP_RUINS      0x41
#define STAGE_MP_SEWERS     0x42
#define STAGE_MP_FELICITY   0x43
#define STAGE_MP_FORTRESS   0x44
#define STAGE_MP_VILLA      0x45
#define STAGE_TEST_MP14     0x46
#define STAGE_MP_GRID       0x47
#define STAGE_TEST_MP16     0x48
#define STAGE_TEST_MP17     0x49
#define STAGE_TEST_MP18     0x4a
#define STAGE_TEST_MP19     0x4b
#define STAGE_TEST_MP20     0x4c
#define STAGE_TEST_UFF      0x4d
#define STAGE_TEST_OLD      0x4e
#define STAGE_DUEL          0x4f
#define STAGE_TEST_LAM      0x50
#define STAGE_TITLE         0x5a
#define STAGE_BOOTPAKMENU   0x5b
#define STAGE_CREDITS       0x5c
#define STAGE_4MBMENU       0x5d

#define STAGEFLAG_CI_IN_TRAINING            0x00000001
#define STAGEFLAG_CI_HOLO_FAILED            0x00000002
#define STAGEFLAG_CI_DEVICE_ABORTING        0x00000004
#define STAGEFLAG_CI_TRIGGER_DEVICE_SUCCESS 0x00000008
#define STAGEFLAG_CI_TRIGGER_DEVICE_FAILURE 0x00000010
#define STAGEFLAG_CI_HOLO_ABORTING          0x00000020
#define STAGEFLAG_CI_TRIGGER_HOLO_SUCCESS   0x00000040
#define STAGEFLAG_CI_TRIGGER_HOLO_FAILURE   0x00000080
#define STAGEFLAG_CI_TRIGGER_UPLINK         0x00000100
#define STAGEFLAG_CI_TRIGGER_ECMMINE        0x00000200
#define STAGEFLAG_CI_TRIGGER_CAMSPY         0x00000400
#define STAGEFLAG_CI_TRIGGER_NIGHTVISION    0x00000800
#define STAGEFLAG_CI_TRIGGER_DOORDECODER    0x00001000
#define STAGEFLAG_CI_TRIGGER_RTRACKER       0x00002000
#define STAGEFLAG_CI_TRIGGER_IR             0x00004000
#define STAGEFLAG_CI_TRIGGER_XRAY           0x00008000
#define STAGEFLAG_CI_TRIGGER_DISGUISE       0x00010000
#define STAGEFLAG_CI_TRIGGER_CLOAK          0x00020000
#define STAGEFLAG_CI_IN_HOLO1               0x00040000
#define STAGEFLAG_CI_IN_HOLO2               0x00080000
#define STAGEFLAG_CI_IN_HOLO3               0x00100000
#define STAGEFLAG_CI_IN_HOLO4               0x00200000
#define STAGEFLAG_CI_IN_HOLO5               0x00400000
#define STAGEFLAG_CI_IN_HOLO6               0x00800000
#define STAGEFLAG_CI_IN_HOLO7               0x01000000
#define STAGEFLAG_CI_GENERAL_PURPOSE        0x02000000
#define STAGEFLAG_EYESPY_DESTROYED          0x00002000

#define STAGEINDEX_MAIANSOS      0x00
#define STAGEINDEX_TEST_SILO     0x01
#define STAGEINDEX_WAR           0x02
#define STAGEINDEX_MP_RAVINE     0x03
#define STAGEINDEX_TEST_ARCH     0x04
#define STAGEINDEX_ESCAPE        0x05
#define STAGEINDEX_TEST_DEST     0x06
#define STAGEINDEX_RETAKING      0x07
#define STAGEINDEX_CRASHSITE     0x08
#define STAGEINDEX_CHICAGO       0x09
#define STAGEINDEX_G5BUILDING    0x0a
#define STAGEINDEX_MP_COMPLEX    0x0b
#define STAGEINDEX_MP_G5BUILDING 0x0c
#define STAGEINDEX_PELAGIC       0x0d
#define STAGEINDEX_EXTRACTION    0x0e
#define STAGEINDEX_TEST_RUN      0x0f
#define STAGEINDEX_MP_TEMPLE     0x11
#define STAGEINDEX_CITRAINING    0x12
#define STAGEINDEX_AIRBASE       0x13
#define STAGEINDEX_MP_PIPES      0x15
#define STAGEINDEX_SKEDARRUINS   0x16
#define STAGEINDEX_VILLA         0x18
#define STAGEINDEX_DEFENSE       0x19
#define STAGEINDEX_TEST_ASH      0x1a
#define STAGEINDEX_INFILTRATION  0x1b
#define STAGEINDEX_DEFECTION     0x1c
#define STAGEINDEX_AIRFORCEONE   0x1d
#define STAGEINDEX_MP_SKEDAR     0x1e
#define STAGEINDEX_INVESTIGATION 0x1f
#define STAGEINDEX_ATTACKSHIP    0x20
#define STAGEINDEX_RESCUE        0x21
#define STAGEINDEX_TEST_LEN      0x22
#define STAGEINDEX_MBR           0x23
#define STAGEINDEX_DEEPSEA       0x24
#define STAGEINDEX_TEST_UFF      0x25
#define STAGEINDEX_TEST_OLD      0x26
#define STAGEINDEX_DUEL          0x27
#define STAGEINDEX_TEST_LAM      0x28
#define STAGEINDEX_MP_BASE       0x29
#define STAGEINDEX_TEST_MP2      0x2a
#define STAGEINDEX_MP_AREA52     0x2b
#define STAGEINDEX_MP_WAREHOUSE  0x2c
#define STAGEINDEX_MP_CARPARK    0x2d
#define STAGEINDEX_TEST_MP6      0x2e
#define STAGEINDEX_TEST_MP7      0x2f
#define STAGEINDEX_TEST_MP8      0x30
#define STAGEINDEX_MP_RUINS      0x31
#define STAGEINDEX_MP_SEWERS     0x32
#define STAGEINDEX_MP_FELICITY   0x33
#define STAGEINDEX_MP_FORTRESS   0x34
#define STAGEINDEX_MP_VILLA      0x35
#define STAGEINDEX_TEST_MP14     0x36
#define STAGEINDEX_MP_GRID       0x37
#define STAGEINDEX_TEST_MP16     0x38
#define STAGEINDEX_TEST_MP17     0x39
#define STAGEINDEX_TEST_MP18     0x3a
#define STAGEINDEX_TEST_MP19     0x3b
#define STAGEINDEX_TEST_MP20     0x3c

#define SURFACETYPE_DEFAULT      0
#define SURFACETYPE_STONE        1
#define SURFACETYPE_WOOD         2
#define SURFACETYPE_METAL        3
#define SURFACETYPE_GLASS        4
#define SURFACETYPE_SHALLOWWATER 5
#define SURFACETYPE_SNOW         6
#define SURFACETYPE_DIRT         7
#define SURFACETYPE_MUD          8
#define SURFACETYPE_TILE         9
#define SURFACETYPE_METALOBJ     10
#define SURFACETYPE_CHR          11
#define SURFACETYPE_GLASSXLU     12
#define SURFACETYPE_NONE         13
#define SURFACETYPE_DEEPWATER    14

#define TEAM_00        0x00
#define TEAM_01        0x01
#define TEAM_ENEMY     0x02
#define TEAM_04        0x04
#define TEAM_ALLY      0x10
#define TEAM_20        0x20
#define TEAM_22        0x22
#define TEAM_32        0x32
#define TEAM_NONCOMBAT 0x80

#define TELEPORTSTATE_INACTIVE 0
#define TELEPORTSTATE_PREENTER 1
#define TELEPORTSTATE_ENTERING 2
#define TELEPORTSTATE_WHITE    3
#define TELEPORTSTATE_EXITING  4

#define TEXCOMPMETHOD_UNCOMPRESSED0      0
#define TEXCOMPMETHOD_UNCOMPRESSED1      1
#define TEXCOMPMETHOD_HUFFMAN            2
#define TEXCOMPMETHOD_HUFFMANPERHCHANNEL 3
#define TEXCOMPMETHOD_RLE                4
#define TEXCOMPMETHOD_LOOKUP             5
#define TEXCOMPMETHOD_HUFFMANLOOKUP      6
#define TEXCOMPMETHOD_RLELOOKUP          7
#define TEXCOMPMETHOD_HUFFMANBLUR        8
#define TEXCOMPMETHOD_RLEBLUR            9

#define TEXFORMAT_RGBA32     0x00 // 32-bit RGBA (8/8/8/8)
#define TEXFORMAT_RGBA16     0x01 // 16-bit RGBA (5/5/5/1)
#define TEXFORMAT_RGB24      0x02 // 24-bit RGB (8/8/8)
#define TEXFORMAT_RGB15      0x03 // 15-bit RGB (5/5/5)
#define TEXFORMAT_IA16       0x04 // 16-bit grayscale+alpha
#define TEXFORMAT_IA8        0x05 // 8-bit grayscale+alpha (4/4)
#define TEXFORMAT_IA4        0x06 // 4-bit grayscale+alpha (3/1)
#define TEXFORMAT_I8         0x07 // 8-bit grayscale
#define TEXFORMAT_I4         0x08 // 4-bit grayscale
#define TEXFORMAT_RGBA16_CI8 0x09 // 16-bit 5551 paletted colour with 8-bit palette indexes
#define TEXFORMAT_RGBA16_CI4 0x0a // 16-bit 5551 paletted colour with 4-bit palette indexes
#define TEXFORMAT_IA16_CI8   0x0b // 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
#define TEXFORMAT_IA16_CI4   0x0c // 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes

#define THREAD_RMON  0
#define THREAD_IDLE  1
#define THREAD_SCHED 2
#define THREAD_MAIN  3
#define THREAD_AUDIO 4
#define THREAD_FAULT 5
#define THREAD_RESET 6
#define NUM_THREADS  7

#define THREADPRI_IDLE  0
#define THREADPRI_MAIN  10
#define THREADPRI_RESET 11
#define THREADPRI_AUDIO 20
#define THREADPRI_SCHED 30
#define THREADPRI_FAULT 40
#define THREADPRI_RMON  250

#define TICKMODE_GE_FADEIN  0
#define TICKMODE_NORMAL     1
#define TICKMODE_WARP       3
#define TICKMODE_MPSWIRL    4
#define TICKMODE_GE_FADEOUT 5
#define TICKMODE_CUTSCENE   6
#define TICKMODE_AUTOWALK   7

#define TICKOP_NONE         0
#define TICKOP_FREE         1
#define TICKOP_DISABLE      2
#define TICKOP_RETICK       3
#define TICKOP_GIVETOPLAYER 4
#define TICKOP_CHANGEDLIST  5

#define TIMEPRECISION_DAYS       0
#define TIMEPRECISION_HOURS      1
#define TIMEPRECISION_MINUTES    2
#define TIMEPRECISION_SECONDS    3
#define TIMEPRECISION_HUNDREDTHS 4

#define TITLEMODE_LEGAL            0
#define TITLEMODE_CHECKCONTROLLERS 1
#define TITLEMODE_PDLOGO           2
#define TITLEMODE_NINTENDOLOGO     3
#define TITLEMODE_RARELOGO         4
#define TITLEMODE_SKIP             5
#define TITLEMODE_NOCONTROLLER     6
#define TITLEMODE_RAREPRESENTS1    7
#define TITLEMODE_RAREPRESENTS2    8
#define TITLEMODE_9                9
#define TITLEMODE_NOEXPANSION      10

#define TITLEAIMODE_RAREPRESENTS1 1
#define TITLEAIMODE_RARELOGO      2
#define TITLEAIMODE_NINTENDOLOGO  3
#define TITLEAIMODE_RAREPRESENTS2 4
#define TITLEAIMODE_PDLOGO        5

#define TRACKTYPE_NONE    0
#define TRACKTYPE_PRIMARY 1
#define TRACKTYPE_NRG     2 // NRG = "energy" (the high-energy X track)
#define TRACKTYPE_MENU    3
#define TRACKTYPE_DEATH   4
#define TRACKTYPE_AMBIENT 5
#define TRACKTYPE_6       6

#define TURNSTATE_TURNING  1 // chr is actively turning
#define TURNSTATE_OFF      2 // chr is not using the turning feature
#define TURNSTATE_ONTARGET 3 // chr is facing the target and will turn if target moves

#define TVCMD_STOPSCROLL     0x00
#define TVCMD_SCROLLRELX     0x01
#define TVCMD_SCROLLRELY     0x02
#define TVCMD_SCROLLABSX     0x03
#define TVCMD_SCROLLABSY     0x04
#define TVCMD_SCALEABSX      0x05
#define TVCMD_SCALEABSY      0x06
#define TVCMD_SETTEXTURE     0x07
#define TVCMD_PAUSE          0x08
#define TVCMD_SETCMDLIST     0x09
#define TVCMD_RANDSETCMDLIST 0x0a
#define TVCMD_RESTART        0x0b
#define TVCMD_YIELD          0x0c
#define TVCMD_SETCOLOUR      0x0d
#define TVCMD_ROTATEABS      0x0e
#define TVCMD_ROTATEREL      0x0f

#define TVCMDLIST_00 0x00
#define TVCMDLIST_01 0x01
#define TVCMDLIST_02 0x02
#define TVCMDLIST_03 0x03
#define TVCMDLIST_04 0x04
#define TVCMDLIST_05 0x05
#define TVCMDLIST_06 0x06
#define TVCMDLIST_07 0x07
#define TVCMDLIST_08 0x08
#define TVCMDLIST_09 0x09
#define TVCMDLIST_0A 0x0a
#define TVCMDLIST_0B 0x0b
#define TVCMDLIST_0C 0x0c
#define TVCMDLIST_0D 0x0d
#define TVCMDLIST_0E 0x0e
#define TVCMDLIST_0F 0x0f
#define TVCMDLIST_10 0x10
#define TVCMDLIST_11 0x11
#define TVCMDLIST_12 0x12
#define TVCMDLIST_13 0x13
#define TVCMDLIST_14 0x14
#define TVCMDLIST_15 0x15
#define TVCMDLIST_16 0x16
#define TVCMDLIST_17 0x17
#define TVCMDLIST_18 0x18
#define TVCMDLIST_19 0x19
#define TVCMDLIST_1A 0x1a
#define TVCMDLIST_1B 0x1b
#define TVCMDLIST_1C 0x1c
#define TVCMDLIST_1D 0x1d
#define TVCMDLIST_1E 0x1e
#define TVCMDLIST_1F 0x1f
#define TVCMDLIST_20 0x20
#define TVCMDLIST_21 0x21
#define TVCMDLIST_22 0x22
#define TVCMDLIST_23 0x23
#define TVCMDLIST_24 0x24
#define TVCMDLIST_25 0x25
#define TVCMDLIST_26 0x26
#define TVCMDLIST_27 0x27
#define TVCMDLIST_28 0x28
#define TVCMDLIST_29 0x29
#define TVCMDLIST_2A 0x2a
#define TVCMDLIST_2B 0x2b
#define TVCMDLIST_2C 0x2c
#define TVCMDLIST_2D 0x2d
#define TVCMDLIST_2E 0x2e
#define TVCMDLIST_2F 0x2f
#define TVCMDLIST_30 0x30
#define TVCMDLIST_31 0x31
#define TVCMDLIST_32 0x32
#define TVCMDLIST_33 0x33
#define TVCMDLIST_34 0x34
#define TVCMDLIST_35 0x35
#define TVCMDLIST_36 0x36

#define USETIMER_CONTINUE 0
#define USETIMER_STOP     1
#define USETIMER_REPEAT   2

#define VEHICLEMODE_OFF         0
#define VEHICLEMODE_ENGINESTART 1
#define VEHICLEMODE_RUNNING     2

#define VIMODE_NONE 0
#define VIMODE_LO   1
#define VIMODE_HI   2

#define VIRES_LO 0
#define VIRES_HI 1

#define VISIONMODE_NORMAL             0
#define VISIONMODE_XRAY               1
#define VISIONMODE_SLAYERROCKET       2
#define VISIONMODE_SLAYERROCKETSTATIC 3

#define VOICEBOX_MALE0  0
#define VOICEBOX_MALE1  1
#define VOICEBOX_MALE2  2
#define VOICEBOX_FEMALE 3

#define VTXSTORETYPE_CHRVTX 0
#define VTXSTORETYPE_OBJVTX 1
#define VTXSTORETYPE_CHRCOL 2
#define VTXSTORETYPE_OBJCOL 3

#define WALLHITTEX_WATER        0x00 // Assigned to water surfaces, but same texture as glass. Likely unused
#define WALLHITTEX_BULLET1      0x01
#define WALLHITTEX_SOFT         0x02
#define WALLHITTEX_GLASS1       0x03
#define WALLHITTEX_GLASS2       0x04
#define WALLHITTEX_GLASS3       0x05
#define WALLHITTEX_BULLET2      0x06
#define WALLHITTEX_SCORCH       0x07
#define WALLHITTEX_PAINT        0x08
#define WALLHITTEX_BLOOD1       0x09
#define WALLHITTEX_BLOOD2       0x0a
#define WALLHITTEX_BLOOD3       0x0b
#define WALLHITTEX_BLOOD4       0x0c
#define WALLHITTEX_BPGLASS1     0x0d
#define WALLHITTEX_BPGLASS2     0x0e
#define WALLHITTEX_BPGLASS3     0x0f
#define WALLHITTEX_WOOD         0x10
#define WALLHITTEX_METAL        0x11

#define WAYMODE_INIT        0
#define WAYMODE_LOST1       1
#define WAYMODE_RETRY       2
#define WAYMODE_LOST2       3
#define WAYMODE_HAVEAIMPOS  4
#define WAYMODE_NEWOBSTACLE 5
#define WAYMODE_MAGIC       6

enum weaponnum {
	/*0x00*/ WEAPON_NONE,
	/*0x01*/ WEAPON_UNARMED,
	/*0x02*/ WEAPON_FALCON2,
	/*0x03*/ WEAPON_FALCON2_SILENCER,
	/*0x04*/ WEAPON_FALCON2_SCOPE,
	/*0x05*/ WEAPON_MAGSEC4,
	/*0x06*/ WEAPON_MAULER,
	/*0x07*/ WEAPON_PHOENIX,
	/*0x08*/ WEAPON_DY357MAGNUM,
	/*0x09*/ WEAPON_DY357LX,
	/*0x0a*/ WEAPON_CMP150,
	/*0x0b*/ WEAPON_CYCLONE,
	/*0x0c*/ WEAPON_CALLISTO,
	/*0x0d*/ WEAPON_RCP120,
	/*0x0e*/ WEAPON_LAPTOPGUN,
	/*0x0f*/ WEAPON_DRAGON,
	/*0x10*/ WEAPON_K7AVENGER,
	/*0x11*/ WEAPON_AR34,
	/*0x12*/ WEAPON_SUPERDRAGON,
	/*0x13*/ WEAPON_SHOTGUN,
	/*0x14*/ WEAPON_REAPER,
	/*0x15*/ WEAPON_SNIPERRIFLE,
	/*0x16*/ WEAPON_FARSIGHT,
	/*0x17*/ WEAPON_DEVASTATOR,
	/*0x18*/ WEAPON_ROCKETLAUNCHER,
	/*0x19*/ WEAPON_SLAYER,
	/*0x1a*/ WEAPON_COMBATKNIFE,
	/*0x1b*/ WEAPON_CROSSBOW,
	/*0x1c*/ WEAPON_TRANQUILIZER,
	/*0x1d*/ WEAPON_LASER,
	/*0x1e*/ WEAPON_GRENADE,
	/*0x1f*/ WEAPON_NBOMB,
	/*0x20*/ WEAPON_TIMEDMINE,
	/*0x21*/ WEAPON_PROXIMITYMINE,
	/*0x22*/ WEAPON_REMOTEMINE,
	/*0x23*/ WEAPON_COMBATBOOST,
	/*0x24*/ WEAPON_PP9I,
	/*0x25*/ WEAPON_CC13,
	/*0x26*/ WEAPON_KL01313,
	/*0x27*/ WEAPON_KF7SPECIAL,
	/*0x28*/ WEAPON_ZZT,
	/*0x29*/ WEAPON_DMC,
	/*0x2a*/ WEAPON_AR53,
	/*0x2b*/ WEAPON_RCP45,
	/*0x2c*/ WEAPON_PSYCHOSISGUN,
	/*0x2d*/ WEAPON_NIGHTVISION,
	/*0x2e*/ WEAPON_EYESPY,
	/*0x2f*/ WEAPON_XRAYSCANNER,
	/*0x30*/ WEAPON_IRSCANNER,
	/*0x31*/ WEAPON_CLOAKINGDEVICE,
	/*0x32*/ WEAPON_HORIZONSCANNER,
	/*0x33*/ WEAPON_TESTER,
	/*0x34*/ WEAPON_ROCKETLAUNCHER_34,
	/*0x35*/ WEAPON_ECMMINE,
	/*0x36*/ WEAPON_DATAUPLINK,
	/*0x37*/ WEAPON_RTRACKER,
	/*0x38*/ WEAPON_PRESIDENTSCANNER,
	/*0x39*/ WEAPON_DOORDECODER,
	/*0x3a*/ WEAPON_AUTOSURGEON,
	/*0x3b*/ WEAPON_EXPLOSIVES,
	/*0x3c*/ WEAPON_SKEDARBOMB,
	/*0x3d*/ WEAPON_COMMSRIDER,
	/*0x3e*/ WEAPON_TRACERBUG,
	/*0x3f*/ WEAPON_TARGETAMPLIFIER,
	/*0x40*/ WEAPON_DISGUISE40,
	/*0x41*/ WEAPON_DISGUISE41,
	/*0x42*/ WEAPON_FLIGHTPLANS,
	/*0x43*/ WEAPON_RESEARCHTAPE,
	/*0x44*/ WEAPON_BACKUPDISK,
	/*0x45*/ WEAPON_KEYCARD45,
	/*0x46*/ WEAPON_KEYCARD46,
	/*0x47*/ WEAPON_KEYCARD47,
	/*0x48*/ WEAPON_KEYCARD48,
	/*0x49*/ WEAPON_KEYCARD49,
	/*0x4a*/ WEAPON_KEYCARD4A,
	/*0x4b*/ WEAPON_KEYCARD4B,
	/*0x4c*/ WEAPON_KEYCARD4C,
	/*0x4d*/ WEAPON_SUITCASE,
	/*0x4e*/ WEAPON_BRIEFCASE,
#if VERSION >= VERSION_NTSC_1_0
	/*0x4f*/ WEAPON_SHIELDTECHITEM,
#endif
	/*0x50*/ WEAPON_NECKLACE,
	/*0x51*/ WEAPON_HAMMER,
	/*0x52*/ WEAPON_SCREWDRIVER,
	/*0x53*/ WEAPON_ROCKET,
	/*0x54*/ WEAPON_HOMINGROCKET,
	/*0x55*/ WEAPON_GRENADEROUND,
	/*0x56*/ WEAPON_BOLT,
	/*0x57*/ WEAPON_BRIEFCASE2,
	/*0x58*/ WEAPON_SKROCKET,
	/*0x59*/ WEAPON_CHOPPERGUN,
	/*0x5a*/ WEAPON_WATCHLASER,
	/*0x5b*/ WEAPON_MPSHIELD,
	/*0x5c*/ WEAPON_DISABLED,
	/*0x5d*/ WEAPON_SUICIDEPILL
};

#define WEAPON_MPLOCATION00 240
#define WEAPON_MPLOCATION01 241
#define WEAPON_MPLOCATION02 242
#define WEAPON_MPLOCATION03 243
#define WEAPON_MPLOCATION04 244
#define WEAPON_MPLOCATION05 245
#define WEAPON_MPLOCATION06 246
#define WEAPON_MPLOCATION07 247
#define WEAPON_MPLOCATION08 248
#define WEAPON_MPLOCATION09 249
#define WEAPON_MPLOCATION10 250
#define WEAPON_MPLOCATION11 251
#define WEAPON_MPLOCATION12 252
#define WEAPON_MPLOCATION13 253
#define WEAPON_MPLOCATION14 254
#define WEAPON_MPLOCATION15 255

#define INVAIMFLAG_MANUALZOOM 0x00000001
#define INVAIMFLAG_AUTOAIM    0x00000002
#define INVAIMFLAG_ACCURATESINGLESHOT   0x00000004

#define WEAPONFLAG_THROWABLE         0x00000001 // Entire weapon is throwable (eg. grendes, mines, knives)
#define WEAPONFLAG_00000004          0x00000004
#define WEAPONFLAG_ONEHANDED         0x00000008 // Makes guards carry the gun with one hand
#define WEAPONFLAG_AICANUSE          0x00000010
#define WEAPONFLAG_DUALFLIP          0x00000020 // For dual wielding, flip the right model to make the left (else have two right hands)
#define WEAPONFLAG_00000040          0x00000040
#define WEAPONFLAG_00000080          0x00000080 // Night vision
#define WEAPONFLAG_TRACKTIMEUSED     0x00000200 // Allow tracking how long gun is used, for weapon of choice
#define WEAPONFLAG_00000400          0x00000400
#define WEAPONFLAG_00000800          0x00000800 // MP briefcase
#define WEAPONFLAG_DUALWIELD         0x00001000 // Allow dual wielding
#define WEAPONFLAG_00002000          0x00002000
#define WEAPONFLAG_00004000          0x00004000
#define WEAPONFLAG_00008000          0x00008000 // Editor: "Special environment mapping"
#define WEAPONFLAG_HASHANDS          0x00020000
#define WEAPONFLAG_HIDEMENUMODEL     0x00040000 // Don't display the rotating model in the inventory menu
#define WEAPONFLAG_GANGSTA           0x00080000 // Allow turning the gun sideways at close range
#define WEAPONFLAG_DONTCOUNTSHOTS    0x00100000 // Laser and MP briefcase
#define WEAPONFLAG_DETERMINER_S_AN   0x00200000 // "An ..." (short version - when vertically split)
#define WEAPONFLAG_DETERMINER_F_AN   0x00400000 // "Picked up an ..." (full version)
#define WEAPONFLAG_DETERMINER_S_THE  0x00800000 // "The ..." (short version - when vertically split)
#define WEAPONFLAG_DETERMINER_F_THE  0x01000000 // "Picked up the ..." (full version)
#define WEAPONFLAG_02000000          0x02000000 // Slayer
#define WEAPONFLAG_04000000          0x04000000
#define WEAPONFLAG_UNDROPPABLE       0x08000000 // Do not drop when disarmed or killed
#define WEAPONFLAG_DETERMINER_S_SOME 0x10000000 // "Some ..." (short version - when vertically split)
#define WEAPONFLAG_DETERMINER_F_SOME 0x20000000 // "Picked up some ..." (full version)
#define WEAPONFLAG_AIMTRACK          0x40000000 // Allow drawing red box around targets in aim mode
#define WEAPONFLAG_FIRETOACTIVATE    0x80000000 // For devices/gadgets

#define WEAPONSET_RANDOMFIVE 0x0c
#define WEAPONSET_RANDOM     0x0d
#define WEAPONSET_CUSTOM     0x0e

#define WEATHERTYPE_RAIN 0
#define WEATHERTYPE_SNOW 1

#define WEATHERCFG_MAX_SKIPROOMS 128
#define WEATHERCFG_MAX_STAGES 16

#define WEATHERFLAG_INCLUDE       0x0001 // rooms listed in skiprooms are the only ones that have weather
#define WEATHERFLAG_CUTSCENE_ONLY 0x0002 // weather only in cutscenes
#define WEATHERFLAG_FORCE_WINDDIR 0x0004 // force constant wind direction

// Reasons for playing X music
#define XREASON_0       0
#define XREASON_DEFAULT 1
#define XREASON_2       2
#define XREASON_3       3

// Heads and bodies
#define HEAD_RANDOM           -1
#define BODY_DJBOND           0x00
#define BODY_CONNERY          0x01
#define BODY_DALTON           0x02
#define BODY_MOORE            0x03
#define HEAD_DARK_COMBAT      0x04
#define HEAD_ELVIS            0x05
#define HEAD_ROSS             0x06
#define HEAD_CARRINGTON       0x07
#define HEAD_MRBLONDE         0x08
#define HEAD_TRENT            0x09
#define HEAD_DDSHOCK          0x0a
#define HEAD_GRAHAM           0x0b
#define HEAD_DARK_FROCK       0x0c
#define HEAD_SECRETARY        0x0d
#define HEAD_CASSANDRA        0x0e
#define HEAD_THEKING          0x0f
#define HEAD_FEM_GUARD        0x10
#define HEAD_JON              0x11
#define HEAD_MARK2            0x12
#define HEAD_CHRIST           0x13
#define HEAD_RUSS             0x14
#define HEAD_GREY             0x15
#define HEAD_DARLING          0x16
#define HEAD_ROBERT           0x17
#define HEAD_BEAU1            0x18
#define HEAD_FEM_GUARD2       0x19
#define HEAD_BRIAN            0x1a
#define HEAD_JAMIE            0x1b
#define HEAD_DUNCAN2          0x1c
#define HEAD_BIOTECH          0x1d
#define HEAD_NEIL2            0x1e
#define HEAD_EDMCG            0x1f
#define HEAD_ANKA             0x20
#define HEAD_LESLIE_S         0x21
#define HEAD_MATT_C           0x22
#define HEAD_PEER_S           0x23
#define HEAD_EILEEN_T         0x24
#define HEAD_ANDY_R           0x25
#define HEAD_BEN_R            0x26
#define HEAD_STEVE_K          0x27
#define HEAD_JONATHAN         0x28
#define HEAD_MAIAN_S          0x29
#define HEAD_SHAUN            0x2a
#define HEAD_BEAU2            0x2b
#define HEAD_EILEEN_H         0x2c
#define HEAD_SCOTT_H          0x2d
#define HEAD_SANCHEZ          0x2e
#define HEAD_DARKAQUA         0x2f
#define HEAD_DDSNIPER         0x30
#define HEAD_BEAU3            0x31
#define HEAD_BEAU4            0x32
#define HEAD_BEAU5            0x33
#define HEAD_BEAU6            0x34
#define HEAD_GRIFFEY          0x35
#define HEAD_MOTO             0x36
#define HEAD_KEITH            0x37
#define HEAD_WINNER           0x38
#define HEAD_A51FACEPLATE     0x39
#define HEAD_ELVIS_GOGS       0x3a
#define HEAD_STEVEM           0x3b
#define HEAD_DARK_SNOW        0x3c
#define HEAD_PRESIDENT        0x3d
#define HEAD_VD               0x3e
#define HEAD_KEN              0x3f
#define HEAD_JOEL             0x40
#define HEAD_TIM              0x41
#define HEAD_GRANT            0x42
#define HEAD_PENNY            0x43
#define HEAD_ROBIN            0x44
#define HEAD_ALEX             0x45
#define HEAD_JULIANNE         0x46
#define HEAD_LAURA            0x47
#define HEAD_DAVEC            0x48
#define HEAD_COOK             0x49
#define HEAD_PRYCE            0x4a
#define HEAD_SILKE            0x4b
#define HEAD_SMITH            0x4c
#define HEAD_GARETH           0x4d
#define HEAD_MURCHIE          0x4e
#define HEAD_WONG             0x4f
#define HEAD_CARTER           0x50
#define HEAD_TINTIN           0x51
#define HEAD_MUNTON           0x52
#define HEAD_STAMPER          0x53
#define HEAD_JONES            0x54
#define HEAD_PHELPS           0x55
#define BODY_DARK_COMBAT      0x56
#define BODY_ELVIS1           0x57
#define BODY_AREA51GUARD      0x58
#define BODY_OVERALL          0x59
#define BODY_CARRINGTON       0x5a
#define BODY_MRBLONDE         0x5b
#define BODY_SKEDAR           0x5c
#define BODY_TRENT            0x5d
#define BODY_DDSHOCK          0x5e
#define BODY_LABTECH          0x5f
#define BODY_STRIPES          0x60
#define BODY_DARK_FROCK       0x61
#define BODY_DARK_TRENCH      0x62
#define BODY_OFFICEWORKER     0x63
#define BODY_OFFICEWORKER2    0x64
#define BODY_SECRETARY        0x65
#define BODY_CASSANDRA        0x66
#define BODY_THEKING          0x67
#define BODY_FEM_GUARD        0x68
#define BODY_DD_LABTECH       0x69
#define BODY_DD_SECGUARD      0x6a
#define BODY_DRCAROLL         0x6b
#define BODY_EYESPY           0x6c
#define BODY_DARK_RIPPED      0x6d
#define BODY_DD_GUARD         0x6e
#define BODY_DD_SHOCK_INF     0x6f
#define BODY_TESTCHR          0x70
#define BODY_BIOTECH          0x71
#define BODY_FBIGUY           0x72
#define BODY_CIAGUY           0x73
#define BODY_A51TROOPER       0x74
#define BODY_A51AIRMAN        0x75
#define BODY_CHICROB          0x76
#define BODY_STEWARD          0x77
#define BODY_STEWARDESS       0x78
#define BODY_PRESIDENT        0x79
#define BODY_STEWARDESS_COAT  0x7a
#define BODY_MINISKEDAR       0x7b
#define BODY_NSA_LACKEY       0x7c
#define BODY_PRES_SECURITY    0x7d
#define BODY_NEGOTIATOR       0x7e
#define BODY_G5_GUARD         0x7f
#define BODY_PELAGIC_GUARD    0x80
#define BODY_G5_SWAT_GUARD    0x81
#define BODY_ALASKAN_GUARD    0x82
#define BODY_MAIAN_SOLDIER    0x83
#define BODY_PRESIDENT_CLONE  0x84
#define BODY_PRESIDENT_CLONE2 0x85
#define BODY_DARK_AF1         0x86
#define BODY_DARKWET          0x87
#define BODY_DARKAQUALUNG     0x88
#define BODY_DARKSNOW         0x89
#define BODY_DARKLAB          0x8a
#define BODY_FEMLABTECH       0x8b
#define BODY_DDSNIPER         0x8c
#define BODY_PILOTAF1         0x8d
#define BODY_CILABTECH        0x8e
#define BODY_CIFEMTECH        0x8f
#define BODY_CARREVENINGSUIT  0x90
#define BODY_JONATHAN         0x91
#define BODY_CISOLDIER        0x92
#define BODY_SKEDARKING       0x93
#define BODY_ELVISWAISTCOAT   0x94
#define BODY_DARK_LEATHER     0x95
#define BODY_DARK_NEGOTIATOR  0x96

#define JO_ACTION_ACTIVATE           0x0001
#define JO_ACTION_RELOAD             0x0002

#ifdef PLATFORM_N64

#define BUTTON_ACCEPT_WPNFORWARD CONT_A
#define BUTTON_CANCEL_USE CONT_B

#define BUTTON_USE BUTTON_CANCEL_USE
#define BUTTON_CANCEL BUTTON_CANCEL_USE
#define BUTTON_RELOAD BUTTON_USE
#define BUTTON_ACCEPT BUTTON_ACCEPT_WPNFORWARD
#define BUTTON_WPNFORWARD BUTTON_ACCEPT_WPNFORWARD

#define BUTTON_UI_ACCEPT BUTTON_ACCEPT_WPNFORWARD
#define BUTTON_UI_CANCEL BUTTON_CANCEL_USE

#else

// xbla behavior
#define BUTTON_ACCEPT_USE     A_BUTTON
// 1964 behavior
#define BUTTON_CANCEL_USE     B_BUTTON

#define BUTTON_RELOAD         X_BUTTON
// dedicated button for mousewheel / xbla parity
#define BUTTON_WPNBACK        L_JPAD
// dedicated button for mousewheel / xbla parity
#define BUTTON_WPNFORWARD     Y_BUTTON
#define BUTTON_RADIAL         D_JPAD
#define BUTTON_ALTMODE        L_TRIG

#define BUTTON_CROUCH_CYCLE   CONT_8000
#define BUTTON_HALF_CROUCH    CONT_4000
#define BUTTON_FULL_CROUCH    CONT_2000

#define BUTTON_UI_ACCEPT      CONT_0010
#define BUTTON_UI_CANCEL      CONT_0020

#define MOUSEAIM_CLASSIC 0 // crosshair moves around the screen in aim mode
#define MOUSEAIM_LOCKED 1  // crosshair locked to the center of the screen in aim mode

#define HUDCENTER_NONE 0   // don't center HUD
#define HUDCENTER_NORMAL 1 // center HUD in 4:3 window
#define HUDCENTER_WIDE 2   // center HUD in 16:9 window

#define CROUCHMODE_HOLD 0   // hold the crouch buttons to keep crouching
#define CROUCHMODE_ANALOG 1 // analog crouch like on n64
#define CROUCHMODE_TOGGLE 2 // press the crouch buttons to toggle stance
#define CROUCHMODE_TOGGLE_ANALOG (CROUCHMODE_ANALOG | CROUCHMODE_TOGGLE)

#define CROSSHAIR_HEALTH_OFF 0
#define CROSSHAIR_HEALTH_ON_GREEN 1
#define CROSSHAIR_HEALTH_ON_WHITE 2

#define EXTRA_SLEEP_TIME 1000LL // 100us

#define MPSETUP_MAXSETUPS 128
#define MPSETUP_MAXNAME 17
#define MPSETUP_BLOCKSIZE 80

#endif

#endif
