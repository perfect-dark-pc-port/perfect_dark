#include <ultra64.h>
#include "constants.h"
#include "game/camdraw.h"
#include "game/phead.h"
#include "game/tex.h"
#include "game/menu.h"
#include "game/stubs/game_102230.h"
#include "game/bg.h"
#include "game/lang.h"
#include "game/pak.h"
#include "game/splat.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/model.h"
#include "lib/mema.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/libc/ll.h"
#include "data.h"
#include "types.h"
#include "string.h"

/**
 * 8-bit textures loaded with LoadBlock on the N64 need the 32-bit words of every
 * odd row swapped (TMEM interleave). The port's renderer reads textures
 * linearly, so no swap is applied there.
 */
#ifdef PLATFORM_N64
#define CAM_TMEMSWAP(row, col) ((((row) & 1)) ? ((((col) & 4)) ? -4 : 4) : 0)
#else
#define CAM_TMEMSWAP(row, col) 0
#endif

// The menu player whose screen gets the camera messages. The game assumed the
// camera is on that player's own controller; PerfectHead lets any player use a
// camera on any controller, so it gives the player using it.
#if PERFECTHEAD
#define CAM_MENUPLAYER(device) pheadMenuPlayer(device)
#else
#define CAM_MENUPLAYER(device) (device)
#endif

// New heads keep the base head's own hair unless the player picks a colour
#if PERFECTHEAD
#define PHEAD_DEFAULT_HAIRCOLOUR PHEAD_HAIRCOLOUR_NORMAL
#else
#define PHEAD_DEFAULT_HAIRCOLOUR 0
#endif

#if PERFECTHEAD
// Auto exposure limits. Exposure is in the sensor's 16 us steps; Pan Docs
// (Game Boy Camera) gives $0030 for objects in direct sunlight, which is also
// the least allowed with the N bit set (pak0f11d118 sets it), $0300 out of
// direct sunlight and $0800 for a room with good light. The longest is the
// original exposure table's last entry used (var8007f904[80]). Gain is the
// register 1 code: the Game Boy Camera ROM uses 0-10 (14-32 dB).
#define CAM_MINEXPOSURE 0x0030
#define CAM_MAXEXPOSURE 0x5eef
#define CAM_MINGAIN     0
#define CAM_STARTGAIN   4 // 20 dB
#define CAM_MAXGAIN     10
#define CAM_MAXSTEPS    8 // exposure steps per capture
#endif

struct camerafile {
	u8 unk00[128];
	u16 unk80;
	u16 unk82;
	u16 unk84;
	u8 unk86;
	u8 unk87;
	u8 unk88;
	u8 unk89;
	u8 unk8a;
	u8 unk8b;
	u8 unk8c;
	u8 unk8d;
	u16 unk8e_00 : 1;
	u16 unk8e_01 : 1;
	u16 unk8e_02 : 8;
	s16 unk90[7];
	u8 unk9e[1024];
};

struct var800a45a0 *var800a45a0;
u32 var800a45a4;
struct texpool var800a45a8;
u32 var800a45b8;
u32 var800a45bc;

char *var8007f8c0 = "Init";
char *var8007f8c4 = "Start Header Read";
char *var8007f8c8 = "Header Read Done";
char *var8007f8cc = "Null";
char *var8007f8d0 = "Exit 1";
char *var8007f8d4 = "Exit 2";
char *var8007f8d8 = "Exit 3";

const u32 var7f1b60a0[][2] = {
	{ 0, 0 },
	{ 0, 1 },
	{ 1, 0 },
	{ 2, 0 },
	{ 1, 1 },
	{ 0, 2 },
	{ 0, 3 },
	{ 1, 2 },
	{ 2, 1 },
	{ 3, 0 },
	{ 4, 0 },
	{ 3, 1 },
	{ 2, 2 },
	{ 1, 3 },
	{ 0, 4 },
	{ 0, 5 },
	{ 1, 4 },
	{ 2, 3 },
	{ 3, 2 },
	{ 4, 1 },
	{ 5, 0 },
	{ 6, 0 },
	{ 5, 1 },
	{ 4, 2 },
	{ 3, 3 },
	{ 2, 4 },
	{ 1, 5 },
	{ 0, 6 },
	{ 0, 7 },
	{ 1, 6 },
	{ 2, 5 },
	{ 3, 4 },
	{ 4, 3 },
	{ 5, 2 },
	{ 6, 1 },
	{ 7, 0 },
	{ 7, 1 },
	{ 6, 2 },
	{ 5, 3 },
	{ 4, 4 },
	{ 3, 5 },
	{ 2, 6 },
	{ 1, 7 },
	{ 2, 7 },
	{ 3, 6 },
	{ 4, 5 },
	{ 5, 4 },
	{ 6, 3 },
	{ 7, 2 },
	{ 7, 3 },
	{ 6, 4 },
	{ 5, 5 },
	{ 4, 6 },
	{ 3, 7 },
	{ 4, 7 },
	{ 5, 6 },
	{ 6, 5 },
	{ 7, 4 },
	{ 7, 5 },
	{ 6, 6 },
	{ 5, 7 },
	{ 6, 7 },
	{ 7, 6 },
	{ 7, 7 },
};

const u32 var7f1b62a0[] = {0x00000000};
const u32 var7f1b62a4[] = {0xffffffff};
const u32 var7f1b62a8[] = {0xdc9785ff};
const u32 var7f1b62ac[] = {0xdc917bff};
const u32 var7f1b62b0[] = {0xdd8d71ff};
const u32 var7f1b62b4[] = {0xdd8367ff};
const u32 var7f1b62b8[] = {0xdf795dff};
const u32 var7f1b62bc[] = {0xdf7953ff};
const u32 var7f1b62c0[] = {0x00000000};
const u32 var7f1b62c4[] = {0xffffffff};
const u32 var7f1b62c8[] = {0xa67b6dff};
const u32 var7f1b62cc[] = {0xa16152ff};
const u32 var7f1b62d0[] = {0x8c6741ff};
const u32 var7f1b62d4[] = {0x81544cff};
const u32 var7f1b62d8[] = {0x6a4031ff};
const u32 var7f1b62dc[] = {0x4f433cff};
const u32 var7f1b62e0[] = {0x00000000};
const u32 var7f1b62e4[] = {0xffffffff};
const u32 var7f1b62e8[] = {0xccb38dff};
const u32 var7f1b62ec[] = {0xddc07cff};
const u32 var7f1b62f0[] = {0xe6a667ff};
const u32 var7f1b62f4[] = {0xd1ab50ff};
const u32 var7f1b62f8[] = {0xc69f6dff};
const u32 var7f1b62fc[] = {0xb27e5bff};
const u32 var7f1b6300[] = {0x00000000};
const u32 var7f1b6304[] = {0xffffffff};
const u32 var7f1b6308[] = {0x5f3a25ff};
const u32 var7f1b630c[] = {0x553a20ff};
const u32 var7f1b6310[] = {0x4b351cff};
const u32 var7f1b6314[] = {0x412d19ff};
const u32 var7f1b6318[] = {0x41260fff};
const u32 var7f1b631c[] = {0x371e04ff};
const u32 var7f1b6320[] = {0x00000000};
const u32 var7f1b6324[] = {0xffffffff};
const u32 var7f1b6328[] = {0xe9d679ff};
const u32 var7f1b632c[] = {0x654f40ff};
const u32 var7f1b6330[] = {0xebb279ff};
const u32 var7f1b6334[] = {0x846b51ff};
const u32 var7f1b6338[] = {0xebc579ff};
const u32 var7f1b633c[] = {0xab8d6fff};
const u32 var7f1b6340[] = {0x00000000};
const u32 var7f1b6344[] = {0xffffffff};
const u32 var7f1b6348[] = {0x26374dff};
const u32 var7f1b634c[] = {0x18253eff};
const u32 var7f1b6350[] = {0x171d20ff};
const u32 var7f1b6354[] = {0x060e16ff};
const u32 var7f1b6358[] = {0x00001cff};
const u32 var7f1b635c[] = {0x000001ff};
const u32 var7f1b6360[] = {0x00000000};
const u32 var7f1b6364[] = {0xffffffff};
const u32 var7f1b6368[] = {0x972f1aff};
const u32 var7f1b636c[] = {0x822817ff};
const u32 var7f1b6370[] = {0x6e1e12ff};
const u32 var7f1b6374[] = {0x5a140dff};
const u32 var7f1b6378[] = {0x460a08ff};
const u32 var7f1b637c[] = {0x200001ff};
const u32 var7f1b6380[] = {0x00000000};
const u32 var7f1b6384[] = {0xffffffff};
const u32 var7f1b6388[] = {0x79b2ebff};
const u32 var7f1b638c[] = {0x007fffff};
const u32 var7f1b6390[] = {0x1f5fdfff};
const u32 var7f1b6394[] = {0x0043caff};
const u32 var7f1b6398[] = {0x00147eff};
const u32 var7f1b639c[] = {0x0c1857ff};

const char var7f1b63a0[] = "RWI : Cam Alloc : Guid=%d -> Total = %u (%d at %s)\n";

struct var8007f8dc *var8007f8dc = NULL;
struct var8007f8e0 *var8007f8e0 = NULL;
u32 var8007f8e4 = 0x00000000;
u32 var8007f8e8 = 0x00000008;
u32 var8007f8ec = 0x0000005a;
u32 var8007f8f0 = 0x00000005;
u32 var8007f8f4 = 0x000000c8;
u32 var8007f8f8 = 0x00000000;
u32 var8007f8fc = 0x00000001;
u32 var8007f900 = 0x00000078;

u16 var8007f904[] = {
	0x0200, 0x0233, 0x024e, 0x026b,
	0x0289, 0x02a9, 0x02ca, 0x02ed,
	0x0312, 0x0338, 0x0360, 0x038b,
	0x03b7, 0x03e5, 0x0416, 0x0449,
	0x047f, 0x04b7, 0x04f2, 0x0530,
	0x0570, 0x05b4, 0x05fc, 0x0646,
	0x0695, 0x06e7, 0x073d, 0x0798,
	0x07f7, 0x085a, 0x08c3, 0x0930,
	0x09a3, 0x0a1b, 0x0a9a, 0x0b1e,
	0x0ba9, 0x0c3b, 0x0cd4, 0x0d74,
	0x0e1c, 0x0ecc, 0x0f85, 0x1047,
	0x1113, 0x11e8, 0x12c8, 0x13b3,
	0x14a9, 0x15ab, 0x16ba, 0x17d6,
	0x18ff, 0x1a38, 0x1b7f, 0x1cd7,
	0x1e3f, 0x1fb9, 0x2146, 0x22e6,
	0x249a, 0x2663, 0x2843, 0x2a3a,
	0x2c49, 0x2e73, 0x30b7, 0x3318,
	0x3596, 0x3834, 0x3af2, 0x3dd3,
	0x40d7, 0x4402, 0x4753, 0x4acf,
	0x4e75, 0x524a, 0x564e, 0x5a84,
	0x5eef, 0x6392, 0x686e, 0x6d87,
	0x72df, 0x787a, 0x7e5c, 0x8487,
	0x8aff, 0x91c7, 0x98e5, 0xa05b,
	0xa82f, 0xb065, 0xb901, 0xc208,
	0xcb81, 0xd570, 0xdfdb, 0xeac8,
	0xf63d, 0xffff,
};

u32 var8007f9d0 = 0x00000000;

/**
 * Fill the texture buffer with a linear gradient: black (left) to white (right).
 */
void func0f149c90(struct textureconfig *tconfig)
{
	s32 x;
	s32 y;

	for (y = 0; y < tconfig->height; y++) {
		for (x = 0; x < tconfig->width; x++) {
			s32 value = (x * 8 / tconfig->width) * 32;
			s32 fudge = CAM_TMEMSWAP(y, x);

			tconfig->textureptr[y * 64 + x + fudge] = value;
		}
	}
}

void *func0f149d58(u32 size, u32 line, char *file)
{
	s32 i;
	void *allocation;

	static u32 var8007f9d4 = 0;

	size = align16(size);

	bgGarbageCollectRooms(size, 1);

	var8007f9d0 += size;
	allocation = memaAlloc(size);
	var8007f9d4++;

	for (i = 0; i < var800a45a0->unk37c; i++) {
		if (var800a45a0->unk380[i] == NULL) {
			// @bug? Is this meant to be writing to [i]?
			// Otherwise it's appending to the array without increasing unk37c.
			var800a45a0->unk380[var800a45a0->unk37c] = allocation;
			var800a45a0->unk3f8[var800a45a0->unk37c] = size;

			return allocation;
		}
	}

	var800a45a0->unk380[var800a45a0->unk37c] = allocation;
	var800a45a0->unk3f8[var800a45a0->unk37c] = size;
	var800a45a0->unk37c++;

	return allocation;
}

void func0f149e58(u8 *arg0, u32 size)
{
	s32 i;

	var8007f9d0 -= size;

	for (i = 0; i < var800a45a0->unk37c; i++) {
		if (arg0 == NULL || arg0 == var800a45a0->unk380[i]) {
			memaFree(var800a45a0->unk380[i], var800a45a0->unk3f8[i]);
			var800a45a0->unk380[i] = 0;
		}
	}

	if (arg0 == NULL) {
		var800a45a0->unk37c = 0;
	}
}

