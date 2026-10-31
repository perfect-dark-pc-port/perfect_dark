#include <ultra64.h>
#include "constants.h"
#include "game/camdraw.h"
#include "game/phead.h"
#include "game/pak.h"
#include "game/filemgr.h"
#include "game/utils.h"
#include "game/body.h"
#include "game/modeldef.h"
#include "game/file.h"
#include "game/texdecompress.h"
#include "game/mplayer/mplayer.h"
#include "game/challenge.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "gbiex.h"
#ifndef PLATFORM_N64
#include <stdio.h>
#include "system.h"
#include "fs.h"
#endif

#if PERFECTHEAD

/**
 * Restored PerfectHead lifecycle code.
 *
 * The final ROM keeps the camera engine (camdraw.c) but not the code that
 * allocated its slots or started/stopped it from the menus. This file provides
 * that glue. See docs/perfecthead.md.
 *
 * Memory (Expansion Pak only, same as the rest of camdraw):
 * - Slot structs: PHEAD_NUM_SLOTS * sizeof(struct var8007f8e0) from the permanent
 *   pool at boot (~23 KB).
 * - Work slot textures/buffers: ~47 KB from the stage pool (plus ~9 KB for the
 *   Take Picture backup slot), allocated the first
 *   time the camera is started in a stage. They are only needed while the
 *   PerfectHead menus are open.
 * - Cam_Start's own capture buffers (~151 KB mema, freed by Cam_Stop) and
 *   thumbnails (~135 KB stage pool, once per stage).
 */

extern struct var8007f8dc *var8007f8dc;
extern struct var8007f8e0 *var8007f8e0;
extern struct var800a45a0 *var800a45a0;

ubool pak0f116cd4(s8 device);
void func0f14b228(struct var8007f8e0 *arg0);
void func0f14c7dc(struct textureconfig *tconfig);
Gfx *func0f14c870(Gfx *gdl, struct textureconfig *tconfig, f32 *arg2, f32 arg3, f32 arg4);

#define PHEAD_TEXHEADER   16 // texSelect peeks at the 8 bytes before textureptr
#define PHEAD_FACETEXSIZE (64 * 64)
#define PHEAD_THUMBSIZE   (16 * 16 / 2)
#define PHEAD_WORKSIZE    (PHEAD_TEXHEADER + PHEAD_FACETEXSIZE + PHEAD_TEXHEADER + PHEAD_THUMBSIZE + PHEAD_FACETEXSIZE + 1024)
#define PHEAD_HEADSIZE    (PHEAD_TEXHEADER + PHEAD_FACETEXSIZE + PHEAD_TEXHEADER + PHEAD_THUMBSIZE + 1024)

// In-game PerfectHead models: room for the display lists to grow while loading
// (as menu.c and player.c allow), and a texture pool of their own
#ifdef PLATFORM_64BIT
#define PHEAD_MODELSLACK 0x6000
#else
#define PHEAD_MODELSLACK 0x4000
#endif
#define PHEAD_MODELTEXPOOL 0x4000 // a base head uses about 11 KB

/**
 * Work slots get texture/image buffers from the stage pool while the
 * PerfectHead menus are in use. The loaded heads (slots 0 to
 * PHEAD_NUM_HEADS - 1) get theirs from the permanent pool at boot, so they
 * survive stage changes; they don't need the raw image buffer (unk01c).
 */
static const u8 g_PheadBufferedSlots[] = {
	PHEAD_FIRSTWORKSLOT + 0,
	PHEAD_FIRSTWORKSLOT + 1,
	PHEAD_FIRSTWORKSLOT + 2,
	PHEAD_FIRSTWORKSLOT + 3,
	PHEAD_PICTURESLOT,
};

struct pheadbackup;
static struct pheadbackup *g_PheadBackup;

static bool g_PheadCameraRunning = false;
static bool g_PheadEditorHasImage = false;
static u8 *g_PheadWorkBuffers = NULL;

void pheadSlotsInit(void)
{
	s32 i;
	u32 size = align16(sizeof(struct var8007f8e0) * PHEAD_NUM_SLOTS);

	var8007f8e0 = mempAlloc(size, MEMPOOL_PERMANENT);
	bzero(var8007f8e0, size);

	for (i = 0; i < PHEAD_NUM_SLOTS; i++) {
		struct var8007f8e0 *slot = &var8007f8e0[i];

		func0f14c7dc(&slot->unk004);
		slot->unk004.textureptr = NULL;

		func0f14c7dc(&slot->unk010);
		slot->unk010.width = 16;
		slot->unk010.height = 16;
		slot->unk010.depth = G_IM_SIZ_4b;
		slot->unk010.textureptr = NULL;

		func0f14b228(slot);

		if (i < ARRAYCOUNT(var800a45a0->unk014)) {
			var800a45a0->unk014[i] = i;
		}
	}

	// Loaded heads: face texture, thumbnail and compressed picture each
	{
		u8 *ptr = mempAlloc(align16(PHEAD_HEADSIZE * PHEAD_NUM_HEADS), MEMPOOL_PERMANENT);

		bzero(ptr, align16(PHEAD_HEADSIZE * PHEAD_NUM_HEADS));

		for (i = 0; i < PHEAD_NUM_HEADS; i++) {
			struct var8007f8e0 *slot = &var8007f8e0[i];

			ptr += PHEAD_TEXHEADER;
			slot->unk004.textureptr = ptr;
			ptr += PHEAD_FACETEXSIZE;

			ptr += PHEAD_TEXHEADER;
			slot->unk010.textureptr = ptr;
			ptr += PHEAD_THUMBSIZE;

			slot->unk020 = ptr;
			ptr += 1024;
		}
	}

	var800a45a0->unk16c = PHEAD_FIRSTWORKSLOT + 0;
	var800a45a0->unk170 = PHEAD_FIRSTWORKSLOT + 1;
	var800a45a0->unk174 = PHEAD_FIRSTWORKSLOT + 2;
	var800a45a0->unk178 = PHEAD_FIRSTWORKSLOT + 3;
	var800a45a0->unk004 = 0;
}

/**
 * Called on stage load (pheadReset). Everything allocated from the stage pool
 * or mema is gone, so forget the pointers.
 */
void pheadStageReset(void)
{
	s32 i;

	if (var800a45a0 == NULL || var8007f8e0 == NULL) {
		return;
	}

	var800a45a0->unk470 = NULL;
	var800a45a0->unk474 = NULL;
	var800a45a0->unk478 = NULL;
	var800a45a0->unk47c = NULL;
	var800a45a0->unk480 = NULL;
	var800a45a0->unk484 = NULL;
	var800a45a0->unk488 = NULL;

	for (i = 0; i < ARRAYCOUNT(g_PheadBufferedSlots); i++) {
		struct var8007f8e0 *slot = &var8007f8e0[g_PheadBufferedSlots[i]];

		slot->unk004.textureptr = NULL;
		slot->unk010.textureptr = NULL;
		slot->unk01c = NULL;
		slot->unk020 = NULL;
	}

	g_PheadWorkBuffers = NULL;
	g_PheadBackup = NULL;
	g_PheadCameraRunning = false;
	g_PheadEditorHasImage = false;
}

void pheadAllocWorkSlots(void)
{
	u8 *ptr;
	s32 i;

	if (g_PheadWorkBuffers) {
		return;
	}

	g_PheadWorkBuffers = mempAlloc(align16(PHEAD_WORKSIZE * ARRAYCOUNT(g_PheadBufferedSlots)), MEMPOOL_STAGE);
	bzero(g_PheadWorkBuffers, align16(PHEAD_WORKSIZE * ARRAYCOUNT(g_PheadBufferedSlots)));

	ptr = g_PheadWorkBuffers;

	for (i = 0; i < ARRAYCOUNT(g_PheadBufferedSlots); i++) {
		struct var8007f8e0 *slot = &var8007f8e0[g_PheadBufferedSlots[i]];

		ptr += PHEAD_TEXHEADER;
		slot->unk004.textureptr = ptr;
		ptr += PHEAD_FACETEXSIZE;

		ptr += PHEAD_TEXHEADER;
		slot->unk010.textureptr = ptr;
		ptr += PHEAD_THUMBSIZE;

		slot->unk01c = ptr;
		ptr += PHEAD_FACETEXSIZE;

		slot->unk020 = ptr;
		ptr += 1024;
	}
}

/**
 * Take Picture's live preview writes every capture into the editor slot. Keep
 * a copy of the editor's picture while it's open, so Cancel can put it back.
 */
void pheadPictureBackup(void)
{
	pheadAllocWorkSlots();
	func0f14c50c(&var8007f8e0[PHEAD_PICTURESLOT], func0f14a06c(-1), 0, "phead.c");
}

void pheadPictureRestore(void)
{
	pheadAllocWorkSlots();
	func0f14c50c(func0f14a06c(-1), &var8007f8e0[PHEAD_PICTURESLOT], 0, "phead.c");
}

bool pheadCameraStart(s32 device)
{
	if (var800a45a0 == NULL || var8007f8e0 == NULL || device < 0 || device >= 4) {
		return false;
	}

	if (g_PheadCameraRunning) {
		if (func0f14a5a4() == device) {
			return true;
		}

		pheadCameraStop();
	}

	pheadAllocWorkSlots();

	// Cam_Start resets the camera device to 0, so select ours afterwards
	func0f14aee0();
	func0f14a594(device);
	pak0f1171b4(device, PAK00C_03, 0);
	func0f14a52c();

	g_PheadCameraRunning = true;

	return true;
}

void pheadCameraStop(void)
{
	if (g_PheadCameraRunning) {
		func0f14b150();
		g_PheadCameraRunning = false;

		// Cam_Stop frees the capture buffers but keeps the pointers
		var800a45a0->unk470 = NULL;
		var800a45a0->unk474 = NULL;
		var800a45a0->unk478 = NULL;
		var800a45a0->unk47c = NULL;
		var800a45a0->unk480 = NULL;
	}
}

bool pheadCameraIsRunning(void)
{
	return g_PheadCameraRunning;
}

/**
 * Ask every Game Boy pak to identify itself and return a bitmask of the
 * devices holding a Game Boy Camera. pak0f116cd4 requests one step of the
 * capture state machine, so call this once per frame for a few frames.
 */
u32 pheadFindCameras(void)
{
	u32 mask = 0;
	s32 i;

	for (i = 0; i < 4; i++) {
		if (_pakGetType(i) == PAKTYPE_GAMEBOY && pak0f116cd4(i)) {
			mask |= 1 << i;
		}
	}

	return mask;
}

/**
 * Return true if a pak is still being probed or a Game Boy pak is still
 * preparing, in which case a camera may not have been identified yet.
 */
bool pheadCamerasPending(void)
{
	s32 i;

	for (i = 0; i < 4; i++) {
		s32 state = g_Paks[i].state;

		if (state == PAKSTATE_PROBE
				|| state == PAKSTATE_GB_PRE_PREPARE
				|| state == PAKSTATE_GB_PREPARE
				|| state == PAKSTATE_GB_POST_PREPARE1
				|| state == PAKSTATE_GB_POST_PREPARE2
				|| state == PAKSTATE_GB_POST_PREPARE3) {
			return true;
		}
	}

	return false;
}

/**
 * Head painting
 *
 * The final ROM has no PerfectHead base head models: HEAD_BEAU2-6 all alias
 * FILE_CHEADBEAU, and no head has the FACE/SIDE/HAIR parts (0x190-0x192) that
 * camdraw's removed Cam_RebuildHead / Cam_ColourHeadOnePart / Cam_SquashOnePart
 * worked on. Instead the base heads ("Change Head") are a selection of the
 * Combat Simulator heads that share one texture layout:
 *
 *   face  CI8 32x64, 256-colour RGBA16 palette, stored upside down (row 0 = chin)
 *   sides CI8 32x64 each (ear, hair)
 *
 * Each palette is split in two: indices 0-127 are a skin ramp and 128-255 a hair
 * ramp, built from camdraw's colour tables. When a head is loaded its texels are
 * sorted into skin and hair by comparing them with the skin of its cheeks. The
 * face then gets the camera picture (cropped by the Position Picture rectangle)
 * except for the hair around it, and the sides keep their own shading, remapped
 * onto the ramps. Shape Head's depth moves the front of the face in or out.
 *
 * Only the menu model's private copy of the head is modified.
 */

