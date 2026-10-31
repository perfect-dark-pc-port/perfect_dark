#include <ultra64.h>
#include "constants.h"
#include "game/camdraw.h"
#include "game/phead.h"
#include "game/menu.h"
#include "game/pak.h"
#include "bss.h"
#include "data.h"
#include "types.h"
#include "game/menugfx.h"
#include "game/lang.h"
#include "game/filelist.h"
#include "game/filemgr.h"
#include "game/game_1531a0.h"
#include "game/tex.h"
#include "game/mplayer/mplayer.h"
#include "lib/joy.h"
#include "lib/vi.h"

#if PERFECTHEAD

/**
 * Restored PerfectHead menus: loading and saving heads on the Controller Pak,
 * camera setup, taking a picture or loading one from the camera's album, and
 * the editor.
 *
 * The dialogs are rebuilt from the original text, which survives in every ROM
 * version as L_MPMENU_294 - L_MPMENU_414 and the camera banners.
 *
 * Flow:
 *   Player Setup -> PerfectHead -> Load A Saved Head / Make A New Head /
 *                                  Edit A Loaded Head
 *   Load A Saved Head -> (where to use it) -> Load PerfectHead -> Editor
 *   Make A New Head -> (where to use it) -> Camera Setup -> (search)
 *                   -> Error / Choose Camera -> PerfectHead (camera)
 *   PerfectHead (camera) -> Take Picture -> Keep Picture / Try Again -> Editor
 *                        -> Load From Camera -> Editor
 *   Editor -> Take Another Picture / Change Head / Position Picture /
 *             Color Face & Hair / Shape Head / Remove Head / Cancel Changes /
 *             Keep Changes -> Save PerfectHead -> Head Complete! (Finish /
 *             Edit Head)
 *   Editor, B after changing a loaded head -> Keep (as Keep Changes) /
 *             Revert (as Cancel Changes)
 *
 * camdraw.c sets g_MpPlayerNum to show its banners (on the player using
 * PerfectHead, see pheadMenuPlayer), so every call into it from here saves and
 * restores g_MpPlayerNum.
 */

extern struct var8007f8dc *var8007f8dc;

struct menudialogdef g_PheadCameraSetupMenuDialog;
struct menudialogdef g_PheadCameraErrorMenuDialog;
struct menudialogdef g_PheadChooseCameraMenuDialog;
struct menudialogdef g_PheadMainMenuDialog;
struct menudialogdef g_PheadTakePictureMenuDialog;
struct menudialogdef g_PheadKeepPictureMenuDialog;
struct menudialogdef g_PheadLoadFromCameraMenuDialog;
struct menudialogdef g_PheadEditorMenuDialog;
struct menudialogdef g_PheadPositionMenuDialog;
struct menudialogdef g_PheadColourMenuDialog;
struct menudialogdef g_PheadShapeMenuDialog;
struct menudialogdef g_PheadRemoveMenuDialog;
struct menudialogdef g_PheadCancelChangesMenuDialog;
struct menudialogdef g_PheadKeepRevertMenuDialog;
struct menudialogdef g_PheadCompleteMenuDialog;
struct menudialogdef g_PheadChooseHeadMenuDialog;
struct menudialogdef g_PheadRootMenuDialog;
struct menudialogdef g_PheadChooseSlotMenuDialog;
struct menudialogdef g_PheadLoadMenuDialog;
struct menudialogdef g_PheadSaveChangesMenuDialog;
struct menudialogdef g_PheadSaveNewMenuDialog;
struct menudialogdef g_PheadUsageNewMenuDialog;
struct menudialogdef g_PheadUsageLoadMenuDialog;

static void pheadMenuOpenEditor(void);
static void pheadMenuMarkDirty(void);
static void pheadMenuMarkChanged(void);
static Gfx *pheadMenuDrawGuide(Gfx *gdl, s32 x, s32 y, s32 size, s32 x1, s32 y1, s32 x2, s32 y2);

#define PHEAD_SEARCH_MIN60 10 // frames before trusting a "found" result
#define PHEAD_SEARCH_MAX60 90 // give up after 1.5 seconds...
#define PHEAD_SEARCH_WAIT60 240 // ...or 4 seconds while paks are still preparing

#define PHEAD_PREVIEW_SIZE  96 // the 64 pixel camera picture at 1.5x
#define PHEAD_MODELSPACE_HEIGHT 0x5a // the editor's space for the 3D head

// The height the menu gave the model item being rendered (menuitem.c)
extern s32 g_MenuItemRenderHeight;

/**
 * Size of the camera picture in a preview item: PHEAD_PREVIEW_SIZE when there
 * is room, smaller when the menu had to shrink the item to fit the dialog (a
 * quarter of the screen with three or four players).
 */
static s32 pheadMenuPreviewSize(void)
{
	s32 size = g_MenuItemRenderHeight - 4;

	if (size > PHEAD_PREVIEW_SIZE) {
		size = PHEAD_PREVIEW_SIZE;
	}

	if (size < 20) {
		size = 20;
	}

	return size;
}

#ifndef PLATFORM_N64
// set by the port's --phead-menu argument: open Camera Setup from the main menu
s32 g_PheadMenuAutoOpen = 0;
#endif

static s32 g_PheadSearchTime60 = -1;
static u32 g_PheadCameraMask = 0;
static s32 g_PheadDevice = -1;
static bool g_PheadLiveCapture = false;
static bool g_PheadPictureKept = false; // Keep Picture chosen: don't restore the old one

static bool pheadMenuIsCurrent(struct menudialogdef *dialogdef)
{
	return g_Menus[g_MpPlayerNum].curdialog && g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef;
}

/**
 * One player at a time
 *
 * There is one camera editor (camdraw's editor slot, capture buffers and
 * calibration), so only one player's menus can be in PerfectHead at a time.
 * g_PheadMenuOwner is that player (menu player number); the others get "in
 * use" instead. It is taken when the PerfectHead dialog opens and given back
 * when it closes, or found stale when that player's menus no longer hold the
 * dialog (their menus closed some other way, e.g. dropping out).
 */
#define PHEAD_MAXNAME 15 // struct mpchrconfig's name

static s32 g_PheadMenuOwner = -1;
static char g_PheadInUseText[PHEAD_MAXNAME + 32];

static bool pheadMenuIsOpenFor(s32 playernum)
{
	s32 i;
	s32 j;

	for (i = 0; i < g_Menus[playernum].depth; i++) {
		struct menulayer *layer = &g_Menus[playernum].layers[i];

		for (j = 0; j < layer->numsiblings; j++) {
			if (layer->siblings[j] && layer->siblings[j]->definition == &g_PheadRootMenuDialog) {
				return true;
			}
		}
	}

	return false;
}

static s32 pheadMenuGetOwner(void)
{
	if (g_PheadMenuOwner >= 0 && !pheadMenuIsOpenFor(g_PheadMenuOwner)) {
		g_PheadMenuOwner = -1;
	}

	return g_PheadMenuOwner;
}

/**
 * The menu player whose screen camdraw's messages belong on: the player
 * using PerfectHead. camdraw itself assumes the camera's controller number,
 * which is only right when that player plugged the camera into their own
 * controller.
 */
s32 pheadMenuPlayer(s32 device)
{
	s32 owner = pheadMenuGetOwner();

	return owner >= 0 ? owner : device;
}

/**
 * "PerfectHead is in use by <name>." It isn't in the game's text tables (the
 * cut feature had no such message). Menu text must be a text ID or a
 * function: on N64 menuResolveText only accepts pointers into code, so a
 * plain string pointer would show nothing.
 */
char *pheadMenuInUseText(struct menuitem *item)
{
	char *name;
	s32 len;
	s32 i;

	// Once the player has finished, keep showing the last message
	if (g_PheadMenuOwner < 0) {
		return g_PheadInUseText;
	}

	name = g_PlayerConfigsArray[g_PheadMenuOwner].base.name;
	len = sprintf(g_PheadInUseText, "PerfectHead is in use by ");

	for (i = 0; name[i] != '\0' && name[i] != '\n' && i < PHEAD_MAXNAME; i++) {
		g_PheadInUseText[len++] = name[i];
	}

	g_PheadInUseText[len++] = '.';
	g_PheadInUseText[len++] = '\n';
	g_PheadInUseText[len] = '\0';

	return g_PheadInUseText;
}

struct menuitem g_PheadInUseMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, (uintptr_t) &pheadMenuInUseText, 0, NULL },
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_300, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadInUseMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_294, // "PerfectHead"
	g_PheadInUseMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Split screen: with two or more players' menus open each dialog gets half the
 * width, and the PerfectHead dialogs' sentences (written one line per label
 * for a full-width menu) get cut off. Then each run of sentence labels is
 * shown as one paragraph, word wrapped to the width the dialog can have, in
 * its first label; the rest of the run show nothing (and take no space). A run
 * is a group of labels next to each other with the same param (Head Complete!
 * shows or hides its groups by param).
 *
 * With three or four players a dialog has only a quarter of the screen, so
 * the paragraphs also use the small font, and Load PerfectHead drops its
 * instructions to leave room for the pictures.
 *
 * The labels are shared by every player's menus, so this follows the number
 * of players in the menus. It is checked when PerfectHead opens and every
 * tick while it is open, and swaps the labels' text between their own text and
 * pheadMenuWrappedText.
 */
#define PHEAD_MAXWRAPITEMS 48
#define PHEAD_WRAPLEN      300

#define PHEAD_WRAP_OFF     0
#define PHEAD_WRAP_HALF    1 // two players
#define PHEAD_WRAP_QUARTER 2 // three or four

extern struct menuitem g_PheadLoadMenuItems[];
extern struct menuitem g_PheadSaveNewMenuItems[];
extern struct menuitem g_PheadCompleteMenuItems[];

static struct {
	struct menuitem *item;
	uintptr_t text;
	u32 flags;
} g_PheadWrapItems[PHEAD_MAXWRAPITEMS];

static s32 g_PheadNumWrapItems = -1;
static s32 g_PheadWrapMode = PHEAD_WRAP_OFF;
static char g_PheadWrapBuffers[2][PHEAD_WRAPLEN];
static s32 g_PheadWrapBufferIndex;