void func0f149f10(void)
{
	// empty
}

void func0f149f18(void)
{
	struct var8007f8e0 *thing;
	s32 i;

	func0f14b394(func0f14a06c(-1));

	thing = func0f14a06c(-1);
	thing->unk3f4_00 = false;

	if (var800a45a0->unk470) {
		for (i = 0; i < 0x4000; i++) {
			var800a45a0->unk470[i] = rngRandom() % 255;
		}
	}
}

void func0f149fc8(s32 index)
{
	func0f14c50c(func0f14a06c(index), func0f14a06c(-1), 1040, "camdraw.c");
}

const char var7f1b63e0[] = "Cam -> Dumping head vertex colour information\n";
const char var7f1b6410[] = "Cam -> Face Col - Pal=%d, Entry=%d, Col=%08x\n";
const char var7f1b6440[] = "Cam -> Hair Col - Pal=%d, Entry=%d, Col=%08x\n";
const char var7f1b6470[] = "Cam_Tick (%d Total Slots)";
const char var7f1b648c[] = "Current Camera Slot = %d\n";
const char var7f1b64a8[] = "Slot %d -> Active = %d";

void func0f14a00c(bool arg0)
{
	struct var8007f8e0 *thing;

	thing = func0f14a06c(-1);
	thing->unk3f4_04 = arg0;

	thing = func0f14a06c(-2);
	thing->unk3f4_04 = arg0;
}

struct var8007f8e0 *func0f14a06c(s32 index)
{
	if (index == -1) {
		return &var8007f8e0[var800a45a0->unk16c];
	}

	if (index == -4) {
		return &var8007f8e0[var800a45a0->unk174];
	}

	if (index == -5) {
		return &var8007f8e0[var800a45a0->unk178];
	}

	if (index == -2) {
		return &var8007f8e0[var800a45a0->unk014[var800a45a0->unk004]];
	}

	if (index == -3) {
		return &var8007f8e0[var800a45a0->unk170];
	}

	return &var8007f8e0[var800a45a0->unk014[index]];
}

void func0f14a16c(s32 arg0)
{
	struct var8007f8dc *thing = func0f14a20c();
	thing->unk100 = arg0;
}

bool func0f14a194(void)
{
	return var8007f8dc[var800a45a0->unk000].unk0d4_04 & 0xff;
}

/**
 * 0 returns "Style 1"
 * 1 returns "Style 2"
 * 2 returns "Style 3"
 * 3 returns "Style 4"
 */
char *phGetStyleName(s32 stylenum)
{
	return langGet(L_MISC_429 + stylenum);
}

/**
 * 0 returns "Blonde"
 * 1 returns "Black"
 * 2 returns "Auburn"
 * 3 returns "Blue Rinse"
 */
char *phGetColourName(s32 colournum)
{
	return langGet(L_MISC_433 + colournum);
}

struct var8007f8dc *func0f14a20c(void)
{
	return &var8007f8dc[func0f14a5a4()];
}

void func0f14a240(void)
{
	s32 i;
	s32 j;

	var800a45a0->unk48c = 0;
	var800a45a0->unk484 = 0;
	var800a45a0->unk470 = 0;
	var800a45a0->unk474 = 0;
	var800a45a0->unk478 = 0;
	var800a45a0->unk47c = 0;
	var800a45a0->unk480 = 0;
	var800a45a0->unk37c = 0;
	var800a45a0->unk000 = 0;
	var800a45a0->unk004 = -1;
	var800a45a0->unk008 = 1;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			// empty
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			// empty
		}
	}
}

bool func0f14a2fc(s32 index, u32 line, char *file)
{
	struct var8007f8e0 *thing = func0f14a06c(index);
	return thing->unk3f4_00;
}

void pheadInit(void)
{
	s32 i;

	if (IS8MB()) {
		u32 size1 = align16(sizeof(struct var8007f8dc) * 4);
		u32 size2 = align16(sizeof(struct var800a45a0));

		var8007f8dc = mempAlloc(size1, MEMPOOL_PERMANENT);
		var800a45a0 = mempAlloc(size2, MEMPOOL_PERMANENT);

		func0f14a240();

		for (i = 0; i < 4; i++) {
			func0f14ad58(i);
		}

#if PERFECTHEAD
		pheadSlotsInit();
#endif
	}
}

void pheadInit2(void)
{
	// empty
}

void pheadReset(void)
{
	s32 i;

	var800a45a0->unk37c = 0;

	for (i = 0; i < 4; i++) {
		var8007f8dc[i].unk0d4_03 = false;
	}

#if PERFECTHEAD
	pheadStageReset();
#endif
}

void func0f14a404(void)
{
	func0f149e58(NULL, 0);
}

void func0f14a428(void)
{
	s32 i;

	for (i = 0; i < 22; i++) {
		func0f14a2fc(i, 1296, "camdraw.c");
	}
}

const char var7f1b64cc[] = "CAM : Cam_StartCamFileRead - Go\n";
const char var7f1b64f0[] = "************ Cam_ActivateAndClearSlot ************ : Slot=%d\n";

void func0f14a480(void)
{
	// empty
}

void func0f14a488(void)
{
	s32 i;

	func0f14aed0(-1);

	for (i = 0; i < 32; i++) {
		func0f149c90(&var800a45a0->unk484[i]);
	}

	for (i = 0; i < 4; i++) {
		var8007f8dc[i].unk0f4 = 0;
		pak0f11d478(i);
	}
}

void func0f14a52c(void)
{
	s32 i;

	for (i = 0; i < 4; i++) {
		var8007f8dc[i].unk0d4_03 = true;
	}
}

void func0f14a560(void)
{
	s32 i;

	for (i = 0; i < 4; i++) {
		var8007f8dc[i].unk0d4_03 = false;
	}
}

void func0f14a594(s32 arg0)
{
	var800a45a0->unk000 = arg0;
}

s32 func0f14a5a4(void)
{
	return var800a45a0->unk000;
}

void func0f14a5b4(s32 index)
{
	var800a45a0->unk004 = index;

	func0f14b394(func0f14a06c(index));
}

void func0f14a5e4(void)
{
	func0f14def0(-1, 1407, "camdraw.c");
}

void func0f14a610(void)
{
	func0f14def0(-1, 1415, "camdraw.c");
	func0f14c50c(func0f14a06c(-2), func0f14a06c(-1), 1416, "camdraw.c");
}

s32 func0f14a668(void)
{
	return var800a45a0->unk004;
}

void func0f14a678(void)
{
	func0f14c50c(func0f14a06c(-2), func0f14a06c(-1), 1433, "camdraw.c");
}

void func0f14a6bc(void)
{
	func0f14c50c(func0f14a06c(-4), func0f14a06c(-1), 1441, "camdraw.c");
	func0f14c50c(func0f14a06c(-1), func0f14a06c(-3), 1442, "camdraw.c");
	func0f14c50c(func0f14a06c(-3), func0f14a06c(-4), 1443, "camdraw.c");
}

const char var7f1b6584[] = "Cam_CopyEditorToUndo\n";

void func0f14a760(void)
{
	func0f14c50c(func0f14a06c(-3), func0f14a06c(-1), 1452, "camdraw.c");
}

const char var7f1b65a8[] = "Cam_CopyUndoToEditor\n";

void func0f14a7a4(void)
{
	func0f14c50c(func0f14a06c(-1), func0f14a06c(-3), 1461, "camdraw.c");
}

void func0f14a7e8(s32 index)
{
	func0f14c50c(func0f14a06c(-1), func0f14a06c(index), 1470, "camdraw.c");
}

void func0f14a830(void)
{
	struct var8007f8dc *thing = func0f14a20c();

	if (thing->unk0d4_00) {
		thing->unk0d4_00 = false;
	} else {
		thing->unk0d4_00 = true;
	}

	if (thing->unk0d4_00) {
		func0f14def0(-1, 1484, "camdraw.c");
	}
}

struct textureconfig *func0f14a89c(s32 index)
{
	struct var8007f8e0 *thing = func0f14a06c(index);

	return &thing->unk004;
}

bool func0f14a8c0(void)
{
	struct var8007f8dc *thing = func0f14a20c();

	return thing->unk0d4_00;
}

bool func0f14a8e8(void)
{
	struct var8007f8dc *thing = func0f14a20c();

	return thing->unk0f8 == 0 ? false : true;
}

void func0f14a91c(s32 arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3bc = arg0;

	func0f14def0(-1, 1518, "camdraw.c");
}

const char var7f1b65f0[] = "Cam -> Setting current hair colour to %s\n";
const char var7f1b661c[] = "Cam -> Setting current face colour to %s\n";

void func0f14a95c(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3bc = 0x80;
}

s32 func0f14a984(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	return thing->unk3bc;
}

void func0f14a9a8(s32 arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3a4 = arg0;
}

void func0f14a9d4(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3a4 = 0;
}

s32 func0f14a9f8(s32 index)
{
	struct var8007f8e0 *thing = func0f14a06c(index);

	return thing->unk3a4;
}

void func0f14aa1c(s32 arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3b8 = arg0;
}

void func0f14aa48(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->unk3b8 = 6;
}

s32 func0f14aa70(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	return thing->unk3b8;
}

void func0f14aa94(s32 colournum)
{
	struct var8007f8e0 *thing;

	phGetColourName(colournum);

	thing = func0f14a06c(-1);
	thing->colournum = colournum;
}

void func0f14aac4(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->colournum = PHEAD_DEFAULT_HAIRCOLOUR;
}

s32 func0f14aae8(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	return thing->colournum;
}

void func0f14ab0c(s32 stylenum)
{
	struct var8007f8e0 *thing;

	phGetStyleName(stylenum);

	thing = func0f14a06c(-1);
	thing->stylenum = stylenum;
}

void func0f14ab3c(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	thing->stylenum = 0;
}

s32 func0f14ab60(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	return thing->stylenum;
}

void func0f14ab84(s32 arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	thing->unk3b4 = arg0;
}

s32 func0f14abb0(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	return thing->unk3b4;
}

void func0f14abd4(s32 index, s32 arg1)
{
	var8007f8dc[index].unk06c = arg1;
}

s32 func0f14abf4(s32 index)
{
	return var8007f8dc[index].unk06c;
}

bool func0f14ac14(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	return thing->unk3f4_02;
}

void func0f14ac3c(bool arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	thing->unk3f4_02 = arg0;

	func0f14def0(-1, 1681, "camdraw.c");
}

const char var7f1b6654[] = "Cam_SetAutoDeArtefact -> State = %d\n";

bool func0f14ac90(void)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);
	return thing->unk3f4_01;
}

void func0f14acb8(bool arg0)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	if (arg0) {
		thing->unk3f4_01 = true;
	} else {
		thing->unk3f4_01 = false;
	}

	func0f14def0(-1, 1699, "camdraw.c");
}

u16 func0f14ad14(s32 device)
{
	return var8007f8dc[device].unk05c & 0xffff;
}

s32 func0f14ad38(s32 device)
{
	return var8007f8dc[device].unk060;
}

const char var7f1b6688[] = "Cam_ClearCameraLoadBuffer -> Camera=%d\n";
const char var7f1b66b0[] = "Cam_StartTemp : Need %u bytes for temp cam images buffer\n";

void func0f14ad58(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];

	thing->unk000 = 0;
	thing->unk004 = 1.0f;
	thing->unk008 = 1.0f;
	thing->unk050 = 110.0f;
	thing->unk00c = 0.1f;
	thing->unk010 = 0.2f;
	thing->unk058 = -1;
	thing->unk068 = -1;
	thing->unk06c = 0;
	thing->unk070 = 0;
	thing->unk074 = 0;
	thing->unk084 = 0;
	thing->unk054 = 30;
	thing->unk014 = 0.0f;
	thing->unk018 = 0.0f;
	thing->unk01c = 0.0f;
	thing->unk020 = 0.0f;
	thing->unk024 = 1.0f;
	thing->unk028 = 1.0f;
	thing->unk02c = 0.0f;
	thing->unk030 = 0.0f;
	thing->unk034 = 0.0f;
	thing->unk038 = 0.0f;
	thing->unk03c = 1.0f;
	thing->unk040 = 0.0f;
	thing->unk044 = 30.0f;
	thing->unk05c = var8007f904[30];
	thing->unk060 = 6;
	thing->unk08c = 14;
	thing->unk048 = 1.00f;
	thing->unk088 = 1;
	thing->unk090 = 0;
	thing->unk064 = 0;
	thing->unk0f4 = 0;
	thing->unk094 = 0;
	thing->unk0d4_00 = false;
	thing->unk0d4_01 = false;
	thing->unk0d4_02 = false;
	thing->unk0d4_04 = true;
	thing->unk0f8 = 0;
	thing->unk0fc = 3;
	thing->unk100 = 3;
	thing->unk0e0 = 0;
	thing->unk0e4 = 0;
	thing->unk0e8 = 0;
	thing->unk0ec = 0;
	thing->unk0f0 = 0;
	thing->unk078 = 0;
	thing->unk07c = 0;
}