/* Same TMEM odd-row swap as camdraw's CAM_TMEMSWAP (8-bit textures) */
#ifdef PLATFORM_N64
#define CAM_PHEAD_TMEMSWAP(row, col) ((((row) & 1)) ? ((((col) & 4)) ? -4 : 4) : 0)
#else
#define CAM_PHEAD_TMEMSWAP(row, col) 0
void videoFreeCachedTextures(const void *start, const void *end);
#endif

#define PHEAD_TEXW     32
#define PHEAD_TEXH     64
#define PHEAD_TEXBYTES (PHEAD_TEXW * PHEAD_TEXH)
#define PHEAD_MAXVTX   1024
#define PHEAD_HAIR     0x80 // texel class bit in the backup; bits 0-6 are the original shade
#define PHEAD_MAXEXTRAS 2   // other CI8 textures (back of the hair) recoloured by palette
#define PHEAD_NECKROWS 6    // side texture rows at the neck (row 0 is the bottom)
#define PHEAD_MINSKIN  80   // smallest side skin patch away from the neck (an ear)

#define PHEAD_SKINSHADE     72  // skin ramp index for the cheeks
#define PHEAD_SKINSHADE_MAX 104 // brightest skin outside the picture
#define PHEAD_HAIRSHADE     56  // hair ramp index for the head's average hair
#define PHEAD_HAIRSHADE_MAX 100 // brightest hair
#define PHEAD_HAIRBANDS     16  // shade bands of the head's own hair colour (Normal hair)

/**
 * Base heads. headnum is a HEAD_* number; the texture numbers are indices into
 * the head's texconfigs. eyerow and mouthrow are face texture rows (0 = chin),
 * used for the placement guides. hairrow is for heads whose hair is too close
 * to their skin colour to be told apart; hairlum and sidered tune the skin/hair
 * test (see pheadClassifyTexels) for heads where the defaults (50, 80) fail.
 * See "Base heads" in docs/perfecthead.md for how to find and tune these.
 */
struct pheadtemplate {
	u8 headnum;
	u8 facetex;
	u8 sidetex[2];
	u8 eyerow;
	u8 mouthrow;
	u8 hairrow;  // face texture rows from here up are always hair
	u8 hairlum;  // texels darker than this % of the skin are hair
	u8 sidered;  // side texels less red than this % of the skin are hair
};

// sidetex for a head with only a face texture (the dataDyne Sniper: the rest
// of his head is helmet and visor): the painter then works on the face alone
#define PHEAD_NOSIDE 0xff
#define PHEAD_NUMTEX(template) ((template)->sidetex[0] == PHEAD_NOSIDE ? 1 : 3)

static const struct pheadtemplate g_PheadTemplates[] = {
	{ HEAD_BEAU1,     0, { 1, 2 }, 31, 12, 47, 65, 55 }, // pale skin, ash brown hair
	{ HEAD_CHRIST,    0, { 1, 2 }, 33, 14, 49, 65, 80 }, // grey-brown hair
	{ HEAD_DAVEC,     0, { 1, 2 }, 35, 12, 55, 50, 80 },
	{ HEAD_NEIL2,     4, { 5, 6 }, 37, 15, 53, 50, 80 },
	{ HEAD_PRYCE,     0, { 1, 2 }, 32, 13, 53, 50, 80 },
	{ HEAD_WONG,      4, { 5, 6 }, 33, 13, 55, 50, 80 },
	{ HEAD_TINTIN,    0, { 1, 2 }, 32, 11, 49, 50, 80 },
	{ HEAD_PEER_S,    0, { 1, 2 }, 31, 13, 50, 50, 80 },
	{ HEAD_ALEX,      0, { 1, 2 }, 34, 15, 57, 50, 80 },
	{ HEAD_JULIANNE,  0, { 1, 2 }, 34, 10, 52, 50, 80 },
	{ HEAD_LAURA,     0, { 1, 2 }, 33, 12, 55, 50, 80 },
	{ HEAD_EILEEN_H,  4, { 5, 6 }, 35, 12, 54, 50, 80 },
	{ HEAD_SCOTT_H,   5, { 4, 6 }, 37, 14, 54, 50, 80 },

	// Elvis (see pheadPaintElvis): his skin texture, no sides or hair. The
	// rows are of the virtual face texture mapped onto his skin.
	{ HEAD_ELVIS,     1, { 1, 1 }, 33, 13, 64, 50, 80 },

	// Face only, under his helmet's purple visor
	{ HEAD_DDSNIPER,  2, { PHEAD_NOSIDE, PHEAD_NOSIDE }, 32, 12, 51, 50, 80 },

	// Shigeru Miyamoto, last: not in the JPN version's heads
#if VERSION != VERSION_JPN_FINAL
	{ HEAD_MOTO,      0, { 1, 2 }, 30, 11, 44, 50, 80 },
#endif
};

// pheadmasks.c has a row per template
typedef char pheadMaskTableSize[ARRAYCOUNT(g_PheadTemplates) == PHEAD_NUM_TEMPLATES ? 1 : -1];

s32 pheadGetNumTemplates(void)
{
	return ARRAYCOUNT(g_PheadTemplates);
}

static const struct pheadtemplate *pheadGetTemplate(s32 templatenum)
{
	if (templatenum < 0 || templatenum >= ARRAYCOUNT(g_PheadTemplates)) {
		templatenum = 0;
	}

	return &g_PheadTemplates[templatenum];
}

/**
 * Whether Change Head offers the base head: a head that the Combat Simulator
 * only offers once it's unlocked (its g_MpHeads requirefeature, Elvis's
 * MPFEATURE_CHR_ELVIS) is offered from then on. A head made before keeps its
 * base head either way.
 */
bool pheadIsTemplateAvailable(s32 templatenum)
{
	s32 headnum = pheadGetTemplate(templatenum)->headnum;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpHeads); i++) {
		if (g_MpHeads[i].headnum == headnum) {
			return g_MpHeads[i].requirefeature == 0 || challengeIsFeatureUnlocked(g_MpHeads[i].requirefeature);
		}
	}

	return true;
}

s32 pheadGetNumAvailableTemplates(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_PheadTemplates); i++) {
		if (pheadIsTemplateAvailable(i)) {
			count++;
		}
	}

	return count;
}

/**
 * The <index>th base head Change Head offers.
 */
s32 pheadGetAvailableTemplate(s32 index)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_PheadTemplates); i++) {
		if (pheadIsTemplateAvailable(i)) {
			if (index == 0) {
				return i;
			}

			index--;
		}
	}

	return 0;
}

/**
 * Where a base head is in Change Head's list (a locked one shows as the one
 * before it).
 */
s32 pheadGetAvailableIndex(s32 templatenum)
{
	s32 index = 0;
	s32 i;

	for (i = 0; i < templatenum && i < ARRAYCOUNT(g_PheadTemplates); i++) {
		if (pheadIsTemplateAvailable(i)) {
			index++;
		}
	}

	if (index > 0 && !pheadIsTemplateAvailable(templatenum)) {
		index--;
	}

	return index;
}

s32 pheadGetTemplateHeadnum(s32 templatenum)
{
	return pheadGetTemplate(templatenum)->headnum;
}

u16 pheadGetTemplateFilenum(s32 templatenum)
{
	return g_HeadsAndBodies[pheadGetTemplate(templatenum)->headnum].filenum;
}

/**
 * Where the template's eyes and mouth are in the 64x64 editor image when the
 * given crop rectangle is applied (for the placement guides).
 */
void pheadGetTemplateGuide(s32 templatenum, s32 y1, s32 y2, s32 *eyey, s32 *mouthy)
{
	const struct pheadtemplate *template = pheadGetTemplate(templatenum);

	*eyey = y2 - ((template->eyerow * 2 + 1) * (y2 - y1)) / (PHEAD_TEXH * 2);
	*mouthy = y2 - ((template->mouthrow * 2 + 1) * (y2 - y1)) / (PHEAD_TEXH * 2);
}

/**
 * camdraw's PerfectHead colour table (var7f1b62a0 - var7f1b639c), copied here
 * because the decomp declares each entry as a separate one-element array, so
 * only the N64 build is guaranteed to lay them out contiguously.
 * 8 palettes of 8 RGBA words: entries 0/1 are black and white, 2-7 run from
 * light to dark. Palettes 0-3 are skin tones, 4-7 hair colours (Blonde, Black,
 * Auburn, Blue Rinse).
 */
static const u32 g_PheadColours[8 * 8] = {
	0x00000000, 0xffffffff, 0xdc9785ff, 0xdc917bff, 0xdd8d71ff, 0xdd8367ff, 0xdf795dff, 0xdf7953ff,
	0x00000000, 0xffffffff, 0xa67b6dff, 0xa16152ff, 0x8c6741ff, 0x81544cff, 0x6a4031ff, 0x4f433cff,
	0x00000000, 0xffffffff, 0xccb38dff, 0xddc07cff, 0xe6a667ff, 0xd1ab50ff, 0xc69f6dff, 0xb27e5bff,
	0x00000000, 0xffffffff, 0x5f3a25ff, 0x553a20ff, 0x4b351cff, 0x412d19ff, 0x41260fff, 0x371e04ff,
	0x00000000, 0xffffffff, 0xe9d679ff, 0x654f40ff, 0xebb279ff, 0x846b51ff, 0xebc579ff, 0xab8d6fff,
	0x00000000, 0xffffffff, 0x26374dff, 0x18253eff, 0x171d20ff, 0x060e16ff, 0x00001cff, 0x000001ff,
	0x00000000, 0xffffffff, 0x972f1aff, 0x822817ff, 0x6e1e12ff, 0x5a140dff, 0x460a08ff, 0x200001ff,
	0x00000000, 0xffffffff, 0x79b2ebff, 0x007fffff, 0x1f5fdfff, 0x0043caff, 0x00147eff, 0x0c1857ff,
};

/**
 * Per loaded head: texel classes and the original vertex depths, captured
 * when the head model is loaded. About 15 KB of stage memory on N64.
 */
struct pheadbackup {
	struct modeldef *def;
	s32 templatenum;
	u8 texels[3][PHEAD_TEXBYTES]; // face, side, side: shade (bits 0-6) and PHEAD_HAIR
	u8 picture[PHEAD_TEXBYTES];    // face texels: how much of the picture shows (0-255)
	s32 hairshade;                 // average shade of the hair
	Vtx *vertices[PHEAD_MAXVTX];
	s16 origz[PHEAD_MAXVTX];
	s32 numvtx;
	s32 zback;
	s32 zfront;
	s32 ybottom;
	s32 ytop;
	f32 applieddepth;              // Shape Head depth currently applied (1 = original)
	s32 skinshade;                 // shade of the cheeks (the skin reference)
	s32 refcr, refcg, reflum, refrg; // the skin reference's colour (pheadIsHairColour)
	s16 numcolours[3];             // palette size of the face and side textures
	s16 elvislods;                 // Elvis: levels of detail of his skin texture
	s16 elvispalette;              // Elvis: offset of its palette
	u8 hairorig[PHEAD_HAIRBANDS][3]; // the head's own hair colour by shade (Normal hair)
	s32 numextras;
	struct {
		u8 *data;
		u8 *palette;
		s16 count;
		u16 orig[256];
	} extras[PHEAD_MAXEXTRAS];     // other hair textures: their palettes
};

// pheadClassifyTexels borrows origz and vertices as scratch space
typedef char pheadScratchFits[sizeof(((struct pheadbackup *) 0)->origz) >= PHEAD_TEXBYTES
	&& sizeof(((struct pheadbackup *) 0)->vertices) >= PHEAD_TEXBYTES * sizeof(u16) ? 1 : -1];

static u32 pheadTableColour(s32 palette, s32 entry)
{
	return g_PheadColours[palette * 8 + entry];
}

static u16 pheadRgba16(s32 r, s32 g, s32 b)
{
	if (r < 0) r = 0; if (r > 255) r = 255;
	if (g < 0) g = 0; if (g > 255) g = 255;
	if (b < 0) b = 0; if (b > 255) b = 255;

	return ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1;
}

/**
 * Shade 0-127 of one colour from camdraw's table, dark to light.
 * The table's six shades of a colour are all about as bright as each other,
 * so a ramp through them has almost no contrast and a photo painted with it
 * comes out flat. Instead the ramp is the colour's average shade, which shade
 * <anchor> (the head's typical skin or hair texel) gets exactly, darkening
 * evenly to 15% at shade 0 and lightening towards white at shade 127.
 */