static s32 pheadMenuMeasure(char *text, s32 len, bool small)
{
	char line[PHEAD_WRAPLEN];
	s32 width;
	s32 height;
	s32 i;

	for (i = 0; i < len; i++) {
		line[i] = text[i];
	}

	line[len] = '\0';

	if (small) {
		textMeasure(&height, &width, line, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	} else {
		textMeasure(&height, &width, line, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
	}

	return width;
}

/**
 * Whether wrap items i - 1 and i are in the same run.
 */
static bool pheadMenuSameRun(s32 i)
{
	return i > 0
		&& g_PheadWrapItems[i].item == g_PheadWrapItems[i - 1].item + 1
		&& g_PheadWrapItems[i].item->param == g_PheadWrapItems[i - 1].item->param;
}

static bool pheadMenuIsLoadInstruction(struct menuitem *item)
{
	return item >= g_PheadLoadMenuItems && item->type == MENUITEMTYPE_LABEL
		&& item < g_PheadLoadMenuItems + 7; // the instructions and "Controller Pak:"
}

/**
 * Sentences left out in a quarter of the screen, where the dialog can't fit
 * them: Load's instructions, Save's "If you want to do this, please ensure a
 * Controller Pak is inserted and choose 'Save'." and Head Complete's "You can
 * now edit your head further if you wish." (its Edit Head button says so).
 */
static bool pheadMenuIsQuarterDropped(struct menuitem *item)
{
	return pheadMenuIsLoadInstruction(item)
		|| (item >= g_PheadSaveNewMenuItems + 2 && item <= g_PheadSaveNewMenuItems + 4)
		|| item == g_PheadCompleteMenuItems + 5;
}

char *pheadMenuWrappedText(struct menuitem *item)
{
	char *out = g_PheadWrapBuffers[g_PheadWrapBufferIndex];
	bool small = (item->flags & MENUITEMFLAG_SMALLFONT) != 0;
	s32 len = 0;
	s32 linestart = 0;
	s32 lastspace = -1;
	s32 maxwidth;
	s32 xmin, ymin, xmax, ymax;
	s32 first = -1;
	s32 i;

	for (i = 0; i < g_PheadNumWrapItems; i++) {
		if (g_PheadWrapItems[i].item == item) {
			first = i;
			break;
		}
	}

	// NULL rather than "": a label without text takes no space (menuCalculateItemSize)
	if (first < 0) {
		return NULL;
	}

	// Not the first of its run: the first label shows the whole paragraph
	if (pheadMenuSameRun(first)) {
		return NULL;
	}

	if (g_PheadWrapMode == PHEAD_WRAP_QUARTER && pheadMenuIsQuarterDropped(item)) {
		return NULL;
	}

	g_PheadWrapBufferIndex ^= 1;

#if VERSION >= VERSION_JPN_FINAL
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax, NULL);
#else
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax);
#endif

	// The dialog's width (dialogCalculatePosition), less the label's
	// padding and a margin (the dialog's border and scroll marks)
	maxwidth = xmax - xmin - 6 - 8 - 24;

	for (i = first; i < g_PheadNumWrapItems && (i == first || pheadMenuSameRun(i)); i++) {
		char *text = menuResolveText(g_PheadWrapItems[i].text, g_PheadWrapItems[i].item);
		s32 j;

		if (g_PheadWrapMode == PHEAD_WRAP_QUARTER && pheadMenuIsQuarterDropped(g_PheadWrapItems[i].item)) {
			continue;
		}

		if (text == NULL) {
			continue;
		}

		for (j = 0; text[j] != '\0' && len < PHEAD_WRAPLEN - 2; j++) {
			char c = text[j] == '\n' ? ' ' : text[j];

			// One space between words, none at the start of a line
			if (c == ' ' && (len == linestart || out[len - 1] == ' ')) {
				continue;
			}

			if (c == ' ') {
				lastspace = len;
			}

			out[len++] = c;

			// Too wide: break the line at the last space
			if (c != ' ' && lastspace > linestart && pheadMenuMeasure(&out[linestart], len - linestart, small) > maxwidth) {
				out[lastspace] = '\n';
				linestart = lastspace + 1;
			}
		}

		if (len > linestart && out[len - 1] != ' ' && len < PHEAD_WRAPLEN - 2) {
			lastspace = len;
			out[len++] = ' ';
		}
	}

	while (len > 0 && out[len - 1] == ' ') {
		len--;
	}

	// End with a line break like the game's text: textMeasure counts lines
	// by their line breaks
	out[len++] = '\n';
	out[len] = '\0';

	return out;
}

extern struct menuitem g_PheadCameraErrorMenuItems[];
extern struct menuitem g_PheadCameraSetupMenuItems[];
extern struct menuitem g_PheadCancelChangesMenuItems[];
extern struct menuitem g_PheadChooseCameraMenuItems[];
extern struct menuitem g_PheadChooseSlotMenuItems[];
extern struct menuitem g_PheadLoadFromCameraMenuItems[];
extern struct menuitem g_PheadRemoveMenuItems[];
extern struct menuitem g_PheadSaveChangesMenuItems[];
extern struct menuitem g_PheadSaveNewMenuItems[];
extern struct menuitem g_PheadUsageMenuItems[];
extern struct menuitem g_PheadCompleteMenuItems[];
extern struct menuitem g_PheadKeepRevertMenuItems[];
extern struct menuitem g_PheadMainMenuItems[];
extern struct menuitem g_PheadPositionMenuItems[];

static void pheadMenuFitLabels(void)
{
	static struct menuitem *lists[] = {
		g_PheadCameraErrorMenuItems,
		g_PheadCameraSetupMenuItems,
		g_PheadCancelChangesMenuItems,
		g_PheadChooseCameraMenuItems,
		g_PheadChooseSlotMenuItems,
		g_PheadLoadFromCameraMenuItems,
		g_PheadLoadMenuItems,
		g_PheadRemoveMenuItems,
		g_PheadSaveChangesMenuItems,
		g_PheadSaveNewMenuItems,
		g_PheadUsageMenuItems,
		g_PheadInUseMenuItems,
		g_PheadCompleteMenuItems,
		g_PheadKeepRevertMenuItems,
	};

	s32 mode = g_MpNumJoined >= 3 ? PHEAD_WRAP_QUARTER : g_MpNumJoined == 2 ? PHEAD_WRAP_HALF : PHEAD_WRAP_OFF;
	s32 i;

	// Find the sentence labels (and their own text and flags) once
	if (g_PheadNumWrapItems < 0) {
		g_PheadNumWrapItems = 0;

		for (i = 0; i < ARRAYCOUNT(lists); i++) {
			struct menuitem *item;

			for (item = lists[i]; item->type != MENUITEMTYPE_END; item++) {
				if (item->type == MENUITEMTYPE_LABEL
						&& (item->flags & MENUITEMFLAG_LESSLEFTPADDING)
						&& item->param2 != 0
						&& g_PheadNumWrapItems < PHEAD_MAXWRAPITEMS) {
					g_PheadWrapItems[g_PheadNumWrapItems].item = item;
					g_PheadWrapItems[g_PheadNumWrapItems].text = item->param2;
					g_PheadWrapItems[g_PheadNumWrapItems].flags = item->flags;
					g_PheadNumWrapItems++;
				}
			}
		}
	}

	if (mode != g_PheadWrapMode) {
		g_PheadWrapMode = mode;

		for (i = 0; i < g_PheadNumWrapItems; i++) {
			struct menuitem *item = g_PheadWrapItems[i].item;

			item->param2 = mode != PHEAD_WRAP_OFF ? (uintptr_t) &pheadMenuWrappedText : g_PheadWrapItems[i].text;
			item->flags = g_PheadWrapItems[i].flags;

			if (mode == PHEAD_WRAP_QUARTER) {
				item->flags |= MENUITEMFLAG_SMALLFONT;
			}
		}

		// The camera menu's choices are too long for half the screen: use the
		// titles of the dialogs they open ("Take Picture", "Load From Camera")
		g_PheadMainMenuItems[1].param2 = mode != PHEAD_WRAP_OFF ? L_MPMENU_333 : L_MPMENU_327;
		g_PheadMainMenuItems[2].param2 = mode != PHEAD_WRAP_OFF ? L_MPMENU_330 : L_MPMENU_328;
	}
}

/**
 * Position Picture's rows ask for no more width than the dialog has: the
 * slider's bar and value are drawn from the items' right edge, so a wider row
 * pushes them out of the dialog. Where there's room for it (a quarter of the
 * screen that's full width, with three players), Brightness and its bar share
 * a row, leaving the picture more height.
 */
static void pheadMenuFitPosition(void)
{
	s32 xmin, ymin, xmax, ymax;
	s32 avail;
	s32 width = g_PheadWrapMode == PHEAD_WRAP_QUARTER ? 0x78 : 0xc8;

#if VERSION >= VERSION_JPN_FINAL
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax, NULL);
#else
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax);
#endif

	// The dialog's width (dialogCalculatePosition)
	avail = xmax - xmin - 6;

	if (width > avail) {
		width = avail;
	}

	g_PheadPositionMenuItems[0].param2 = width;

	// A slider on one row is 150 wide, on two 120 (menuCalculateItemSize)
	if (g_PheadWrapMode == PHEAD_WRAP_QUARTER && avail >= 150) {
		g_PheadPositionMenuItems[1].flags &= ~MENUITEMFLAG_SLIDER_ALTSIZE;
	} else {
		g_PheadPositionMenuItems[1].flags |= MENUITEMFLAG_SLIDER_ALTSIZE;
	}
}

/**
 * Items only wanted when the dialog has more than a quarter of the screen.
 */
MenuItemHandlerResult pheadMenuNotInQuarterHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return g_PheadWrapMode == PHEAD_WRAP_QUARTER;
	}

	return 0;
}

/**
 * Entry point: "PerfectHead" in the Combat Simulator's Player Setup menu.
 * Hidden without the Expansion Pak, as camdraw only allocates its state then.
 */
MenuItemHandlerResult pheadMenuEntryHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return !IS8MB();
	}

	if (operation == MENUOP_SET) {
		s32 owner = pheadMenuGetOwner();

		pheadMenuFitLabels();

		if (owner >= 0 && owner != g_MpPlayerNum) {
			menuPushDialog(&g_PheadInUseMenuDialog);
		} else {
			g_PheadMenuOwner = g_MpPlayerNum;
			menuPushDialog(&g_PheadRootMenuDialog);
		}
	}

	return 0;
}

/**
 * With three or four players a dialog has a quarter of the screen, too short
 * for the picture or head at full size. The menu then shrinks the picture's
 * row but still scrolls the dialog by what didn't fit, which pushes the
 * contents up over the title. So size the picture/head rows and picture lists
 * of the dialog on show to what fits (the dialog's content height is
 * recalculated each tick). A list keeps room for at least one row.
 */
#define PHEAD_MINMODELHEIGHT 24
#define PHEAD_MAXFITITEMS    16

MenuItemHandlerResult pheadMenuPreviewHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pheadMenuModelSpaceHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pheadMenuPositionPreviewHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pheadMenuSlotListHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pheadMenuLoadListHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pheadMenuLoadModelHandler(s32 operation, struct menuitem *item, union handlerdata *data);

// Each fitted item's own height, recorded the first time it is fitted
static struct {
	struct menuitem *item;
	s32 full;
} g_PheadFitItems[PHEAD_MAXFITITEMS];

static s32 g_PheadNumFitItems = 0;

static s32 pheadMenuFullHeight(struct menuitem *item)
{
	s32 i;

	for (i = 0; i < g_PheadNumFitItems; i++) {
		if (g_PheadFitItems[i].item == item) {
			return g_PheadFitItems[i].full;
		}
	}

	if (g_PheadNumFitItems < PHEAD_MAXFITITEMS) {
		g_PheadFitItems[g_PheadNumFitItems].item = item;
		g_PheadFitItems[g_PheadNumFitItems].full = item->param3;
		g_PheadNumFitItems++;
	}

	return item->param3;
}

static void pheadMenuFitModels(void)
{
	struct menudialog *dialog = g_Menus[g_MpPlayerNum].curdialog;
	struct menuitem *item;
	s32 xmin, ymin, xmax, ymax;
	s32 avail;

	if (dialog == NULL) {
		return;
	}

#if VERSION >= VERSION_JPN_FINAL
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax, dialog);
#else
	menuFindAvailableSize(&xmin, &ymin, &xmax, &ymax);
#endif

	// The dialog's height (dialogCalculatePosition)
	avail = ymax - ymin - 6;

	for (item = dialog->definition->items; item->type != MENUITEMTYPE_END; item++) {
		s32 min = PHEAD_MINMODELHEIGHT;
		s32 fitmax = 0x7fff;
		s32 full;
		s32 height;

		if (item->type == MENUITEMTYPE_MODEL
				&& (item->handler == pheadMenuModelSpaceHandler
					|| item->handler == pheadMenuLoadModelHandler
					|| item->handler == pheadMenuPreviewHandler
					|| item->handler == pheadMenuPositionPreviewHandler)) {
			// fits as it is
		} else if (item->type == MENUITEMTYPE_LIST
				&& (item->handler == pheadMenuSlotListHandler || item->handler == pheadMenuLoadListHandler)) {
			union handlerdata data;

			// One row and the pak's name
			data.list.value = 0;
			item->handler(MENUOP_GETOPTIONHEIGHT, item, &data);

			if (data.list.value + LINEHEIGHT + 2 > min) {
				min = data.list.value + LINEHEIGHT + 2;
			}

			// In a quarter, show the pak's name and one row, and scroll the
			// rest. The list keeps the chosen row half its height down,
			// rounded down to whole rows, and the pak's name is a row tall
			// (the custom row height), so under two rows high the name and
			// row sit at the top, and any taller leaves a gap above them.
			// Load PerfectHead's list is two rows high instead, so the chosen
			// picture sits below the pak's name once the list has scrolled
			// to a later file, rather than under it.
			if (g_PheadWrapMode == PHEAD_WRAP_QUARTER) {
				s32 need = data.list.value * 2 - (item->handler == pheadMenuLoadListHandler ? 0 : 1);

				fitmax = need > min ? need : min;
			}
		} else {
			continue;
		}

		full = pheadMenuFullHeight(item);

		if (full > fitmax) {
			full = fitmax;
		}

		// The rest of the dialog takes what it took before
		height = avail - (dialog->contentheight - item->param3);

		if (height > full) {
			height = full;
		}

		if (height < min) {
			height = min;
		}

		item->param3 = height;
	}
}

