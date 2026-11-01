#include <ultra64.h>
#include "constants.h"
#include "game/botmgr.h"
#include "game/chr.h"
#include "game/body.h"
#include "game/prop.h"
#include "game/botcmd.h"
#include "game/botinvinit.h"
#include "game/mplayer/mplayer.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "lib/ailist.h"
#include "data.h"
#include "types.h"
#if PERFECTHEAD
#include "game/phead.h"
#endif

void botmgrRemoveAll(void)
{
	s32 i;

	for (i = 0; i < MAX_BOTS; i++) {
		g_MpBotChrPtrs[i] = NULL;
	}

	g_BotCount = 0;
}

void botmgrAllocateBot(s32 chrnum, s32 aibotnum)
{
	RoomNum rooms[1];
	struct prop *prop;
	struct chrdata *chr;
	struct aibot *aibot;
	struct model *model;
	s32 i;
	s32 bodynum;
	s32 headnum;
#if PERFECTHEAD
	s32 pheadnum;
#endif

	rooms[0] = -1;

#if PERFECTHEAD
	pheadnum = pheadGetMpHead(g_BotConfigsArray[aibotnum].base.mpheadnum);

	if (pheadnum < 0 && g_BotConfigsArray[aibotnum].base.mpheadnum >= mpGetNumHeads2()) {
		// The PerfectHead isn't loaded (any more)
		headnum = mpGetHeadId(MPHEAD_DARK_COMBAT);
	} else
#endif
	headnum = mpGetHeadId(g_BotConfigsArray[aibotnum].base.mpheadnum);
	bodynum = mpGetBodyId(g_BotConfigsArray[aibotnum].base.mpbodynum);

	if (IS4MB()) {
		headnum = HEAD_DDSHOCK;
		bodynum = BODY_DDSHOCK;
	}

#if PERFECTHEAD
	if (pheadnum >= 0) {
		struct modeldef *headmodeldef = pheadLoadHeadModel(pheadnum, bodynum, &headnum);

		model = body0f02d338(bodynum, headnum, NULL, headmodeldef, false, true);
	} else
#endif
	model = bodyAllocateModel(bodynum, headnum, 0);

	if (model != NULL) {
		struct coord pos = {0.0f, 0.0f, 0.0f};
		u32 stack;

		prop = chrAllocate(model, &pos, rooms, 0.0f, ailistFindById(GAILIST_AIBOT_INIT));

		if (prop != NULL) {
			propActivate(prop);
			propEnable(prop);

			chr = prop->chr;

			chrSetChrnum(chr, chrnum);

			chr->chrnum = chrnum;
			chr->padpreset1 = -1;
			chr->chrpreset1 = -1;
			chr->hearingscale = 1;
			chr->visionrange = 80;
			chr->headnum = headnum;
			chr->bodynum = bodynum;
			chr->race = bodyGetRace(chr->bodynum);
			chr->flags = CHRFLAG0_CAN_EXAMINE_BODY; // reused flag?
			chr->flags2 = 0;
			chr->team = 1 << g_BotConfigsArray[aibotnum].base.team;
			chr->squadron = 0;

			if (g_BotCount < MAX_BOTS) {
				g_MpBotChrPtrs[g_BotCount] = chr;
				g_BotCount++;

				aibot = mempAlloc(sizeof(struct aibot), MEMPOOL_STAGE);
				chr->aibot = aibot;

				if (aibot != NULL) {
					chr->tude = 0;
					chr->voicebox = rngRandom() % 3;

					if (g_HeadsAndBodies[chr->bodynum].ismale == false) {
						chr->voicebox = VOICEBOX_FEMALE;
					}

					chr->target = -1;
					chr->naturalanim = 0;
					chr->myspecial = -1;
					chr->yvisang = 0;
					chr->teamscandist = 0;
					chr->convtalk = 0;
					chr->myaction = MA_AIBOTMAINLOOP;

					g_MpAllChrPtrs[g_MpNumChrs] = chr;
					g_MpAllChrConfigPtrs[g_MpNumChrs] = &g_BotConfigsArray[aibotnum].base;
					g_MpNumChrs++;

					aibot->ammoheld = mempAlloc(36 * sizeof(s32), MEMPOOL_STAGE);

					for (i = 0; i < 33; i++) {
						aibot->ammoheld[i] = 0;
					}

					aibot->config = &g_BotConfigsArray[aibotnum];

					switch (g_BotConfigsArray[aibotnum].difficulty) {
					case BOTDIFF_MEAT:
						aibot->followchance = 0;
						break;
					case BOTDIFF_EASY:
						aibot->followchance = 10;
						break;
					default:
					case BOTDIFF_NORMAL:
						aibot->followchance = 20;
						break;
					case BOTDIFF_HARD:
						aibot->followchance = 40;
						break;
					case BOTDIFF_PERFECT:
						aibot->followchance = 60;
						break;
					case BOTDIFF_DARK:
						aibot->followchance = 0;
						break;
					}

					aibot->aibotnum = aibotnum;
					aibot->flags = 0;
					aibot->gotoprop = NULL;
					aibot->timeuntilreload60[0] = 0;
					aibot->timeuntilreload60[1] = 0;
					aibot->throwtimer60 = 0;
					aibot->unk040 = 0.0f;
					aibot->loadedammo[0] = 0;
					aibot->loadedammo[1] = 0;
					aibot->fadeintimer60 = 0;
					aibot->respawning = false;
					aibot->nextbullettimer60[0] = 0;
					aibot->nextbullettimer60[1] = 0;
#if VERSION < VERSION_PAL_BETA
					aibot->unk030 = 301;
#endif
					aibot->unk038 = 0;
					aibot->punchtimer60[HAND_LEFT] = -1;
					aibot->punchtimer60[HAND_RIGHT] = 0;
					aibot->attackingplayernum = -1;
					aibot->followingplayernum = -1;
					aibot->dangerouspropnum = -1;
					aibot->attackanimconfig = NULL;
					aibot->speedmultforwards = 0.0f;
					aibot->speedmultsideways = 0.0f;
					aibot->distmode = -1;
					aibot->lastkilledbyplayernum = -1;
					aibot->feudplayernum = -1;
					aibot->command = AIBOTCMD_NORMAL;

					aibot->defendholdrot = 0.0f;
					aibot->defendholdpos.x = pos.x;
					aibot->defendholdpos.y = pos.y;
					aibot->defendholdpos.z = pos.z;

					aibot->weaponnum = WEAPON_UNARMED;

					aibot->burstsdone[0] = 0;
					aibot->burstsdone[1] = 0;
					aibot->skrocket = NULL;
					aibot->htbheldtimer60 = 0;

					aibot->gunfunc = FUNC_PRIMARY;
					aibot->ismeleeweapon = true;
					aibot->teamisonlyai = false;
					aibot->hasbriefcase = false;
					aibot->hascase = false;
					aibot->cloakdeviceenabled = false;
					aibot->rcp120cloakenabled = false;
					aibot->unk04c_04 = false;
					aibot->unk04c_03 = false;
					aibot->hasuplink = false;
					aibot->inhill = false;

					aibot->hillpadnum = -1;
					aibot->hillcovernum = -1;

					aibot->lastknownhill = -1;
					aibot->cyclonedischarging[HAND_LEFT] = false;
					aibot->cyclonedischarging[HAND_RIGHT] = false;
					aibot->changeguntimer60 = 0;
					aibot->distmodettl60 = 0;
					aibot->forcemainloop = false;
					aibot->returntodefendtimer60 = 0;
					aibot->reaperspeed[HAND_LEFT] = 0;
					aibot->reaperspeed[HAND_RIGHT] = 0;
					aibot->maulercharge[HAND_LEFT] = 0.0f;
					aibot->maulercharge[HAND_RIGHT] = 0.0f;
					aibot->roty = modelGetChrRotY(chr->model);
					aibot->angleoffset = 0.0f;
					aibot->speedtheta = 0.0f;
					aibot->lookangle = modelGetChrRotY(chr->model);

					aibot->moveratex = 0.0f;
					aibot->moveratey = 0.0f;
					aibot->shotspeed.x = 0.0f;
					aibot->shotspeed.y = 0.0f;
					aibot->shotspeed.z = 0.0f;
					aibot->commandtimer60 = 0;
					aibot->shootdelaytimer60 = 0;
					aibot->targetlastseen60 = -1;
					aibot->lastseenanytarget60 = -1;
					aibot->targetinsight = false;
					aibot->queryplayernum = 0;

					for (i = 0; i < MAX_MPCHRS; i++) {
						aibot->chrnumsbydistanceasc[i] = -1;
						aibot->chrdistances[i] = U32_MAX;
						aibot->chrsinsight[i] = 0;
						aibot->chrslastseen60[i] = -1;
						aibot->chrrooms[i] = -1;
					}

					aibot->extraangle = 0.0f;
					aibot->extraanglerate = 0.0f;
					aibot->extraanglebase = 0.0f;
					aibot->random3ttl60 = -1;
					aibot->random3 = 0;
					aibot->targetinsighttemperature = 0.0f;
					aibot->realignangleframe = -1;
					aibot->waypoints[0] = NULL;
					aibot->numwaystepstotarget = 0;
					aibot->random1 = rngRandom();
					aibot->random1ttl60 = 0;

					for (i = 0; i < ARRAYCOUNT(aibot->killsbygunfunc); i++) {
						aibot->killsbygunfunc[i][0] = 0.0f;
						aibot->killsbygunfunc[i][1] = 0.0f;

						aibot->suicidesbygunfunc[i][0] = 0.0f;
						aibot->suicidesbygunfunc[i][1] = 0.0f;

						aibot->equipdurations60[i][0] = 0;
						aibot->equipdurations60[i][1] = 0;

						aibot->equipextrascores[i] = 0;
					}

					aibot->equipextrascorestimer60 = 0;
					aibot->dampensuicidesttl60 = 0;
					aibot->rcpcloaktimer60 = 0.0f;
					aibot->targetcloaktimer60 = 0;
					aibot->canseecloaked = false;

					aibot->random2ttl60 = 0;
					aibot->random2 = rngRandom();
					aibot->randomfrac = RANDOMFRAC();
					aibot->cheap = false;
#if VERSION >= VERSION_NTSC_1_0
					aibot->forceslowupdates = 0;
					aibot->distoverrideprop = NULL;
					aibot->distoverridetimer60 = 0;
#endif
				} else {
					g_MpBotChrPtrs[--g_BotCount] = NULL;
					return;
				}

				botinvInit(chr, 10);
			}
		}
	}
}