static u16 pheadRampColour(s32 palette, s32 anchor, s32 shade)
{
	s32 base[3] = { 0, 0, 0 };
	s32 rgb[3];
	s32 i, j;

	for (i = 2; i < 8; i++) {
		u32 c = pheadTableColour(palette, i);
		base[0] += (c >> 24) & 0xff;
		base[1] += (c >> 16) & 0xff;
		base[2] += (c >> 8) & 0xff;
	}

	for (j = 0; j < 3; j++) {
		base[j] /= 6;

		if (shade <= anchor) {
			rgb[j] = base[j] * (15 * anchor + 85 * shade) / (100 * anchor);
		} else {
			rgb[j] = base[j] + (255 - base[j]) * 6 * (shade - anchor) / (10 * (127 - anchor));
		}
	}

	return pheadRgba16(rgb[0], rgb[1], rgb[2]);
}

/**
 * The face and side textures' palettes have between about 100 and 256
 * colours (the TLUT the game loads is that size), so the first half is the
 * skin ramp and the rest the hair ramp, each covering shades 0-127.
 */
static s32 pheadSkinEntries(s32 numcolours)
{
	return numcolours / 2;
}

/**
 * Shade 0-127 of the head's own hair colour (the Normal hair colour),
 * between the bands recorded by pheadCaptureHairColours.
 */
static u16 pheadOrigHairColour(s32 shade)
{
	s32 pos = shade * PHEAD_HAIRBANDS - 64; // in 1/128ths of a band, from the middle of band 0
	s32 band = pos < 0 ? 0 : pos / 128;
	s32 frac = pos < 0 ? 0 : pos % 128;
	s32 next = band + 1 < PHEAD_HAIRBANDS ? band + 1 : band;
	s32 rgb[3];
	s32 j;

	for (j = 0; j < 3; j++) {
		rgb[j] = (g_PheadBackup->hairorig[band][j] * (128 - frac) + g_PheadBackup->hairorig[next][j] * frac) / 128;
	}

	return pheadRgba16(rgb[0], rgb[1], rgb[2]);
}

/**
 * Ramp colour for a palette from g_PheadColours, or the head's own hair
 * colour for palette -1.
 */
static u16 pheadShadeColour(s32 palette, s32 anchor, s32 shade)
{
	if (palette < 0) {
		return pheadOrigHairColour(shade);
	}

	return pheadRampColour(palette, anchor, shade);
}

static void pheadWriteRamp(u8 *dst, s32 palette, s32 anchor, s32 count)
{
	s32 i;

	for (i = 0; i < count; i++) {
		u16 c = pheadShadeColour(palette, anchor, i * 127 / (count - 1));

		dst[i * 2 + 0] = c >> 8;
		dst[i * 2 + 1] = c & 0xff;
	}
}

/**
 * Palette entry for a pheadTexelIndex value (bit 7 = hair, bits 0-6 = shade)
 * in a palette of numcolours entries.
 */
static u8 pheadPaletteIndex(s32 value, s32 numcolours)
{
	s32 skin = pheadSkinEntries(numcolours);

	if (value & 0x80) {
		return skin + (value & 0x7f) * (numcolours - skin - 1) / 127;
	}

	return value * (skin - 1) / 127;
}

/**
 * A model texture's data, for the CPU. On N64 texLoad stores the physical
 * address in the texture config (it is what the RDP reads), so add the KSEG0
 * base; the port stores a normal pointer.
 */
#ifdef PLATFORM_N64
#define PHEAD_TEXDATA(ptr) ((ptr) ? (u8 *) ((u32) (ptr) | K0BASE) : NULL)
#else
#define PHEAD_TEXDATA(ptr) ((u8 *) (ptr))
#endif

/**
 * The texture's record in the pool it was loaded into. Dedicated texture
 * pools keep their records at the end, growing down (texLoad), so walk down
 * from the pool's end until the record with this data, or something that
 * can't be a record. A menu model loads the body into the same pool first,
 * so the head's records come after the body's: Joanna's combat body alone
 * has 34 textures.
 */
#define PHEAD_MAXTEXRECORDS 128

static struct tex *pheadFindTex(u8 *texpoolend, u8 *data)
{
	struct tex *tex = (struct tex *) texpoolend;
	s32 i;

	if (texpoolend == NULL || data == NULL) {
		return NULL;
	}

	for (i = 0; i < PHEAD_MAXTEXRECORDS; i++) {
		tex--;

		if (tex->data == data) {
			return tex;
		}

		if (tex->data == NULL || tex->data >= (u8 *) tex) {
			break;
		}
	}

	return NULL;
}

/**
 * Bytes of image data before a zlib texture's palette: every level of detail,
 * each row padded to 8 bytes (texInflateZlib, texAlignIndices). Only for
 * textures whose smaller levels were made by halving (no LOD data in the
 * file), which is all the base heads' textures. -1 if unknown.
 */
static s32 pheadTexImageBytes(struct tex *tex)
{
	s32 width = tex->width;
	s32 height = tex->height;
	s32 numlods = tex->numlods > 1 ? tex->numlods : 1;
	s32 total = 0;
	s32 lod;

	if (tex->hasloddata || tex->gbiformat != G_IM_FMT_CI || tex->depth > G_IM_SIZ_8b) {
		return -1;
	}

	for (lod = 0; lod < numlods; lod++) {
		s32 rowbytes = tex->depth == G_IM_SIZ_8b ? width : (width + 1) / 2;

		total += ((rowbytes + 7) & ~7) * height;
		width = (width + 1) >> 1;
		height = (height + 1) >> 1;
	}

	return total;
}

/**
 * Offset of texel x in row y of a 32-wide 8-bit texture as stored in memory.
 * On N64 the odd rows are TMEM swizzled (see texSwizzle).
 */
static s32 pheadCi8Offset(s32 x, s32 y)
{
#ifdef PLATFORM_N64
	return y * PHEAD_TEXW + (x ^ ((y & 1) << 2));
#else
	return y * PHEAD_TEXW + x;
#endif
}

/**
 * Elvis: a Maian, whose head is one 32x48 CI4 skin texture (16 shades of
 * grey, 5 levels of detail) projected flat onto the front of his head, row 0
 * at the chin; his eyes are separate parts of the model, over the skin. The
 * face is treated as a virtual 32x64 face texture over part of his skin, so
 * the picture, its levels and the guides work as for the other heads: virtual
 * column c and row r are skin texel PHEAD_ELVIS_S0 + c * PHEAD_ELVIS_SW / 32,
 * PHEAD_ELVIS_T0 + r * PHEAD_ELVIS_TH / 64. Measured from his model: his skin
 * column is 0.095 * x + 15.4 and row 0.106 * y + 9.7 (model units), with his
 * eyes at row 17.8 and the chin at row 3.9. The virtual rows put the
 * picture's eyes (eyerow) a row above his, so more of the face shows below
 * them.
 */
#define PHEAD_ISELVIS(template) ((template)->headnum == HEAD_ELVIS)
#define PHEAD_ELVIS_TEXW 32
#define PHEAD_ELVIS_TEXH 48
#define PHEAD_ELVIS_ROWBYTES 16 // 4 bits per texel
#define PHEAD_ELVIS_S0 6.0f
#define PHEAD_ELVIS_SW 19.0f
#define PHEAD_ELVIS_T0 5.4f
#define PHEAD_ELVIS_TH 25.9f

static bool pheadHeadIsPaintable(struct modeldef *def, const struct pheadtemplate *template)
{
	s32 i;

	if (def == NULL) {
		return false;
	}

	if (PHEAD_ISELVIS(template)) {
		return template->facetex < def->numtexconfigs
			&& def->texconfigs[template->facetex].width == PHEAD_ELVIS_TEXW
			&& def->texconfigs[template->facetex].height == PHEAD_ELVIS_TEXH
			&& def->texconfigs[template->facetex].textureptr != NULL;
	}

	for (i = 0; i < PHEAD_NUMTEX(template); i++) {
		s32 texnum = i == 0 ? template->facetex : template->sidetex[i - 1];

		if (texnum >= def->numtexconfigs
				|| def->texconfigs[texnum].width != PHEAD_TEXW
				|| def->texconfigs[texnum].height != PHEAD_TEXH
				|| def->texconfigs[texnum].textureptr == NULL) {
			return false;
		}
	}

	// A texture that didn't fit in its pool points at the start of the pool,
	// so two of them would be the same memory: don't paint over it
	if (PHEAD_NUMTEX(template) == 3
			&& (def->texconfigs[template->facetex].textureptr == def->texconfigs[template->sidetex[0]].textureptr
				|| def->texconfigs[template->facetex].textureptr == def->texconfigs[template->sidetex[1]].textureptr
				|| def->texconfigs[template->sidetex[0]].textureptr == def->texconfigs[template->sidetex[1]].textureptr)) {
		return false;
	}

	return true;
}

static u8 *pheadGetHeadTexture(struct modeldef *def, const struct pheadtemplate *template, s32 index)
{
	return PHEAD_TEXDATA(def->texconfigs[index == 0 ? template->facetex : template->sidetex[index - 1]].textureptr);
}

/**
 * Save the vertices of the head's own node tree, except those under the hat
 * toggle (on these heads that is the hair cap, which Shape Head leaves alone).
 * Once the head is attached to a body its top-level nodes' parent is the
 * body's headspot, so climbing back up must stop at depth 0 rather than at a
 * NULL parent.
 *
 * The vertices are saved by address: the node tree is walked only here,
 * because the walk is not stable once the model has been rendered.
 */
static void pheadSaveVertices(struct modeldef *def)
{
	struct modelnode *hat = modelGetPart(def, MODELPART_HEAD_HAT);
	struct modelnode *top;
	s32 count = 0;

	g_PheadBackup->zback = 0x7fff;
	g_PheadBackup->zfront = -0x8000;
	g_PheadBackup->ybottom = 0x7fff;
	g_PheadBackup->ytop = -0x8000;

	for (top = def->rootnode; top; top = top->next) {
		struct modelnode *node = top;
		s32 depthlevel = 0;

		while (node) {
			if ((node->type & 0xff) == MODELNODETYPE_DL) {
				struct modelrodata_dl *dl = &node->rodata->dl;
				struct modelnode *ancestor = node;
				bool inhat = false;
				s32 i;

				for (i = 0; i <= depthlevel; i++) {
					if (ancestor == hat) {
						inhat = true;
						break;
					}

					ancestor = ancestor->parent;
				}

				for (i = 0; !inhat && i < dl->numvertices && count < PHEAD_MAXVTX; i++) {
					Vtx *vtx = &dl->vertices[i];

					g_PheadBackup->vertices[count] = vtx;
					g_PheadBackup->origz[count] = vtx->z;
					count++;

					if (vtx->z < g_PheadBackup->zback) g_PheadBackup->zback = vtx->z;
					if (vtx->z > g_PheadBackup->zfront) g_PheadBackup->zfront = vtx->z;
					if (vtx->y < g_PheadBackup->ybottom) g_PheadBackup->ybottom = vtx->y;
					if (vtx->y > g_PheadBackup->ytop) g_PheadBackup->ytop = vtx->y;
				}
			}

			if (node->child) {
				node = node->child;
				depthlevel++;
			} else {
				while (depthlevel > 0 && node->next == NULL) {
					node = node->parent;
					depthlevel--;
				}

				node = depthlevel > 0 ? node->next : NULL;
			}
		}
	}

	g_PheadBackup->numvtx = count;
}

/**
 * Shape Head depth: only the face moves. Vertices in the front part of the
 * head are pushed forward or back in proportion to how far forward they are,
 * and the effect fades out above the brow, so the back of the head, the ears
 * and the hair keep their shape. The head faces +Z with +Y up.
 */
static void pheadApplyDepth(f32 depth)
{
	f32 scale = depth - 1.0f;
	s32 zstart = g_PheadBackup->zfront - (g_PheadBackup->zfront - g_PheadBackup->zback) * 2 / 5;
	s32 zrange = g_PheadBackup->zfront - zstart;
	s32 height = g_PheadBackup->ytop - g_PheadBackup->ybottom;
	s32 ybrow = g_PheadBackup->ybottom + height * 3 / 4;
	s32 yfade = height / 8;
	s32 i;

	if (zrange <= 0 || yfade <= 0) {
		return;
	}

	for (i = 0; i < g_PheadBackup->numvtx; i++) {
		Vtx *vtx = g_PheadBackup->vertices[i];
		s32 z = g_PheadBackup->origz[i];

		if (z > zstart && vtx->y < ybrow) {
			f32 offset = z - zstart;
			f32 weight = offset / zrange;

			if (vtx->y > ybrow - yfade) {
				weight *= (f32) (ybrow - vtx->y) / yfade;
			}

			z += (s32) (offset * weight * scale);
		}

		vtx->z = z;
	}

	// The vertices are spread over the head's display list nodes; the data
	// cache is only 8 KB, so writing it all back is cheaper than per vertex
	osWritebackDCacheAll();

	g_PheadBackup->applieddepth = depth;
}