bool func0f14aea0(s32 device)
{
	return var8007f8dc[device].unk0d4_03;
}

void func0f14aec8(void)
{
	// empty
}

void func0f14aed0(s32 device)
{
	// empty
}

void func0f14aed8(void)
{
	// empty
}

void func0f14aee0(void)
{
	s32 i;
	s32 j;

	if (var800a45a0->unk484 == NULL) {
		struct textureconfig tconfig;
		u8 *ptr;
		s32 texturesize;
		s32 count = 33;
		s32 totalsize;
		u32 stack[2];

		func0f14c7dc(&tconfig);

		texturesize = align32(func0f14c814(&tconfig));

		totalsize = align16(sizeof(struct textureconfig) * count);
		totalsize += align16(texturesize * count);

		ptr = mempAlloc(totalsize, MEMPOOL_STAGE);

		var800a45a0->unk484 = (struct textureconfig *)ptr;
		ptr += sizeof(struct textureconfig) * count;

		for (i = 0; i < count; i++) {
			func0f14c7dc(&var800a45a0->unk484[i]);
			var800a45a0->unk484[i].textureptr = ptr;
			ptr += texturesize;
		}

		for (i = 0; i < count; i++) {
			func0f149c90(&var800a45a0->unk484[i]);
		}

		var800a45a0->unk488 = var800a45a0->unk484 + 32;
	}

	var800a45a0->unk37c = 0;

	var800a45a0->unk470 = func0f149d58(0x4000, 1915, "camdraw.c");
	var800a45a0->unk474 = func0f149d58(0x10000, 1916, "camdraw.c");
	var800a45a0->unk478 = func0f149d58(0x10000, 1917, "camdraw.c");
	var800a45a0->unk47c = func0f149d58(0x78, 1918, "camdraw.c");
	var800a45a0->unk480 = func0f149d58(0x1000, 1919, "camdraw.c");

	for (i = 0; i < 4; i++) {
		struct var8007f8dc *thing = &var8007f8dc[i];
		thing->unk0dc = (u32 *)var800a45a0->unk47c;
		thing->unk0d8 = var800a45a0->unk480;
		thing->unk07c = -1;
		thing->unk078 = 0;

		pak0f117150(i, thing->unk0d8);
		pak0f1171b4(SAVEDEVICE_CONTROLLERPAK1, PAK00C_03, 0);
		func0f14aed0(i);

		thing->unk094 = 0;

		for (j = 0; j < ARRAYCOUNT(thing->unk098); j++) {
			thing->unk098[j] = 0xff;
			thing->unk0b6[j] = 0xff;
		}
	}

	func0f14a594(0);
}

const char var7f1b6728[] = "Camera -> Cam_Start for Camera %d\n";
const char var7f1b674c[] = "Camera -> Cam_Stop\n";
const char var7f1b6760[] = "Camera -> Cam_Finish - Done\n";
const char var7f1b6780[] = "Camera ->Created a new slot - Id=%d\n";
const char var7f1b67a8[] = "Camera -> CD_DeleteSlot - Dumping slot %d\n";

void func0f14b150(void)
{
	var800a45a0->unk48c = 0;

	func0f14b178();
}

void func0f14b178(void)
{
	s32 i;

	func0f149e58(var800a45a0->unk470, 0x4000);
	func0f149e58(var800a45a0->unk474, 0x10000);
	func0f149e58(var800a45a0->unk478, 0x10000);
	func0f149e58(var800a45a0->unk47c, 0x78);
	func0f149e58(var800a45a0->unk480, 0x1000);

	for (i = 0; i < 4; i++) {
		struct var8007f8dc *thing = &var8007f8dc[i];

		thing->unk0d4_03 = false;
		thing->unk0dc = NULL;
		thing->unk0d8 = 0;
	}
}

void func0f14b228(struct var8007f8e0 *arg0)
{
	s32 i;

	arg0->unk03c = 0;
	arg0->unk3f4_00 = false;

	for (i = 0; i < ARRAYCOUNT(arg0->unk040); i++) {
		arg0->unk040[i] = 0xff;
	}

	for (i = 0; i < ARRAYCOUNT(arg0->unk2fc); i++) {
		arg0->unk2fc[i] = 0xff;
	}

	for (i = 0; i < ARRAYCOUNT(arg0->unk108); i++) {
		arg0->unk108[i] = 0xff;
	}

	for (i = 0; i < ARRAYCOUNT(arg0->unk3d0); i++) {
		arg0->unk3d0[i] = 1.0f;
	}

	arg0->unk036 = 3;
	arg0->unk3f4_01 = false;
	arg0->unk3f4_02 = true;
	arg0->unk3f4_03 = false;
	arg0->unk028 = 0;
	arg0->unk02c = 0;
	arg0->unk034 = 0;
	arg0->unk030 = 1.0f;
	arg0->unk3f4_04 = false;
	arg0->colournum = PHEAD_DEFAULT_HAIRCOLOUR;
	arg0->stylenum = 0;
	arg0->unk3a4 = 0;
	arg0->unk3a8 = -1;
	arg0->unk3ac = -1;
	arg0->unk3b0 = -1;
	arg0->unk3b4 = 0x10;
	arg0->unk3b8 = 6;
	arg0->unk3bc = 0x80;
	arg0->unk3c0 = 10;
	arg0->unk3c4 = 0x36;
	arg0->unk3c8 = 3;
	arg0->unk3cc = 0x3d;
	arg0->unk3ec = 0;
	arg0->unk3f0 = 0;
	arg0->fileguid.fileid = 0;
	arg0->fileguid.deviceserial = 0;
}

void func0f14b358(void)
{
	// empty
}

void func0f14b360(s32 index)
{
	struct var8007f8e0 *thing = func0f14a06c(index);

	func0f14b228(thing);
	func0f14b394(thing);
}

void func0f14b394(struct var8007f8e0 *arg0)
{
	s32 size;
	s32 i;

	size = align32(func0f14c814(&arg0->unk004));

	for (i = 0; i < size; i++) {
		arg0->unk004.textureptr[i] = rngRandom() % 0xff;
	}

	size = align32(func0f14c814(&arg0->unk010));

	for (i = 0; i < size; i++) {
		arg0->unk010.textureptr[i] = rngRandom() % 0xff;
	}

	func0f14b228(arg0);
}

#if PERFECTHEAD
/**
 * Make the picture brighter (steps > 0) or darker by <steps> of about 5.5%,
 * one step of the original exposure table. The exposure changes; the gain
 * only changes once the exposure is at its shortest or longest. A gain step
 * is 1.5 dB or more, several exposure steps, so after one the next capture is
 * measured first, and the exposure makes the finer corrections after it.
 */
static void editor_step_exposure(struct var8007f8dc *thing, s32 steps)
{
	if (steps > CAM_MAXSTEPS) {
		steps = CAM_MAXSTEPS;
	}

	if (steps < -CAM_MAXSTEPS) {
		steps = -CAM_MAXSTEPS;
	}

	for (; steps < 0; steps++) {
		if (thing->unk05c > CAM_MINEXPOSURE) {
			thing->unk05c -= thing->unk05c / 18 + 1;

			if (thing->unk05c < CAM_MINEXPOSURE) {
				thing->unk05c = CAM_MINEXPOSURE;
			}
		} else if (thing->unk060 > CAM_MINGAIN) {
			thing->unk060--;
			return;
		} else {
			return;
		}
	}

	for (; steps > 0; steps--) {
		if (thing->unk05c < CAM_MAXEXPOSURE) {
			thing->unk05c += thing->unk05c / 17 + 1;

			if (thing->unk05c > CAM_MAXEXPOSURE) {
				thing->unk05c = CAM_MAXEXPOSURE;
			}
		} else if (thing->unk060 < CAM_MAXGAIN) {
			thing->unk060++;
			return;
		} else {
			return;
		}
	}
}
#endif

bool func0f14b484(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];

	thing->unk004 = var8007f8e8 * 0.01f;
	thing->unk008 = var8007f8ec * 0.01f;
	thing->unk00c = var8007f8f0 * 0.01f;
	thing->unk010 = var8007f8f4 * 0.01f;
	thing->unk000 = var8007f8fc;
	thing->unk050 = var8007f900;
	thing->unk020 = thing->unk024;
	thing->unk038 = thing->unk03c;
	thing->unk024 = osGetCount() * 0.0000001f;
	thing->unk028 = thing->unk024 - thing->unk020;

	if (thing->unk028 > 15) {
		thing->unk028 = 15;
	}

#if PERFECTHEAD
	// osGetCount wraps every 92 seconds
	if (thing->unk028 <= 0) {
		thing->unk028 = 1;
	}
#endif

	thing->unk03c = thing->unk04c - thing->unk050;
	thing->unk01c = thing->unk03c;
#if PERFECTHEAD
	// @bug: The derivative term used the error itself divided by the time step
	// (unk03c / unk028) instead of the error's rate of change. Together with
	// the minus sign below this pushes exposure the wrong way whenever frames
	// arrive faster than about every 0.47 s, so the picture runs away to white.
	thing->unk018 = (thing->unk03c - thing->unk038) / thing->unk028;
#else
	thing->unk018 = thing->unk03c / thing->unk028;
#endif
	thing->unk014 += thing->unk03c * thing->unk028;

	if (thing->unk014 > 500) {
		thing->unk014 = 500;
	}

	if (thing->unk014 < -500) {
		thing->unk014 = -500;
	}

#if PERFECTHEAD
	// Standard PID: the derivative term damps the correction
	thing->unk034 = thing->unk010 * thing->unk018;
	thing->unk02c = thing->unk008 * thing->unk01c;
	thing->unk030 = thing->unk00c * thing->unk014;
	thing->unk040 = -thing->unk004 * (thing->unk034 + thing->unk02c + thing->unk030);
#else
	thing->unk034 = thing->unk010 * -1.0f * thing->unk018;
	thing->unk02c = thing->unk008 * thing->unk01c;
	thing->unk030 = thing->unk00c * thing->unk014;
	thing->unk040 = -thing->unk004 * ((thing->unk010 * -1.0f * thing->unk018) + thing->unk008 * thing->unk01c + thing->unk00c * thing->unk014);
#endif

#if PERFECTHEAD
	// @bug: The exposure was set from the table entry unk054, which still
	// pointed at entry 30 (0x085a) after calibration had found the exposure,
	// so the first correction threw that away; the table's shortest exposure
	// (0x0200) is too long for a bright room; and the gain could never change,
	// as unk054 was clamped to the table before the checks for running off it.
	// In bright light the picture stayed white. The exposure is now stepped
	// from where it is, with the gain at the limits (editor_step_exposure).
	if (thing->unk000) {
		if (ABS(thing->unk03c) > 10.0f) {
			if (ABS(thing->unk040) >= 1.0f) {
				editor_step_exposure(thing, (s32)thing->unk040);
				thing->unk014 = 0.0f;
			}
		} else {
			thing->unk014 = 0.0f;
		}
	}
#else
	if (thing->unk000) {
		if (ABS(thing->unk03c) > 10.0f) {
			if (ABS(thing->unk040) >= 1.0f) {
				thing->unk054 += (s32)thing->unk040;

				if (thing->unk054 > 80) {
					thing->unk054 = 80;
				}

				if (thing->unk054 < 1) {
					thing->unk054 = 1;
				}

				thing->unk044 += thing->unk040;

				if (thing->unk044 > 80) {
					thing->unk044 = 80;
				}

				if (thing->unk044 < 1) {
					thing->unk044 = 1;
				}

				thing->unk05c = var8007f904[thing->unk054];

				if (thing->unk05c < var8007f904[1]) {
					thing->unk05c = var8007f904[1];
					thing->unk060--;

					if (thing->unk060 < 2) {
						thing->unk060 = 2;
					}
				} else if (thing->unk05c > var8007f904[80]) {
					thing->unk05c = var8007f904[80];
					thing->unk060++;

					if (thing->unk060 > 10) {
						thing->unk060 = 10;
					}
				}

				thing->unk014 = 0.0f;
			}
		} else {
			thing->unk014 = 0.0f;
		}
	}
#endif

	if (thing->unk04c < 70 || thing->unk04c > 160) {
		return false;
	}

	return true;
}