MenuDialogHandlerResult pheadMenuRootDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_CLOSE && g_PheadMenuOwner == g_MpPlayerNum) {
		g_PheadMenuOwner = -1;
	}

	if (operation == MENUOP_TICK) {
		pheadMenuFitLabels();
		pheadMenuFitModels();
	}

	return 0;
}

/**
 * PerfectHead (root): Load A Saved Head / Make A New Head /
 * Edit A Loaded Head. Up to PHEAD_NUM_HEADS heads are kept in memory.
 */
#define PHEAD_CHOOSE_EDIT 0
#define PHEAD_CHOOSE_NEW  1
#define PHEAD_CHOOSE_LOAD 2

static s32 g_PheadChooseMode = PHEAD_CHOOSE_EDIT;
static s32 g_PheadLoadHead = 0; // loaded head slot that Load PerfectHead loads into
static s32 g_PheadLoadListNum = -1;
static s32 g_PheadLoadFocus = -1;      // list row with the highlight
static s32 g_PheadLoadLastFocus = -1;
static s32 g_PheadLoadFocusTime60 = 0; // how long the highlight has stayed on it
static struct fileguid g_PheadLoadShown; // file shown in the head space
static bool g_PheadLoadShowing = false;
static s32 g_PheadNewHead = 0;  // loaded head slot that Make A New Head makes
static bool g_PheadMakingNew = false; // the editor holds a new head, not one already loaded
static s32 g_PheadUsage = PHEAD_USE_COMBAT;

// Both ask where the head will be used first
static void pheadMenuStartNewHead(s32 headnum)
{
	g_PheadNewHead = headnum;
	g_PheadMakingNew = true;
	menuPushDialog(&g_PheadUsageNewMenuDialog);
}

static void pheadMenuStartLoad(s32 headnum)
{
	g_PheadLoadHead = headnum;
	g_PheadMakingNew = false;
	menuPushDialog(&g_PheadUsageLoadMenuDialog);
}

MenuItemHandlerResult pheadMenuRootLoadHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 headnum = pheadFindFreeHead();

		if (headnum >= 0) {
			pheadMenuStartLoad(headnum);
		} else {
			// All slots are in use: choose one to replace
			g_PheadChooseMode = PHEAD_CHOOSE_LOAD;
			menuPushDialog(&g_PheadChooseSlotMenuDialog);
		}
	}

	return 0;
}

MenuItemHandlerResult pheadMenuRootNewHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 headnum = pheadFindFreeHead();

		if (headnum >= 0) {
			pheadMenuStartNewHead(headnum);
		} else {
			g_PheadChooseMode = PHEAD_CHOOSE_NEW;
			menuPushDialog(&g_PheadChooseSlotMenuDialog);
		}
	}

	return 0;
}

MenuItemHandlerResult pheadMenuRootEditHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return pheadCountLoadedHeads() == 0;
	}

	if (operation == MENUOP_SET) {
		g_PheadChooseMode = PHEAD_CHOOSE_EDIT;
		menuPushDialog(&g_PheadChooseSlotMenuDialog);
	}

	return 0;
}

struct menuitem g_PheadRootMenuItems[] = {
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_295, 0, pheadMenuRootLoadHandler }, // "Load A Saved Head"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_296, 0, pheadMenuRootNewHandler }, // "Make A New Head"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_297, 0, pheadMenuRootEditHandler }, // "Edit A Loaded Head"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_300, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadRootMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_294, // "PerfectHead"
	g_PheadRootMenuItems,
	pheadMenuRootDialog,
	0,
	NULL,
};

/**
 * "Where do you want to be able to use this head?", asked by
 * Make A New Head and Load A Saved Head before they go on. The item's param
 * is the usage (PHEAD_USE_*). A loaded head gets the answer in place of the
 * one saved in its file.
 */
MenuItemHandlerResult pheadMenuUsageHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		bool isnew = pheadMenuIsCurrent(&g_PheadUsageNewMenuDialog);

		g_PheadUsage = item->param;

		menuPopDialog();

		if (isnew) {
			pheadNewHead(g_PheadNewHead);
			pheadSetUsage(-1, g_PheadUsage);
			menuPushDialog(&g_PheadCameraSetupMenuDialog);
		} else {
			menuPushDialog(&g_PheadLoadMenuDialog);
		}
	}

	return 0;
}

struct menuitem g_PheadUsageMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_303, 0, NULL }, // "Where do you want to be able"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_304, 0, NULL }, // "to use this head?"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, PHEAD_USE_COMBAT, 0, L_MPMENU_305, 0, pheadMenuUsageHandler }, // "Combat Simulator"
	{ MENUITEMTYPE_SELECTABLE, PHEAD_USE_MISSIONS, 0, L_MPMENU_306, 0, pheadMenuUsageHandler }, // "All Missions"
	{ MENUITEMTYPE_SELECTABLE, PHEAD_USE_COMBAT | PHEAD_USE_MISSIONS, 0, L_MPMENU_307, 0, pheadMenuUsageHandler }, // "Both"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_308, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadUsageNewMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_301, // "Make A New Head"
	g_PheadUsageMenuItems,
	NULL,
	0,
	NULL,
};

struct menudialogdef g_PheadUsageLoadMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_302, // "Load A Saved Head"
	g_PheadUsageMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Edit A PerfectHead: "Choose slot to edit:" with a picture of each loaded
 * head. Also used to pick a slot to replace when all of them are in use.
 */
#define PHEAD_SLOT_SCALE  0.375f
#define PHEAD_SLOT_HEIGHT 28 // 64 * PHEAD_SLOT_SCALE + 4

MenuItemHandlerResult pheadMenuSlotListHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = PHEAD_NUM_HEADS;
		break;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = PHEAD_SLOT_HEIGHT;
		break;
	case MENUOP_RENDER:
		{
			Gfx *gdl = data->type19.gdl;
			struct menuitemrenderdata *renderdata = data->type19.renderdata2;
			s32 headnum = data->list.unk04;
			s32 x = renderdata->x + 4;
			s32 y = renderdata->y + 2;
			s32 tx = renderdata->x + 34;
			s32 ty = renderdata->y + 9;

			// A loaded head shows its picture, an empty slot "<none>"
			if (pheadIsHeadLoaded(headnum)) {
				gdl = pheadRenderHeadImage(gdl, headnum, x, y, PHEAD_SLOT_SCALE);
			} else {
				gdl = text0f153628(gdl);
				gdl = textRenderProjected(gdl, &tx, &ty, langGet(L_MPMENU_350), g_CharsHandelGothicSm, g_FontHandelGothicSm,
						renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);
				gdl = text0f153780(gdl);
			}

			return (uintptr_t) gdl;
		}
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_SET:
		{
			s32 headnum = data->list.value;

			if (g_PheadChooseMode == PHEAD_CHOOSE_EDIT) {
				if (pheadIsHeadLoaded(headnum)) {
					menuPopDialog();
					g_PheadMakingNew = false;
					pheadEditHead(headnum);
					pheadMenuOpenEditor();
				}
			} else if (g_PheadChooseMode == PHEAD_CHOOSE_NEW) {
				menuPopDialog();
				pheadMenuStartNewHead(headnum);
			} else {
				menuPopDialog();
				pheadMenuStartLoad(headnum);
			}
		}
		break;
	}

	return 0;
}