static void pheadReadTexel(u8 *tex, s32 offset, s32 *r, s32 *g, s32 *b)
{
	u8 *pal = tex + PHEAD_TEXBYTES;
	u16 c = (pal[tex[offset] * 2] << 8) | pal[tex[offset] * 2 + 1];

	*r = ((c >> 11) & 0x1f) << 3;
	*g = ((c >> 6) & 0x1f) << 3;
	*b = ((c >> 1) & 0x1f) << 3;
}

/**
 * Sort the head's texels into skin and hair, and save their shading.
 *
 * The skin reference is the average colour of the face texture's cheeks.
 * A texel is hair if its hue is far from the skin's, if it is much darker, or
 * if it is much less red (blonde hair). A 5x5 majority filter removes speckle.
 */
static void pheadClassifyTexels(struct modeldef *def, const struct pheadtemplate *template)
{
	u8 *face = pheadGetHeadTexture(def, template, 0);
	s32 refr = 0, refg = 0, refb = 0;
	s32 refcr, refcg, reflum, refrg;
	s32 n = 0;
	s32 tex;
	s32 x, y;
	s32 i;

	for (y = 18; y < 26; y++) {
		for (x = 5; x < 27; x++) {
			if (x >= 10 && x < 22) {
				continue;
			}

			{
				s32 r, g, b;
				pheadReadTexel(face, pheadCi8Offset(x, y), &r, &g, &b);
				refr += r;
				refg += g;
				refb += b;
				n++;
			}
		}
	}

	refr /= n;
	refg /= n;
	refb /= n;
	reflum = (refr * 3 + refg * 6 + refb) / 10;
	g_PheadBackup->skinshade = reflum >> 1;
	refcr = refr * 256 / (refr + refg + refb + 1);
	refcg = refg * 256 / (refr + refg + refb + 1);
	refrg = (refr - refg) * 256 / (reflum + 1);
	g_PheadBackup->refcr = refcr;
	g_PheadBackup->refcg = refcg;
	g_PheadBackup->reflum = reflum;
	g_PheadBackup->refrg = refrg;

	for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
		u8 *px = pheadGetHeadTexture(def, template, tex);
		u8 *dst = g_PheadBackup->texels[tex];
		u8 *tmp = (u8 *) g_PheadBackup->origz; // scratch (PHEAD_TEXBYTES): the vertices are saved later

		for (y = 0; y < PHEAD_TEXH; y++) {
			for (x = 0; x < PHEAD_TEXW; x++) {
				s32 r, g, b, lum, sum, dist, rg;
				bool ishair;

				pheadReadTexel(px, pheadCi8Offset(x, y), &r, &g, &b);

				lum = (r * 3 + g * 6 + b) / 10;
				sum = r + g + b + 1;
				dist = ABS(r * 256 / sum - refcr) + ABS(g * 256 / sum - refcg);
				rg = (r - g) * 256 / (lum + 1);

				// The sides have more hair and less skin than the face, so they
				// can use a stricter redness test (catches blonde hair)
				ishair = dist > 22 || lum * 100 < reflum * (tex == 0 ? 50 : template->hairlum)
					|| rg * 100 < refrg * (tex == 0 ? 55 : template->sidered)
					|| (tex == 0 && y >= template->hairrow);

				dst[y * PHEAD_TEXW + x] = (lum >> 1) | (ishair ? PHEAD_HAIR : 0);
			}
		}

		// 5x5 majority filter
		for (y = 0; y < PHEAD_TEXH; y++) {
			for (x = 0; x < PHEAD_TEXW; x++) {
				s32 count = 0;
				s32 dx, dy;

				for (dy = -2; dy <= 2; dy++) {
					for (dx = -2; dx <= 2; dx++) {
						s32 yy = y + dy < 0 ? 0 : y + dy >= PHEAD_TEXH ? PHEAD_TEXH - 1 : y + dy;
						s32 xx = x + dx < 0 ? 0 : x + dx >= PHEAD_TEXW ? PHEAD_TEXW - 1 : x + dx;

						if (dst[yy * PHEAD_TEXW + xx] & PHEAD_HAIR) {
							count++;
						}
					}
				}

				tmp[y * PHEAD_TEXW + x] = count >= 13 ? PHEAD_HAIR : 0;
			}
		}

		for (i = 0; i < PHEAD_TEXBYTES; i++) {
			dst[i] = (dst[i] & ~PHEAD_HAIR) | tmp[i];
		}

		// Sides: the skin there (neck, ear, side of the cheek) reaches down to
		// the neck at the bottom of the texture, or is a big patch (an ear cut
		// off by dark hair). Smaller patches of "skin" elsewhere are hair that
		// looked like skin (fair or ash brown hair), which would otherwise stay
		// skin coloured when the hair colour changes.
		if (tex > 0) {
			u8 *seen = tmp;
			// Scratch: room for PHEAD_TEXBYTES u16s (4 KB) even with 32-bit
			// pointers, and the vertices are saved later
			u16 *queue = (u16 *) g_PheadBackup->vertices;

			bzero(seen, PHEAD_TEXBYTES);

			for (i = 0; i < PHEAD_TEXBYTES; i++) {
				s32 head = 0;
				s32 tail = 0;
				bool neck = false;

				if ((dst[i] & PHEAD_HAIR) || seen[i]) {
					continue;
				}

				queue[tail++] = i;
				seen[i] = true;

				while (head < tail) {
					s32 j = queue[head++];
					s32 jx = j % PHEAD_TEXW;
					s32 jy = j / PHEAD_TEXW;
					s32 k;

					if (jy < PHEAD_NECKROWS) {
						neck = true;
					}

					for (k = 0; k < 4; k++) {
						s32 nx = jx + (k == 0) - (k == 1);
						s32 ny = jy + (k == 2) - (k == 3);
						s32 n = ny * PHEAD_TEXW + nx;

						if (nx >= 0 && nx < PHEAD_TEXW && ny >= 0 && ny < PHEAD_TEXH
								&& !(dst[n] & PHEAD_HAIR) && !seen[n]) {
							seen[n] = true;
							queue[tail++] = n;
						}
					}
				}

				if (!neck && tail < PHEAD_MINSKIN) {
					while (tail > 0) {
						dst[queue[--tail]] |= PHEAD_HAIR;
					}
				}
			}
		}
	}

	// Face: hair only counts if it is connected to the hairline, so dark
	// shading on the cheeks and jaw stays skin. The marks are grown by
	// alternating forward and backward sweeps, using the vertex backup (not
	// captured yet) as scratch space.
	{
		u8 *f = g_PheadBackup->texels[0];
		u8 *mark = (u8 *) g_PheadBackup->origz;
		s32 pass;

		for (i = 0; i < PHEAD_TEXBYTES; i++) {
			mark[i] = (f[i] & PHEAD_HAIR) && i / PHEAD_TEXW >= template->hairrow;
		}

		for (pass = 0; pass < 32; pass++) {
			bool changed = false;

			for (i = 0; i < PHEAD_TEXBYTES; i++) {
				s32 j = (pass & 1) ? PHEAD_TEXBYTES - 1 - i : i;

				if (!mark[j] && (f[j] & PHEAD_HAIR)) {
					s32 jx = j % PHEAD_TEXW;

					if ((jx > 0 && mark[j - 1])
							|| (jx < PHEAD_TEXW - 1 && mark[j + 1])
							|| (j >= PHEAD_TEXW && mark[j - PHEAD_TEXW])
							|| (j < PHEAD_TEXBYTES - PHEAD_TEXW && mark[j + PHEAD_TEXW])) {
						mark[j] = 1;
						changed = true;
					}
				}
			}

			if (!changed) {
				break;
			}
		}

		for (i = 0; i < PHEAD_TEXBYTES; i++) {
			f[i] = (f[i] & ~PHEAD_HAIR) | (mark[i] ? PHEAD_HAIR : 0);
		}
	}

	// Hand-painted hair masks (tools/pheadmask, src/game/pheadmasks.c) take
	// the place of everything above for the textures that have one
	for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
		const u32 *mask = g_PheadHairMasks[template - g_PheadTemplates][tex];

		if (mask) {
			u8 *dst = g_PheadBackup->texels[tex];

			for (y = 0; y < PHEAD_TEXH; y++) {
				for (x = 0; x < PHEAD_TEXW; x++) {
					u8 *texel = &dst[y * PHEAD_TEXW + x];

					*texel = (*texel & ~PHEAD_HAIR) | ((mask[y] >> x) & 1 ? PHEAD_HAIR : 0);
				}
			}
		}
	}

	// Face: the picture goes on the face texture's skin, i.e. everything not
	// marked as hair (by the hand-painted mask or the classifier above), so the
	// head's own hairline is kept. The face square (the same square as the
	// placement guide) still limits it at the sides and below the chin, so the
	// picture's background doesn't reach the neck. Above the square's middle
	// only the hair and hairrow limit it.
	{
		u8 *f = g_PheadBackup->texels[0];
		s32 span = template->eyerow - template->mouthrow;
		s32 top = template->eyerow + span * 13 / 10;
		s32 bottom = template->mouthrow - span * 3 / 5;
		f32 cy = (top + bottom) * 0.5f;
		f32 ry = (top - bottom) * 0.5f;
		s32 hairsum = 0;
		s32 haircount = 0;

		for (y = 0; y < PHEAD_TEXH; y++) {
			for (x = 0; x < PHEAD_TEXW; x++) {
				f32 dx = (x + 0.5f - PHEAD_TEXW * 0.5f) / PHEAD_FACEBOX_RX;
				f32 dy = (y + 0.5f - cy) / ry;
				f32 dist = MAX(dx * dx, dy < 0.0f ? dy * dy : 0.0f); // squared, 1 = on the square's edge
				s32 weight = 0;
				u8 *texel = &f[y * PHEAD_TEXW + x];

				// Full picture in the middle, fading into the head's own skin
				// towards the square's edge, the hair and hairrow
				if (dist < 1.0f && y < template->hairrow && (*texel & PHEAD_HAIR) == 0) {
					s32 hairdist = 3; // Chebyshev distance to the nearest hair texel, up to 3
					s32 nx, ny;

					weight = dist < 0.75f ? 255 : (s32) ((1.0f - dist) * (255.0f / 0.25f));

					if (template->hairrow - y < 4) {
						weight = weight * (template->hairrow - y) / 4;
					}

					for (ny = y - 2; ny <= y + 2; ny++) {
						for (nx = x - 2; nx <= x + 2; nx++) {
							if (ny >= 0 && ny < PHEAD_TEXH && nx >= 0 && nx < PHEAD_TEXW
									&& (f[ny * PHEAD_TEXW + nx] & PHEAD_HAIR)) {
								s32 d = MAX(ABS(nx - x), ABS(ny - y));

								if (d < hairdist) {
									hairdist = d;
								}
							}
						}
					}

					weight = weight * hairdist / 3;
				}

				g_PheadBackup->picture[y * PHEAD_TEXW + x] = weight;
			}
		}

		// Average hair shade, so hair can be normalised like skin
		for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
			for (i = 0; i < PHEAD_TEXBYTES; i++) {
				if (g_PheadBackup->texels[tex][i] & PHEAD_HAIR) {
					hairsum += g_PheadBackup->texels[tex][i] & ~PHEAD_HAIR;
					haircount++;
				}
			}
		}

		g_PheadBackup->hairshade = haircount ? hairsum / haircount : PHEAD_HAIRSHADE;
	}
}

/**
 * Palette index for a saved texel outside the picture. Skin and hair are
 * rescaled so that the head's cheeks and its average hair sit at fixed points
 * on their ramps, whatever the original head's colouring, and highlights
 * don't reach the white end of the ramp.
 */
static u8 pheadTexelIndex(u8 texel)
{
	s32 shade = texel & ~PHEAD_HAIR;

	if (texel & PHEAD_HAIR) {
		// Also soften the contrast, so streaks and partings don't turn white
		shade = PHEAD_HAIRSHADE + (shade - g_PheadBackup->hairshade) * PHEAD_HAIRSHADE * 3 / 5 / (g_PheadBackup->hairshade + 1);

		if (shade < 0) {
			shade = 0;
		}

		return 128 + (shade > PHEAD_HAIRSHADE_MAX ? PHEAD_HAIRSHADE_MAX : shade);
	}

	shade = shade * PHEAD_SKINSHADE / (g_PheadBackup->skinshade + 1);

	return shade > PHEAD_SKINSHADE_MAX ? PHEAD_SKINSHADE_MAX : shade;
}

