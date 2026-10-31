#ifndef _IN_GAME_PHEAD_H
#define _IN_GAME_PHEAD_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

/**
 * PerfectHead restoration (PERFECTHEAD builds only), see docs/perfecthead.md.
 *
 * Physical camera slots (var8007f8e0):
 *   0-17  heads (only 0 to PHEAD_NUM_HEADS - 1 have buffers: the loaded heads)
 *   18    editor   (camdraw logical index -1)
 *   19    undo     (logical -3)
 *   20    temp     (logical -4)
 *   21    balance  (logical -5)
 *   22    the editor's picture while Take Picture is open, so Cancel can put
 *         it back (no logical index: camdraw's slot map has 22 entries)
 */
#define PHEAD_NUM_SLOTS     23
#define PHEAD_FIRSTWORKSLOT 18
#define PHEAD_NUM_WORKSLOTS 4
#define PHEAD_PICTURESLOT   22

// Heads that can be loaded at once. Each costs about 5 KB of the permanent
// pool (face texture, thumbnail, compressed picture).
#define PHEAD_NUM_HEADS 4

// Half width of the face square, in face texels (the face texture is 32 wide).
// The picture shows inside it; the placement guide is drawn to match.
#define PHEAD_FACEBOX_RX 15

// Hair colours (camdraw's colournum): the original four (Blonde, Black,
// Auburn, Blue Rinse), then Normal, which keeps the base head's own hair
// colour. Saves keep the value in a whole byte.
#define PHEAD_HAIRCOLOUR_NORMAL 4
#define PHEAD_NUM_HAIRCOLOURS   5

// Base heads (g_PheadTemplates) and their hand-painted hair masks
// (src/game/pheadmasks.c, made with tools/pheadmask): for each template, the
// face and two side textures, NULL where the automatic classifier is used.
// A mask is 64 rows (row 0 = the bottom), bit x = texel x is hair.
#if VERSION == VERSION_JPN_FINAL
#define PHEAD_NUM_TEMPLATES 15 // no Miyamoto
#else
#define PHEAD_NUM_TEMPLATES 16
#endif
extern const u32 *const g_PheadHairMasks[PHEAD_NUM_TEMPLATES][3];

void pheadSlotsInit(void);
void pheadStageReset(void);