struct menuitem g_PheadChooseSlotMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_299, 0, NULL }, // "Choose slot to edit:"
	{ MENUITEMTYPE_LIST, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_SLOT_HEIGHT * PHEAD_NUM_HEADS, pheadMenuSlotListHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_300, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadChooseSlotMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_298, // "Edit A PerfectHead"
	g_PheadChooseSlotMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Load PerfectHead: the PerfectHead files on all inserted Controller Paks,
 * shown with the thumbnails stored in the files (filemgr's thumbnail cache).
 *
 * Above the list, the file with the highlight is shown as its head: once the
 * highlight has stayed on a file for PHEAD_LOADPREVIEW_WAIT60 frames (so
 * scrolling past files doesn't read each one), the file is read into the
 * editor slot and the menu model shows the editor slot, as in the editor.
 * The head space replaces the instructions and "Controller Pak:" (the list
 * names each pak), which are only shown while there are no files to choose
 * from. The list shows one picture at a time (see PHEAD_LOADLIST_HEIGHT), so
 * the head has room. A quarter of the screen has no room for the head: there
 * the list is shown on its own, and no file is read until one is chosen.
 */
#define PHEAD_LOADPREVIEW_WAIT60 15

static void pheadMenuConfigureModel(void);
static void pheadMenuTickModel(void);

/**
 * Load PerfectHead's list: each file's picture. The file only carries a 16x16
 * thumbnail (filelist's pheadGetTexture reads it from the pak), which
 * filemgr draws at its own size; draw it 2x larger (its own size in a
 * quarter of the screen), point sampled like the Game Boy Camera's own
 * pictures.
 *
 * The list shows the chosen picture under the pak's name, and the head above
 * shows the chosen file. Each picture shows its place in the list ("1/2") so
 * it is clear there are more, and the chosen one is
 * framed in the menu's highlight colour with the others dimmed (a picture
 * can't change colour the way a line of text does).
 */
#define PHEAD_LOADTHUMB_FULL 32

// The file's own size in a quarter of the screen (three or four players)
#define PHEAD_LOADTHUMB_SIZE (g_MpNumJoined >= 3 ? 16 : PHEAD_LOADTHUMB_FULL)
#define PHEAD_LOADTHUMB_ROW  (PHEAD_LOADTHUMB_SIZE + 6)

// Load PerfectHead's head space and list, sized to fit the full screen
// together (the list is two rows high: the pak's name and the chosen file
// sit one above the other, the chosen file a row down even once the list has
// scrolled to a later file). In a quarter both are fitted as usual.
#define PHEAD_LOADMODEL_HEIGHT 0x48
#define PHEAD_LOADLIST_HEIGHT  ((PHEAD_LOADTHUMB_FULL + 6) * 2)

struct textureconfig *pheadGetTexture(s32 playernum, s32 fileid, u16 deviceserial);

static Gfx *pheadMenuRenderFileThumbnail(Gfx *gdl, struct menuitemrenderdata *renderdata, s32 fileid, s32 deviceserial,
		s32 index, s32 count)
{
	struct textureconfig *texture = pheadGetTexture(g_MpPlayerNum, fileid, deviceserial & 0xffff);
	s32 x = renderdata->x + (renderdata->width - PHEAD_LOADTHUMB_SIZE) / 2;
	s32 y = renderdata->y + 3;
	u8 shade = renderdata->unk10 ? 0xff : 0x80;

	if (count > 1) {
		char text[24];
		s32 tx = x + PHEAD_LOADTHUMB_SIZE + 6;
		s32 ty = y + (PHEAD_LOADTHUMB_SIZE - 8) / 2;

		sprintf(text, "%d/%d", index + 1, count);

		gdl = text0f153628(gdl);
		gdl = textRenderProjected(gdl, &tx, &ty, text, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);
		gdl = text0f153780(gdl);
	}

	if (texture == NULL) {
		return gdl;
	}

	if (renderdata->unk10) {
		gdl = menugfxDrawCheckbox(gdl, x - 2, y - 2, PHEAD_LOADTHUMB_SIZE + 3, false, renderdata->colour, 0);
	}

	gSPDisplayList(gdl++, &var800613a0);
	gSPDisplayList(gdl++, &var80061360);

	gDPPipeSync(gdl++);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);

	texSelect(&gdl, texture, 1, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetEnvColor(gdl++, shade, shade, shade, renderdata->colour);

	gDPSetCombineLERP(gdl++,
			TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
			TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);

	gDPLoadSync(gdl++);
	gDPTileSync(gdl++);

	// The thumbnail is stored bottom up, hence starting at t = 16 and stepping
	// back. Steps are s5.10 texels per pixel: 16 texels over the size.
	gSPTextureRectangle(gdl++,
			(x << 2) * g_ScaleX,
			y << 2,
			((x + PHEAD_LOADTHUMB_SIZE) << 2) * g_ScaleX,
			(y + PHEAD_LOADTHUMB_SIZE) << 2,
			G_TX_RENDERTILE, 0, 16 << 5,
			(16 << 10) / (PHEAD_LOADTHUMB_SIZE * g_ScaleX), -(16 << 10) / PHEAD_LOADTHUMB_SIZE);

	gDPLoadSync(gdl++);
	gDPTileSync(gdl++);
	gDPPipeSync(gdl++);

	return gdl;
}

MenuItemHandlerResult pheadMenuLoadListHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct filelist *list;

	if (g_PheadLoadListNum < 0 || g_FileLists[g_PheadLoadListNum] == NULL) {
		return 0;
	}

	list = g_FileLists[g_PheadLoadListNum];

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = list->numfiles;
		break;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = PHEAD_LOADTHUMB_ROW;
		break;
	case MENUOP_RENDER:
		{
			Gfx *gdl = data->type19.gdl;
			struct menuitemrenderdata *renderdata = data->type19.renderdata2;
			struct filelistfile *file = &list->files[data->list.unk04];

			if (renderdata->unk10) {
				g_PheadLoadFocus = data->list.unk04;
			}

			gdl = pheadMenuRenderFileThumbnail(gdl, renderdata, file->fileid, file->deviceserial,
					data->list.unk04, list->numfiles);

			return (uintptr_t) gdl;
		}
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_25:
		// The paks are read after the dialog opens, so the menu set its
		// scroll target for an empty list; once the pak's name is added
		// above the files, that leaves the chosen picture a line low. Have
		// the menu work the target out again every tick (any value other
		// than the index here makes menuitemListTick do so).
		data->list.unk0c = data->list.value + 1;
		break;
	case MENUOP_SET:
		{
			struct filelistfile *file = &list->files[data->list.value];
			struct fileguid guid;

			guid.fileid = file->fileid;
			guid.deviceserial = file->deviceserial;

			menuPopDialog();

			// The head slot to load into goes in filemgr's mpplayernum
			if (filemgrSaveOrLoad(&guid, FILEOP_103, g_PheadLoadHead)) {
				pheadSetUsage(-1, g_PheadUsage);
				pheadSetUsage(g_PheadLoadHead, g_PheadUsage);
				pheadMenuOpenEditor();
			}
		}
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = list->numdevices;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return filemgrGetDeviceNameOrStartIndex(g_PheadLoadListNum, operation, data->list.value);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = filemgrGetDeviceNameOrStartIndex(g_PheadLoadListNum, operation, data->list.value);
		return 0;
	}

	return 0;
}

static bool pheadMenuLoadHasFiles(void)
{
	return g_PheadLoadListNum >= 0 && g_FileLists[g_PheadLoadListNum]
		&& g_FileLists[g_PheadLoadListNum]->numfiles > 0;
}

/**
 * Show the file with the highlight once it has stayed there a moment.
 */
static void pheadMenuTickLoadPreview(void)
{
	struct filelist *list;
	struct filelistfile *file;

	// No head in a quarter of the screen (no room), nor without files (a
	// pak taken out): don't leave the last one showing
	if (!pheadMenuLoadHasFiles() || g_PheadWrapMode == PHEAD_WRAP_QUARTER) {
		if (g_PheadLoadShowing) {
			menuUnsetModel(&g_Menus[g_MpPlayerNum].menumodel);
			g_PheadLoadShowing = false;
		}

		g_PheadLoadShown.fileid = 0;
		g_PheadLoadShown.deviceserial = 0;
		return;
	}

	list = g_FileLists[g_PheadLoadListNum];

	if (g_PheadLoadFocus != g_PheadLoadLastFocus) {
		g_PheadLoadLastFocus = g_PheadLoadFocus;
		g_PheadLoadFocusTime60 = 0;
	}

	if (g_PheadLoadFocus >= 0 && g_PheadLoadFocus < list->numfiles) {
		file = &list->files[g_PheadLoadFocus];

		if (file->fileid != g_PheadLoadShown.fileid || file->deviceserial != g_PheadLoadShown.deviceserial) {
			g_PheadLoadFocusTime60 += g_Vars.diffframe60;

			if (g_PheadLoadFocusTime60 >= PHEAD_LOADPREVIEW_WAIT60) {
				struct menumodel *menumodel = &g_Menus[g_MpPlayerNum].menumodel;
				s32 oldtemplate = g_PheadLoadShowing ? func0f14a9f8(-1) : -1;

				// Tried once per file, even if the read fails
				g_PheadLoadShown.fileid = file->fileid;
				g_PheadLoadShown.deviceserial = file->deviceserial;
				g_PheadLoadFocusTime60 = 0;

				g_PheadLoadShowing = pheadPreviewFile(file->fileid, file->deviceserial);

				if (!g_PheadLoadShowing) {
					// Don't leave the last file's head showing for this one
					menuUnsetModel(menumodel);
				} else if (func0f14a9f8(-1) != oldtemplate) {
					// Let the RDP finish with the old head before its memory is reused
					menumodel->loaddelay = 8;
				}

				pheadMenuMarkDirty();
			}
		}
	}

	if (g_PheadLoadShowing) {
		pheadMenuTickModel();
	}
}

MenuDialogHandlerResult pheadMenuLoadDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_PheadLoadListNum = filelistFindOrCreate(FILETYPE_CAMERA);
		pheadAllocateTextures(g_MpPlayerNum, NULL);

		// The head space shows the editor slot, which holds the file shown
		g_PheadLoadFocus = -1;
		g_PheadLoadLastFocus = -1;
		g_PheadLoadFocusTime60 = 0;
		g_PheadLoadShown.fileid = 0;
		g_PheadLoadShown.deviceserial = 0;
		g_PheadLoadShowing = false;
		pheadSetEditing(true);
		menuUnsetModel(&g_Menus[g_MpPlayerNum].menumodel);
		pheadMenuConfigureModel();
		break;
	case MENUOP_CLOSE:
		pheadFreeTextures(g_MpPlayerNum);
		pheadSetEditing(false);
		break;
	case MENUOP_TICK:
		if (pheadMenuIsCurrent(dialogdef)) {
			pheadMenuTickLoadPreview();
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuLoadEmptyHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	// "<none>" when no PerfectHead files were found
	if (operation == MENUOP_CHECKHIDDEN) {
		return pheadMenuLoadHasFiles();
	}

	return 0;
}

MenuItemHandlerResult pheadMenuLoadInstructionHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	// The instructions, while there's nothing to choose from
	if (operation == MENUOP_CHECKHIDDEN) {
		return pheadMenuLoadHasFiles();
	}

	return 0;
}

MenuItemHandlerResult pheadMenuLoadModelHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	// The head space, while there are files and room for it
	if (operation == MENUOP_CHECKHIDDEN) {
		return !pheadMenuLoadHasFiles() || g_PheadWrapMode == PHEAD_WRAP_QUARTER;
	}

	return pheadMenuModelSpaceHandler(operation, item, data);
}

struct menuitem g_PheadLoadMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_LOADMODEL_HEIGHT, pheadMenuLoadModelHandler },
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_339, 0, pheadMenuLoadInstructionHandler }, // "Insert the Controller Pak"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_340, 0, pheadMenuLoadInstructionHandler }, // "containing the head you"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_341, 0, pheadMenuLoadInstructionHandler }, // "wish to load into any"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_342, 0, pheadMenuLoadInstructionHandler }, // "controller."
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, pheadMenuLoadInstructionHandler },
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_343, 0, pheadMenuLoadInstructionHandler }, // "Controller Pak:"
	{ MENUITEMTYPE_LIST, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_LOADLIST_HEIGHT, pheadMenuLoadListHandler }, // the pak's name and a picture
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_350, 0, pheadMenuLoadEmptyHandler }, // "<none>"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_344, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadLoadMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_338, // "Load PerfectHead"
	g_PheadLoadMenuItems,
	pheadMenuLoadDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

static void pheadMenuOpenMain(s32 device)
{
	g_PheadDevice = device;

	// Replace the setup dialog so backing out of PerfectHead leaves the flow
	func0f0f3704(&g_PheadMainMenuDialog);
}

/**
 * The camera is still finding its exposure (camdraw's auto calibration, with
 * the "Calibrating Camera" banner), or hasn't yet delivered a picture taken
 * with the exposure it found: the picture on show is the last calibration
 * capture until the next ones finish. Pictures until then are over or under
 * exposed and look zoomed in, so they aren't shown or taken.
 *
 * g_PheadFreshCaptures counts the captures finished since calibration ended
 * (Take Picture's tick); the first is already taken with the new exposure.
 */
#define PHEAD_SETTLECAPTURES 1

static s32 g_PheadFreshCaptures = 0;

static bool pheadMenuIsCalibrating(void)
{
	return pheadCameraIsRunning()
		&& (func0f14a8e8() || g_PheadFreshCaptures < PHEAD_SETTLECAPTURES);
}

/**
 * Preview item: draws the editor slot (live camera image or kept picture).
 * The item's param says which dialog it is in: 1 = PerfectHead (camera),
 * 2 = Take Picture, 0 = Keep Picture.
 */
MenuItemHandlerResult pheadMenuPreviewHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		// The main dialog only shows a picture once one has been kept
		if (item->param == 1 && !pheadEditorHasImage()) {
			return true;
		}
	}

	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		s32 size = pheadMenuPreviewSize();
		s32 x = renderdata->x + (renderdata->width - size) / 2;

		// Take Picture: nothing until the camera has calibrated
		if (item->param == 2 && pheadMenuIsCalibrating()) {
			return (uintptr_t) gdl;
		}

		gdl = pheadRenderEditorImage(gdl, x, renderdata->y + 2, size / 64.0f);

		// Take Picture: show where the face should be
		if (item->param == 2) {
			s32 x1, y1, x2, y2;

			pheadGetCrop(-1, &x1, &y1, &x2, &y2);
			gdl = pheadMenuDrawGuide(gdl, x, renderdata->y + 2, size, x1, y1, x2, y2);
		}

		return (uintptr_t) gdl;
	}

	return 0;
}

/**
 * Camera Setup
 */
MenuItemHandlerResult pheadMenuCameraSetupOkHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_PheadSearchTime60 < 0) {
			g_PheadSearchTime60 = 0;
			g_PheadCameraMask = 0;
			menuSetBanner(MENUBANNER_SEARCHINGFORCAMERA, false);
		}
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuCameraSetupDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		// Let Game Boy paks finish preparing (PAKSTATE_GB_PRE_PREPARE)
		func0f14a52c();
		g_PheadSearchTime60 = -1;
		break;
	case MENUOP_CLOSE:
		if (g_PheadSearchTime60 >= 0) {
			menuSetBanner(-1, false);
			g_PheadSearchTime60 = -1;
		}
		break;
	case MENUOP_TICK:
		if (g_PheadSearchTime60 >= 0 && pheadMenuIsCurrent(dialogdef)) {
			s32 playernum = g_MpPlayerNum;
			u32 mask = pheadFindCameras();

			g_MpPlayerNum = playernum;
			g_PheadSearchTime60 += g_Vars.diffframe60;

			if (g_PheadSearchTime60 >= PHEAD_SEARCH_MIN60) {
				g_PheadCameraMask |= mask;
			}

			if ((g_PheadCameraMask && g_PheadSearchTime60 >= PHEAD_SEARCH_MIN60 * 3)
					|| (g_PheadSearchTime60 >= PHEAD_SEARCH_MAX60 && !pheadCamerasPending())
					|| g_PheadSearchTime60 >= PHEAD_SEARCH_WAIT60) {
				g_PheadSearchTime60 = -1;
				menuSetBanner(-1, false);

				if (g_PheadCameraMask == 0) {
					menuPushDialog(&g_PheadCameraErrorMenuDialog);
				} else if ((g_PheadCameraMask & (g_PheadCameraMask - 1)) == 0) {
					s32 i;

					for (i = 0; i < 4; i++) {
						if (g_PheadCameraMask & (1 << i)) {
							pheadMenuOpenMain(i);
							break;
						}
					}
				} else {
					menuPushDialog(&g_PheadChooseCameraMenuDialog);
				}
			}
		}
		break;
	}

	return 0;
}