/**
 * Record the original colour of the head's hair, for the Normal hair colour:
 * the average colour of the hair texels in each band of PHEAD_HAIRBANDS
 * shades, by the shade the painter gives them (pheadTexelIndex). Bands
 * with no hair take the nearest band's colour, scaled to their shade.
 * Must run before the textures are first painted.
 */
static void pheadCaptureHairColours(struct modeldef *def, const struct pheadtemplate *template)
{
	s32 sums[PHEAD_HAIRBANDS][4];
	s32 tex, i, j;
	s32 nearest;

	bzero(sums, sizeof(sums));

	for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
		u8 *px = pheadGetHeadTexture(def, template, tex);
		u8 *texels = g_PheadBackup->texels[tex];

		for (i = 0; i < PHEAD_TEXBYTES; i++) {
			if (texels[i] & PHEAD_HAIR) {
				s32 band = (pheadTexelIndex(texels[i]) & 0x7f) * PHEAD_HAIRBANDS / 128;
				s32 r, g, b;

				pheadReadTexel(px, pheadCi8Offset(i % PHEAD_TEXW, i / PHEAD_TEXW), &r, &g, &b);

				sums[band][0] += r;
				sums[band][1] += g;
				sums[band][2] += b;
				sums[band][3]++;
			}
		}
	}

	for (i = 0; i < PHEAD_HAIRBANDS; i++) {
		// Nearest band with hair, preferring the darker one
		nearest = -1;

		for (j = 0; j < PHEAD_HAIRBANDS && nearest < 0; j++) {
			if (i - j >= 0 && sums[i - j][3]) {
				nearest = i - j;
			} else if (i + j < PHEAD_HAIRBANDS && sums[i + j][3]) {
				nearest = i + j;
			}
		}

		for (j = 0; j < 3; j++) {
			s32 c;

			if (nearest < 0) {
				c = 0x30; // no hair at all: dark grey
			} else {
				c = sums[nearest][j] / sums[nearest][3] * (i * 2 + 1) / (nearest * 2 + 1);
			}

			g_PheadBackup->hairorig[i][j] = c > 255 ? 255 : c;
		}
	}
}

/**
 * Is this colour hair, by the side textures' test (see pheadClassifyTexels)?
 */
static bool pheadIsHairColour(const struct pheadtemplate *template, s32 r, s32 g, s32 b)
{
	s32 lum = (r * 3 + g * 6 + b) / 10;
	s32 sum = r + g + b + 1;
	s32 dist = ABS(r * 256 / sum - g_PheadBackup->refcr) + ABS(g * 256 / sum - g_PheadBackup->refcg);
	s32 rg = (r - g) * 256 / (lum + 1);

	return dist > 22 || lum * 100 < g_PheadBackup->reflum * template->hairlum
		|| rg * 100 < g_PheadBackup->refrg * template->sidered;
}

/**
 * Check the face and side textures are what the painter writes (CI8, 32x64,
 * one level, palette straight after) and note their palette sizes. Find the
 * head's other CI8 textures of at least 32x32 (on these heads, the back of a
 * ponytail; the small CI4 ones are sunglasses and the HUD piece) and save
 * their palettes, which are recoloured instead of their texels.
 */
static void pheadCaptureTextures(struct modeldef *def, const struct pheadtemplate *template, u8 *texpoolend)
{
	s32 i;

	g_PheadBackup->numextras = 0;

	for (i = 0; i < PHEAD_NUMTEX(template); i++) {
		struct tex *tex = pheadFindTex(texpoolend, pheadGetHeadTexture(def, template, i));

		g_PheadBackup->numcolours[i] = 0;

		if (tex && tex->depth == G_IM_SIZ_8b && tex->width == PHEAD_TEXW && tex->height == PHEAD_TEXH
				&& pheadTexImageBytes(tex) == PHEAD_TEXBYTES && tex->unk0a >= 15) {
			g_PheadBackup->numcolours[i] = tex->unk0a + 1;
		}
	}

	for (i = 0; i < def->numtexconfigs && g_PheadBackup->numextras < PHEAD_MAXEXTRAS; i++) {
		u8 *data = PHEAD_TEXDATA(def->texconfigs[i].textureptr);
		struct tex *tex;
		s32 imagebytes;
		s32 j, k;

		if (i == template->facetex || i == template->sidetex[0] || i == template->sidetex[1]) {
			continue;
		}

		tex = pheadFindTex(texpoolend, data);

		if (tex == NULL || tex->depth != G_IM_SIZ_8b || tex->width * tex->height < 32 * 32) {
			continue;
		}

		imagebytes = pheadTexImageBytes(tex);

		if (imagebytes <= 0) {
			continue;
		}

		j = g_PheadBackup->numextras++;
		g_PheadBackup->extras[j].data = data;
		g_PheadBackup->extras[j].palette = data + imagebytes;
		g_PheadBackup->extras[j].count = tex->unk0a + 1;

		for (k = 0; k < g_PheadBackup->extras[j].count; k++) {
			u8 *pal = g_PheadBackup->extras[j].palette;
			g_PheadBackup->extras[j].orig[k] = (pal[k * 2] << 8) | pal[k * 2 + 1];
		}
	}
}

#ifndef PLATFORM_N64
/**
 * With --phead-texdump, write the base head's face and side textures (before
 * painting) to the save folder for the hair mask editor, tools/pheadmask:
 * pheadtex_<template>_<0 face, 1-2 sides>.bin, holding "PHTX", the template,
 * the texture, the width, the height and the number of colours as 32-bit
 * little-endian values, then the 32x64 palette indices (row 0 = the bottom)
 * and the RGBA5551 palette.
 */
static void pheadExportTextures(struct modeldef *def, const struct pheadtemplate *template, s32 templatenum)
{
	char name[32];
	s32 i;

	if (!sysArgCheck("--phead-texdump")) {
		return;
	}

	for (i = 0; i < PHEAD_NUMTEX(template); i++) {
		u8 *px = pheadGetHeadTexture(def, template, i);
		s32 hdr[6];
		s32 y;
		FILE *f;

		if (g_PheadBackup->numcolours[i] == 0) {
			continue;
		}

		sprintf(name, "$S/pheadtex_%02d_%d.bin", templatenum, i);
		f = fsFileOpenWrite(name);

		if (!f) {
			continue;
		}

		hdr[0] = 0x58544850; // "PHTX"
		hdr[1] = templatenum;
		hdr[2] = i;
		hdr[3] = PHEAD_TEXW;
		hdr[4] = PHEAD_TEXH;
		hdr[5] = g_PheadBackup->numcolours[i];
		fwrite(hdr, sizeof(hdr), 1, f);

		for (y = 0; y < PHEAD_TEXH; y++) {
			fwrite(&px[pheadCi8Offset(0, y)], PHEAD_TEXW, 1, f);
		}

		fwrite(px + PHEAD_TEXBYTES, g_PheadBackup->numcolours[i] * 2, 1, f);
		fclose(f);
	}

	sysLogPrintf(LOG_NOTE, "phead: exported template %d's textures for tools/pheadmask", templatenum);
}
#endif

/**
 * Offset of texel x in row y of a 4-bit texture level with <rowbytes> bytes a
 * row. On N64 the odd rows are TMEM swizzled like 8-bit ones (texSwizzle).
 */
static s32 pheadCi4Offset(s32 x, s32 y, s32 rowbytes)
{
#ifdef PLATFORM_N64
	return y * rowbytes + ((x >> 1) ^ ((y & 1) << 2));
#else
	return y * rowbytes + (x >> 1);
#endif
}

static s32 pheadCi4Get(u8 *data, s32 x, s32 y, s32 rowbytes)
{
	u8 byte = data[pheadCi4Offset(x, y, rowbytes)];

	return x & 1 ? byte & 0xf : byte >> 4;
}

static void pheadCi4Set(u8 *data, s32 x, s32 y, s32 rowbytes, s32 value)
{
	u8 *byte = &data[pheadCi4Offset(x, y, rowbytes)];

	*byte = x & 1 ? (*byte & 0xf0) | value : (*byte & 0x0f) | (value << 4);
}

/**
 * Elvis's virtual face texel for his skin texel <s>, <t> (see PHEAD_ELVIS_S0),
 * or false if it's outside the virtual face.
 */
static bool pheadElvisVirtual(s32 s, s32 t, s32 *c, s32 *r)
{
	f32 fc = (s + 0.5f - PHEAD_ELVIS_S0) * PHEAD_TEXW / PHEAD_ELVIS_SW;
	f32 fr = (t + 0.5f - PHEAD_ELVIS_T0) * PHEAD_TEXH / PHEAD_ELVIS_TH;

	if (fc < 0.0f || fr < 0.0f || fc >= PHEAD_TEXW || fr >= PHEAD_TEXH) {
		return false;
	}

	*c = (s32) fc;
	*r = (s32) fr;

	return true;
}

static s32 pheadElvisLum(u8 *pal, s32 index)
{
	u16 c = (pal[index * 2] << 8) | pal[index * 2 + 1];

	return ((((c >> 11) & 0x1f) << 3) * 3 + (((c >> 6) & 0x1f) << 3) * 6 + (((c >> 1) & 0x1f) << 3)) / 10;
}

/**
 * Elvis's backup: his skin's own texels (texels[0], unpacked), the virtual
 * face's picture weights (a square like the other heads' face area, limited
 * at the top too as he has no hair) and the shade of his skin under it.
 */
static void pheadCaptureElvis(struct modeldef *def, const struct pheadtemplate *template, u8 *texpoolend)
{
	u8 *px = pheadGetHeadTexture(def, template, 0);
	struct tex *tex = pheadFindTex(texpoolend, px);
	s32 span = template->eyerow - template->mouthrow;
	s32 top = template->eyerow + span * 13 / 10;
	s32 bottom = template->mouthrow - span * 3 / 5;
	f32 cy = (top + bottom) * 0.5f;
	f32 ry = (top - bottom) * 0.5f;
	s32 sum = 0;
	s32 count = 0;
	s32 x, y, c, r;

	g_PheadBackup->numextras = 0;
	g_PheadBackup->numcolours[0] = g_PheadBackup->numcolours[1] = g_PheadBackup->numcolours[2] = 0;

	if (tex == NULL || tex->depth != G_IM_SIZ_4b || tex->width != PHEAD_ELVIS_TEXW || tex->height != PHEAD_ELVIS_TEXH
			|| pheadTexImageBytes(tex) <= 0 || tex->unk0a + 1 != 16) {
		return;
	}

	g_PheadBackup->elvislods = tex->numlods > 1 ? tex->numlods : 1;
	g_PheadBackup->elvispalette = pheadTexImageBytes(tex);

	for (y = 0; y < PHEAD_TEXH; y++) {
		for (x = 0; x < PHEAD_TEXW; x++) {
			f32 dx = (x + 0.5f - PHEAD_TEXW * 0.5f) / PHEAD_FACEBOX_RX;
			f32 dy = (y + 0.5f - cy) / ry;
			f32 dist = MAX(dx * dx, dy * dy);

			g_PheadBackup->picture[y * PHEAD_TEXW + x] = dist >= 1.0f ? 0
				: dist < 0.75f ? 255 : (s32) ((1.0f - dist) * (255.0f / 0.25f));
		}
	}

	for (y = 0; y < PHEAD_ELVIS_TEXH; y++) {
		for (x = 0; x < PHEAD_ELVIS_TEXW; x++) {
			s32 index = pheadCi4Get(px, x, y, PHEAD_ELVIS_ROWBYTES);

			g_PheadBackup->texels[0][y * PHEAD_ELVIS_TEXW + x] = index;

			if (pheadElvisVirtual(x, y, &c, &r) && g_PheadBackup->picture[r * PHEAD_TEXW + c] >= 128) {
				sum += pheadElvisLum(px + g_PheadBackup->elvispalette, index);
				count++;
			}
		}
	}

	g_PheadBackup->skinshade = count ? sum / count : 128;
	g_PheadBackup->numcolours[0] = g_PheadBackup->numcolours[1] = g_PheadBackup->numcolours[2] = 16;
}