void func0f14b8ac(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];

	mainOverrideVariable("kg", &var8007f8e8);
	mainOverrideVariable("Kp", &var8007f8ec);
	mainOverrideVariable("Ki", &var8007f8f0);
	mainOverrideVariable("Kd", &var8007f8f4);
	mainOverrideVariable("tInt", &var8007f8f8);
	mainOverrideVariable("gocal", &var8007f8fc);
	mainOverrideVariable("aim", &var8007f900);

	if (thing->unk0d4_00) {
		func0f14cf6c();
		return;
	}

	func0f14a95c();

	if (pak0f11d3f8(func0f14a5a4())) {
		pak0f11d620(func0f14a5a4());

		if (thing->unk0f8 == 0 && var8007f8dc[var800a45a0->unk000].unk0d4_04 == false) {
			func0f14e790(var800a45a0->unk470);
			func0f14a16c(1);
			func0f14bc04();
			return;
		}

		switch (pakGetUnk008(func0f14a5a4())) {
		case PAK008_12:
			func0f14a95c();
			func0f14aa48();
			func0f14e790(var800a45a0->unk470);
			func0f14cf6c();
			pak0f11e3bc(func0f14a5a4());
			func0f14a16c(1);
			func0f14bc04();
			break;
		case PAK008_01:
			pak0f11d4dc(func0f14a5a4());
			break;
		case PAK008_11:
			pak0f11d9c4(func0f14a5a4(), var800a45a0->unk470, 0, 0);
			func0f14d064(index, var800a45a0->unk470);
			pak0f11d478(func0f14a5a4());

			if (thing->unk0f8 == 0) {
				func0f14a16c(3);
				func0f14bc04();

				if (var8007f8dc[var800a45a0->unk000].unk0d4_04) {
					struct var8007f8e0 *thing2 = func0f14a06c(-1);
					thing2->unk3f4_00 = true;
				}
			} else {
				func0f14a16c(4);
				func0f14bc04();
			}

			if (!func0f14a8e8()) {
				func0f14b484(var800a45a0->unk000);
			}
			break;
		}
	} else {
		func0f14cf6c();
		func0f14e7e0(var800a45a0->unk470);
		func0f14a16c(3);
		func0f14bc04();
	}
}

const char var7f1b67f8[] = "CAM : Cam_MakeTextures\n";

void func0f14bc04(void)
{
	struct var8007f8e0 *thing1 = func0f14a06c(-1);
	struct var8007f8dc *thing2 = func0f14a20c();

	thing1->unk3f4_02 = true;

	switch (thing2->unk100) {
	case 0:
		break;
	case 1:
		thing1->unk3f4_02 = false;
		func0f14d714(var800a45a0->unk470, thing1->unk01c);
		break;
	case 4:
		func0f14d714(var800a45a0->unk470, thing1->unk01c);
		break;
	case 2:
		func0f14d2c8(var800a45a0->unk470, thing1->unk01c);
		break;
	case 3:
		func0f14d4f0(var800a45a0->unk470, thing1->unk01c);
		break;
	case 5:
		func0f14ff94(thing1);
		break;
	}

	func0f14def0(-1, 2433, "camdraw.c");

#if PERFECTHEAD
	// The thumbnail is made from the camera's capture buffer, which only
	// exists while the camera is running. A head loaded from the Controller
	// Pak brings its own thumbnail (pheadLoadFile).
	if (thing2->unk100 != 4 && var800a45a0->unk470 != NULL) {
#else
	if (thing2->unk100 != 4) {
#endif
		phead0f14dac0(var800a45a0->unk470, &thing1->unk010);
	}

	thing1->unk3f4_03 = false;
}

const char var7f1b681c[] = "pD->ReadMode = %s";
const char var7f1b6830[] = "Camera -> Getting Header Info\n";
const char var7f1b6850[] = "Camera -> Item=%d, pD->CamHeader[i]=%d\n";
const char var7f1b6878[] = "Camera -> Found %d Valid images\n";
const char var7f1b689c[] = "Camera -> Item=%d, Valid=%s, Bank=%d\n";
const char var7f1b68c4[] = "YES";
const char var7f1b68c8[] = "NO";
const char var7f1b68cc[] = "CD_TransferCurrentCacheToEditorAndExit : Player=%d, pD->CamRdImg=%d\n";
const char var7f1b6914[] = "Camera -> CD_CycleHandleRead - Need to load up the camera header\n";
const char var7f1b6958[] = "Camera -> CD_CycleHandleRead - Image %d needs loading for editor coppying on menu exit\n";
const char var7f1b69b0[] = "Camera -> CD_CycleHandleRead - Error ekCamReadModeExit2 in Null cycle\n";
const char var7f1b69f8[] = "CAMERA : COPY SLOT (%d of %s): Slot %d(%s) -> Slot %d(%s)\n";
const char var7f1b6a34[] = "ACTIVE";
const char var7f1b6a3c[] = "NULL";
const char var7f1b6a44[] = "ACTIVE";
const char var7f1b6a4c[] = "NULL";
const char var7f1b6a54[] = "RWI : Warning -> Not copying the thumbnail texture\n";
const char var7f1b6a88[] = "CD_DrawTexture : pTex=%x\n";
const char var7f1b6aa4[] = "Cam -> WARNING - Not done intensity table recalc :- No samples within threshold\n";
const char var7f1b6af8[] = "Camera -> Call to reset auto calibrate\n";
const char var7f1b6b20[] = "AC -> ekCamAutoCalStatusReset - pD->tTime = %d, pD->tTimeNxAdd = %d\n";
const char var7f1b6b68[] = "Cam 0 -> Time = %d, Target = %d, tInt = %.2f\n";
const char var7f1b6b98[] = "Cam 0 -> Too Low -> Adding Time\n";
const char var7f1b6bbc[] = "Cam 0 -> Too High -> Subbing Time\n";
const char var7f1b6be0[] = "Cam -> ekCamAutoCalStatusSettingTime - New time = %d\n";
const char var7f1b6c18[] = "%s%sAC -> Gain = %d, Target = %.2f, tInt = %.2f\n";
const char var7f1b6c4c[] = "";
const char var7f1b6c50[] = "";
const char var7f1b6c54[] = "Auto Calibrate is removing the menu message\n";
const char var7f1b6c84[] = "Auto Calibrate is shutting down\n";
const char var7f1b6ca8[] = "Cam -> Cam_BalanceSlot %d -> tZroShift = %d\n";
const char var7f1b6cd8[] = "Cam_BuildFaceTexture (slot=%d): %d of %s\n";

void func0f14bd34(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];

	if (thing->unk080 != -1) {
		pak0f1171b4(func0f14a5a4(), PAK00C_03, 0);
		pak0f11d540(func0f14a5a4(), thing->unk0b6[thing->unk080]);
	}
}

void func0f14bdbc(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];
	struct var8007f8e0 *thing2;
	s32 i;
	s32 value;

	for (i = 0; i < 30; i++) {
		thing->unk0dc[i] = 0;
	}

	thing->unk094 = 0;

	for (i = 0; i < ARRAYCOUNT(thing->unk098); i++) {
		thing->unk098[i] = 0xff;
		thing->unk0b6[i] = 0xff;
	}

	value = var8007f8dc[index].unk0f4;

	if (value == 4 || value == 5) {
		thing->unk0f4 = 6;
		g_MpPlayerNum = CAM_MENUPLAYER(index);
		menuSetBanner(-1, false);

		thing2 = func0f14a06c(-1);
		thing2->unk3f4_00 = false;

		g_MpPlayerNum = CAM_MENUPLAYER(index);

		menuPopDialog();

		g_MpPlayerNum = 0;
	} else {
		pak0f11e3bc(func0f14a5a4());

		thing->unk0f4 = 0;
	}
}

void func0f14bec8(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];
	struct var8007f8e0 *thing2 = func0f14a06c(-1);
	struct textureconfig *thing3;
	s32 i;

	if (thing->unk0f4 == 4 && thing->unk080 == thing->unk074) {
		thing->unk0f4 = 5;
	}

	switch (thing->unk0f4) {
	case 1:
		pak0f11d9c4(index, NULL, thing->unk098, 0);
		thing->unk0f4 = 2;

#if PERFECTHEAD
		// The camera's state vector is indexed by memory slot and holds that
		// slot's album number (0xff = free). The original took the album number
		// as the slot, which only holds until a picture is deleted on the
		// camera. List the pictures in album order, each read from its own slot
		// (slot i is at SRAM 0x2000 + i * 0x1000, location i + 2).
		thing->unk094 = 0;

		{
			s32 albumnum;

			for (albumnum = 0; albumnum < ARRAYCOUNT(thing->unk098); albumnum++) {
				for (i = 0; i < ARRAYCOUNT(thing->unk098); i++) {
					if (thing->unk098[i] == albumnum) {
						thing->unk0b6[thing->unk094] = i + 2;
						thing->unk094++;
						break;
					}
				}
			}
		}
#else
		for (i = 0; i < ARRAYCOUNT(thing->unk098); i++) {
			if (thing->unk098[i] != 0xff) {
				thing->unk098[i] += 2;
			}
		}

		for (i = 0, thing->unk094 = 0; i < ARRAYCOUNT(thing->unk098); i++) {
			if (thing->unk098[i] != 0xff) {
				thing->unk0b6[thing->unk094] = thing->unk098[i];
				thing->unk094++;
			}
		}
#endif

		for (i = thing->unk094; i < ARRAYCOUNT(thing->unk0b6); i++) {
			thing->unk0b6[i] = 0xff;
		}

		if (thing->unk094);

		for (i = 0; i < ARRAYCOUNT(thing->unk0b6); i++);

		pak0f1171b4(func0f14a5a4(), PAK00C_03, 0);
		pak0f11d478(func0f14a5a4());
		break;
	case 3:
		pak0f11d9c4(func0f14a5a4(), var800a45a0->unk470, 0, 1);
		thing3 = &var800a45a0->unk484[thing->unk080];
		func0f14a16c(2);
		func0f14bc04();
		func0f14c75c(thing3, &thing2->unk004);
		thing->unk0dc[thing->unk080] = 1;
		pak0f11d478(func0f14a5a4());
		break;
	case 4:
		pak0f11d478(func0f14a5a4());
		break;
	case 5:
		pak0f11d9c4(func0f14a5a4(), var800a45a0->unk470, 0, 1);
		func0f14a16c(3);
		func0f14bc04();
		pak0f11d478(func0f14a5a4());
		g_MpPlayerNum = CAM_MENUPLAYER(index);
		menuSetBanner(-1, false);
		func0f14a678();
		thing2->unk3f4_00 = true;
		g_MpPlayerNum = CAM_MENUPLAYER(index);
		stub0f102230();
		g_MpPlayerNum = 0;
		thing->unk0f4 = 6;
		break;
	case 2:
	case 6:
		break;
	}
}

void func0f14c1cc(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];
	s32 count = func0f14f008(index);
	s32 max = count / 2 + 1;
	s32 i;

	switch (thing->unk0f4) {
	case 0:
		pak0f1171b4(func0f14a5a4(), PAK00C_00, 0);
		pak0f11d5b0(func0f14a5a4());
		thing->unk0f4 = 1;
		break;
	case 2:
		thing->unk0f4 = 3;
		break;
	case 4:
		if (thing->unk074 != thing->unk080) {
			thing->unk0dc[thing->unk074] = 0;
			thing->unk080 = thing->unk074;
			thing->unk0f4 = 5;

			func0f14bd34(index);
		}
		break;
	case 3:
		thing->unk080 = -1;

#if PERFECTHEAD
		// An empty album (count 0) would divide by zero below, which traps
		if (count == 0) {
			max = 0;
		}
#endif

		for (i = 0; i < max; i++) {
			s32 a = (thing->unk074 + i) % count;
			s32 b = thing->unk074 - i;

			if (b < 0) {
				b += count;
			}

			if (thing->unk0dc[a] == 0 && thing->unk0b6[a] != 0xff) {
				thing->unk080 = a;
				break;
			}

			if (thing->unk0dc[b] == 0 && thing->unk0b6[b] != 0xff) {
				thing->unk080 = b;
				break;
			}
		}
		func0f14bd34(index);
		break;
	case 1:
	case 5:
		break;
	}
}

void func0f14c3a4(s32 index)
{
	struct var8007f8dc *thing = &var8007f8dc[index];
	s32 i;

	if (pak0f11d3f8(func0f14a5a4()) == 0) {
		for (i = 0; i < 30; i++) {
			thing->unk0dc[i] = 0;
		}

		for (i = 0; i < ARRAYCOUNT(thing->unk098); i++) {
			thing->unk094 = 0;
			thing->unk098[i] = 0xff;
			thing->unk0b6[i] = 0xff;
		}
	} else {
		pak0f11d620(func0f14a5a4());

		switch (pakGetUnk008(func0f14a5a4())) {
		case PAK008_01:
			func0f14c1cc(index);
			break;
		case PAK008_11:
			func0f14bec8(index);
			break;
		case PAK008_12:
			func0f14bdbc(index);
			break;
		}
	}
}

void func0f14c4c0(s32 index)
{
	var8007f8dc[index].unk0f4 = 4;
	g_MpPlayerNum = CAM_MENUPLAYER(index);

	menuSetBanner(MENUBANNER_DOWNLOADINGIMAGE, false);
}