struct menuitem g_PheadCameraSetupMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_310, 0, NULL }, // "Insert your camera into your"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_311, 0, NULL }, // "Game Boy Pak."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_312, 0, NULL }, // "Then, insert your Game Boy Pak"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_313, 0, NULL }, // "into any controller."
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_314, 0, pheadMenuCameraSetupOkHandler }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_315, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadCameraSetupMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_309, // "Camera Setup"
	g_PheadCameraSetupMenuItems,
	pheadMenuCameraSetupDialog,
	0,
	NULL,
};

/**
 * Error
 */
struct menuitem g_PheadCameraErrorMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_317, 0, NULL }, // "Game Boy Camera Not Detected!"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_318, 0, NULL }, // "Please check all connections"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_319, 0, NULL }, // "and try again."
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_320, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadCameraErrorMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_316, // "Error"
	g_PheadCameraErrorMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Choose Camera (more than one detected)
 */
MenuItemHandlerResult pheadMenuChooseCameraHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return (g_PheadCameraMask & (1 << item->param)) == 0;
	}

	if (operation == MENUOP_SET) {
		// Close Choose Camera, then replace Camera Setup with PerfectHead
		menuPopDialog();
		pheadMenuOpenMain(item->param);
	}

	return 0;
}

struct menuitem g_PheadChooseCameraMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_322, 0, NULL }, // "More than one camera detected."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_323, 0, NULL }, // "Please select which camera you want to use"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_346, 0, pheadMenuChooseCameraHandler }, // "Pak 1"
	{ MENUITEMTYPE_SELECTABLE, 1, 0, L_MPMENU_347, 0, pheadMenuChooseCameraHandler }, // "Pak 2"
	{ MENUITEMTYPE_SELECTABLE, 2, 0, L_MPMENU_348, 0, pheadMenuChooseCameraHandler }, // "Pak 3"
	{ MENUITEMTYPE_SELECTABLE, 3, 0, L_MPMENU_349, 0, pheadMenuChooseCameraHandler }, // "Pak 4"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_325, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadChooseCameraMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_321, // "Choose Camera"
	g_PheadChooseCameraMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * PerfectHead
 */
MenuDialogHandlerResult pheadMenuMainDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		pheadSetEditorHasImage(false);
		pheadCameraStart(g_PheadDevice);
		break;
	case MENUOP_CLOSE:
		{
			s32 playernum = g_MpPlayerNum;

			pheadCameraStop();
			func0f14a560();
			menuSetBanner(-1, false);

			g_MpPlayerNum = playernum;
		}
		break;
	}

	return 0;
}

struct menuitem g_PheadMainMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 1, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_PREVIEW_SIZE + 4, pheadMenuPreviewHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_327, 0, (void *)&g_PheadTakePictureMenuDialog }, // "Take A Picture Now"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_328, 0, (void *)&g_PheadLoadFromCameraMenuDialog }, // "Load Picture From Camera"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_329, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadMainMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_326, // "PerfectHead"
	g_PheadMainMenuItems,
	pheadMenuMainDialog,
	0,
	NULL,
};

/**
 * Take Picture: live preview from the camera, one capture step per frame.
 */
MenuItemHandlerResult pheadMenuShootHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	// No half-calibrated pictures. Cancel stays available in case calibration
	// never finishes.
	if (operation == MENUOP_CHECKDISABLED) {
		return pheadMenuIsCalibrating();
	}

	if (operation == MENUOP_SET && !pheadMenuIsCalibrating()) {
		// Freeze the last captured frame (the editor slot) and ask to keep it
		g_PheadLiveCapture = false;
		menuPushDialog(&g_PheadKeepPictureMenuDialog);
	}

	return 0;
}

extern struct menuitem g_PheadTakePictureMenuItems[];

MenuDialogHandlerResult pheadMenuTakePictureDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	s32 playernum = g_MpPlayerNum;

	switch (operation) {
	case MENUOP_OPEN:
		// The live preview overwrites the editor's picture: keep it for Cancel
		pheadPictureBackup();
		g_PheadPictureKept = false;

		func0f14ce84(); // start auto calibration
		g_PheadLiveCapture = true;
		g_PheadFreshCaptures = 0;
		break;
	case MENUOP_CLOSE:
		g_PheadLiveCapture = false;
		menuSetBanner(-1, false);

		if (!g_PheadPictureKept) {
			pheadPictureRestore();
			pheadMenuMarkDirty();
		}
		break;
	case MENUOP_TICK:
		if (pheadMenuIsCurrent(dialogdef)) {
			g_PheadLiveCapture = true;
		}

		if (g_PheadLiveCapture && pheadCameraIsRunning()) {
			// A capture has finished when the pak reaches PAK008_11
			// (func0f14b8ac then takes it)
			if (func0f14a8e8()) {
				g_PheadFreshCaptures = 0;
			} else if (g_PheadFreshCaptures < PHEAD_SETTLECAPTURES) {
				if (pakGetUnk008(g_PheadDevice) == PAK008_11) {
					g_PheadFreshCaptures++;

					// While Shoot! was disabled the menu moved the cursor to
					// Cancel: put it back now Shoot! can be used
					if (g_PheadFreshCaptures >= PHEAD_SETTLECAPTURES && pheadMenuIsCurrent(dialogdef)) {
						g_Menus[g_MpPlayerNum].curdialog->focuseditem = &g_PheadTakePictureMenuItems[1];
					}
				}

				// camdraw takes its banner down when calibration ends: keep
				// it up until the picture is ready
				menuSetBanner(g_PheadFreshCaptures < PHEAD_SETTLECAPTURES ? MENUBANNER_CALIBRATINGCAMERA : -1, false);
			}

			func0f14b8ac(g_PheadDevice);
		}
		break;
	}

	g_MpPlayerNum = playernum;

	return 0;
}

struct menuitem g_PheadTakePictureMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 2, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_PREVIEW_SIZE + 4, pheadMenuPreviewHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_334, 0, pheadMenuShootHandler }, // "Shoot!"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_335, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadTakePictureMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_333, // "Take Picture"
	g_PheadTakePictureMenuItems,
	pheadMenuTakePictureDialog,
	0,
	NULL,
};

/**
 * Keep Picture / Try Again
 */
MenuItemHandlerResult pheadMenuKeepPictureHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		// The picture is in the editor slot: continue in the editor
		g_PheadPictureKept = true;
		pheadSetEditorHasImage(true);
		pheadPictureChanged();
		menuPopDialog(); // Keep Picture
		menuPopDialog(); // Take Picture
		pheadMenuOpenEditor();
		pheadMenuMarkChanged();
	}

	return 0;
}

struct menuitem g_PheadKeepPictureMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_PREVIEW_SIZE + 4, pheadMenuPreviewHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_336, 0, pheadMenuKeepPictureHandler }, // "Keep Picture"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_337, 0, NULL }, // "Try Again"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadKeepPictureMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_333, // "Take Picture"
	g_PheadKeepPictureMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Load From Camera: browse the Game Boy Camera's album.
 * The carousel with param2 0x7b is drawn by menuitemCarouselRender using the
 * album thumbnails (func0f14f07c), a path that survives unused in the final ROM.
 */
MenuItemHandlerResult pheadMenuAlbumHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->carousel.value = func0f14f008(g_PheadDevice);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->carousel.value = func0f14f05c(g_PheadDevice);
		break;
	case MENUOP_SET:
		func0f14f03c(g_PheadDevice, data->carousel.value);
		break;
	case MENUOP_21:
		return false;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuAlbumOkHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		return func0f14f008(g_PheadDevice) <= 0;
	}

	if (operation == MENUOP_SET) {
		s32 playernum = g_MpPlayerNum;

		// Shows "Downloading Image"; the album tick finishes the transfer
		func0f14c4c0(g_PheadDevice);

		g_MpPlayerNum = playernum;
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuLoadFromCameraDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	s32 playernum = g_MpPlayerNum;

	switch (operation) {
	case MENUOP_OPEN:
		func0f14a488();
		func0f14f03c(g_PheadDevice, 0);

		// The "thumbnail loaded" flags live in an uninitialised mema buffer and
		// camdraw only clears them while no camera is present, so clear them
		// before browsing.
		if (var8007f8dc[g_PheadDevice].unk0dc) {
			s32 i;

			for (i = 0; i < 30; i++) {
				var8007f8dc[g_PheadDevice].unk0dc[i] = 0;
			}
		}
		break;
	case MENUOP_CLOSE:
		menuSetBanner(-1, false);
		break;
	case MENUOP_TICK:
		if (pheadMenuIsCurrent(dialogdef) && pheadCameraIsRunning()) {
			// A live capture from Take Picture may have completed after the
			// dialog opened. The album reader only starts from an idle pak
			// (PAK008_01) and has no case for a finished capture in its first
			// state, so acknowledge it here.
			if (var8007f8dc[g_PheadDevice].unk0f4 == 0 && pakGetUnk008(g_PheadDevice) == PAK008_11) {
				pak0f11d478(g_PheadDevice);
			}

			func0f14c3a4(g_PheadDevice);

			// Thumbnails are written by the CPU and drawn by the RDP. The N64's
			// data cache is only 8 KB, so writing it all back is cheap.
			osWritebackDCacheAll();

			g_MpPlayerNum = playernum;

			// 6 = the chosen picture is now in the editor slot
			if (var8007f8dc[g_PheadDevice].unk0f4 == 6) {
				var8007f8dc[g_PheadDevice].unk0f4 = 0;
				menuSetBanner(-1, false);
				pheadSetEditorHasImage(true);
				pheadPictureChanged();
				menuPopDialog();
				pheadMenuOpenEditor();
				pheadMenuMarkChanged();
			}
		}
		break;
	}

	g_MpPlayerNum = playernum;

	return 0;
}

struct menuitem g_PheadLoadFromCameraMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_331, 0, NULL }, // "Choose picture to read from camera:"
	{ MENUITEMTYPE_CAROUSEL, 0, 0, 0x7b, 0x48, pheadMenuAlbumHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_314, 0, pheadMenuAlbumOkHandler }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_332, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadLoadFromCameraMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_330, // "Load From Camera"
	g_PheadLoadFromCameraMenuItems,
	pheadMenuLoadFromCameraDialog,
	0,
	NULL,
};

/**
 * PerfectHead Editor
 *
 * The editor works on camdraw's editor slot (-1); the head being edited is
 * camdraw's current head (-2, see pheadGetCurrentHead). While the editor is
 * open the menu model shows the chosen base head on its own, painted from the
 * editor slot (pheadSetEditing), so every change is visible on the 3D head
 * straight away.
 */

#define PHEAD_GUIDE_COLOUR 0x40ff40a0
#define PHEAD_GUIDE_SCALE  80 // guide size, percent
#define PHEAD_MODEL_OFFSETY 8 // the head's middle is above the model's origin
#define PHEAD_MODEL_ZOOM 18     // half the head's height, in menu units

static bool g_PheadEditorDirty = false; // the menu head needs repainting
static bool g_PheadEditorKept = false;
static bool g_PheadEditorChanged = false; // the player has changed the head since the editor opened

// values saved when a sub-dialog opens, restored by its Cancel
static bool g_PheadSavedChanged;
static s32 g_PheadSavedCrop[4];
static s32 g_PheadSavedBrightness;
static s32 g_PheadSavedFace;
static s32 g_PheadSavedHair;
static u32 g_PheadSavedDepth;
static s32 g_PheadSavedTemplate;
static s32 g_PheadPositionTime60;
static s32 g_PheadPositionPulse60; // the crop frame's pulse
static bool g_PheadPositionMoved;  // moved or resized since Position Image opened
static u8 g_PheadPositionHeld;     // PHEAD_HELD_* pressed at the last step

#define PHEAD_HELD_LEFT  0x01
#define PHEAD_HELD_RIGHT 0x02
#define PHEAD_HELD_UP    0x04
#define PHEAD_HELD_DOWN  0x08
#define PHEAD_HELD_SIZE  0x10