static void pheadCaptureBackup(struct modeldef *def, s32 templatenum, u8 *texpoolend)
{
	const struct pheadtemplate *template = pheadGetTemplate(templatenum);

	if (g_PheadBackup == NULL) {
		g_PheadBackup = mempAlloc(align16(sizeof(struct pheadbackup)), MEMPOOL_STAGE);
	}

	g_PheadBackup->def = def;
	g_PheadBackup->templatenum = templatenum;

	if (PHEAD_ISELVIS(template)) {
		pheadCaptureElvis(def, template, texpoolend);
	} else {
		pheadClassifyTexels(def, template);
		pheadCaptureHairColours(def, template);
		pheadCaptureTextures(def, template, texpoolend);

#ifndef PLATFORM_N64
		pheadExportTextures(def, template, templatenum);
#endif
	}

	pheadSaveVertices(def);
	g_PheadBackup->applieddepth = 1.0f;
}

/**
 * Shape Head (camdraw's squash Z, 0.25-1.9)
 */
static void pheadApplySlotDepth(struct var8007f8e0 *slot)
{
	f32 depth = slot->unk3d0[6];

	if (depth < 0.1f || depth > 1.9f) {
		depth = 1.0f;
	}

	if (depth != g_PheadBackup->applieddepth) {
		pheadApplyDepth(depth);
	}
}

/**
 * Elvis's skin: the picture through the crop over his virtual face, levelled
 * like the other heads' faces but onto the shades of his own palette (which
 * is left as it is: his grey stays, and skin tone and hair colour don't
 * apply). The smaller levels of detail are remade from the first.
 */
static void pheadPaintElvis(struct modeldef *def, const struct pheadtemplate *template, struct textureconfig *face,
		s32 x1, s32 y1, s32 x2, s32 y2, s32 lummin, s32 lummed, s32 lummax, s32 photoskin)
{
	u8 *px = pheadGetHeadTexture(def, template, 0);
	u8 *pal = px + g_PheadBackup->elvispalette;
	s32 lums[16];
	s32 lummaxpal = 0;
	s32 skinlum = g_PheadBackup->skinshade;
	s32 width = PHEAD_ELVIS_TEXW;
	s32 height = PHEAD_ELVIS_TEXH;
	s32 rowbytes = PHEAD_ELVIS_ROWBYTES;
	u8 *level = px;
	s32 x, y, c, r, i, lod;

	for (i = 0; i < 16; i++) {
		lums[i] = pheadElvisLum(pal, i);

		if (lums[i] > lummaxpal) {
			lummaxpal = lums[i];
		}
	}

	for (y = 0; y < PHEAD_ELVIS_TEXH; y++) {
		for (x = 0; x < PHEAD_ELVIS_TEXW; x++) {
			s32 orig = g_PheadBackup->texels[0][y * PHEAD_ELVIS_TEXW + x];
			s32 weight = 0;
			s32 best = orig;

			if (pheadElvisVirtual(x, y, &c, &r)) {
				weight = g_PheadBackup->picture[r * PHEAD_TEXW + c];
			}

			if (weight > 0) {
				s32 ix = x1 + ((c * 2 + 1) * (x2 - x1)) / (PHEAD_TEXW * 2);
				s32 iy = y2 - ((r * 2 + 1) * (y2 - y1)) / (PHEAD_TEXH * 2);
				s32 inside = MIN(MIN(ix, 63 - ix), MIN(iy, 63 - iy));

				// Past the picture (Position Image) he keeps his own skin
				if (inside < 0) {
					weight = 0;
				} else if (inside < 2) {
					weight = weight * (inside + 1) / 3;
				}

				if (weight > 0) {
					s32 row = 63 - iy;
					s32 shade = face->textureptr[row * 64 + ix + CAM_PHEAD_TMEMSWAP(row, ix)];
					s32 target;
					s32 lum;

					// As the other heads: the face's own skin becomes
					// photoskin, which here is his skin's shade
					if (shade <= lummed) {
						shade = photoskin - (lummed - shade) * photoskin / (lummed - lummin);
					} else {
						shade = photoskin + (shade - lummed) * (127 - photoskin) / (lummax - lummed);
					}

					if (shade <= photoskin) {
						target = skinlum * shade / photoskin;
					} else {
						target = skinlum + (shade - photoskin) * (lummaxpal - skinlum) / (127 - photoskin);
					}

					lum = (target * weight + lums[orig] * (255 - weight)) / 255;

					for (i = 0, best = 0; i < 16; i++) {
						if (ABS(lums[i] - lum) < ABS(lums[best] - lum)) {
							best = i;
						}
					}
				}
			}

			pheadCi4Set(px, x, y, PHEAD_ELVIS_ROWBYTES, best);
		}
	}

	// Smaller levels of detail: the nearest shade to each 2x2's average
	for (lod = 1; lod < g_PheadBackup->elvislods && width > 1; lod++) {
		u8 *next = level + rowbytes * height;
		s32 nwidth = (width + 1) >> 1;
		s32 nheight = (height + 1) >> 1;
		s32 nrowbytes = (((nwidth + 1) >> 1) + 7) & ~7;

		for (y = 0; y < nheight; y++) {
			for (x = 0; x < nwidth; x++) {
				s32 sum = 0;
				s32 n = 0;
				s32 best = 0;
				s32 dx, dy;

				for (dy = 0; dy < 2; dy++) {
					for (dx = 0; dx < 2; dx++) {
						if (x * 2 + dx < width && y * 2 + dy < height) {
							sum += lums[pheadCi4Get(level, x * 2 + dx, y * 2 + dy, rowbytes)];
							n++;
						}
					}
				}

				sum /= n;

				for (i = 1; i < 16; i++) {
					if (ABS(lums[i] - sum) < ABS(lums[best] - sum)) {
						best = i;
					}
				}

				pheadCi4Set(next, x, y, nrowbytes, best);
			}
		}

		level = next;
		width = nwidth;
		height = nheight;
		rowbytes = nrowbytes;
	}

	osWritebackDCache(px, g_PheadBackup->elvispalette);

#ifndef PLATFORM_N64
	videoFreeCachedTextures(px, px + g_PheadBackup->elvispalette + 32);
#endif
}

/**
 * Camera slot <slotindex>'s crop rectangle (Position Image) in the 64x64
 * picture, which may reach past its edges. A slot without a usable one (a new
 * head) gets the default framing.
 */
void pheadGetCrop(s32 slotindex, s32 *x1, s32 *y1, s32 *x2, s32 *y2)
{
	struct var8007f8e0 *slot = func0f14a06c(slotindex);

	*x1 = slot->unk3c0;
	*y1 = slot->unk3c8;
	*x2 = slot->unk3c4;
	*y2 = slot->unk3cc;

	if (*x2 <= *x1 + 4 || *y2 <= *y1 + 4) {
		*x1 = 10;
		*y1 = 3;
		*x2 = 54;
		*y2 = 61;
	}
}

/**
 * Paint camera slot <slotindex> (camdraw logical index, -1 = editor) onto a
 * freshly loaded or already painted head model. The base head is the slot's
 * template (camdraw's unk3a4, "Change Head").
 */
void pheadPaintHead(struct modeldef *def, s32 slotindex, bool fresh, u8 *texpoolend)
{
	struct var8007f8e0 *slot;
	struct textureconfig *face;
	const struct pheadtemplate *template;
	s32 templatenum;
	s32 x1, y1, x2, y2;
	s32 skin;
	s32 hair;
	s32 tex;
	s32 tx, ty;
	s32 lummin, lummed, lummax;
	s32 photoskin;
	s32 level;
	u8 *px;

	if (var8007f8e0 == NULL) {
		return;
	}

	slot = func0f14a06c(slotindex);
	templatenum = slot->unk3a4;

	if (templatenum < 0 || templatenum >= ARRAYCOUNT(g_PheadTemplates)) {
		templatenum = 0;
	}

	template = pheadGetTemplate(templatenum);
	face = &slot->unk004;

	if (face->textureptr == NULL || !pheadHeadIsPaintable(def, template)) {
		return;
	}

	// A freshly loaded head has its original textures and vertices again
	if (fresh || g_PheadBackup == NULL || g_PheadBackup->def != def || g_PheadBackup->templatenum != templatenum) {
		pheadCaptureBackup(def, templatenum, texpoolend);
	}

	// Only write textures known to be what the painter expects
	for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
		if (g_PheadBackup->numcolours[tex] == 0) {
			return;
		}
	}

	skin = slot->stylenum & 3;
	hair = slot->colournum == PHEAD_HAIRCOLOUR_NORMAL ? -1 : 4 + (slot->colournum & 3);

	pheadGetCrop(slotindex, &x1, &y1, &x2, &y2);

	// Auto-levels: stretch the face over the skin ramp. The Game Boy Camera
	// only has four shades and the editor image is blurred, so faces otherwise
	// sit in a narrow band of the ramp. Only the part of the picture that is
	// painted (the face oval) counts, without its darkest and brightest few
	// percent: a dark shirt or bright background around a real face would
	// otherwise take the whole range and flatten the face.
	{
		u16 histogram[64];
		s32 count = 0;
		s32 cut;
		s32 sum;

		bzero(histogram, sizeof(histogram));

		for (ty = 0; ty < PHEAD_TEXH; ty++) {
			s32 iy = y2 - ((ty * 2 + 1) * (y2 - y1)) / (PHEAD_TEXH * 2);

			// The crop may reach past the picture (Position Image): only
			// the picture itself counts
			if (iy < 0 || iy > 63) {
				continue;
			}

			for (tx = 0; tx < PHEAD_TEXW; tx++) {
				if (g_PheadBackup->picture[ty * PHEAD_TEXW + tx] >= 128) {
					s32 ix = x1 + ((tx * 2 + 1) * (x2 - x1)) / (PHEAD_TEXW * 2);
					s32 row = 63 - iy;

					if (ix < 0 || ix > 63) {
						continue;
					}

					histogram[face->textureptr[row * 64 + ix + CAM_PHEAD_TMEMSWAP(row, ix)] >> 2]++;
					count++;
				}
			}
		}

		cut = count / 25; // 4%
		lummin = 0;
		lummed = 128;
		lummax = 255;

		for (sum = 0, level = 0; level < 64; level++) {
			sum += histogram[level];

			if (sum > cut) {
				lummin = level * 4;
				break;
			}
		}

		for (sum = 0, level = 63; level >= 0; level--) {
			sum += histogram[level];

			if (sum > cut) {
				lummax = level * 4 + 3;
				break;
			}
		}

		// The median: the face's own skin
		for (sum = 0, level = 0; level < 64; level++) {
			sum += histogram[level];

			if (sum > count / 2) {
				lummed = level * 4 + 2;
				break;
			}
		}

		if (count == 0 || lummax - lummin < 16) {
			lummin = 0;
			lummed = 128;
			lummax = 255;
		}

		if (lummed <= lummin) lummed = lummin + 1;
		if (lummed >= lummax) lummed = lummax - 1;
	}

	// Position Image's Brightness (camdraw's unk3bc, 128 = neutral). The
	// levels above take out the picture's own brightness, so apply it here:
	// it moves the face's skin up or down the ramp.
	photoskin = PHEAD_SKINSHADE + (slot->unk3bc - 128) * PHEAD_SKINSHADE / 160;

	if (photoskin < 16) photoskin = 16;
	if (photoskin > 112) photoskin = 112;

	if (PHEAD_ISELVIS(template)) {
		pheadPaintElvis(def, template, face, x1, y1, x2, y2, lummin, lummed, lummax, photoskin);
		pheadApplySlotDepth(slot);
		return;
	}

	// Face: the picture through the crop rectangle, except the head's own hair
	// around it. Texture row 0 is the chin.
	px = pheadGetHeadTexture(def, template, 0);

	for (ty = 0; ty < PHEAD_TEXH; ty++) {
		s32 iy = y2 - ((ty * 2 + 1) * (y2 - y1)) / (PHEAD_TEXH * 2);

		for (tx = 0; tx < PHEAD_TEXW; tx++) {
			u8 class = g_PheadBackup->texels[0][ty * PHEAD_TEXW + tx];
			s32 ix;
			s32 row;
			u8 lum;

			s32 weight = g_PheadBackup->picture[ty * PHEAD_TEXW + tx];
			s32 inside;

			ix = x1 + ((tx * 2 + 1) * (x2 - x1)) / (PHEAD_TEXW * 2);

			// The crop may reach past the picture (Position Image). There
			// the head keeps its own skin, the picture fading out over its
			// last two pixels
			inside = MIN(MIN(ix, 63 - ix), MIN(iy, 63 - iy));

			if (inside < 0) {
				weight = 0;
			} else if (inside < 2) {
				weight = weight * (inside + 1) / 3;
			}

			if (weight == 0) {
				px[pheadCi8Offset(tx, ty)] = pheadPaletteIndex(pheadTexelIndex(class), g_PheadBackup->numcolours[0]);
				continue;
			}

			// The slot's face texture is stored upside down (and swizzled on N64)
			row = 63 - iy;
			lum = face->textureptr[row * 64 + ix + CAM_PHEAD_TMEMSWAP(row, ix)];

			// The face's own skin becomes photoskin (the head's cheek shade
			// at neutral brightness), so the picture blends into the rest of
			// the head; darker and lighter parts are stretched to the ends of
			// the skin ramp
			if (lum <= lummed) {
				level = photoskin - (lummed - lum) * photoskin / (lummed - lummin);
			} else {
				level = photoskin + (lum - lummed) * (127 - photoskin) * (PHEAD_SKINSHADE_MAX - PHEAD_SKINSHADE) / ((127 - PHEAD_SKINSHADE) * (lummax - lummed));
			}

			if (level < 0) level = 0;
			if (level > 127) level = 127;

			// Fade into plain skin rather than the head's own shading, which
			// may have stubble or a mouth near the edge of the face area
			if (weight < 255) {
				level = (level * weight + PHEAD_SKINSHADE * (255 - weight)) / 255;
			}

			px[pheadCi8Offset(tx, ty)] = pheadPaletteIndex(level, g_PheadBackup->numcolours[0]);
		}
	}

	// Sides: keep the original shading, remapped onto the skin/hair ramps
	for (tex = 1; tex < PHEAD_NUMTEX(template); tex++) {
		px = pheadGetHeadTexture(def, template, tex);

		for (ty = 0; ty < PHEAD_TEXH; ty++) {
			for (tx = 0; tx < PHEAD_TEXW; tx++) {
				// bit 7 selects the hair half of the palette, bits 0-6 the shade
				px[pheadCi8Offset(tx, ty)] = pheadPaletteIndex(pheadTexelIndex(g_PheadBackup->texels[tex][ty * PHEAD_TEXW + tx]),
						g_PheadBackup->numcolours[tex]);
			}
		}
	}

	for (tex = 0; tex < PHEAD_NUMTEX(template); tex++) {
		s32 numcolours = g_PheadBackup->numcolours[tex];
		s32 skinentries = pheadSkinEntries(numcolours);

		px = pheadGetHeadTexture(def, template, tex);

		pheadWriteRamp(px + PHEAD_TEXBYTES, skin, PHEAD_SKINSHADE, skinentries);
		pheadWriteRamp(px + PHEAD_TEXBYTES + skinentries * 2, hair, PHEAD_HAIRSHADE, numcolours - skinentries);

		osWritebackDCache(px, PHEAD_TEXBYTES + numcolours * 2);

#ifndef PLATFORM_N64
		videoFreeCachedTextures(px, px + PHEAD_TEXBYTES + numcolours * 2);
#endif
	}

	// Other hair textures: recolour their palettes, keeping each colour's
	// shade and transparency
	for (tex = 0; tex < g_PheadBackup->numextras; tex++) {
		u8 *pal = g_PheadBackup->extras[tex].palette;
		s32 i;

		for (i = 0; i < g_PheadBackup->extras[tex].count; i++) {
			u16 orig = g_PheadBackup->extras[tex].orig[i];
			s32 r = ((orig >> 11) & 0x1f) << 3;
			s32 g = ((orig >> 6) & 0x1f) << 3;
			s32 b = ((orig >> 1) & 0x1f) << 3;
			bool ishair = pheadIsHairColour(template, r, g, b);
			s32 value = pheadTexelIndex(((r * 3 + g * 6 + b) / 10 >> 1) | (ishair ? PHEAD_HAIR : 0));
			u16 c = ishair
				? pheadShadeColour(hair, PHEAD_HAIRSHADE, value & 0x7f)
				: pheadRampColour(skin, PHEAD_SKINSHADE, value & 0x7f);

			c = (c & ~1) | (orig & 1);

			// Normal hair: hair colours are left as they were
			if (ishair && hair < 0) {
				c = orig;
			}

			pal[i * 2 + 0] = c >> 8;
			pal[i * 2 + 1] = c & 0xff;
		}

		osWritebackDCache(pal, g_PheadBackup->extras[tex].count * 2);

#ifndef PLATFORM_N64
		videoFreeCachedTextures(g_PheadBackup->extras[tex].data, pal + g_PheadBackup->extras[tex].count * 2);
#endif
	}

	pheadApplySlotDepth(slot);
}