void func0f14c50c(struct var8007f8e0 *dst, struct var8007f8e0 *src, u32 line, char *file)
{
	struct var8007f8e0 *thing;
	s32 i;
	s32 j;
	s32 k;
	s32 l;
	s32 m;

	for (i = 0; i < 22; i++) {
		thing = &var8007f8e0[i];
		if (thing);
	}

	if (src->unk3f4_00);

	dst->unk3f4_00 = src->unk3f4_00;
	dst->unk3f4_01 = src->unk3f4_01;
	dst->unk3f4_02 = src->unk3f4_02;
	dst->unk3f4_03 = src->unk3f4_03;
	dst->colournum = src->colournum;
	dst->stylenum = src->stylenum;
	dst->unk3a4 = src->unk3a4;
	dst->unk3b4 = src->unk3b4;
	dst->unk3b8 = src->unk3b8;
	dst->unk3bc = src->unk3bc;
	dst->unk3c0 = src->unk3c0;
	dst->unk3c4 = src->unk3c4;
	dst->unk3c8 = src->unk3c8;
	dst->unk3cc = src->unk3cc;
	dst->unk3ec = src->unk3ec;
	dst->unk3f0 = src->unk3f0;

	dst->fileguid.fileid = src->fileguid.fileid;
	dst->fileguid.deviceserial = src->fileguid.deviceserial;

	for (i = 0; i < ARRAYCOUNT(dst->unk3d0); i++) {
		dst->unk3d0[i] = src->unk3d0[i];
	}

	if (dst->unk01c) {
		for (j = 63, m = 0, l = 63 * 64; (l ^ 0) != -0x40;) {
			for (k = 0; k != 64;) {
				s32 fudge = CAM_TMEMSWAP(j, k);
				u8 tmp = *(u8 *)(src->unk004.texturenum + (0, fudge) + l + k);

				dst->unk01c[m + k] = tmp;
				k++;
			}

			l -= 64;
			j--;
			m += 64;
		}
	}

	func0f14c75c(&dst->unk004, &src->unk004);

	if (dst->unk010.textureptr && src->unk010.textureptr) {
		func0f14c75c(&dst->unk010, &src->unk010);
	}

	if (src->unk3f4_03) {
		for (i = 0; i < 1024; i++) {
			dst->unk020[i] = src->unk020[i];
		}
	}
}

void func0f14c75c(struct textureconfig *arg0, struct textureconfig *arg1)
{
	s32 i;
	u32 size = align32(func0f14c814(arg0));

	align32(func0f14c814(arg1));

	for (i = 0; i < size; i++) {
		arg0->textureptr[i] = arg1->textureptr[i];
	}
}

void func0f14c7dc(struct textureconfig *tconfig)
{
	tconfig->width = 64;
	tconfig->height = 64;
	tconfig->level = 0;
	tconfig->format = 4;
	tconfig->depth = 1;
	tconfig->s = 0;
	tconfig->t = 1;
	tconfig->unk0b = 1;
}

void func0f14c80c(void)
{
	// empty
}

u32 func0f14c814(struct textureconfig *tconfig)
{
	u32 size = tconfig->width * tconfig->height;

	switch (tconfig->depth) {
	case 0:
		size >>= 1;
		break;
	case 2:
		size *= 2;
		break;
	case 3:
		size *= 4;
		break;
	case 1:
		break;
	}

	return size;
}

Gfx *func0f14c870(Gfx *gdl, struct textureconfig *tconfig, f32 *arg2, f32 arg3, f32 arg4)
{
	u32 x = (arg2[0] + 0.5f) * 4.0f;
	u32 y = (arg2[1] + 0.5f) * 4.0f;

	u32 width = ((arg2[0] - 0.5f) + (arg3 * tconfig->width)) * 4.0f;
	u32 height = ((arg2[1] - 0.5f) + (arg3 * tconfig->height)) * 4.0f;

	u32 stack;
	s32 sp28 = (tconfig->height - 0.5f) * 32.0f;

	gDPPipeSync(gdl++);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetCombineMode(gdl++, G_CC_MODULATEI, G_CC_MODULATEI);

	texSelect(&gdl, tconfig, 1, 0, 2, 1, &var800a45a8);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);

	gSPTextureRectangle(gdl++,
			x * g_ScaleX, y,
			width * g_ScaleX, height,
			G_TX_RENDERTILE, 16, sp28,
			(s32)(1024.0f / arg3) / g_ScaleX,
			(s32)(1024.0f / arg4));

	return gdl;
}

void func0f14cdb8(s32 index, u8 *arg1)
{
	s32 count;
	s32 i;
	s32 j;

	var8007f8dc[index].unk04c = 0;

	count = 0;

	for (i = 0x1000; i < 0x3000; i += 0x80) {
		for (j = 0x20; j < 0x60; j++) {
			count++;
			var8007f8dc[index].unk04c += arg1[i + j];
		}
	}

	if (count > 0) {
		var8007f8dc[index].unk04c /= count;
		return;
	}

	var8007f8dc[index].unk04c = 0;
}

void func0f14ce84(void)
{
	if (pak0f11d3f8(func0f14a5a4())) {
		if (pakGetUnk008(func0f14a5a4()) != PAK008_12) {
			var8007f8dc[func0f14a5a4()].unk0f8 = 1;
			var8007f8dc[func0f14a5a4()].unk0d4_00 = false;

			func0f14a95c();
			func0f14aa48();
			func0f14aac4();
			func0f14ab3c();
		} else {
			pak0f11e3bc(func0f14a5a4());
		}
	}
}

void func0f14cf6c(void)
{
	g_MpPlayerNum = CAM_MENUPLAYER(0);

	menuSetBanner(-1, false);

	if (var8007f8dc[func0f14a5a4()].unk0f8) {
		switch (var8007f8dc[func0f14a5a4()].unk0f8) {
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			pak0f1171b4(func0f14a5a4(), PAK00C_03, 0);
			g_MpPlayerNum = CAM_MENUPLAYER(0);
			menuSetBanner(-1, false);
			break;
		}

		var8007f8dc[func0f14a5a4()].unk0f8 = 0;
	}
}

void func0f14d064(s32 index, u8 *arg1)
{
	struct var8007f8dc *thing = &var8007f8dc[index];
	f32 a;
	f32 b;

	if (thing->unk0f8 == 1) {
		thing->unk05c = 0x4000;
		thing->unk064 = 0x2000;
#if PERFECTHEAD
		// Start at 20 dB, within the Game Boy Camera ROM's own 14-32 dB
		// (Pan Docs) rather than at 23 dB
		thing->unk060 = CAM_STARTGAIN;
#else
		thing->unk060 = 6;
#endif
		g_MpPlayerNum = CAM_MENUPLAYER(index);
		menuSetBanner(MENUBANNER_CALIBRATINGCAMERA, false);
		func0f14a16c(1);
		pak0f1171b4(func0f14a5a4(), PAK00C_01, 1);
		thing->unk0f8 = 3;
		return;
	}

	if (thing->unk0f8 == 3) {
		func0f14cdb8(index, arg1);
		a = thing->unk04c;

		if (a > 130) {
			b = a - 130;
		} else {
			b = -(a - 130);
		}

		if (b < 10 || thing->unk064 == 64) {
			thing->unk0f8 = 2;
		} else {
			if (a < 130) {
				thing->unk05c += thing->unk064;
			} else {
				thing->unk05c -= thing->unk064;
			}

			thing->unk064 >>= 1;
		}
	}

	if (thing->unk0f8 == 2) {
		func0f14cdb8(index, arg1);

#if PERFECTHEAD
		if (thing->unk060 >= CAM_MAXGAIN || thing->unk04c > 120) {
#else
		if (thing->unk060 == 12 || thing->unk04c > 120) {
#endif
			thing->unk0f8 = 6;
		} else {
			thing->unk060 += 2;
		}
	}

	if (thing->unk0f8 == 6) {
		pak0f1171b4(func0f14a5a4(), PAK00C_03, 0);
		thing->unk0f8 = 6;
		thing->unk100 = thing->unk0fc;
		g_MpPlayerNum = CAM_MENUPLAYER(index);
		menuSetBanner(-1, false);
		pak0f1171b4(func0f14a5a4(), PAK00C_03, 0);
		var8007f8dc[var800a45a0->unk000].unk0d4_04 = true;
		thing->unk0f8 = 0;
	}

	if (thing->unk0f8 == 0) {
		func0f14cdb8(index, arg1);
	}
}

void func0f14d2c8(u8 *arg0, u8 *arg1)
{
	f32 *s1 = (f32 *)var800a45a0->unk474;
	f32 *s0 = (f32 *)var800a45a0->unk478;
	s32 size = 128;
	s32 i;
	s32 j;
	s32 k;
	s32 l;
	f32 sp50[] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	s32 four = 4;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			s1[i * size + j] = *(j + arg0 + i * size);
		}
	}

	func0f14d84c(sp50, 3);
	func0f14d8d8(s1, s0, 128, sp50, 3);

	for (k = 0; k < 63 * 64; k += 64) {
		for (l = 0; l < 63; l++) {
			f32 *ptr = &s0[k * four + l * 2];
			f32 f0;

			f0 = (ptr[0] + ptr[1] + ptr[128] + ptr[129]) / 4.0f;

			if (f0 < 0.0f) {
				f0 = 0.0f;
			}

			if (f0 > 255.0f) {
				f0 = 255.0f;
			}

			arg1[k + l] = f0;
		}
	}
}

/**
 * Generate a 64x64 thumbnail from a 128x128 source.
 */
void func0f14d4f0(u8 *src, u8 *dst)
{
	f32 *s1 = (f32 *) var800a45a0->unk474;
	f32 *s0 = (f32 *) var800a45a0->unk478;
	s32 i;
	s32 j;
	s32 x;
	s32 y;
	s32 size = 128;

	f32 sp50[] = {
		0.0029690000228584f,
		0.013306000269949f,
		0.021937999874353f,
		0.013306000269949f,
		0.0029690000228584f,
		0.013306000269949f,
		0.059634000062943f,
		0.098319999873638f,
		0.059634000062943f,
		0.013306000269949f,
		0.021937999874353f,
		0.098319999873638f,
		0.16210299730301f,
		0.098319999873638f,
		0.021937999874353f,
		0.013306000269949f,
		0.059634000062943f,
		0.098319999873638f,
		0.059634000062943f,
		0.013306000269949f,
		0.0029690000228584f,
		0.013306000269949f,
		0.021937999874353f,
		0.013306000269949f,
		0.0029690000228584f,
	};

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			s32 index = i * size + j;
			s1[index] = src[index];
		}
	}

	func0f14d8d8(s1, s0, size, sp50, 5);

	for (y = 0; y < 63; y++) {
		for (x = 0; x < 63; x++) {
			f32 *ptr = &s0[y * sizeof(u16) * size + x * sizeof(u16)];
			f32 value = (ptr[0] + ptr[1] + ptr[128] + ptr[129]) * 0.25f;

			if (value < 0.0f) {
				value = 0.0f;
			}

			if (value > 255.0f) {
				value = 255.0f;
			}

			dst[y * 64 + x] = value;
		}
	}
}

void func0f14d714(u8 *arg0, u8 *arg1)
{
	s32 x;
	s32 y;

	for (y = 0; y < 63; y++) {
		for (x = 0; x < 63; x++) {
			f32 value = (arg0[y * 256 + x * 2]
					+ arg0[y * 256 + x * 2 + 1]
					+ arg0[y * 256 + x * 2 + 0x80]
					+ arg0[y * 256 + x * 2 + 0x81]) * 0.25f;

			if (value < 0.0f) {
				value = 0.0f;
			}

			if (value > 255.0f) {
				value = 255.0f;
			}

			arg1[y * 64 + x] = value;
		}
	}
}

void func0f14d84c(f32 *arg0, s32 arg1)
{
	f32 sum = 0;
	s32 len = arg1 * arg1;
	f32 mult;
	s32 i;

	for (i = 0; i < len; i++) {
		sum += arg0[i];
	}

	mult = 1 / sum;

	for (i = 0; i < len; i++) {
		arg0[i] *= mult;
	}
}

void func0f14d8d8(f32 *arg0, f32 *arg1, s32 arg2, f32 *arg3, s32 arg4)
{
	s32 t0 = arg4 / 2;
	s32 t1 = t0 + 1;
	s32 v0 = arg2 - 1;
	s32 a3 = -t0;
	s32 i;
	s32 j;
	s32 k;
	s32 l;
	f32 sp5c[19][19];
	s32 m;
	s32 n;

	for (i = 0; i < arg4; i++) {
		for (j = 0; j < arg4; j++) {
			sp5c[i][j] = arg3[i * arg4 + j];
		}
	}

	for (k = 0; k < arg2; k++) {
		for (l = 0; l < arg2; l++) {
			f32 f0 = 0.0f;

			for (m = a3; m < t1; m++) {
				s32 t5 = k + m;

				if (t5 > v0) {
					t5 = v0;
				}

				if (t5 < 0) {
					t5 = 0;
				}

				for (n = a3; n < t1; n++) {
					s32 s2 = l + n;

					if (s2 > v0) {
						s2 = v0;
					}

					if (s2 < 0) {
						s2 = 0;
					}

					f0 += sp5c[m + t0][n + t0] * arg0[t5 * arg2 + s2];
				}
			}

			arg1[k * arg2 + l] = f0;
		}
	}
}

void phead0f14dac0(u8 *arg0, struct textureconfig *arg1)
{
	s32 i;
	s32 j;
	s32 k;
	s32 l;

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			f32 f12 = 0.0f;

			for (k = 0; k < 8; k++) {
				for (l = 0; l < 8; l++) {
					f12 += arg0[(i * 8 + l) * 128 + j * 8 + k];
				}
			}

			f12 *= 0.015625f;

			if (f12 > 255.0f) {
				f12 = 255.0f;
			}

			f12 *= 0.0625f;

			if (arg1 != NULL) {
				func0f14e884(arg1, 15 - i, j, f12);
			}
		}
	}
}