void pheadAllocWorkSlots(void);
void pheadPictureBackup(void);
void pheadPictureRestore(void);
bool pheadCameraStart(s32 device);
void pheadCameraStop(void);
bool pheadCameraIsRunning(void);
u32 pheadFindCameras(void);
bool pheadCamerasPending(void);
bool pheadEditorHasImage(void);
void pheadSetEditorHasImage(bool value);
Gfx *pheadRenderEditorImage(Gfx *gdl, s32 x, s32 y, f32 scale);
Gfx *pheadRenderHeadImage(Gfx *gdl, s32 headnum, s32 x, s32 y, f32 scale);
void pheadPaintHead(struct modeldef *def, s32 slotindex, bool fresh, u8 *texpoolend);
s32 pheadGetNumTemplates(void);
bool pheadIsTemplateAvailable(s32 templatenum);
s32 pheadGetNumAvailableTemplates(void);
s32 pheadGetAvailableTemplate(s32 index);
s32 pheadGetAvailableIndex(s32 templatenum);
s32 pheadGetTemplateHeadnum(s32 templatenum);
u16 pheadGetTemplateFilenum(s32 templatenum);
void pheadGetTemplateGuide(s32 templatenum, s32 y1, s32 y2, s32 *eyey, s32 *mouthy);
void pheadGetCrop(s32 slotindex, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
bool pheadMenuModelLoaded(struct menumodel *menumodel);
s32 pheadGetPaintSlot(s32 perfectheadnum);
s32 pheadGetCurrentHead(void);
bool pheadIsHeadLoaded(s32 headnum);
s32 pheadCountLoadedHeads(void);
s32 pheadFindFreeHead(void);
void pheadNewHead(s32 headnum);
void pheadEditHead(s32 headnum);
void pheadKeepEditor(void);
void pheadBeginEdit(void);
void pheadRevertEdit(void);
void pheadPictureChanged(void);
s32 pheadLoadHead(s8 device, s32 fileid, u16 serial, s32 headnum);
bool pheadPreviewFile(s32 fileid, u16 serial);
s32 pheadSaveHead(s8 device, s32 fileid, u16 serial);
void pheadMenuFileSaved(void);
void pheadSetEditing(bool editing);

// Where a head can be used
#define PHEAD_USE_COMBAT   1
#define PHEAD_USE_MISSIONS 2

void pheadSetUsage(s32 slotindex, s32 usage); // a loaded head, -1 = editor, -2 = current head
s32 pheadGetUsage(s32 slotindex);
bool pheadIsCombatHead(s32 headnum);
bool pheadIsMissionHead(s32 headnum);
s32 pheadGetMpHead(s32 mpheadnum);
struct modeldef *pheadLoadHeadModel(s32 headnum, s32 bodynum, s32 *outheadnum);
void pheadAddMissionHeads(s32 *maleheads, s32 *femaleheads, s32 numpergender);
s32 pheadFindHeadByGuid(struct fileguid *guid);

MenuItemHandlerResult pheadMenuEntryHandler(s32 operation, struct menuitem *item, union handlerdata *data);
s32 pheadMenuPlayer(s32 device); // menu player for camdraw's messages (see pheadmenu.c)

extern struct menudialogdef g_PheadRootMenuDialog;

#ifndef PLATFORM_N64
extern s32 g_PheadMenuAutoOpen;
#endif

/**
 * camdraw.c functions used by the restored menus. Names in comments come from
 * the debug strings left in camdraw.c.
 */
void func0f14a488(void);           // Cam_StartCamFileRead: reset album browsing
void func0f14a594(s32 device);     // set camera device
s32 func0f14a5a4(void);            // get camera device
struct textureconfig *func0f14a89c(s32 index); // slot face texture (64x64 I8)
void func0f14aee0(void);           // Cam_Start: allocate capture buffers
void func0f14b150(void);           // Cam_Stop: free capture buffers
void func0f14b8ac(s32 device);     // live capture tick (one step per frame)
void func0f14c3a4(s32 device);     // album read tick (header, thumbnails)
void func0f14c4c0(s32 device);     // download the selected album picture
void func0f14ce84(void);           // start auto calibration
void func0f14f03c(s32 device, s32 value); // set selected album picture
s32 func0f14f05c(s32 device);      // get selected album picture
void func0f14a610(void);           // rebuild editor and copy it into the current head (-2)
void func0f14a760(void);           // Cam_CopyEditorToUndo
void func0f14a7a4(void);           // Cam_CopyUndoToEditor
void func0f14a91c(s32 brightness); // set editor brightness (0-255, 128 = neutral)
void func0f14a9a8(s32 templatenum); // set editor base head ("Change Head")
s32 func0f14a9f8(s32 index);       // get a slot's base head
void func0f14aa94(s32 colournum);  // set hair colour (Blonde, Black, Auburn, Blue Rinse)
s32 func0f14aae8(void);            // get hair colour
void func0f14ab0c(s32 stylenum);   // set face colour (skin tone)
s32 func0f14ab60(void);            // get face colour
void func0f14b360(s32 index);      // reset a slot
u32 func0f14ed64(void);            // get Shape Head depth (0-165)
void func0f14a7e8(s32 index);      // copy a slot into the editor
bool pheadLoadFile(s8 device, s32 fileid, u16 serial, s32 index); // Cam_LoadFromPakIntoSlot
s32 pheadSaveFile(s8 device, s32 fileid, u16 serial); // Cam_SaveEditSlotToParamPakItem
s32 pakCreateCameraFile(s8 device, s32 *outfileid);
void pheadAllocateTextures(s32 playernum, struct perfectheadtexturelist *textures); // filelist.c: pak thumbnail cache
void pheadFreeTextures(s32 playernum);
void func0f14ee18(u32 value);      // set Shape Head depth (0-165)

#endif