static bool g_PheadEditing = false;

/**
 * While the PerfectHead Editor is open the menu model shows the editor slot.
 */
void pheadSetEditing(bool editing)
{
	g_PheadEditing = editing;
}

s32 pheadGetPaintSlot(s32 perfectheadnum)
{
	return g_PheadEditing ? -1 : perfectheadnum;
}

static void pheadSetToggle(struct menumodel *menumodel, s32 partnum, bool visible)
{
	struct modelnode *node = modelGetPart(menumodel->bodymodeldef, partnum);

	if (node && (node->type & 0xff) == MODELNODETYPE_TOGGLE) {
		union modelrwdata *rwdata = modelGetNodeRwData(&menumodel->bodymodel, node);

		if (rwdata) {
			rwdata->toggle.visible = visible;
		}
	}
}

/**
 * Called by the menu when it has loaded a model from a file. The PerfectHead
 * Editor shows the base head on its own: if this is the editor's base head,
 * set it up like body.c does for a head on a body and paint it.
 */
bool pheadMenuModelLoaded(struct menumodel *menumodel)
{
	s32 slotindex;

	if (var8007f8e0 == NULL || menumodel->bodymodeldef == NULL) {
		return false;
	}

	if (g_PheadEditing) {
		slotindex = -1;
	} else if (menumodel->isperfecthead && pheadIsHeadLoaded(menumodel->perfectheadnum)) {
		// A PerfectHead in the Combat Simulator's head carousel
		slotindex = menumodel->perfectheadnum;
	} else {
		return false;
	}

	if (MENUMODELPARAMS_GET_FILENUM(menumodel->newparams) != pheadGetTemplateFilenum(func0f14a9f8(slotindex))) {
		return false;
	}

	pheadSetToggle(menumodel, MODELPART_HEAD_SUNGLASSES, false);
	pheadSetToggle(menumodel, MODELPART_HEAD_HUDPIECE, false);
	pheadSetToggle(menumodel, MODELPART_HEAD_EYESCLOSED, false);
	pheadSetToggle(menumodel, MODELPART_HEAD_EYESOPEN, true);

	pheadPaintHead(menumodel->bodymodeldef, slotindex, true, menumodel->allocstart + menumodel->alloclen);

	return true;
}

/**
 * Loaded heads and the Controller Pak
 *
 * camdraw's "current head" (logical slot -2) is var800a45a0->unk004. The
 * editor always works on the editor slot (-1); Keep Changes copies it into
 * the current head.
 */
s32 pheadGetCurrentHead(void)
{
	return var800a45a0->unk004;
}

static void pheadSetCurrentHead(s32 headnum)
{
	var800a45a0->unk004 = headnum;
}

bool pheadIsHeadLoaded(s32 headnum)
{
	return var8007f8e0 && headnum >= 0 && headnum < PHEAD_NUM_HEADS && func0f14a06c(headnum)->unk3f4_00;
}

s32 pheadCountLoadedHeads(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < PHEAD_NUM_HEADS; i++) {
		if (pheadIsHeadLoaded(i)) {
			count++;
		}
	}

	return count;
}

/**
 * Where each head can be used (PHEAD_USE_*). Make A New Head and Load A Saved
 * Head ask, and the answer stays with the loaded head in memory. It isn't
 * saved in the camera file: the file's two flag bits that look free are
 * camdraw's picture settings (unk3f4_01 is Cam_SetAutoDeArtefact's, and
 * unk3f4_02 turns on the side-lighting correction, which Cam_MakeTextures,
 * func0f14bc04, sets again with every capture).
 */
static u8 g_PheadUsages[PHEAD_NUM_HEADS];
static u8 g_PheadEditorUsage = PHEAD_USE_COMBAT;

static u8 *pheadUsageFor(s32 slotindex)
{
	if (slotindex == -2) {
		slotindex = pheadGetCurrentHead();
	}

	if (slotindex >= 0 && slotindex < PHEAD_NUM_HEADS) {
		return &g_PheadUsages[slotindex];
	}

	return &g_PheadEditorUsage;
}

/**
 * slotindex is a loaded head, -1 for the editor or -2 for the current head.
 */
void pheadSetUsage(s32 slotindex, s32 usage)
{
	*pheadUsageFor(slotindex) = usage;
}

s32 pheadGetUsage(s32 slotindex)
{
	return *pheadUsageFor(slotindex);
}

s32 pheadFindFreeHead(void)
{
	s32 i;

	for (i = 0; i < PHEAD_NUM_HEADS; i++) {
		if (!pheadIsHeadLoaded(i)) {
			return i;
		}
	}

	return -1;
}

/**
 * Start a new head in <headnum>: the editor is reset to the defaults.
 */
void pheadNewHead(s32 headnum)
{
	struct var8007f8e0 *editor;

	pheadAllocWorkSlots();
	pheadSetCurrentHead(headnum);

	editor = func0f14a06c(-1);
	func0f14b228(editor);

	// Start on a random base head rather than always the first
	editor->unk3a4 = pheadGetAvailableTemplate(rngRandom() % pheadGetNumAvailableTemplates());

	editor->fileguid.fileid = 0;
	editor->fileguid.deviceserial = 0;

	g_PheadEditorHasImage = false;
}

/**
 * Copy loaded head <headnum> into the editor.
 */
void pheadEditHead(s32 headnum)
{
	pheadAllocWorkSlots();
	pheadSetCurrentHead(headnum);

	// Copies the settings, the face texture and the compressed picture, and
	// rebuilds the editor's raw picture from the face texture
	func0f14a7e8(headnum);

	g_PheadEditorUsage = g_PheadUsages[headnum];
	g_PheadEditorHasImage = true;
}

/**
 * Keep Changes: store the editor in the current head, which is then loaded,
 * with the editor's usage.
 */
void pheadKeepEditor(void)
{
	// Rebuilds the editor and copies it into the current head (slot -2)
	func0f14a610();

	func0f14a06c(-2)->unk3f4_00 = true;
	pheadSetUsage(-2, g_PheadEditorUsage);
}

/**
 * Opening the editor copies it to camdraw's undo slot, and reverting it (Cancel
 * Changes, Keep / Revert, or backing out) copies it back. camdraw's crop and
 * depth setters (Position Picture, Shape Head) also write the head being
 * edited (slot -2), whose crop its picture builder reads while editing, so
 * that head's crop and depth are kept here too and put back on a revert.
 */
static s32 g_PheadEditCrop[6];
static f32 g_PheadEditDepth[7];

void pheadBeginEdit(void)
{
	struct var8007f8e0 *head = func0f14a06c(-2);
	s32 i;

	func0f14a760();

	g_PheadEditCrop[0] = head->unk3c0;
	g_PheadEditCrop[1] = head->unk3c4;
	g_PheadEditCrop[2] = head->unk3c8;
	g_PheadEditCrop[3] = head->unk3cc;
	g_PheadEditCrop[4] = head->unk3ec;
	g_PheadEditCrop[5] = head->unk3f0;

	for (i = 0; i < ARRAYCOUNT(head->unk3d0); i++) {
		g_PheadEditDepth[i] = head->unk3d0[i];
	}
}

void pheadRevertEdit(void)
{
	struct var8007f8e0 *head = func0f14a06c(-2);
	s32 i;

	func0f14a7a4();

	head->unk3c0 = g_PheadEditCrop[0];
	head->unk3c4 = g_PheadEditCrop[1];
	head->unk3c8 = g_PheadEditCrop[2];
	head->unk3cc = g_PheadEditCrop[3];
	head->unk3ec = g_PheadEditCrop[4];
	head->unk3f0 = g_PheadEditCrop[5];

	for (i = 0; i < ARRAYCOUNT(head->unk3d0); i++) {
		head->unk3d0[i] = g_PheadEditDepth[i];
	}
}

/**
 * The editor's picture has changed (a new photo was kept), so the compressed
 * copy that pheadSaveFile reuses is out of date.
 */