void func0f14dc30(s32 index, bool arg1)
{
	f32 f26 = func0f14a984() - 128;
	s32 sp80;
	s32 sp7c;
	s32 y;
	s32 j;
	struct var8007f8e0 *thing = func0f14a06c(index);
	f32 f22;

	f22 = thing->unk3f4_02 ? func0f14e4ac(0, &sp80, &sp7c) : 0.0f;

	for (y = 0; y < 64; y++) {
		for (j = 0; j < 64; j++) {
			f32 f14 = thing->unk01c[y * 64 + j];
			f32 f12;
			f32 f2;

			if (arg1 && thing->unk3f4_02 && sp7c) {
				f32 f0 = sp80 - j;
				f32 fVar8 = f0 > 0.0f ? f0 : -f0;

				f12 = fVar8 / sp7c;

				if (f12 > 1.0f) {
					f12 = 1.0f;
				}

				f2 = f22 * f12;

				if (j > sp80) {
					f2 *= -1.0f;
				}

				f2 *= var8007f8dc->unk088;
			} else {
				f2 = 0.0f;
			}

			f12 = f14 + f2 + f26;

			if (f12 < 0.0f) {
				f12 = 0.0f;
			}

			if (f12 > 255.0f) {
				f12 = 255.0f;
			}

			thing->unk004.textureptr[(u32)(63 - y) * 64 + j + CAM_TMEMSWAP((u32)(63 - y), j)] = f12;
		}
	}
}

void func0f14def0(s32 index, u32 line, char *file)
{
	struct var8007f8e0 *a = func0f14a06c(-1);
	struct var8007f8e0 *b = (index != -1 ? func0f14a06c(index) : NULL);
	struct var8007f8e0 *c = func0f14a06c(-4);
	struct var8007f8e0 *d = func0f14a06c(-5);

	func0f14c50c(d, a, 3508, "camdraw.c");

	func0f14dc30(-1, false);

	if (b != NULL) {
		func0f14e1c4(b);
	}

	func0f14dc30(-5, true);

	func0f14e1c4(d);
	func0f14c50c(c, d, 3519, "camdraw.c");
}

const char var7f1b6d1c[] = "Cam %d -> Balance : No Data Available\n";

s32 func0f14dfc0(struct var8007f8e0 *arg0, s32 arg1, s32 arg2)
{
	u32 sp3c[5];
	u32 sp28[5];
	s32 i;
	s32 j;
	s32 len;

	sp28[0] = *(arg0->unk004.textureptr + arg2 * 64 + CAM_TMEMSWAP(arg2, arg1) + arg1);
	sp28[1] = *(arg0->unk004.textureptr + (arg2 - 1) * 64 + CAM_TMEMSWAP(arg2 - 1, arg1) + arg1);
	sp28[2] = *(arg0->unk004.textureptr + arg2 * 64 + CAM_TMEMSWAP(arg2, arg1 - 1) + arg1 - 1);
	sp28[3] = *(arg0->unk004.textureptr + (arg2 + 1) * 64 + CAM_TMEMSWAP(arg2 + 1, arg1) + arg1);
	sp28[4] = *(arg0->unk004.textureptr + arg2 * 64 + CAM_TMEMSWAP(arg2, arg1 + 1) + arg1 + 1);

	len = 0;

	for (i = 0; i < ARRAYCOUNT(sp28); i++) {
		s32 bestvalue = 9999;
		s32 bestindex = -1;

		for (j = 0; j < ARRAYCOUNT(sp28); j++) {
			if (sp28[j] < bestvalue) {
				bestindex = j;
				bestvalue = sp28[j];
			}
		}

		if (bestindex != -1) {
			sp3c[len] = sp28[bestindex];
			len++;

			sp28[bestindex] = 99999;
		}
	}

	return (sp3c[1] + sp3c[2] + sp3c[3]) / 3;
}

#if PERFECTHEAD
/**
 * Position Image lets the crop rectangle reach past the 64x64 picture (the
 * painter shows the head's own skin there). These routines index the picture
 * by the crop, so give them the part inside it: the range the crop always had.
 */
static void camdrawClampCrop(s32 *x1, s32 *y1, s32 *x2, s32 *y2)
{
	if (*x1 < 0) *x1 = 0;
	if (*y1 < 0) *y1 = 0;
	if (*x2 > 64) *x2 = 64;
	if (*y2 > 64) *y2 = 64;
}
#endif

void func0f14e1c4(struct var8007f8e0 *arg0)
{
	s32 spbc;
	s32 spb8;
	s32 spb4;
	s32 spb0;
	s32 mid1;
	s32 mid2;
	f32 f26;
	s32 a;
	s32 b;
	s32 value1;
	s32 value2;
	f32 f22;
	f32 f20;
	s32 i;

	func0f14ecd8(&spbc, &spb8, &spb4, &spb0);

#if PERFECTHEAD
	camdrawClampCrop(&spbc, &spb8, &spb4, &spb0);
#endif

	mid1 = (u32) (spb0 + spb8) >> 1;
	mid2 = (u32) (spb4 + spbc) >> 1;

	f26 = (f32) (spb4 - spbc);

	a = mid2 - (u32)(f26 * 0.25f);
	b = (u32)(f26 * 0.25f) + mid2;

	value1 = func0f14dfc0(arg0, a, mid1);
	value2 = func0f14dfc0(arg0, b, mid1);

	f22 = f26 * 0.5f;

	for (f20 = 0.0f; f20 < f22; f20 += 1.0f) {
		s32 size = f22 - sqrtf(2.0f * f22 * f20 - f20 * f20);

		s32 y1 = ((s32)f20 + spb8);
		s32 y2 = (spb0 - (s32)f20);

		for (i = 0; i < size; i++) {
			s32 x1 = spbc + i;
			s32 x2 = spb4 - i;

			arg0->unk004.textureptr[y1 * 64 + x1 + CAM_TMEMSWAP(y1, x1)] = value1;
			arg0->unk004.textureptr[y2 * 64 + x1 + CAM_TMEMSWAP(y2, x1)] = value1;
			arg0->unk004.textureptr[y1 * 64 + x2 + CAM_TMEMSWAP(y1, x2)] = value2;
			arg0->unk004.textureptr[y2 * 64 + x2 + CAM_TMEMSWAP(y2, x2)] = value2;
		}
	}
}

f32 func0f14e4ac(s32 arg0, s32 *arg1, s32 *arg2)
{
	struct var8007f8e0 *spa4 = func0f14a06c(-1);
	s32 i;
	s32 j;
	s32 sp98;
	s32 sp94;
	s32 sp90;
	s32 sp8c;
	s32 range;
	s32 count1;
	s32 count2;
	f32 sum1;
	f32 sum2;
	s32 avg1;
	s32 avg2;
	s32 upper1;
	s32 upper2;
	s32 lower1;
	s32 lower2;
	s32 halfdiff1;
	s32 halfdiff2;
	f32 result;

	func0f14ecd8(&sp98, &sp94, &sp90, &sp8c);

#if PERFECTHEAD
	camdrawClampCrop(&sp98, &sp94, &sp90, &sp8c);
#endif

	if (arg1 != NULL) {
		*arg1 = 0;
	}

	count1 = 0;
	count2 = 0;
	sum1 = 0.0f;

	if (arg2 != NULL) {
		*arg2 = 0;
	}

	sum2 = 0.0f;
	range = sp90 - sp98;

	if (range < 32) {
		lower1 = ((sp90 + sp98) >> 1) - 16;

		if (lower1 < 0) {
			lower1 = 0;
		}

		upper1 = lower1 + 32;
	} else {
		lower1 = ((sp90 + sp98) >> 1) - (range >> 1);
		upper1 = lower1 + range;
	}

	range = sp8c - sp94;

	if (range < 32) {
		lower2 = ((sp8c + sp94) >> 1) - 16;

		if (lower2 < 0) {
			lower2 = 0;
		}

		upper2 = lower2 + 32;
	} else {
		lower2 = ((sp8c + sp94) >> 1) - (range >> 1);
		upper2 = lower2 + range;
	}

	halfdiff1 = (upper1 - lower1) >> 1;
	halfdiff2 = (upper2 - lower2) >> 1;

	for (i = 0; i < halfdiff2; i++) {
		s32 start = halfdiff1 - 16;

		if (start < 0) {
			start = 0;
		}

		avg1 = (lower1 + upper1) >> 1;
		avg2 = (lower2 + upper2) >> 1;

		for (j = start; j < halfdiff1; j++) {
			u8 value1 = spa4->unk01c[(avg2 - i) * 64 + avg1 - j];
			u8 value2 = spa4->unk01c[(avg2 + i) * 64 + avg1 + j];

			if (value1 >= 21 && value1 < 200 && value2 >= 21 && value2 < 200) {
				sum1 += value1;
				sum2 += value2;
				count1++;
				count2++;
			}
		}
	}

	if (count1 != 0 && count2 != 0) {
		sum1 *= 1.0f / count1;
		sum2 *= 1.0f / count2;

		if (arg1 != NULL) {
			*arg1 = (lower1 + upper1) >> 1;
		}

		if (arg2 != NULL) {
			*arg2 = halfdiff1;
		}

		result = (sum1 + sum2) * 0.5f - sum1;

		if (1);

		var8007f8dc[arg0].unk090 = result;

		return result;
	}

	return 0.0f;
}

void func0f14e790(u8 *arg0)
{
	s32 i;
	s32 j;

	for (i = 0; i < 128; i++) {
		for (j = 0; j < 128; j++) {
			arg0[i * 128 + j] = j / 16 * 32;
		}
	}
}

void func0f14e7e0(u8 *arg0)
{
	s32 i;
	s32 j;

	for (i = 0; i < 128; i++) {
		for (j = 0; j < 128; j++) {
			arg0[i * 128 + j] = rngRandom() % 255;
		}
	}
}

void func0f14e884(struct textureconfig *tconfig, s32 numrows, s32 arg2, u64 arg3)
{
	s32 sp5c;
	s32 numbytes;
	s32 bitsperpixel;
	u32 stack;
	s32 pixelsperdword;
	s64 sp40;
	s64 mask;
	s64 *sp34;

	switch (tconfig->depth) {
	case G_IM_SIZ_4b:
		pixelsperdword = 16;
		mask = 0xf;
		sp5c = arg2 >> 1;
		bitsperpixel = 4;
		numbytes = (tconfig->width >> 1) * numrows;
		break;
	case G_IM_SIZ_8b:
		pixelsperdword = 8;
		mask = 0xff;
		sp5c = arg2;
		bitsperpixel = 8;
		numbytes = tconfig->width * numrows;
		break;
	case G_IM_SIZ_16b:
		pixelsperdword = 4;
		mask = 0xffff;
		sp5c = arg2 * 2;
		bitsperpixel = 16;
		numbytes = tconfig->width * 2 * numrows;
		break;
	case G_IM_SIZ_32b:
		pixelsperdword = 2;
		mask = 0xffffffff;
		sp5c = arg2 * 4;
		bitsperpixel = 32;
		numbytes = tconfig->width * 4 * numrows;
		break;
#ifndef PLATFORM_N64
	default: // the four sizes are all a texture can have
		return;
#endif
	}

#ifdef PLATFORM_N64
	sp40 = ((arg2 % pixelsperdword) * bitsperpixel);
	sp40 = (0x40 - sp40 - bitsperpixel) & 0x3f;

	if (numrows & 1) {
		sp40 = (sp40 + 0x20) & 0x3f;
	}

	sp34 = (s64 *)(tconfig->textureptr + ((sp5c >> 3) + (numbytes >> 3)) * 8);
	*sp34 = (*sp34 & ~(mask << sp40)) + (arg3 << sp40);
#else
	// The port's textures are linear (no odd-row TMEM swap) and keep the N64's
	// big-endian texel order, so write the texel's bytes/nibble directly
	// instead of doing a host-endian 64-bit read-modify-write.
	{
		u8 *dst = tconfig->textureptr + numbytes + sp5c;

		if (bitsperpixel == 4) {
			if (arg2 & 1) {
				*dst = (*dst & 0xf0) | (arg3 & 0xf);
			} else {
				*dst = (*dst & 0x0f) | ((arg3 & 0xf) << 4);
			}
		} else {
			s32 i;

			for (i = bitsperpixel / 8 - 1; i >= 0; i--) {
				dst[i] = arg3 & 0xff;
				arg3 >>= 8;
			}
		}
	}
#endif
}

Gfx *func0f14eb04(Gfx *gdl, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
	f32 sp30[2];
	struct var8007f8e0 *thing;
	u32 stack[2];

	sp30[0] = arg2;
	sp30[1] = arg3;

	thing = func0f14a06c(arg1);

	gdl = func0f14c870(gdl, &thing->unk004, sp30, (arg4 - arg2) * 0.015625f, (arg5 - arg3) * 0.015625f);

	return gdl;
}