// Where the head is drawn: the centre of the head space rendered last, as a
// menu model position. Menu models are projected inside their dialog
// (menuRenderModel sets the view to the dialog's scissor), so it is relative
// to the dialog's centre, which in split screen isn't the screen's centre.
// Valid when g_PheadModelPosValid.
static f32 g_PheadModelPos[2];
static bool g_PheadModelPosValid;
static f32 g_PheadBobTime;
static s32 g_PheadBobFrame; // frame the bob last moved on

static void pheadMenuMarkDirty(void)
{
	g_PheadEditorDirty = true;
}

// The player changed the head: repaint it, and ask before backing out loses it
static void pheadMenuMarkChanged(void)
{
	g_PheadEditorChanged = true;
	pheadMenuMarkDirty();
}

static void pheadMenuOpenEditor(void)
{
	if (!pheadMenuIsCurrent(&g_PheadEditorMenuDialog)) {
		menuPushDialog(&g_PheadEditorMenuDialog);
	}

	pheadMenuMarkDirty();
}

/**
 * Called while rendering the dialog.
 */
static void pheadMenuSetModelRect(s32 x, s32 y, s32 width, s32 height)
{
	struct menudialog *dialog = g_Menus[g_MpPlayerNum].curdialog;

	if (dialog == NULL) {
		return;
	}

	{
		// The box the model is projected into (menuRenderDialog's scissor)
		s32 top = dialog->y + LINEHEIGHT;
		s32 bottom = dialog->y + dialog->height - 1;
		f32 scale;

		if (bottom > viGetViewTop() + viGetViewHeight()) {
			bottom = viGetViewTop() + viGetViewHeight();
		}

		if (bottom - top < 10) {
			return;
		}

		// menuRenderModel places the model from the player's view, then
		// projects it into this box: offsets come out scaled by the box's
		// height over the view's, so scale them up by the reverse
		scale = (f32) viGetViewHeight() / (bottom - top);

		g_PheadModelPos[0] = (x + width / 2 - (dialog->x + dialog->width / 2)) * scale;
		g_PheadModelPos[1] = (y + height / 2 + PHEAD_MODEL_OFFSETY - (top + bottom) / 2) * scale;
		g_PheadModelPosValid = width > 0;
	}
}

/**
 * Show the edited head on its own, close up.
 */
static void pheadMenuConfigureModel(void)
{
	struct menumodel *menumodel = &g_Menus[g_MpPlayerNum].menumodel;

	menuConfigureModel(menumodel, 0, 0, 0, 0, 0, 0, 1, MENUMODELFLAG_HASSCALE);

	menumodel->curposx = 0;
	menumodel->curposy = 0;
	menumodel->newposx = 0;
	menumodel->newposy = 0;
	menumodel->curscale = 0.01f;
	menumodel->newscale = 1;
	menumodel->zoom = 30;

	// As the Character menu does: give the RDP time to finish with the
	// previous model before its memory is reused for the new one
	menumodel->removingpiece = false;
	menumodel->loaddelay = 8;

	g_PheadModelPosValid = false;
	g_PheadBobTime = 0;
}

/**
 * Called every tick by the dialogs that show the head. The head sways, nods
 * and tilts at different rates like a bobblehead, centred in the head space
 * that was rendered last.
 */
static void pheadMenuTickModel(void)
{
	struct menumodel *menumodel = &g_Menus[g_MpPlayerNum].menumodel;
	u32 params = MENUMODELPARAMS_SET_FILENUM(pheadGetTemplateFilenum(func0f14a9f8(-1)));
	f32 t;

	// Once a frame: a dialog over the editor (Position Image, Change Head...)
	// ticks the head as well as the editor below it, which made it bob twice
	// as fast there
	if (g_PheadBobFrame != g_Vars.thisframestartt) {
		g_PheadBobFrame = g_Vars.thisframestartt;

#if VERSION >= VERSION_PAL_BETA
		g_PheadBobTime += g_Vars.diffframe60freal * (1.0f / 60.0f);
#else
		g_PheadBobTime += g_Vars.diffframe60f * (1.0f / 60.0f);
#endif
	}

	if (g_PheadBobTime > 1000.0f) {
		g_PheadBobTime -= 1000.0f;
	}

	t = g_PheadBobTime;

	menumodel->newroty = menumodel->curroty = 0.6f * sinf(t * 1.3f);
	menumodel->newrotx = menumodel->currotx = 0.14f * sinf(t * 2.7f);
	menumodel->newrotz = menumodel->currotz = 0.07f * sinf(t * 1.9f + 1.0f);

	if (menumodel->curparams != params && menumodel->newparams != params) {
		menumodel->newparams = params;
	}

	menumodel->newanimnum = 0;
	// A part visibility list writes toggle values into the model's rwdata by
	// part number, which on 64-bit can clobber a neighbouring node's pointer.
	// The toggles are set directly instead (pheadMenuModelLoaded).
	menumodel->partvisibility = NULL;
	menumodel->zoom = PHEAD_MODEL_ZOOM;

	// Centre the head in its space
	if (g_PheadModelPosValid) {
		menumodel->newposx = menumodel->curposx = g_PheadModelPos[0];
		menumodel->newposy = menumodel->curposy = g_PheadModelPos[1];
	}

	// Repaint the loaded head whenever the editor slot changed
	if (g_PheadEditorDirty
			&& menumodel->isperfecthead
			&& menumodel->curparams == params
			&& menumodel->bodymodeldef) {
		pheadPaintHead(menumodel->bodymodeldef, -1, false, menumodel->allocstart + menumodel->alloclen);
		g_PheadEditorDirty = false;
	}
}

/**
 * Placement guide over the 64x64 editor picture drawn at x, y (1.5 scale): a
 * face outline with the base head's eye and mouth lines, for the given crop
 * rectangle.
 */
static Gfx *pheadMenuDrawGuide(Gfx *gdl, s32 x, s32 y, s32 size, s32 x1, s32 y1, s32 x2, s32 y2)
{
	u32 colour = PHEAD_GUIDE_COLOUR;
	s32 eyey, mouthy;
	s32 top, bottom;
	s32 cx = (x1 + x2) * size / 128;
	s32 rx = (x2 - x1) * size * PHEAD_FACEBOX_RX / 2048; // PHEAD_FACEBOX_RX of the 32 face texels, at size/64 scale

	pheadGetTemplateGuide(func0f14a9f8(-1), y1, y2, &eyey, &mouthy);

	eyey = eyey * size / 64;
	mouthy = mouthy * size / 64;
	top = eyey - (mouthy - eyey) * 13 / 10;
	bottom = mouthy + (mouthy - eyey) * 3 / 5;

	// Shrink the whole guide about its centre: a face a little smaller than
	// the square maps onto the head better
	{
		s32 mid = (top + bottom) / 2;

		eyey = mid + (eyey - mid) * PHEAD_GUIDE_SCALE / 100;
		mouthy = mid + (mouthy - mid) * PHEAD_GUIDE_SCALE / 100;
		top = mid + (top - mid) * PHEAD_GUIDE_SCALE / 100;
		bottom = mid + (bottom - mid) * PHEAD_GUIDE_SCALE / 100;
		rx = rx * PHEAD_GUIDE_SCALE / 100;
	}

	if (bottom <= top) {
		return gdl;
	}

	// Plain fills, in the same (unprojected) screen space as the picture
	gdl = textSetPrimColour(gdl, colour);

	gDPFillRectangleScaled(gdl++, x + cx - rx, y + top, x + cx + rx + 1, y + top + 1);
	gDPFillRectangleScaled(gdl++, x + cx - rx, y + bottom, x + cx + rx + 1, y + bottom + 1);
	gDPFillRectangleScaled(gdl++, x + cx - rx, y + top, x + cx - rx + 1, y + bottom + 1);
	gDPFillRectangleScaled(gdl++, x + cx + rx, y + top, x + cx + rx + 1, y + bottom + 1);

	gDPFillRectangleScaled(gdl++, x + cx - rx * 7 / 10, y + eyey, x + cx - rx / 6, y + eyey + 1);
	gDPFillRectangleScaled(gdl++, x + cx + rx / 6, y + eyey, x + cx + rx * 7 / 10, y + eyey + 1);
	gDPFillRectangleScaled(gdl++, x + cx - rx * 2 / 5, y + mouthy, x + cx + rx * 2 / 5, y + mouthy + 1);

	return text0f153838(gdl);
}

MenuDialogHandlerResult pheadMenuEditorDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		pheadAllocWorkSlots();
		pheadSetEditing(true);
		g_PheadEditorKept = false;
		g_PheadEditorChanged = false;

		// Keep a copy so Cancel Changes / backing out can revert
		pheadBeginEdit();

		pheadMenuConfigureModel();
		pheadMenuMarkDirty();
		break;
	case MENUOP_CLOSE:
		if (!g_PheadEditorKept) {
			pheadRevertEdit();
		}

		pheadSetEditing(false);
		break;
	case MENUOP_TICK:
		pheadMenuTickModel();

		// Backing out after changing a head that was already loaded asks
		// whether to keep the changes (Keep / Revert) instead of quietly
		// throwing them away. The tick runs before the menu acts on B.
		if (pheadMenuIsCurrent(dialogdef) && data->dialog2.inputs->back
				&& g_PheadEditorChanged && !g_PheadMakingNew) {
			data->dialog2.inputs->back = false;
			menuPushDialog(&g_PheadKeepRevertMenuDialog);
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuTakeAnotherHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return !pheadCameraIsRunning();
	}

	if (operation == MENUOP_SET) {
		menuPushDialog(&g_PheadTakePictureMenuDialog);
	}

	return 0;
}

/**
 * Store the editor into the head being edited (camdraw slot -2) and offer to
 * save it to the Controller Pak. The editor must be the current dialog: the
 * save dialog replaces it.
 */
static void pheadMenuKeepChanges(void)
{
	pheadKeepEditor();

	g_PheadEditorKept = true;
	g_PheadMakingNew = false; // it's a loaded head now (Head Complete!'s Edit Head)

	if (func0f14a06c(-1)->fileguid.fileid != 0) {
		func0f0f3704(&g_PheadSaveChangesMenuDialog);
	} else {
		func0f0f3704(&g_PheadSaveNewMenuDialog);
	}
}

MenuItemHandlerResult pheadMenuKeepChangesHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		pheadMenuKeepChanges();
	}

	return 0;
}

MenuItemHandlerResult pheadMenuModelSpaceHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	// Empty space at the top of the dialog for the 3D head
	if (operation == MENUOP_RENDER) {
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;

		pheadMenuSetModelRect(renderdata->x, renderdata->y, renderdata->width, g_MenuItemRenderHeight);

		return (uintptr_t) data->type19.gdl;
	}

	return 0;
}

struct menuitem g_PheadEditorMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_MODELSPACE_HEIGHT, pheadMenuModelSpaceHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_352, 0, pheadMenuTakeAnotherHandler }, // "Take Another Picture"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_353, 0, (void *)&g_PheadChooseHeadMenuDialog }, // "Change Head"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_354, 0, (void *)&g_PheadPositionMenuDialog }, // "Position Picture"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_355, 0, (void *)&g_PheadColourMenuDialog }, // "Color Face & Hair"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_356, 0, (void *)&g_PheadShapeMenuDialog }, // "Shape Head"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_357, 0, (void *)&g_PheadRemoveMenuDialog }, // "Remove Head"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_OPENSDIALOG, L_MPMENU_358, 0, (void *)&g_PheadCancelChangesMenuDialog }, // "Cancel Changes"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_359, 0, pheadMenuKeepChangesHandler }, // "Keep Changes"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadEditorMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_351, // "PerfectHead Editor"
	g_PheadEditorMenuItems,
	pheadMenuEditorDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

/**
 * Position Image: C buttons move the crop rectangle, L/R make it smaller or
 * larger, and the slider sets the brightness. The rectangle is in the 64x64
 * editor image and selects the part of the picture that becomes the face.
 *
 * The crop is read from the editor slot (pheadGetCrop); camdraw's own getter,
 * func0f14ecd8, reads the current head slot instead.
 */

/**
 * The crop frame, with hints that it can be moved and resized (no ROM version
 * has text for these controls): an arrow inside each side, pointing out, and
 * a mark in each corner. Until the player first moves or resizes the frame,
 * the frame and its marks pulse; afterwards the marks stay dim. The arrow for
 * the direction being pressed, or the corners while resizing, light up white.
 */