void pheadPictureChanged(void)
{
	func0f14a06c(-1)->unk3f4_03 = false;
}

/**
 * Load a head from the Controller Pak into loaded head <headnum> (and the
 * editor). Returns 0 or a pak error, for filemgrAttemptOperation.
 */
s32 pheadLoadHead(s8 device, s32 fileid, u16 serial, s32 headnum)
{
	if (headnum < 0 || headnum >= PHEAD_NUM_HEADS) {
		return 1;
	}

	pheadAllocWorkSlots();

	if (!pheadLoadFile(device, fileid, serial, -1)) {
		return g_FilemgrLastPakError ? g_FilemgrLastPakError : 1;
	}

	pheadSetCurrentHead(headnum);
	pheadKeepEditor();

	g_PheadEditorHasImage = true;

	return 0;
}

/**
 * Load PerfectHead's preview: read Controller Pak file <fileid> into the
 * editor slot and rebuild its face texture, so the menu model can show it
 * painted before the player chooses it. Outside the editor the editor slot is
 * free (Make A New Head resets it, and loading a head reads the file again),
 * and no head slot is touched. Returns whether the file was read.
 */
bool pheadPreviewFile(s32 fileid, u16 serial)
{
	s8 device = pakFindBySerial(serial);

	if (device < 0) {
		return false;
	}

	pheadAllocWorkSlots();

	if (!pheadLoadFile(device, fileid, serial, -1)) {
		return false;
	}

	func0f14def0(-1, 0, "phead.c");

	return true;
}

/**
 * Save the editor to the Controller Pak. fileid 0 makes a new file.
 *
 * Perfect Dark replaces a file by writing the new data into a vacant file of
 * the same type and then marking the old one vacant (_pakSaveAtGuid). Game
 * and setup files have vacant files made with the note, but camera files are
 * large, so they are made on demand (pakCreateCameraFile): one to save into
 * if this is a new file, plus a vacant one to swap with.
 */
// The most files pakGetFileIdsByType can list, plus its terminating 0: a full
// 32 KB pak (123 pages after the system area) of the smallest PD file, a
// setup file (16-byte header + 0x31 bytes, aligned to 0x60)
#define PHEAD_MAXPAKFILES (123 * 256 / 0x60 + 1)

s32 pheadSaveHead(s8 device, s32 fileid, u16 serial)
{
	struct var8007f8e0 *editor = func0f14a06c(-1);
	struct fileguid oldguids[PHEAD_NUM_HEADS];
	u32 fileids[PHEAD_MAXPAKFILES];
	s32 vacant = 0;
	s32 ret;
	s32 i;

	if (pakGetFileIdsByType(device, PAKFILETYPE_CAMERA, fileids) == 0) {
		for (i = 0; fileids[i] != 0; i++) {
			struct pakfileheader header;

			if (pakFindFile(device, fileids[i], &header) != -1 && !header.occupied && fileids[i] != fileid) {
				if (fileid == 0) {
					fileid = fileids[i];
				} else {
					vacant++;
				}
			}
		}
	}

	while (fileid == 0 || vacant == 0) {
		s32 newfileid;

		ret = pakCreateCameraFile(device, &newfileid);

		if (ret != 0) {
			return ret;
		}

		if (fileid == 0) {
			fileid = newfileid;
		} else {
			vacant++;
		}
	}

	// pheadSaveFile gives the new file guid to every head that had the
	// editor's old guid. For a head that has never been saved that is 0, which
	// all the other unsaved heads have too.
	for (i = 0; i < PHEAD_NUM_HEADS; i++) {
		oldguids[i] = func0f14a06c(i)->fileguid;
	}

	ret = pheadSaveFile(device, fileid, serial);

	for (i = 0; i < PHEAD_NUM_HEADS; i++) {
		if (i != pheadGetCurrentHead() && oldguids[i].fileid == 0) {
			func0f14a06c(i)->fileguid = oldguids[i];
		}
	}

	if (ret == 0) {
		// The current head is the editor's copy
		func0f14a06c(pheadGetCurrentHead())->fileguid = editor->fileguid;
	}

	return ret;
}

bool pheadEditorHasImage(void)
{
	return g_PheadEditorHasImage;
}

void pheadSetEditorHasImage(bool value)
{
	g_PheadEditorHasImage = value;
}

/**
 * Draw a slot's 64x64 face texture at x, y (menu coordinates).
 */
static Gfx *pheadRenderImage(Gfx *gdl, struct textureconfig *tconfig, s32 x, s32 y, f32 scale)
{
	f32 pos[2];

	if (tconfig->textureptr == NULL) {
		return gdl;
	}

	pos[0] = x;
	pos[1] = y;

	// The CPU writes these textures (every capture, for the editor's): flush
	// it from the data cache so the RDP sees it (a no-op on the port), and
	// drop the port's cached copy
	osWritebackDCache(tconfig->textureptr, PHEAD_FACETEXSIZE);

#ifndef PLATFORM_N64
	gDPInvalTexCacheEXT(gdl++, tconfig->textureptr);
#endif

	return func0f14c870(gdl, tconfig, pos, scale, scale);
}

/**
 * Draw loaded head <headnum>'s picture.
 */
Gfx *pheadRenderHeadImage(Gfx *gdl, s32 headnum, s32 x, s32 y, f32 scale)
{
	if (!pheadIsHeadLoaded(headnum)) {
		return gdl;
	}

	return pheadRenderImage(gdl, &func0f14a06c(headnum)->unk004, x, y, scale);
}

/**
 * Draw the editor's picture: the live camera image or the kept picture.
 */
Gfx *pheadRenderEditorImage(Gfx *gdl, s32 x, s32 y, f32 scale)
{
	if (!g_PheadCameraRunning && !g_PheadEditorHasImage) {
		return gdl;
	}

	return pheadRenderImage(gdl, func0f14a89c(-1), x, y, scale);
}

/**
 * Using PerfectHeads in the game
 */
bool pheadIsCombatHead(s32 headnum)
{
	return IS8MB() && pheadIsHeadLoaded(headnum) && (pheadGetUsage(headnum) & PHEAD_USE_COMBAT);
}

bool pheadIsMissionHead(s32 headnum)
{
	return IS8MB() && pheadIsHeadLoaded(headnum) && (pheadGetUsage(headnum) & PHEAD_USE_MISSIONS);
}

/**
 * The Combat Simulator numbers PerfectHeads after its own heads
 * (mpheadnum >= mpGetNumHeads2(), as in the original code). Returns the
 * loaded head for <mpheadnum>, or -1 if it isn't a usable PerfectHead.
 */
s32 pheadGetMpHead(s32 mpheadnum)
{
	s32 headnum = mpheadnum - mpGetNumHeads2();

	return headnum >= 0 && pheadIsCombatHead(headnum) ? headnum : -1;
}

/**
 * Load a copy of loaded head <headnum>'s base head and paint it. As with
 * every Combat Simulator head on 8 MB, each player or simulant gets their own
 * copy (stage pool), fitted to their body (bodynum, or -1 to leave it as it
 * is). *outheadnum is set to the base head, which the chr then uses for
 * everything else (height, gender, voice).
 */
struct modeldef *pheadLoadHeadModel(s32 headnum, s32 bodynum, s32 *outheadnum)
{
	s32 basehead = pheadGetTemplateHeadnum(func0f14a9f8(headnum));
	u16 filenum = g_HeadsAndBodies[basehead].filenum;
	u32 filelen = ALIGN64(fileGetInflatedSize(filenum, LOADTYPE_MODEL)) + PHEAD_MODELSLACK;
	u32 prevloadedsize = g_FileInfo[filenum].loadedsize;
	u32 prevallocsize = g_FileInfo[filenum].allocsize;
	u8 *mem = mempAlloc(PHEAD_MODELTEXPOOL + filelen, MEMPOOL_STAGE);
	const struct pheadtemplate *template = pheadGetTemplate(func0f14a9f8(headnum));
	struct texpool texpool;
	struct modeldef *def;
	s32 i;

	*outheadnum = basehead;

	if (mem == NULL) {
		// Out of memory: load it the way the Combat Simulator loads any head,
		// unpainted
		def = modeldefLoadToNew(filenum);
		g_FileInfo[filenum].loadedsize = 0;

		if (bodynum >= 0) {
			bodyCalculateHeadOffset(def, basehead, bodynum);
		}

		modelAllocateRwData(def);

		return def;
	}

	// The head's textures come from the texture bank. Loaded without a pool
	// they would go in the shared texture cache, so painting them would
	// paint every chr with this base head. Give the copy its own textures.
	texInitPool(&texpool, mem, PHEAD_MODELTEXPOOL);
	def = modeldefLoad(filenum, mem + PHEAD_MODELTEXPOOL, filelen, &texpool);

	// Give back the room the display lists didn't need. The file's loaded size
	// is for this copy only; put back what other loads of the file expect.
	mempRealloc(mem, PHEAD_MODELTEXPOOL + ALIGN16(fileGetLoadedSize(filenum)), MEMPOOL_STAGE);
	g_FileInfo[filenum].loadedsize = prevloadedsize;
	g_FileInfo[filenum].allocsize = prevallocsize;

	if (bodynum >= 0) {
		bodyCalculateHeadOffset(def, basehead, bodynum);
	}

	modelAllocateRwData(def);

	if (!pheadIsHeadLoaded(headnum) || !pheadHeadIsPaintable(def, template)) {
		return def;
	}

	// When a pool is too full, texLoad points the texture at the start of the
	// pool instead of loading it. Only paint textures that really were loaded
	// into this copy's pool, each one whole.
	for (i = 0; i < (PHEAD_ISELVIS(template) ? 1 : PHEAD_NUMTEX(template)); i++) {
		u8 *px = pheadGetHeadTexture(def, template, i);

		if (px < mem || px + PHEAD_TEXBYTES + 512 > mem + PHEAD_MODELTEXPOOL
				|| (i > 0 && px == pheadGetHeadTexture(def, template, 0))) {
			return def;
		}
	}

	pheadPaintHead(def, headnum, true, mem + PHEAD_MODELTEXPOOL);

	return def;
}

/**
 * Called by bodiesReset once the stage's guard heads have been chosen. In
 * missions, put the mission heads in the pools of guard heads, by gender.
 * The pools hold head numbers; -1 - n is PerfectHead n (bodyAllocateChr and
 * body0f02ce8c already handle that). Each one takes the place of a normal
 * head, at most half of each pool.
 */
void pheadAddMissionHeads(s32 *maleheads, s32 *femaleheads, s32 numpergender)
{
	s32 nummale = 0;
	s32 numfemale = 0;
	s32 i;

	if (g_Vars.mplayerisrunning || var8007f8e0 == NULL) {
		return;
	}

	for (i = 0; i < PHEAD_NUM_HEADS; i++) {
		if (pheadIsMissionHead(i)) {
			s32 basehead = pheadGetTemplateHeadnum(func0f14a9f8(i));

			if (g_HeadsAndBodies[basehead].ismale) {
				if (nummale < numpergender / 2) {
					maleheads[nummale++] = -1 - i;
				}
			} else {
				if (numfemale < numpergender / 2) {
					femaleheads[numfemale++] = -1 - i;
				}
			}
		}
	}
}

/**
 * A player file names its PerfectHead by the Controller Pak file it was saved
 * in. Find that head among the loaded heads, or load it from the pak if the
 * pak is inserted and there is room. Returns the loaded head or -1.
 */
s32 pheadFindHeadByGuid(struct fileguid *guid)
{
	s32 headnum;
	s8 device;

	if (var8007f8e0 == NULL || IS4MB() || guid->fileid == 0) {
		return -1;
	}

	// The player file is for the Combat Simulator, so that is where the head
	// is used
	for (headnum = 0; headnum < PHEAD_NUM_HEADS; headnum++) {
		struct var8007f8e0 *head = func0f14a06c(headnum);

		if (pheadIsHeadLoaded(headnum)
				&& head->fileguid.fileid == guid->fileid
				&& head->fileguid.deviceserial == guid->deviceserial) {
			pheadSetUsage(headnum, pheadGetUsage(headnum) | PHEAD_USE_COMBAT);
			return headnum;
		}
	}

	device = pakFindBySerial(guid->deviceserial);
	headnum = pheadFindFreeHead();

	if (device >= 0 && headnum >= 0
			&& pheadLoadHead(device, guid->fileid, guid->deviceserial, headnum) == 0) {
		pheadSetUsage(headnum, PHEAD_USE_COMBAT);
		return headnum;
	}

	return -1;
}

#endif