Gfx *func0f14eb98(Gfx *gdl, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
	f32 sp30[2];
	struct var8007f8e0 *thing;
	u32 stack[2];

	sp30[0] = arg2;
	sp30[1] = arg3;

	thing = func0f14a06c(arg1);

	gdl = func0f14c870(gdl, &thing->unk010, sp30, (arg4 - arg2) * 0.0625f, (arg5 - arg3) * 0.0625f);

	return gdl;
}

void func0f14ec2c(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	struct var8007f8e0 *thing1 = func0f14a06c(-1);
	struct var8007f8e0 *thing2 = func0f14a06c(-2);

	if (thing1) {
		thing1->unk3c0 = arg0;
		thing1->unk3c8 = arg1;
		thing1->unk3c4 = arg2;
		thing1->unk3cc = arg3;
		thing1->unk3ec = 0;
		thing1->unk3f0 = 0;
	}

	if (thing2) {
		thing2->unk3c0 = arg0;
		thing2->unk3c8 = arg1;
		thing2->unk3c4 = arg2;
		thing2->unk3cc = arg3;
		thing2->unk3ec = 0;
		thing2->unk3f0 = 0;
	}

	func0f14def0(-1, 4159, "camdraw.c");
}

const char var7f1b6d50[] = "Cam_SetSquashZ : %u, %u, %f\n";

void func0f14ecd8(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
	struct var8007f8e0 *thing = func0f14a06c(-2);

	*arg0 = thing->unk3c0;
	*arg1 = thing->unk3c8;
	*arg2 = thing->unk3c4;
	*arg3 = thing->unk3cc;
}

void func0f14ed38(void)
{
	func0f14ec2c(10, 3, 54, 61);
}

u32 func0f14ed64(void)
{
	f32 foo[7];

	func0f14ef50(foo);

	return (u32)(foo[6] * 100) - 25;
}

void func0f14ee18(u32 arg0)
{
	f32 foo[7];

	arg0 += 25;

	func0f14ef50(foo);

	foo[6] = arg0 * 0.01f;

	if (foo[6] > 1.9f) {
		foo[6] = 1.9f;
	}

	if (foo[6] < 0.1f) {
		foo[6] = 0.1f;
	}

	func0f14eeb0(foo);
}

// @bug? Nothing is done with tmp
void func0f14eeb0(f32 arg0[7])
{
	struct var8007f8e0 *thing1 = func0f14a06c(-1);
	struct var8007f8e0 *thing2 = func0f14a06c(-2);
	s32 i;

	for (i = 0; i < ARRAYCOUNT(thing1->unk3d0); i++) {
		f32 tmp = arg0[i];

		if (tmp > 1.9f) {
			tmp = 1.9f;
		}

		if (tmp < 0.1f) {
			tmp = 0.1f;
		}

		if (thing1) {
			thing1->unk3d0[i] = arg0[i];
		}

		if (thing2) {
			thing2->unk3d0[i] = arg0[i];
		}
	}

	func0f14def0(-1, 4240, "camdraw.c");
}

void func0f14ef50(f32 *arg0)
{
	struct var8007f8e0 *thing;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(thing->unk3d0); i++) {
		thing = func0f14a06c(-2);

		arg0[i] = thing->unk3d0[i];
	}
}

const char var7f1b6d7c[] = "Cam : Alloc for copy of Vtx %d bytes\n";
const char var7f1b6da4[] = "CAM : Cam_AllocAndCopyAllVtx -> Ptr all-ready allocted - No extra needed\n";

void func0f14efa8(void)
{
	f32 array[] = {1, 1, 1, 1, 1, 1, 1};
	func0f14eeb0(array);
}

s32 func0f14f008(s32 index)
{
	if (index == -1) {
		index = var800a45a0->unk000;
	}

	return var8007f8dc[index].unk094;
}

void func0f14f03c(s32 index, s32 value)
{
	var8007f8dc[index].unk074 = value;
}

s32 func0f14f05c(s32 index)
{
	return var8007f8dc[index].unk074;
}

Gfx *func0f14f07c(Gfx *gdl, s32 headorbodynum, s32 x1, s32 y1, s32 x2, s32 y2)
{
	struct textureconfig *thing = &var800a45a0->unk484[headorbodynum];
	f32 sp34[2];
	u32 stack;

	sp34[0] = x1;
	sp34[1] = y1;

	if (func0f14a20c()->unk0b6[headorbodynum] != 0xff) {
		if (func0f14a20c()->unk0dc[headorbodynum] == 0) {
			gdl = func0f14c870(gdl, var800a45a0->unk488, sp34,
					(x2 - x1) * (1.0f / 64.0f),
					(y2 - y1) * (1.0f / 64.0f));
		} else {
			gdl = func0f14c870(gdl, thing, sp34,
					(x2 - x1) * (1.0f / 64.0f),
					(y2 - y1) * (1.0f / 64.0f));
		}
	}

	return gdl;
}

/**
 * Calculate the bounding box of the src coordinates and store it in dst.
 *
 * Note that each element in src is 12 bytes
 * but only the first 6 are coordinates.
 */
void func0f14f1d4(s16 *src, s32 len, s32 *dst)
{
	s32 i;

	if (len > 0) {
		dst[0] = dst[3] = src[0];
		dst[1] = dst[4] = src[1];
		dst[2] = dst[5] = src[2];

		for (i = 1; i < len; i++) {
			if (src[i * 6] > dst[3]) {
				dst[3] = src[i * 6];
			} else if (src[i * 6] < dst[0]) {
				dst[0] = src[i * 6];
			}

			if (src[i * 6 + 1] > dst[4]) {
				dst[4] = src[i * 6 + 1];
			} else if (src[i * 6 + 1] < dst[1]) {
				dst[1] = src[i * 6 + 1];
			}

			if (src[i * 6 + 2] > dst[5]) {
				dst[5] = src[i * 6 + 2];
			} else if (src[i * 6 + 2] < dst[2]) {
				dst[2] = src[i * 6 + 2];
			}
		}
	}
}

s32 func0f14f2b4(struct modeldef *modeldef, Vtx **dst, u32 *len)
{
	struct modelnode *node1 = modelGetPart(modeldef, MODELPART_HEAD_0190);
	struct modelnode *node2 = modelGetPart(modeldef, MODELPART_HEAD_0191);
	struct modelnode *node3 = modelGetPart(modeldef, MODELPART_HEAD_0192);

	s32 totalverts = 0;

	if (len != NULL) {
		*len = 0;
	}

	if (node1 && node2 && node3) {
		struct modelrodata_dl *node1rodata = &node1->rodata->dl;
		struct modelrodata_dl *node2rodata = &node2->rodata->dl;
		struct modelrodata_dl *node3rodata = &node3->rodata->dl;

		s32 node1numverts = node1rodata->numvertices;
		s32 node2numverts = node2rodata->numvertices;
		s32 node3numverts = node3rodata->numvertices;

		s32 size;

		totalverts = node1numverts + node2numverts + node3numverts;
		size = align16(totalverts * sizeof(Vtx));

		if (*dst == 0) {
			*dst = func0f149d58(size, 4429, "camdraw.c");
		}

		memcpy(*dst, node1rodata->vertices, node1numverts * sizeof(Vtx));
		memcpy(*dst + node1numverts, node2rodata->vertices, node2numverts * sizeof(Vtx));
		memcpy(*dst + node1numverts + node2numverts, node3rodata->vertices, node3numverts * sizeof(Vtx));

		if (len != NULL) {
			*len = size;
		}
	}

	return totalverts;
}

const char var7f1b6dfc[] = "Cam_RebuildHead : Slot=%d\n";
const char var7f1b6e18[] = "Cam -> Cam_ColourHeadOnePart - Remapping %s\n";
const char var7f1b6e48[] = "Cam_ColourHead : Face Col = %u, Hair Col = %u\n";
const char var7f1b6e78[] = "FACE";
const char var7f1b6e80[] = "SIDE";
const char var7f1b6e88[] = "HAIR";
const char var7f1b6e90[] = "Cam -> Cam_SquashOnePart - pExtents=%08x, pVcopy=%08x, \n";
const char var7f1b6ecc[] = "Cam -> Cam_SquashOnePart - Head centre at (%d,%d,%d)\n";
const char var7f1b6f04[] = "Cam -> Cam_SquashOnePart - Scale Param 1 = %s%s%.3f\n";
const char var7f1b6f3c[] = "";
const char var7f1b6f40[] = "";
const char var7f1b6f44[] = "Cam -> Cam_SquashOnePart - Scale Param 2 = %s%s%.3f\n";
const char var7f1b6f7c[] = "";
const char var7f1b6f80[] = "";
const char var7f1b6f84[] = "Cam -> Cam_SquashOnePart - Scale Param 3 = %s%s%.3f\n";
const char var7f1b6fbc[] = "";
const char var7f1b6fc0[] = "";
const char var7f1b6fc4[] = "Cam -> Cam_SquashOnePart - Scale Param 4 = %s%s%.3f\n";
const char var7f1b6ffc[] = "";
const char var7f1b7000[] = "";
const char var7f1b7004[] = "Cam -> Cam_SquashOnePart - Scale Param 5 = %s%s%.3f\n";
const char var7f1b703c[] = "";
const char var7f1b7040[] = "";
const char var7f1b7044[] = "Cam -> Cam_SquashOnePart - Scale Param 6 = %s%s%.3f\n";
const char var7f1b707c[] = "";
const char var7f1b7080[] = "";
const char var7f1b7084[] = "Cam -> Cam_SquashOnePart - Scale Param 7 = %s%s%.3f\n";
const char var7f1b70bc[] = "";
const char var7f1b70c0[] = "";
const char var7f1b70c4[] = "Cam_DctInitialise_Read -> %x\n";
const char var7f1b70e4[] = "Cam_DctUnCompressSlot -> %x\n";
const char var7f1b7104[] = "Cam_LoadFromPakIntoSlot -> Pak=%d, fileGuid=%u, pakGuid=%u, Slot=%d, pS=%x, pE=%x\n";


void func0f14f444(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	// empty
}

void func0f14f458(void)
{
	// empty
}

void func0f14f460(void)
{
	// empty
}

void func0f14f468(void)
{
	// empty
}

void func0f14f470(void)
{
	// empty
}

void func0f14f478(void)
{
	// empty
}

void func0f14f480(void)
{
	// empty
}

void func0f14f488(void)
{
	// empty
}

void func0f14f490(void)
{
	// empty
}

void func0f14f498(void)
{
	// empty
}

u32 func0f14f4a0(u32 arg0)
{
	return arg0;
}

void func0f14f4a8(void)
{
	// empty
}

void func0f14f4b0(u32 arg0)
{
	// empty
}

void func0f14f4b8(struct var8007f8e0 *arg0)
{
	s32 i;

	arg0->unk028 = 0;
	arg0->unk02c = 0;

	for (i = 0; i < 64 * 64; i++) {
		arg0->unk01c[i] = 0;
	}
}

void func0f14f4e4(struct var8007f8e0 *arg0)
{
	s32 i;

	arg0->unk028 = 0;
	arg0->unk02c = 0;

	for (i = 0; i < 64 * 64; i++) {
		arg0->unk024[i] = 0;
	}
}

void func0f14f510(s32 arg0)
{
	s32 i;
	s32 j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			var800a45a0->unk06c[i][j] = (i + 1 + j) * arg0 + 1;
		}
	}

	var800a45a0->unk010 = 0;
	var800a45a0->unk00c = 0;

	for (j = 0; j < 8; j++) {
		var800a45a0->unk17c[0][j] = 1 / sqrtf(8.0f);
		var800a45a0->unk27c[j][0] = var800a45a0->unk17c[0][j];
	}

	for (i = 1; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			s32 v0 = j * 2 + 1;
			f32 angle = v0 * 3.141592502594f * i / 16;

			var800a45a0->unk17c[i][j] = sqrtf(0.25f) * cosf(angle);
			var800a45a0->unk27c[j][i] = var800a45a0->unk17c[i][j];
		}
	}
}

void func0f14f700(struct var8007f8e0 *arg0, u32 arg1, u32 arg2)
{
	u32 bit;

	for (bit = 1 << (arg2 - 1); bit != 0; bit >>= 1) {
		if (bit & arg1) {
			s32 remainder = arg0->unk02c % 8;
			s32 index = arg0->unk02c / 8;
			u8 mask = 1 << (7 - remainder);

			arg0->unk024[index] |= mask;
		}

		arg0->unk02c++;
	}
}

u32 func0f14f76c(struct var8007f8e0 *arg0, u32 arg1)
{
	u32 bit;
	u32 bits;

	for (bit = 1 << (arg1 - 1), bits = 0; bit != 0; bit >>= 1) {
		u32 remainder = arg0->unk02c % 8;
		u32 index = arg0->unk02c / 8;
		u8 mask = 1 << (7 - remainder);

		if (arg0->unk020[index] & mask) {
			bits |= bit;
		}

		arg0->unk02c++;
	}

	return bits;
}