static Gfx *pheadMenuDrawCropFrame(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2)
{
	s32 cx = (x1 + x2) / 2;
	s32 cy = (y1 + y2) / 2;
	s32 level = g_PheadPositionPulse60 % 60;
	u32 frame;
	u32 dim;
	u32 lit = 0xffffffff;
	s32 i;

	level = level < 30 ? level : 60 - level; // 0 to 30 and back each second

	if (g_PheadPositionMoved) {
		frame = 0xffff00ff;
		dim = 0xffff0070;
	} else {
		frame = 0xffff0000 | (0x60 + level * 5);
		dim = frame;
	}

	gdl = textSetPrimColour(gdl, frame);
	gDPFillRectangleScaled(gdl++, x1, y1, x2, y1 + 1);
	gDPFillRectangleScaled(gdl++, x1, y2 - 1, x2, y2);
	gDPFillRectangleScaled(gdl++, x1, y1, x1 + 1, y2);
	gDPFillRectangleScaled(gdl++, x2 - 1, y1, x2, y2);
	gdl = text0f153838(gdl);

	// Arrows: 1, 3 and 5 pixels wide from the point, 2 pixels in from the side
	gdl = textSetPrimColour(gdl, (g_PheadPositionHeld & PHEAD_HELD_LEFT) ? lit : dim);

	for (i = 0; i < 3; i++) {
		gDPFillRectangleScaled(gdl++, x1 + 2 + i, cy - i, x1 + 3 + i, cy + i + 1);
	}

	gdl = text0f153838(gdl);
	gdl = textSetPrimColour(gdl, (g_PheadPositionHeld & PHEAD_HELD_RIGHT) ? lit : dim);

	for (i = 0; i < 3; i++) {
		gDPFillRectangleScaled(gdl++, x2 - 3 - i, cy - i, x2 - 2 - i, cy + i + 1);
	}

	gdl = text0f153838(gdl);
	gdl = textSetPrimColour(gdl, (g_PheadPositionHeld & PHEAD_HELD_UP) ? lit : dim);

	for (i = 0; i < 3; i++) {
		gDPFillRectangleScaled(gdl++, cx - i, y1 + 2 + i, cx + i + 1, y1 + 3 + i);
	}

	gdl = text0f153838(gdl);
	gdl = textSetPrimColour(gdl, (g_PheadPositionHeld & PHEAD_HELD_DOWN) ? lit : dim);

	for (i = 0; i < 3; i++) {
		gDPFillRectangleScaled(gdl++, cx - i, y2 - 3 - i, cx + i + 1, y2 - 2 - i);
	}

	gdl = text0f153838(gdl);

	// Corner marks (resizing): an L of 4 pixels inside each corner
	gdl = textSetPrimColour(gdl, (g_PheadPositionHeld & PHEAD_HELD_SIZE) ? lit : dim);
	gDPFillRectangleScaled(gdl++, x1 + 1, y1 + 1, x1 + 5, y1 + 2);
	gDPFillRectangleScaled(gdl++, x1 + 1, y1 + 2, x1 + 2, y1 + 5);
	gDPFillRectangleScaled(gdl++, x2 - 5, y1 + 1, x2 - 1, y1 + 2);
	gDPFillRectangleScaled(gdl++, x2 - 2, y1 + 2, x2 - 1, y1 + 5);
	gDPFillRectangleScaled(gdl++, x1 + 1, y2 - 2, x1 + 5, y2 - 1);
	gDPFillRectangleScaled(gdl++, x1 + 1, y2 - 5, x1 + 2, y2 - 2);
	gDPFillRectangleScaled(gdl++, x2 - 5, y2 - 2, x2 - 1, y2 - 1);
	gDPFillRectangleScaled(gdl++, x2 - 2, y2 - 5, x2 - 1, y2 - 2);

	return text0f153838(gdl);
}

/**
 * The crop rectangle may reach PHEAD_CROP_MARGIN pixels past each edge of the
 * 64x64 picture, so a face that fills the picture (or sits near an edge) can
 * still be framed whole. What falls outside the picture shows the base head's
 * own skin (pheadPaintHead). The preview draws the picture in a dark surround
 * of that width, the whole fitting in the space the picture used to have.
 */
#define PHEAD_CROP_MARGIN 16

MenuItemHandlerResult pheadMenuPositionPreviewHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		s32 box = pheadMenuPreviewSize();
		s32 size = box * 64 / (64 + PHEAD_CROP_MARGIN * 2); // the picture
		s32 bx = renderdata->x + 4;
		s32 by = renderdata->y + 2;
		s32 x = bx + (box - size) / 2;
		s32 y = by + (box - size) / 2;
		s32 x1, y1, x2, y2;

		// The picture on the left, the head on the right
		{
			// The item asks for more width than a quarter-screen dialog has:
			// keep the head inside the dialog
			struct menudialog *dialog = g_Menus[g_MpPlayerNum].curdialog;
			s32 right = renderdata->x + renderdata->width;

			if (dialog && right > dialog->x + dialog->width - 4) {
				right = dialog->x + dialog->width - 4;
			}

			pheadMenuSetModelRect(bx + box, renderdata->y, right - (bx + box), g_MenuItemRenderHeight);
		}

		// The room around the picture that the frame can reach
		gdl = textSetPrimColour(gdl, 0x101018ff);
		gDPFillRectangleScaled(gdl++, bx, by, bx + box, by + box);
		gdl = text0f153838(gdl);

		gdl = pheadRenderEditorImage(gdl, x, y, size / 64.0f);

		pheadGetCrop(-1, &x1, &y1, &x2, &y2);
		gdl = pheadMenuDrawGuide(gdl, x, y, size, x1, y1, x2, y2);

		x1 = x + x1 * size / 64;
		x2 = x + x2 * size / 64;
		y1 = y + y1 * size / 64;
		y2 = y + y2 * size / 64;

		gdl = pheadMenuDrawCropFrame(gdl, x1, y1, x2, y2);

		return (uintptr_t) gdl;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuBrightnessHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = func0f14a984();
		break;
	case MENUOP_SET:
		func0f14a91c(data->slider.value);
		pheadMenuMarkChanged();
		break;
	}

	return 0;
}

static void pheadMenuSetCrop(s32 x1, s32 y1, s32 x2, s32 y2)
{
	// camdraw's setter updates the editor and current head slots
	func0f14ec2c(x1, y1, x2, y2);
}

MenuItemHandlerResult pheadMenuPositionCancelHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		pheadMenuSetCrop(g_PheadSavedCrop[0], g_PheadSavedCrop[1], g_PheadSavedCrop[2], g_PheadSavedCrop[3]);
		func0f14a91c(g_PheadSavedBrightness);
		g_PheadEditorChanged = g_PheadSavedChanged;
		pheadMenuMarkDirty();
		menuPopDialog();
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuPositionDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		pheadMenuFitPosition();
		g_PheadSavedChanged = g_PheadEditorChanged;
		pheadGetCrop(-1, &g_PheadSavedCrop[0], &g_PheadSavedCrop[1], &g_PheadSavedCrop[2], &g_PheadSavedCrop[3]);
		g_PheadSavedBrightness = func0f14a984();
		g_PheadPositionTime60 = 0;
		g_PheadPositionPulse60 = 0;
		g_PheadPositionMoved = false;
		g_PheadPositionHeld = 0;
		break;
	case MENUOP_TICK:
		pheadMenuTickModel();

		// Players joining or leaving change the dialog's room
		if (pheadMenuIsCurrent(dialogdef)) {
			pheadMenuFitPosition();
		}

		// The Combat Simulator's menus give sliders two rows (unk6e). A
		// quarter of the screen can't spare the second: put Brightness and
		// its bar on one row so the picture gets the room (only where the
		// row fits: pheadMenuFitPosition)
		if (g_PheadWrapMode == PHEAD_WRAP_QUARTER && pheadMenuIsCurrent(dialogdef)) {
			g_Menus[g_MpPlayerNum].curdialog->unk6e = 0;
		}

		if (pheadMenuIsCurrent(dialogdef)) {
			s8 contpad1;
			s8 contpad2;
			u32 buttons;
			s32 x1, y1, x2, y2;
			s32 dx = 0;
			s32 dy = 0;
			s32 grow = 0;

			g_PheadPositionTime60 += g_Vars.diffframe60;
			g_PheadPositionPulse60 += g_Vars.diffframe60;

			// one step every two frames at 60 Hz
			if (g_PheadPositionTime60 < 2) {
				break;
			}

			g_PheadPositionTime60 = 0;

			menuGetContPads(&contpad1, &contpad2);
			buttons = joyGetButtons(contpad1, U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);

			if (contpad2 >= 0) {
				buttons |= joyGetButtons(contpad2, U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);
			}

			if (buttons & L_CBUTTONS) dx = -1;
			if (buttons & R_CBUTTONS) dx = 1;
			if (buttons & U_CBUTTONS) dy = -1;
			if (buttons & D_CBUTTONS) dy = 1;
			if (buttons & L_TRIG) grow = -1;
			if (buttons & R_TRIG) grow = 1;

			g_PheadPositionHeld = (dx < 0 ? PHEAD_HELD_LEFT : 0) | (dx > 0 ? PHEAD_HELD_RIGHT : 0)
				| (dy < 0 ? PHEAD_HELD_UP : 0) | (dy > 0 ? PHEAD_HELD_DOWN : 0)
				| (grow ? PHEAD_HELD_SIZE : 0);

			if (dx || dy || grow) {
				pheadGetCrop(-1, &x1, &y1, &x2, &y2);

				x1 += dx - grow * 3;
				x2 += dx + grow * 3;
				y1 += dy - grow * 4;
				y2 += dy + grow * 4;

				// keep it within the margin around the picture, and at least a
				// quarter of the picture
				if (x2 - x1 >= 16 && y2 - y1 >= 20
						&& x1 >= -PHEAD_CROP_MARGIN && y1 >= -PHEAD_CROP_MARGIN
						&& x2 <= 64 + PHEAD_CROP_MARGIN && y2 <= 64 + PHEAD_CROP_MARGIN) {
					pheadMenuSetCrop(x1, y1, x2, y2);
					pheadMenuMarkChanged();
					g_PheadPositionMoved = true;
				}
			}
		}
		break;
	}

	return 0;
}

struct menuitem g_PheadPositionMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0xc8, PHEAD_PREVIEW_SIZE + 4, pheadMenuPositionPreviewHandler },
	{ MENUITEMTYPE_SLIDER, 0, MENUITEMFLAG_SLIDER_ALTSIZE, L_MPMENU_365, 0xff, pheadMenuBrightnessHandler }, // "Brightness"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, pheadMenuNotInQuarterHandler },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_366, 0, NULL }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_367, 0, pheadMenuPositionCancelHandler }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadPositionMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_364, // "Position Image"
	g_PheadPositionMenuItems,
	pheadMenuPositionDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

/**
 * Head Color: face (skin tone) and hair colour.
 */
MenuItemHandlerResult pheadMenuFaceColourHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 4;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) phGetStyleName(data->dropdown.value);
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = func0f14ab60();
		break;
	case MENUOP_SET:
		func0f14ab0c(data->dropdown.value);
		pheadMenuMarkChanged();
		break;
	}

	return 0;
}

/**
 * The list shows Normal first, then the original four. The stored colour
 * numbers stay as they were (Normal = 4), so saved heads are unchanged.
 */
#define PHEAD_HAIRLIST_TO_COLOUR(index) ((index) == 0 ? PHEAD_HAIRCOLOUR_NORMAL : (index) - 1)
#define PHEAD_HAIRCOLOUR_TO_LIST(colour) ((colour) == PHEAD_HAIRCOLOUR_NORMAL ? 0 : (colour) + 1)