s32 phead0f14f7d4(struct var8007f8e0 *arg0)
{
	s32 sp24;
	s32 v0;

	if (var800a45a0->unk00c > 0) {
		var800a45a0->unk00c--;
		return 0;
	}

	sp24 = func0f14f76c(arg0, 2);

	if (sp24 == 0) {
		var800a45a0->unk00c = func0f14f76c(arg0, 4);
		return 0;
	}

	if (sp24 == 1) {
		sp24 = func0f14f76c(arg0, 1) + 1;
	} else {
		sp24 = func0f14f76c(arg0, 2) + sp24 * 4 - 5;
	}

	v0 = func0f14f76c(arg0, sp24);

	if ((1 << (sp24 - 1)) & v0) {
		return v0;
	}

	return (v0 - (1 << sp24)) + 1;
}

void phead0f14f8cc(struct var8007f8e0 *arg0, s32 arg1[8][8])
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(var7f1b60a0); i++) {
		s32 v0 = phead0f14f7d4(arg0);

		s32 upper = var7f1b60a0[i][0];
		s32 lower = var7f1b60a0[i][1];

		arg1[upper][lower] = var800a45a0->unk06c[upper][lower] * v0;
	}
}

void func0f14f974(struct var8007f8e0 *arg0, s32 arg1)
{
	s32 absarg1;
	s32 sp28;
	s32 sp24;

	if (arg1 == 0) {
		var800a45a0->unk010++;
		return;
	}

	if (var800a45a0->unk010 != 0) {
		while (var800a45a0->unk010 > 0) {
			func0f14f700(arg0, 0, 2);

			if (var800a45a0->unk010 < 17) {
				func0f14f700(arg0, var800a45a0->unk010 - 1, 4);
				var800a45a0->unk010 = 0;
			} else {
				func0f14f700(arg0, 15, 4);
				var800a45a0->unk010 -= 16;
			}
		}
	}

	absarg1 = ABS(arg1);
	sp28 = 1;
	sp24 = 1;

	while (sp28 < absarg1) {
		sp28 = sp28 * 2 + 1;
		sp24++;
	}

	if (sp24 < 3) {
		func0f14f700(arg0, sp24 + 1, 3);
	} else {
		func0f14f700(arg0, sp24 + 5, 4);
	}

	if (arg1 > 0) {
		func0f14f700(arg0, arg1, sp24);
	} else {
		func0f14f700(arg0, arg1 + sp28, sp24);
	}
}

void func0f14faf8(struct var8007f8e0 *arg0, s32 arg1[8][8])
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(var7f1b60a0); i++) {
		u32 upper = var7f1b60a0[i][0];
		u32 lower = var7f1b60a0[i][1];
		f32 f0 = (f32) arg1[upper][lower] / (f32) var800a45a0->unk06c[upper][lower];
		s32 a1;

		if (f0 < 0.0f) {
			a1 = f0 - 0.5f;
		} else {
			a1 = f0 + 0.5f;
		}

		func0f14f974(arg0, a1);
	}
}

void func0f14fbfc(u8 *arg0[8], s32 arg1[8][8])
{
	f32 sp30[8][8];
	s32 i;
	s32 j;
	s32 k;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			sp30[i][j] = 0.0f;

			for (k = 0; k < 8; k++) {
				sp30[i][j] += (arg0[i][k] - 0x80) * var800a45a0->unk27c[k][j];
			}
		}
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			f32 f0 = 0.0f;

			for (k = 0; k < 8; k++) {
				f0 += var800a45a0->unk17c[i][k] * sp30[k][j];
			}

			if (f0 < 0.0f) {
				arg1[i][j] = f0 - 0.5f;
			} else {
				arg1[i][j] = f0 + 0.5f;
			}
		}
	}
}

void phead0f14fdb0(s32 arg0[8][8], u8 **arg1)
{
	f32 sp30[8][8];
	s32 i;
	s32 j;
	s32 k;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			sp30[i][j] = 0.0f;

			for (k = 0; k < 8; k++) {
				sp30[i][j] += arg0[i][k] * var800a45a0->unk17c[k][j];
			}
		}
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			f32 f0 = 0.0f;

			for (k = 0; k < 8; k++) {
				f0 += var800a45a0->unk27c[i][k] * sp30[k][j];
			}

			f0 += 128.0f;

			if (f0 > 255.0f) {
				f0 = 255.0f;
			}

			if (f0 < 0.0f) {
				f0 = 0.0f;
			}

			if (f0 < 0.0f) {
				arg1[i][j] = (s8)(f0 - 0.5f);
			} else {
				arg1[i][j] = (s8)(f0 + 0.5f);
			}
		}
	}
}

void func0f14ff94(struct var8007f8e0 *arg0)
{
	s32 i;
	s32 j;
	s32 k;
	s32 tmp;
	s32 sp60[8][8];
	u8 *sp40[8];

	func0f14f4b8(arg0);
	tmp = func0f14f76c(arg0, 8);
	func0f14f510(tmp);

	for (i = 0; i < 64; i += 8) {
		for (j = 0; j < 64; j += 8) {
			phead0f14f8cc(arg0, sp60);

			for (k = 0; k < 8; k++) {
				sp40[k] = &arg0->unk01c[(i + k) * 64 + j];
			}

			phead0f14fdb0(sp60, sp40);
		}
	}
}

void func0f150068(struct var8007f8e0 *arg0, s32 arg1)
{
	s32 i;
	s32 j;
	s32 k;
	s32 sp64[8][8];
	u8 *sp44[8];

	func0f14f4e4(arg0);
	func0f14f510(arg1);
	func0f14f700(arg0, arg1, 8);

	for (i = 0; i < 0x40; i += 8) {
		for (j = 0; j < 0x40; j += 8) {
			for (k = 0; k < 8; k++) {
				sp44[k] = &arg0->unk01c[(i + k) * 0x40 + j];
			}

			func0f14fbfc(sp44, sp64);
			func0f14faf8(arg0, sp64);
		}
	}

	func0f14f974(arg0, 1);

	arg0->unk034 = arg0->unk02c / 8;
}

bool func0f15015c(s8 device, s32 filenum, u8 *arg2)
{
	u8 stack[0x420];
	u8 buffer[128];
	s32 ret;
	s32 i;

	ret = pakReadBodyAtGuid(device, filenum, buffer, 128);

	if (ret == 0) {
		for (i = 0; i < sizeof(buffer); i++) {
			arg2[i] = buffer[i];
		}

		return true;
	}

	if (ret == 10) {
		s32 i;
		s32 size = 128;

		for (i = 0; i < size; i++) {
			arg2[i] = rngRandom();
		}

		return true;
	}

	return false;
}

bool pheadLoadFile(s8 device, s32 fileid, u16 serial, s32 arg3)
{
	s32 i;
	s32 ret;
	struct camerafile file;
	u32 stack;

	struct var8007f8e0 *s0 = func0f14a06c(arg3 == -1 ? -1 : arg3);
	struct var8007f8e0 *s1 = func0f14a06c(-1);

	ret = pakReadBodyAtGuid(device, fileid, (u8 *)&file, 0);

	if (ret == 0) {
		s0->fileguid.fileid = fileid;
		s0->fileguid.deviceserial = serial;

		if (s1 != s0) {
			s0->unk01c = s1->unk01c;
		}

		s0->unk3f4_01 = (s32)file.unk8e_00;
		s0->unk3f4_02 = (s32)file.unk8e_01;
		s0->unk3a4 = file.unk8e_02;
		s0->unk3bc = file.unk82;
		s0->unk3b8 = file.unk84;
		s0->colournum = file.unk8c;
		s0->stylenum = file.unk8d;
#if PERFECTHEAD
		// Signed, so a crop reaching past the picture (Position Image) comes
		// back as saved; older files only have 0 to 64
		s0->unk3c0 = (s8) file.unk86;
		s0->unk3c4 = (s8) file.unk87;
		s0->unk3c8 = (s8) file.unk88;
		s0->unk3cc = (s8) file.unk89;
#else
		s0->unk3c0 = file.unk86;
		s0->unk3c4 = file.unk87;
		s0->unk3c8 = file.unk88;
		s0->unk3cc = file.unk89;
#endif
		s0->unk3ec = file.unk8a;
		s0->unk3f0 = file.unk8b;
		s0->unk3f4_03 = true;
		s0->unk3f4_00 = true;

		for (i = 0; i < ARRAYCOUNT(s0->unk3d0); i++) {
			s0->unk3d0[i] = file.unk90[i] / 1000.0f;
		}

		for (i = 0; i < 1024; i++) {
			s0->unk020[i] = file.unk9e[i];
		}

		if (s1 != s0) {
			func0f14c50c(s1, s0, 6494, "camdraw.c");
		}

		func0f14a00c(false);
		func0f14a16c(5);
		func0f14bc04();

		for (i = 0; i < 128; i++) {
			s1->unk010.textureptr[i] = file.unk00[i];
		}

		if (s1 != s0) {
			func0f14c50c(s0, s1, 6509, "camdraw.c");
		}

		return true;
	}

	g_FilemgrLastPakError = ret;

	return 0;
}

const char var7f1b7170[] = "Cam -> Compressing Editor Slot\n";
const char var7f1b7190[] = "Cam -> Trying image compression at quality %u\n";
const char var7f1b71c0[] = "Cam -> Failed - Too big - This=%u, Max=%d\n";
const char var7f1b71ec[] = "Cam -> Trying lower quality setting\n";
const char var7f1b7214[] = "Cam -> Save Failed - Cant get it small enough - oo-er\n";
const char var7f1b724c[] = "Cam -> Sucess at quality %u - Size=%u, Max=%d\n";
const char var7f1b727c[] = "\nCam_SaveEditSlotToParamPakItem -> Save failed\n";
const char var7f1b72ac[] = "Camera Save Error Result: %d\n";
const char var7f1b72cc[] = "Magic Guid set to %d\n";

s32 pheadSaveFile(s8 device, s32 fileid, u16 serial)
{
	u32 stack[2];
	struct camerafile file;
	struct var8007f8e0 *thing = func0f14a06c(-1);
	s32 ret;
	s32 writtenfileid;
	u8 sp44[0x1000];
	s32 i;

	if (!thing->unk3f4_03) {
		thing->unk036 = 1;
		thing->unk024 = sp44;

		while (true) {
			func0f150068(thing, thing->unk036);

			file.unk80 = thing->unk02c / 8;

			if ((u32)file.unk80 >= 1024) {
				thing->unk036++;

				if (thing->unk036 >= 12) {
					return -1;
				}
			} else {
				break;
			}
		}

		for (i = 0; i < (u32)file.unk80; i++) {
			thing->unk020[i] = thing->unk024[i];
		}

		for (i = file.unk80; i < 1024; i++) {
			thing->unk020[i] = 0;
		}

		thing->unk3f4_03 = true;
	}

	file.unk8e_00 = thing->unk3f4_01;
	file.unk8e_01 = thing->unk3f4_02;
	file.unk8e_02 = thing->unk3a4;

	file.unk82 = thing->unk3bc;
	file.unk84 = thing->unk3b8;
	file.unk8c = thing->colournum;
	file.unk8d = thing->stylenum;
	file.unk86 = thing->unk3c0;
	file.unk87 = thing->unk3c4;
	file.unk88 = thing->unk3c8;
	file.unk89 = thing->unk3cc;
	file.unk8a = thing->unk3ec;
	file.unk8b = thing->unk3f0;

	for (i = 0; i != ARRAYCOUNT(file.unk90); i++) {
		file.unk90[i] = thing->unk3d0[i] * 1000.0f;
	}

	for (i = 0; i < ARRAYCOUNT(file.unk00); i++) {
		file.unk00[i] = thing->unk010.textureptr[i];
	}

	for (i = 0; i < ARRAYCOUNT(file.unk9e); i++) {
		file.unk9e[i] = thing->unk020[i];
	}

	var80075bd0[3] = true;

	ret = pakSaveAtGuid(device, fileid, PAKFILETYPE_CAMERA, (u8 *)&file, &writtenfileid, NULL);

	if (ret == 0) {
		s32 i;
		for (i = 0; i < 18; i++) {
			struct var8007f8e0 *thing2 = func0f14a06c(i);

			if (thing2->fileguid.fileid == thing->fileguid.fileid
					&& thing2->fileguid.deviceserial == thing->fileguid.deviceserial) {
				thing2->fileguid.fileid = writtenfileid;
				thing2->fileguid.deviceserial = serial;

				if (i >= 12) {
					g_Vars.modifiedfiles |= MODFILE_GAME;
				}
			}
		}

		thing->fileguid.fileid = writtenfileid;
		thing->fileguid.deviceserial = serial;

		return 0;
	}

	g_FilemgrLastPakError = ret;

	return -1;
}

void phGetGuid(s32 index, struct fileguid *guid)
{
	struct var8007f8e0 *thing = func0f14a06c(index);

	guid->fileid = thing->fileguid.fileid;
	guid->deviceserial = thing->fileguid.deviceserial;
}

void phSetFileId(s32 fileid)
{
	struct var8007f8e0 *thing = func0f14a06c(-1);

	thing->fileguid.deviceserial = 0;
	thing->fileguid.fileid = fileid;
}