MenuItemHandlerResult pheadMenuHairColourHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 colour;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = PHEAD_NUM_HAIRCOLOURS;
		break;
	case MENUOP_GETOPTIONTEXT:
		colour = PHEAD_HAIRLIST_TO_COLOUR(data->dropdown.value);

		if (colour == PHEAD_HAIRCOLOUR_NORMAL) {
			return (uintptr_t) langGet(L_MISC_084); // "Normal" (no new text, so every ROM version has it)
		}

		return (uintptr_t) phGetColourName(colour);
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = PHEAD_HAIRCOLOUR_TO_LIST(func0f14aae8());
		break;
	case MENUOP_SET:
		func0f14aa94(PHEAD_HAIRLIST_TO_COLOUR(data->dropdown.value));
		pheadMenuMarkChanged();
		break;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuColourCancelHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f14ab0c(g_PheadSavedFace);
		func0f14aa94(g_PheadSavedHair);
		g_PheadEditorChanged = g_PheadSavedChanged;
		pheadMenuMarkDirty();
		menuPopDialog();
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuColourDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_PheadSavedChanged = g_PheadEditorChanged;
		g_PheadSavedFace = func0f14ab60();
		g_PheadSavedHair = func0f14aae8();
		break;
	case MENUOP_TICK:
		pheadMenuTickModel();
		break;
	}

	return 0;
}

struct menuitem g_PheadColourMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_MODELSPACE_HEIGHT, pheadMenuModelSpaceHandler },
	{ MENUITEMTYPE_DROPDOWN, 0, 0, L_MPMENU_410, 0, pheadMenuFaceColourHandler }, // "Face:"
	{ MENUITEMTYPE_DROPDOWN, 0, 0, L_MPMENU_411, 0, pheadMenuHairColourHandler }, // "Hairstyle:"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_412, 0, NULL }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_413, 0, pheadMenuColourCancelHandler }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadColourMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_409, // "Head Color"
	g_PheadColourMenuItems,
	pheadMenuColourDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

/**
 * Shape Head: depth of the head (camdraw's squash Z, 0-165 = 0.25-1.9).
 */
MenuItemHandlerResult pheadMenuDepthHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = func0f14ed64();
		break;
	case MENUOP_SET:
		func0f14ee18(data->slider.value);
		pheadMenuMarkChanged();
		break;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuShapeCancelHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f14ee18(g_PheadSavedDepth);
		g_PheadEditorChanged = g_PheadSavedChanged;
		pheadMenuMarkDirty();
		menuPopDialog();
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuShapeDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_PheadSavedChanged = g_PheadEditorChanged;
		g_PheadSavedDepth = func0f14ed64();
		break;
	case MENUOP_TICK:
		pheadMenuTickModel();
		break;
	}

	return 0;
}

struct menuitem g_PheadShapeMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_MODELSPACE_HEIGHT, pheadMenuModelSpaceHandler },
	{ MENUITEMTYPE_SLIDER, 0, MENUITEMFLAG_SLIDER_ALTSIZE, L_MPMENU_361, 165, pheadMenuDepthHandler }, // "Depth"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_362, 0, NULL }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_363, 0, pheadMenuShapeCancelHandler }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadShapeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_360, // "Shape Head"
	g_PheadShapeMenuItems,
	pheadMenuShapeDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

/**
 * Choose Head: the base head the picture is put on. The carousel's arrows go
 * through the heads, shown above it.
 */
MenuItemHandlerResult pheadMenuChooseHeadHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->carousel.value = pheadGetNumAvailableTemplates();
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->carousel.value = pheadGetAvailableIndex(func0f14a9f8(-1));
		break;
	case MENUOP_SET:
		func0f14a9a8(pheadGetAvailableTemplate(data->carousel.value));
		// Let the RDP finish with the old head before its memory is reused
		g_Menus[g_MpPlayerNum].menumodel.loaddelay = 8;
		pheadMenuMarkChanged();
		break;
	case MENUOP_21:
		return false;
	}

	return 0;
}

MenuItemHandlerResult pheadMenuChooseHeadCancelHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f14a9a8(g_PheadSavedTemplate);
		g_PheadEditorChanged = g_PheadSavedChanged;
		pheadMenuMarkDirty();
		menuPopDialog();
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuChooseHeadDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_PheadSavedChanged = g_PheadEditorChanged;
		g_PheadSavedTemplate = func0f14a9f8(-1);
		break;
	case MENUOP_TICK:
		pheadMenuTickModel();
		break;
	}

	return 0;
}

struct menuitem g_PheadChooseHeadMenuItems[] = {
	{ MENUITEMTYPE_MODEL, 0, MENUITEMFLAG_LIST_CUSTOMRENDER, 0x78, PHEAD_MODELSPACE_HEIGHT, pheadMenuModelSpaceHandler },
	{ MENUITEMTYPE_CAROUSEL, 0, 0, 0x7c, 0x0c, pheadMenuChooseHeadHandler },
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_369, 0, NULL }, // "OK"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CENTRE, L_MPMENU_370, 0, pheadMenuChooseHeadCancelHandler }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadChooseHeadMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_368, // "Choose Head"
	g_PheadChooseHeadMenuItems,
	pheadMenuChooseHeadDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

/**
 * Remove PerfectHead
 */
MenuItemHandlerResult pheadMenuRemoveHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		struct var8007f8e0 *head = func0f14a06c(-2);

		head->unk3f4_00 = false;
		pheadSetEditorHasImage(false);

		// Nothing to revert to: the head is gone
		g_PheadEditorKept = true;

		menuPopDialog(); // Remove PerfectHead
		menuPopDialog(); // PerfectHead Editor
	}

	return 0;
}

struct menuitem g_PheadRemoveMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_372, 0, NULL }, // "Are you sure you want to"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_373, 0, NULL }, // "remove this head from memory?"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_374, 0, pheadMenuRemoveHandler }, // "Delete"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_375, 0, NULL }, // "Cancel"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadRemoveMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_371, // "Remove PerfectHead"
	g_PheadRemoveMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Cancel Changes
 */
MenuItemHandlerResult pheadMenuThrowAwayHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		// Closing the editor without keeping reverts it from the undo slot
		menuPopDialog(); // Cancel Changes, or Keep / Revert
		menuPopDialog(); // PerfectHead Editor
	}

	return 0;
}

struct menuitem g_PheadCancelChangesMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_377, 0, NULL }, // "Are you sure you want to"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_378, 0, NULL }, // "throw away your changes?"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_379, 0, pheadMenuThrowAwayHandler }, // "Throw Away"
	{ MENUITEMTYPE_SELECTABLE, 0, MENUITEMFLAG_SELECTABLE_CLOSESDIALOG, L_MPMENU_380, 0, NULL }, // "Go Back To Editor"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadCancelChangesMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_376, // "Cancel Changes"
	g_PheadCancelChangesMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Keep / Revert, when B is pressed in the editor after changing a head that
 * was already loaded (the original's text, which it never showed). Keep goes
 * on as Keep Changes does and Revert as Throw Away; B goes back to the editor.
 */
MenuItemHandlerResult pheadMenuKeepHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog(); // Keep / Revert
		pheadMenuKeepChanges();
	}

	return 0;
}

struct menuitem g_PheadKeepRevertMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_382, 0, NULL }, // "You have edited this head."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_383, 0, NULL }, // "Do you want to keep your changes?"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_384, 0, pheadMenuKeepHandler }, // "Keep"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_385, 0, pheadMenuThrowAwayHandler }, // "Revert"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadKeepRevertMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_381, // "Save PerfectHead"
	g_PheadKeepRevertMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * Save PerfectHead, shown after Keep Changes. A head that is
 * already on a Controller Pak is saved over its file; a new one goes through
 * filemgr's "Where" dialog (FILETYPE_CAMERA, operation 5) to pick a pak.
 * Either way the save is filemgr's FILEOP_005, which calls pheadSaveHead and,
 * on success, pheadMenuFileSaved.
 */
static bool g_PheadSaved = false;

void pheadMenuFileSaved(void)
{
	g_PheadSaved = true;
}

static bool pheadMenuHeadIsOnPak(void)
{
	return func0f14a06c(-1)->fileguid.fileid != 0;
}

MenuItemHandlerResult pheadMenuSaveHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_PheadSaved = false;

		if (pheadMenuHeadIsOnPak()) {
			struct fileguid guid = func0f14a06c(-1)->fileguid;

			filemgrSaveOrLoad(&guid, FILEOP_005, 0);
		} else {
			filemgrPushSelectLocationDialog(5, FILETYPE_CAMERA);
		}
	}

	return 0;
}

MenuItemHandlerResult pheadMenuDontSaveHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f3704(&g_PheadCompleteMenuDialog);
	}

	return 0;
}

MenuDialogHandlerResult pheadMenuSaveDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_PheadSaved = false;
		break;
	case MENUOP_TICK:
		// filemgr has closed its own dialogs once the save is done
		if (g_PheadSaved && pheadMenuIsCurrent(dialogdef)) {
			g_PheadSaved = false;
			func0f0f3704(&g_PheadCompleteMenuDialog);
		}
		break;
	}

	return 0;
}

// For a head that is already on a Controller Pak
struct menuitem g_PheadSaveChangesMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_387, 0, NULL }, // "Your changes will be kept in memory."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_388, 0, NULL }, // "Would you like to save your changes"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_389, 0, NULL }, // "to the Controller Pak?"
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_390, 0, pheadMenuSaveHandler }, // "Save"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_391, 0, pheadMenuDontSaveHandler }, // "Don't Save"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadSaveChangesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_386, // "Save PerfectHead"
	g_PheadSaveChangesMenuItems,
	pheadMenuSaveDialog,
	0,
	NULL,
};

// For a new head
struct menuitem g_PheadSaveNewMenuItems[] = {
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_393, 0, NULL }, // "You can now save your head on to"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_394, 0, NULL }, // "a Controller Pak if you wish."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_395, 0, NULL }, // "If you want to do this, please"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_396, 0, NULL }, // "ensure a Controller Pak is inserted"
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING, L_MPMENU_397, 0, NULL }, // "and choose "Save"."
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_398, 0, pheadMenuSaveHandler }, // "Save"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_399, 0, pheadMenuDontSaveHandler }, // "Don't Save"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadSaveNewMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_392, // "Save PerfectHead"
	g_PheadSaveNewMenuItems,
	pheadMenuSaveDialog,
	0,
	NULL,
};

/**
 * Head Complete!
 */
MenuItemHandlerResult pheadMenuFinishHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog(); // Head Complete!

		// Back to the PerfectHead root, leaving the camera menus if the head
		// was made with the camera
		if (pheadMenuIsCurrent(&g_PheadMainMenuDialog)) {
			menuPopDialog();
		}
	}

	return 0;
}

MenuItemHandlerResult pheadMenuEditHeadHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f3704(&g_PheadEditorMenuDialog);
	}

	return 0;
}

// The item's param is the usage the line is about
MenuItemHandlerResult pheadMenuUsageLineHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		return (pheadGetUsage(-2) & item->param) == 0;
	}

	return 0;
}

struct menuitem g_PheadCompleteMenuItems[] = {
	{ MENUITEMTYPE_LABEL, PHEAD_USE_COMBAT, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_401, 0, pheadMenuUsageLineHandler }, // "To use your head in the combat simulator,"
	{ MENUITEMTYPE_LABEL, PHEAD_USE_COMBAT, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_402, 0, pheadMenuUsageLineHandler }, // "select the head from the character menu"
	{ MENUITEMTYPE_LABEL, PHEAD_USE_COMBAT, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_403, 0, pheadMenuUsageLineHandler }, // "of the player/simulant settings menu."
	{ MENUITEMTYPE_LABEL, PHEAD_USE_MISSIONS, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_404, 0, pheadMenuUsageLineHandler }, // "Your Mission heads will now appear"
	{ MENUITEMTYPE_LABEL, PHEAD_USE_MISSIONS, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_405, 0, pheadMenuUsageLineHandler }, // "in any of the missions you play."
	{ MENUITEMTYPE_LABEL, 0, MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT, L_MPMENU_406, 0, NULL }, // "You can now edit your head further if you wish."
	{ MENUITEMTYPE_SEPARATOR, 0, 0, 0, 0, NULL },
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_407, 0, pheadMenuFinishHandler }, // "Finish"
	{ MENUITEMTYPE_SELECTABLE, 0, 0, L_MPMENU_408, 0, pheadMenuEditHeadHandler }, // "Edit Head"
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PheadCompleteMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPMENU_400, // "Head Complete!"
	g_PheadCompleteMenuItems,
	NULL,
	0,
	NULL,
};

#endif
