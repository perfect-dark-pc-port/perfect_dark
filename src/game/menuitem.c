#include <ultra64.h>
#include "constants.h"
#include "game/camdraw.h"
#include "game/game_006900.h"
#include "game/objectives.h"
#include "game/tex.h"
#include "game/menugfx.h"
#include "game/menuitem.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/objectives.h"
#include "game/game_1531a0.h"
#include "game/lv.h"
#include "game/challenge.h"
#include "game/training.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/str.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "input.h"
#define MENU_KEYBOARD_ROWS 6
#else
#define MENU_KEYBOARD_ROWS 5
#endif

u8 g_MpSelectedPlayersForStats[MAX_PLAYERS];

#if VERSION >= VERSION_NTSC_1_0
s32 g_ScissorX1 = 0;
s32 g_ScissorX2 = 0;
s32 g_ScissorY1 = 0;
s32 g_ScissorY2 = 0;
#endif

s32 var800711a0 = 0;
s32 var800711a4 = 0;
s32 g_LineHeight = LINEHEIGHT;

u8 g_KeyboardKeys[5][10] = {
	{ '0','1','2','3','4','5','6','7','8','9' },
	{ 'A','B','C','D','E','F','G','H','I','J' },
	{ 'K','L','M','N','O','P','Q','R','S','T' },
	{ 'U','V','W','X','Y','Z',' ','?','!','.' },
	{ '1','2','1','2','1','2','3','1','2','3' },
};

static s32 deferredindex = -1;

s32 func0f0e5ce0(s32 value)
{
	if (value < var800711a4) {
		var800711a0++;

		if (var800711a0 > SCREEN_320) {
			var800711a0 = 0;
		}
	}

	var800711a4 = value;

	return var800711a0;
}

s32 menuitem0f0e5d2c(s32 arg0, struct menuitem *item)
{
	union handlerdata data;
	s32 s0;
	s32 s1;
	s32 a0;
	bool done;
	s32 numgroups;

	if (item->handler == NULL) {
		return 0;
	}

	data.list.value = 0;
	data.list.unk0c = 0;

	item->handler(MENUOP_GETOPTGROUPCOUNT, item, &data);

	if (data.list.value == 0) {
		s1 = arg0 / g_LineHeight;
	} else {
		numgroups = data.list.value;
		s0 = arg0;
		done = false;
		s1 = 0;
		data.list.values32 = 0;
		data.list.unk04 = 0;

		while (!done) {
			if (data.list.values32 < numgroups) {
				item->handler(MENUOP_GETGROUPSTARTINDEX, item, &data);
				a0 = data.list.groupstartindex;
			} else {
				a0 = 9999;
			}

			data.list.value++;

			if (s1 + s0 / g_LineHeight >= a0) {
				s0 = s0 - (a0 - s1) * g_LineHeight - LINEHEIGHT;
				s1 += a0 - s1;
			} else {
				s1 += s0 / g_LineHeight;
				done = true;
			}
		}
	}

	if (s1 < 0) {
		s1 = 0;
	}

	return s1;
}

s16 menuitemListGetOffsetY(s16 optionindex, struct menuitem *item)
{
	union handlerdata data;
	bool done;
	s32 numlines;
	s32 numgroups;

	if (item->handler == NULL) {
		return 0;
	}

	if (optionindex < 0) {
		optionindex = 0;
	}

	data.list.value = 0;
	data.list.unk0c = 0;
	item->handler(MENUOP_GETOPTGROUPCOUNT, item, &data);

	if (data.list.value == 0) {
		return optionindex * g_LineHeight;
	}

	done = false;
	numlines = 0;

	numgroups = data.list.value;
	data.list.unk04 = 0;

	for (data.list.value = 0; !done && data.list.values32 < numgroups; data.list.value++) {
		item->handler(MENUOP_GETGROUPSTARTINDEX, item, &data);

		if (optionindex >= data.list.groupstartindex) {
			numlines++;
		} else {
			done = true;
		}
	}

	return optionindex * g_LineHeight + numlines * LINEHEIGHT;
}

Gfx *menuitemListRenderHeader(Gfx *gdl, s16 x1, s16 y1, s16 width, s16 arg4, s16 height, char *text, struct menudialog *dialog)
{
	s32 x;
	s32 y;
	u32 colour;

	colour = MIXCOLOUR(dialog, listgroup_headerbg);

	if (dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x2c) & 0xffffff00) | (colour & 0xff);
	}

	gdl = text0f153780(gdl);
#if VERSION >= VERSION_NTSC_1_0
	gdl = menugfxDrawListGroupHeader(gdl, x1, y1, x1 + width, y1 + height, x1 + arg4, colour & 0xff);
#else
	gdl = menugfxDrawListGroupHeader(gdl, x1, y1, x1 + width, y1 + height, x1 + arg4);
#endif
	gdl = text0f153628(gdl);

	x = x1 + 3;
	y = y1 + 2;

	colour = MIXCOLOUR(dialog, listgroup_headerfg);

	if (dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x2c) & 0xffffff00) | (colour & 0xff);
	}

	textSetWaveColours(g_MenuWave2Colours[dialog->type].listgroup_headerfg, g_MenuWave1Colours[dialog->type].listgroup_headerfg);

	gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, width, height, 0, 0);

	return gdl;
}

Gfx *menuitemListOverlay(Gfx *gdl, s16 x, s16 y, s16 x2, s16 y2)
{
	gDPFillRectangleScaled(gdl++, x, y, x + x2, y + y2);
	return gdl;
}

Gfx *menuitemListRender(Gfx *gdl, struct menurendercontext *context)
{
	struct menuitemdata_list *itemdata = &context->data->list;
	union handlerdata sp15c;
	union handlerdata sp14c;
	union handlerdata sp13c;
	char *text2;
	s32 firstonscreenoptionindex;
	s32 x;
	s32 y;
	s32 sp128;
	s32 halfheight;
	bool done2 = false;
	s32 left = context->x + 2;
	s32 tmp;
	s32 s4;
	char *text;
	s32 height;
	s32 numoptions;
	s32 sp104;
	s32 numgroups;
	s32 nextgroupstartindex = 9999;
	s32 optionindex;
	struct font *font = g_FontHandelGothicSm;
	u32 selectedindex;
	s16 width;
	u8 donestickyheader = false;
	u8 done;
	struct fontchar *chars = g_CharsHandelGothicSm;
	u32 colour;
	union handlerdata spd4;
	char *title;

	if (context->item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
		context->item->handler(MENUOP_GETOPTIONHEIGHT, context->item, &spd4);
		g_LineHeight = spd4.list.value;
	} else {
		g_LineHeight = LINEHEIGHT;
	}

	width = context->dialog->width;

	if (context->item->flags & MENUITEMFLAG_LIST_AUTOWIDTH) {
		width = context->width;
	}

#if VERSION >= VERSION_NTSC_1_0
	g_ScissorX1 = context->x * g_ScaleX;
	g_ScissorX2 = (context->x + width) * g_ScaleX;
	g_ScissorY1 = context->y;
	g_ScissorY2 = context->y + context->height;

	if (g_ScissorX1 < 0) {
		g_ScissorX1 = 0;
	}

	if (g_ScissorX2 < 0) {
		g_ScissorX2 = 0;
	}

	if (g_ScissorY1 < 0) {
		g_ScissorY1 = 0;
	}

	if (g_ScissorY2 < 0) {
		g_ScissorY2 = 0;
	}

	if (g_ScissorX1 > viGetBufWidth()) {
		g_ScissorX1 = viGetBufWidth();
	}

	if (g_ScissorX2 > viGetBufWidth()) {
		g_ScissorX2 = viGetBufWidth();
	}

	if (g_ScissorY1 > viGetBufHeight()) {
		g_ScissorY1 = viGetBufHeight();
	}

	if (g_ScissorY2 > viGetBufHeight()) {
		g_ScissorY2 = viGetBufHeight();
	}

	if (g_ScissorX1 > g_ScissorX2) {
		g_ScissorX2 = g_ScissorX1;
	}

	if (g_ScissorY1 > g_ScissorY2) {
		g_ScissorY2 = g_ScissorY1;
	}

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE,
			context->x * g_ScaleX, context->y,
			(context->x + width) * g_ScaleX, context->y + context->height);
#endif

	halfheight = context->height / 2;
	halfheight /= g_LineHeight;
	halfheight *= g_LineHeight;

	itemdata->viewheight = context->height;

	if (context->item->type == MENUITEMTYPE_DROPDOWN || context->item->type == MENUITEMTYPE_PLAYERSTATS) {
		gdl = menugfxDrawDropdownBackground(gdl, context->x, context->y, context->x + context->width, context->y + context->height);

		gdl = menugfxDrawShimmer(gdl, context->x, context->y, context->x + 1, context->y + context->height, 0x0000007f, true, 15, true);
		gdl = menugfxDrawShimmer(gdl, context->x + context->width, context->y, context->x + context->width + 1, context->y + context->height, 0x0000007f, false, 15, true);
		gdl = menugfxDrawShimmer(gdl, context->x, context->y, context->x + context->width, context->y + 1, 0x0000007f, false, 15, true);
		gdl = menugfxDrawShimmer(gdl, context->x, context->y + context->height, context->x + context->width, context->y + context->height + 1, 0x0000007f, false, 15, false);
	}

	if (context->item->handler == NULL) {
		return gdl;
	}

	sp15c.list.value = 0;
	context->item->handler(MENUOP_GETSELECTEDINDEX, context->item, &sp15c);
	selectedindex = sp15c.list.value;

	if (selectedindex >= 0x10000) {
		selectedindex = -1;
	}

	sp104 = context->y + 1;

	context->item->handler(MENUOP_GETOPTIONCOUNT, context->item, &sp15c);
	numoptions = sp15c.list.value;

	tmp = itemdata->curoffsety - halfheight;
	firstonscreenoptionindex = menuitem0f0e5d2c(itemdata->curoffsety - halfheight, context->item);
	y = context->y + tmp;

	gdl = text0f153628(gdl);

	optionindex = firstonscreenoptionindex;

	sp15c.list.unk04 = 0;

	s4 = menuitemListGetOffsetY(optionindex, context->item) + halfheight - itemdata->curoffsety;

	sp14c.list.value = 0;
	sp14c.list.unk0c = 0;
	context->item->handler(MENUOP_GETOPTGROUPCOUNT, context->item, &sp14c);
	numgroups = sp14c.list.values32 & 0xffffffff;

	if (numoptions > 0) {
		// If this list uses option groups, draw a sticky group header
		// if the player has scroll down far enough
		if (numgroups != 0) {
			s32 spc8;
			done = false;
			spc8 = 0;
			nextgroupstartindex = 0;
			sp14c.list.values32 = 0;
			sp14c.list.unk04 = 0;

			// Iterate all groups ascending until we find the first one that's
			// on-screen, then take the one before it
			while (!done && sp14c.list.values32 < numgroups) {
				context->item->handler(MENUOP_GETGROUPSTARTINDEX, context->item, &sp14c);
				tmp = sp14c.list.groupstartindex & 0xffffffff;

				if (tmp <= firstonscreenoptionindex) {
					spc8 = sp14c.list.values32;
					nextgroupstartindex = tmp;
					sp14c.list.values32++;
				} else {
					done = true;
				}
			}

			sp13c.list.values32 = spc8;
			sp13c.list.unk04 = 0;
			sp13c.list.unk0c = sp14c.list.unk0c;

			if (nextgroupstartindex < firstonscreenoptionindex || s4 < LINEHEIGHT) {
				text = (char *) context->item->handler(MENUOP_GETOPTGROUPTEXT, context->item, &sp13c);

				if (s4 + g_LineHeight > 0) {
					gdl = menuitemListRenderHeader(gdl, context->x, context->y, context->width, width, LINEHEIGHT, text, context->dialog);
					donestickyheader = true;
				}

				sp104 += LINEHEIGHT;

				sp13c.list.value++;

				if (sp14c.list.values32 < numgroups) {
					context->item->handler(MENUOP_GETGROUPSTARTINDEX, context->item, &sp14c);
					tmp = sp14c.list.groupstartindex & 0xffffffff;
					nextgroupstartindex = tmp;
					sp14c.list.values32++;
				} else {
					nextgroupstartindex = 9999;
				}
			}
		}

		if (firstonscreenoptionindex == nextgroupstartindex) {
			s4 -= LINEHEIGHT;
		}

		// Iterate the options to be drawn
		while (!done2) {
			colour = MIXCOLOUR(context->dialog, item_unfocused);

			if (context->dialog->dimmed) {
				colour = (colourBlend(colour, 0x00000000, 127) & 0xffffff00) | (colour & 0xff);
			}

			textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_unfocused, g_MenuWave1Colours[context->dialog->type].item_unfocused);

			if (optionindex == nextgroupstartindex) {
				// Draw a group header. Note that optionindex is not incremented
				// in this branch, but nextgroupstartindex is changed so next time the loop
				// iterates it will enter the else branch.
				if (context->item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
					gdl = menuApplyScissor(gdl);
				}

				title = (char *) context->item->handler(MENUOP_GETOPTGROUPTEXT, context->item, &sp13c);
				sp13c.list.value++;

				height = context->height - s4;

				if (height > LINEHEIGHT) {
					height = LINEHEIGHT;
				}

				gdl = menuitemListRenderHeader(gdl, context->x, context->y + s4, context->width, width, height, title, context->dialog);

				if (sp14c.list.values32 < numgroups) {
					context->item->handler(MENUOP_GETGROUPSTARTINDEX, context->item, &sp14c);
					tmp = sp14c.list.groupstartindex & 0xffffffff;
					nextgroupstartindex = tmp;
					sp14c.list.value++;
				} else {
					nextgroupstartindex = 9999;
				}

				s4 += LINEHEIGHT;
			} else {
				if (optionindex < numoptions) {
					// Draw an option
					union handlerdata spb8;
					bool spb4 = false;

					if ((s32)selectedindex == optionindex) {
						colour |= 0xffffff00;
					}

					if (optionindex == itemdata->index && context->focused) {
						u32 spb0;
						u32 weight = menuGetSinOscFrac(40.0f) * 255.0f;

						spb0 = MIXCOLOUR(context->dialog, item_focused_inner);
						colour = colourBlend(colour, colour & 0xff, 127);
						colour = colourBlend(colour, spb0, weight);

#if VERSION >= VERSION_NTSC_1_0
						// If not transitioning the dialog type
						if ((!(context->dialog->transitionfrac >= 0.0f) || context->dialog->type2 != 0)
								&& (!(context->dialog->transitionfrac < 0.0f) || context->dialog->type != 0)) {
							text0f156024(1);
							spb4 = true;
						}
#else
						text0f156024(1);
						spb4 = true;
#endif
					}

					if (context->item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
						// The handler wants to override the default rendering
						struct menuitemrenderdata renderdata;
						s32 sp94left;
						s32 sp90top;
						s32 sp8cright;
						s32 sp88bottom;

						renderdata.x = context->x;
						renderdata.y = context->y + s4;
						renderdata.width = context->width;
						renderdata.colour = colour;
						renderdata.unk10 = optionindex == itemdata->index;

						sp94left = renderdata.x;
						sp90top = renderdata.y;
						sp8cright = renderdata.x + renderdata.width;
						sp88bottom = renderdata.y + g_LineHeight;

						if (sp90top < context->y) {
							sp90top = context->y;
						}

						if (sp88bottom > context->y + context->height - 1) {
							sp88bottom = context->y + context->height - 1;
						}

						if (donestickyheader && sp90top < context->y + LINEHEIGHT) {
							sp90top = context->y + LINEHEIGHT;
						}

						if (sp94left < 0) {
							sp94left = 0;
						}

						if (sp8cright < 0) {
							sp8cright = 0;
						}

						// This logic doesn't look right...
						if (sp94left / g_ScaleX > viGetWidth()) {
							sp94left = viGetWidth() / g_ScaleX;
						}

						if (sp8cright / g_ScaleX > viGetWidth()) {
							sp8cright = viGetWidth() / g_ScaleX;
						}

						gDPPipeSync(gdl++);

#if VERSION >= VERSION_NTSC_1_0
						g_ScissorX1 = sp94left * g_ScaleX;
						g_ScissorX2 = sp8cright * g_ScaleX;
						g_ScissorY1 = sp90top;
						g_ScissorY2 = sp88bottom;

						if (g_ScissorX1 < 0) {
							g_ScissorX1 = 0;
						}

						if (g_ScissorX2 < 0) {
							g_ScissorX2 = 0;
						}

						if (g_ScissorY1 < 0) {
							g_ScissorY1 = 0;
						}

						if (g_ScissorY2 < 0) {
							g_ScissorY2 = 0;
						}

						if (g_ScissorX1 > viGetBufWidth()) {
							g_ScissorX1 = viGetBufWidth();
						}

						if (g_ScissorX2 > viGetBufWidth()) {
							g_ScissorX2 = viGetBufWidth();
						}

						if (g_ScissorY1 > viGetBufHeight()) {
							g_ScissorY1 = viGetBufHeight();
						}

						if (g_ScissorY2 > viGetBufHeight()) {
							g_ScissorY2 = viGetBufHeight();
						}

						if (g_ScissorX2 < g_ScissorX1) {
							g_ScissorX2 = g_ScissorX1;
						}

						if (g_ScissorY2 < g_ScissorY1) {
							g_ScissorY2 = g_ScissorY1;
						}

						gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
						gDPSetScissor(gdl++, G_SC_NON_INTERLACE, sp94left * g_ScaleX, sp90top, sp8cright * g_ScaleX, sp88bottom);
#endif

						spb8.type19.gdl = gdl;
						spb8.type19.unk04 = optionindex;
						spb8.type19.renderdata2 = (void *)((uintptr_t)&renderdata);
						spb8.type19.unk0c = sp15c.list.unk04;

						gdl = (Gfx *) context->item->handler(MENUOP_RENDER, context->item, &spb8);

						sp15c.list.unk04 = spb8.type19.unk0c;
					} else {
						// Default/simple option (label and optional checkbox)
						sp15c.list.value = optionindex;
						text2 = (char *) context->item->handler(MENUOP_GETOPTIONTEXT, context->item, &sp15c);
						sp128 = 0;
						y = context->y + s4 + 1;

						if (context->item->type == MENUITEMTYPE_DROPDOWN || context->item->type == MENUITEMTYPE_PLAYERSTATS) {
							x = left;
						} else {
							x = left + 8;
						}

						if (y < sp104) {
							sp128 = y - sp104;
							y = sp104;
						}

						height = context->y + context->height - y;

						if (height < 0) {
							height = 0;
						}

						// reuse the label color flag here
						if (context->item->flags & MENUITEMFLAG_LABEL_CUSTOMCOLOUR) {
							u32 savedunk04 = sp15c.list.unk04;
							sp15c.list.unk04 = colour;
							context->item->handler(MENUOP_GETCOLOUR, context->item, &sp15c);
							colour = sp15c.list.unk04;
							sp15c.list.unk04 = savedunk04;
						}

						gdl = textRenderProjected(gdl, &x, &y, text2, chars, font, colour, context->width - left + context->x, height, sp128, 0);

						// Consider a checkbox
						spb8.list.value = optionindex;
						spb8.list.unk04 = 255;

						context->item->handler(MENUOP_GETLISTITEMCHECKBOX, context->item, &spb8);

						if (spb8.list.unk04 != 255) {
							gdl = menugfxDrawCheckbox(gdl, left, context->y + s4 + 1, 6, spb8.list.unk04, colour, 0xff00007f);
						}
					}

					if (spb4) {
						text0f156024(0);
					}
				}

				optionindex++;
				s4 += g_LineHeight;

				if (optionindex >= numoptions) {
					done2 = true;
				}
			}

			if (context->height < s4) {
				done2 = true;
			}
		}

		gdl = menuApplyScissor(gdl);
	} else {
		// The list has no options in it
		colour = MIXCOLOUR(context->dialog, item_unfocused);

		if (context->dialog->dimmed) {
			colour = (colourBlend(colour, 0x00000000, 0x0000007f) & 0xffffff00) | (colour & 0xff);
		}

		x = left + 8;
		y = context->y + context->height / 2;

		// "< Empty >"
		gdl = textRenderProjected(gdl, &x, &y, langGet(L_OPTIONS_313), chars, font, colour, context->width - left + context->x, viGetHeight(), sp128, 0);
	}

	gdl = text0f153780(gdl);

	return gdl;
}

bool menuitemListTick(struct menuitem *item, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	f32 f0;
	union handlerdata handlerdata;
	union handlerdata handlerdata2;
	s16 min;
	s16 max;
	s32 tmp;
	s32 i;
	u32 last;
	s16 prev2;
	s16 prev;

	if (item->handler == NULL) {
		return true;
	}

	if (item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
		item->handler(MENUOP_GETOPTIONHEIGHT, item, &handlerdata2);
		g_LineHeight = handlerdata2.list.value;
	} else {
		g_LineHeight = LINEHEIGHT;
	}

	if (item->type == MENUITEMTYPE_DROPDOWN || item->type == MENUITEMTYPE_PLAYERSTATS) {
		min = data->list.viewheight / 2;
		min = min / g_LineHeight;
		min *= g_LineHeight;

		data->list.targetoffsety = menuitemListGetOffsetY(data->list.index, item);

		if (data->list.targetoffsety < min) {
			data->list.targetoffsety = min;
		}

		item->handler(MENUOP_GETOPTIONCOUNT, item, &handlerdata);

		max = (s16) handlerdata.list.value * g_LineHeight - data->list.viewheight + min;

		if (data->list.targetoffsety > max) {
			data->list.targetoffsety = max;
		}
	}

	if (data->list.curoffsety != data->list.targetoffsety) {
		f0 = data->list.curoffsety;
		prev = data->list.curoffsety;

		for (i = 0; i < g_Vars.diffframe60; i++) {
			f0 = data->list.targetoffsety * (PAL ? 0.404f : 0.35f) + (PAL ? 0.59599995613098f : 0.65f) * f0;
		}

		data->list.curoffsety = f0;

		if (data->list.curoffsety != data->list.targetoffsety && prev == data->list.curoffsety) {
			if (data->list.curoffsety < data->list.targetoffsety) {
				data->list.curoffsety++;
			} else {
				data->list.curoffsety--;
			}
		}
	}

	if (tickflags & MENUTICKFLAG_ITEMISFOCUSED) {
		item->handler(MENUOP_GETOPTIONCOUNT, item, &handlerdata);

		if (handlerdata.list.value) {
			last = handlerdata.list.value - 1;

			if (data->list.index > last) {
				data->list.index = last;
				data->list.targetoffsety = menuitemListGetOffsetY(data->list.index, item);
			}

#ifdef PLATFORM_N64
			if (inputs->updown) {
				prev2 = data->list.index;
				data->list.index += inputs->updown;
#else
			if (inputs->updown || inputs->mousescroll) {
				prev2 = data->list.index;
				data->list.index += inputs->updown + inputs->mousescroll;
#endif

				if (data->list.index < 0) {
					data->list.index = handlerdata.list.value - 1;
				}

				if (data->list.index >= (s16)handlerdata.list.value) {
					data->list.index = 0;
				}

				data->list.targetoffsety = menuitemListGetOffsetY(data->list.index, item);

				if (prev2 != data->list.index) {
					handlerdata.list.value = data->list.index;
					item->handler(MENUOP_LISTITEMFOCUS, item, &handlerdata);

					menuPlaySound(MENUSOUND_SUBFOCUS);
				}
			}

			if (inputs->select) {
				handlerdata.list.value = data->list.index;
				handlerdata.list.unk04 = 0;

				if (inputs->start) {
					handlerdata.list.unk04 = 1;
				}

				item->handler(MENUOP_SET, item, &handlerdata);

				menuPlaySound(MENUSOUND_SELECT);

				if (handlerdata.list.unk04 == 2) {
					inputs->start = false;
				}
			}

			inputs->updown = false;
		}
	}

	tmp = data->list.index;
	handlerdata.list.value = tmp;
	handlerdata.list.unk04 = 1;
	handlerdata.list.unk0c = tmp;
	handlerdata.list.groupstartindex = (tickflags & MENUTICKFLAG_ITEMISFOCUSED) ? 1 : 0;

	item->handler(MENUOP_25, item, &handlerdata);

	if (handlerdata.list.unk0c != handlerdata.list.value) {
		data->list.index = handlerdata.list.value;
		data->list.targetoffsety = menuitemListGetOffsetY(data->list.index, item);
	}

	return true;
}

void menuitemDropdownInit(struct menuitem *item, union menuitemdata *data)
{
	uintptr_t (*handler)(s32 operation, struct menuitem *item, union handlerdata *data);
	union handlerdata handlerdata;
	union handlerdata handlerdata2;

	data->dropdown.list.curoffsety = 0;
	data->dropdown.list.index = 0;
	data->dropdown.unk08 = 0;
	data->dropdown.unk0a = 0;

	handlerdata.dropdown.value = 0;

	handler = item->handler;

	if (item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
		handler(MENUOP_GETOPTIONHEIGHT, item, &handlerdata2);
		g_LineHeight = handlerdata2.dropdown.value;
	} else {
		g_LineHeight = LINEHEIGHT;
	}

	item->handler(MENUOP_GETSELECTEDINDEX, item, &handlerdata);

	if (handlerdata.dropdown.value < 0xffff) {
		data->dropdown.list.index = (u16) handlerdata.dropdown.value;
	} else {
		// The value won't fit in unk02.
		// Maybe MENUOP_25 is getting a scaled-down value?
		// But then how does it know the value is scaled?
		handlerdata.dropdown.value = 0;
		handlerdata.dropdown.unk04 = 0;

		item->handler(MENUOP_25, item, &handlerdata);
		data->dropdown.list.index = handlerdata.dropdown.value;
	}

	data->dropdown.list.targetoffsety = menuitemListGetOffsetY(data->dropdown.list.index, item);

	item->handler(MENUOP_LISTITEMFOCUS, item, &handlerdata);
}

Gfx *menuitemDropdownRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 colour;
	char *text;
	s32 x;
	s32 y;
	union handlerdata data;

	x = context->x;
	y = context->y;
	text = menuResolveParam2Text(context->item);

	colour = MIXCOLOUR(context->dialog, item_unfocused);

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
	}

	if (context->focused != 0) {
		if ((context->focused & 2) != 0) {
			u32 tmpcolour;
			u32 weight = menuGetSinOscFrac(20) * 255.0f; \
			tmpcolour = MIXCOLOUR(context->dialog, item_focused_inner);

			colour = colourBlend(colour, colour & 0xff, 0x7f);
			colour = colourBlend(colour, tmpcolour, weight);
		} else {
			u32 tmpcolour;
			u32 weight = menuGetSinOscFrac(40) * 255.0f; \
			tmpcolour = MIXCOLOUR(context->dialog, item_focused_inner);

			colour = colourBlend(colour, colour & 0xff, 0x7f);
			colour = colourBlend(colour, tmpcolour, weight);
		}

		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_focused_inner, g_MenuWave1Colours[context->dialog->type].item_focused_inner);
	} else {
		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_unfocused, g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	if (menuIsItemDisabled(context->item, context->dialog)) {
		colour = MIXCOLOUR(context->dialog, item_disabled);

		if (context->dialog->dimmed) {
			colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
		}

		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_disabled, g_MenuWave1Colours[context->dialog->type].item_disabled);
	}

	gdl = text0f153628(gdl);

	x = context->x + 10;
	y = context->y + 2;
	gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, context->width, context->height, 0, 0);

	if (context->dialog->unk6e) {
		x = context->x + 80;
	} else {
		x = context->x + context->width - 60;
	}

	if (context->item->flags & MENUITEMFLAG_DROPDOWN_BELOW) {
		x = context->x + 30;
	}

	y = context->y + 2;

	if (context->item->handler != NULL) {
		s32 textheight;
		s32 textwidth;

		data.list.value = 0;
		context->item->handler(MENUOP_GETSELECTEDINDEX, context->item, &data);

		data.list.unk04 = 0;
		text = (char *)context->item->handler(MENUOP_GETOPTIONTEXT, context->item, &data);

		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		x = context->x + context->width - textwidth - 10;
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, context->width, context->height, 0, 0);
	}

	gdl = text0f153780(gdl);

	return gdl;
}

bool menuitemDropdownTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	u32 stack;

	if ((tickflags & MENUTICKFLAG_ITEMISFOCUSED) && item->handler) {
		if (dialog->dimmed && item == dialog->focuseditem) {
			menuitemListTick(item, inputs, tickflags, data);

			if (mpIsPlayerLockedOut(g_MpPlayerNum)) {
				if ((item->flags & MENUITEMFLAG_LOCKABLEMAJOR) || (dialog->definition->flags & MENUDIALOGFLAG_MPLOCKABLE)) {
					dialog->dimmed = false;
				}
			}
		}

		if (inputs->back && dialog->dimmed) {
			dialog->dimmed = false;
			inputs->back = false;
			menuPlaySound(MENUSOUND_TOGGLEOFF);
		}

		if (inputs->select) {
			if (dialog->dimmed) {
				dialog->dimmed = false;
			} else {
				union handlerdata handlerdata;

				dialog->dimmed = true;
				menuitemDropdownInit(item, data);

				handlerdata.dropdown.value = 0;
				item->handler(MENUOP_GETSELECTEDINDEX, item, &handlerdata);
				data->dropdown.unk0e = (u32)handlerdata.dropdown.value * g_LineHeight;
				menuPlaySound(MENUSOUND_TOGGLEOFF);
			}
		}
	}

	return true;
}

Gfx *menuitemDropdownOverlay(Gfx *gdl, s16 x, s16 y, s16 x2, s16 y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data)
{
	s32 stack;
	s32 textwidth;
	s32 textheight;
	s32 numoptions;
	s32 i;
	char *text;
	struct menurendercontext context;

#if VERSION >= VERSION_NTSC_1_0
	if (dialog->unk6e) {
		context.x = x + 78;
	} else if (x2 < 0) {
		context.x = x + 2;
	} else {
		context.x = x + x2 - 62;
	}
#else
	if (dialog->unk6e) {
		context.x = x + 78;
	} else {
		context.x = x + x2 - 62;
	}
#endif

	if (item->flags & MENUITEMFLAG_DROPDOWN_BELOW) {
		context.x = x + 30;
	}

	context.y = y + 1;
	context.width = 56;
	context.item = item;
	context.focused = true;
	context.dialog = dialog;
	context.data = data;

	if (dialog->dimmed && item->handler && dialog->focuseditem == item) {
		union handlerdata handlerdata;

		if (item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
			union handlerdata handlerdata2;
			item->handler(MENUOP_GETOPTIONHEIGHT, item, &handlerdata2); \
			g_LineHeight = handlerdata2.dropdown.value; \
		} else { \
			g_LineHeight = LINEHEIGHT;
		}

		item->handler(MENUOP_GETOPTIONCOUNT, item, &handlerdata);
		numoptions = handlerdata.dropdown.value;

		context.width = 0;
		context.height = g_LineHeight * numoptions;

		for (i = 0; i != numoptions; i++) {
			handlerdata.dropdown.value = i;
			text = (char *)item->handler(MENUOP_GETOPTIONTEXT, item, &handlerdata);
			textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
			textwidth += 6;

			if (textwidth > context.width) {
				context.width = textwidth;
			}
		}

#if VERSION >= VERSION_NTSC_1_0
		if (x2 > 0) {
			context.x = x + x2 - context.width - 7;
		}
#else
		context.x = x + x2 - context.width - 7;
#endif

		if (context.y + context.height > dialog->y + dialog->height + 2) {
			if (context.height > dialog->height) {
				i = dialog->height;
				i = i / (u32)g_LineHeight;
				i = i * g_LineHeight;
				context.height = i;
			}

			context.y = dialog->y + dialog->height - context.height + 2;
		}

		gdl = menuitemListRender(gdl, &context);
	}

	return gdl;
}

bool menuitemKeyboardIsStringEmptyOrSpaces(char *text)
{
	s32 i;

	if (text[0] == '\0') {
		return true;
	}

	i = 0;

	while (text[i] != '\0') {
		if (text[i] != ' ') {
			return false;
		}

		i++;
	}

	return true;
}

/**
 * Keyboard reference:
 * +---------------------------------------+
 * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
 * |---+---+---+---+---+---+---+---+---+---|
 * | a | b | c | d | e | f | g | h | i | j |
 * |---+---+---+---+---+---+---+---+---+---|
 * | k | l | m | n | o | p | q | r | s | t |
 * |---+---+---+---+---+---+---+---+---+---|
 * | u | v | w | x | y | z |   | ? | ! | . |
 * |-------+-----------+-----------+-------|
 * |  DEL  |    CAPS   |   CANCEL  |   OK  |
 * +-------+-----------+-----------+-------+
 */
Gfx *menuitemKeyboardRender(Gfx *gdl, struct menurendercontext *context)
{
	char label[8];
	s32 x;
	s32 y;
	struct menuitemdata_keyboard *data = &context->data->keyboard;
	u32 cursorcolour;
	u32 textcolour;
	u32 tmpcolour;
	s32 buttonwidth;
	s32 row;
	s32 col;
	s32 rowspan;
	u32 alpha;
	s32 textheight;
	s32 textwidth;

	label[1] = '\n';
	label[2] = '\0';

	textSetWaveColours(
			g_MenuWave2Colours[context->dialog->type].item_unfocused,
			g_MenuWave1Colours[context->dialog->type].item_unfocused);

	// Draw input field background
	gdl = textSetPrimColour(gdl, 0x0000ff7f);

	if (context->item->param3 == 0) {
		// Half width
		gDPFillRectangleScaled(gdl++,
				context->x + 4, context->y + 1,
				context->x + 63, context->y + 10);
	} else {
		// Full width
		gDPFillRectangleScaled(gdl++,
				context->x + 4, context->y + 1,
				context->x + 125, context->y + 10);
	}

	// Render text value
	gdl = text0f153838(gdl);

	x = context->x + 4;
	y = context->y + 2;

	gdl = text0f153628(gdl);
	gdl = textRenderProjected(gdl, &x, &y, data->string, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0xffffffff, context->width, context->height, 0, 0);
	gdl = text0f153780(gdl);

	// Render cursor
	alpha = menuGetSinOscFrac(40) * 255;

	if (context->dialog->transitionfrac < 0) {
		cursorcolour = g_MenuColours[context->dialog->type].item_focused_inner;
	} else {
		cursorcolour = colourBlend(
				g_MenuColours[context->dialog->type2].item_focused_inner,
				g_MenuColours[context->dialog->type].item_focused_inner,
				context->dialog->colourweight);
	}

	cursorcolour = colourBlend(colourBlend(0x0000ffff, 0x000000ff, 127), cursorcolour, alpha);

	gdl = textSetPrimColour(gdl, cursorcolour);

	gDPFillRectangleScaled(gdl++, x + 1, context->y + 2, x + 3, context->y + 9);

	gdl = text0f153838(gdl);

	// Render horizontal grid lines
	for (row = 0; row < MENU_KEYBOARD_ROWS + 1; row++) {
		gdl = menugfxDrawFilledRect(gdl, context->x + 4, context->y + row * 11 + 13,
				context->x + 124, context->y + row * 11 + 14, 0x00ffff7f, 0x00ffff7f);
	}

	// Render vertical grid lines
	for (col = 0; col < 11; col++) {
		rowspan = 5;

		if (col == 1 || col == 3 || col == 4 || col == 6 || col == 7 || col == 9) {
			rowspan = 4;
		}

		gdl = menugfxDrawFilledRect(gdl, context->x + col * 12 + 4, context->y + 13,
				context->x + col * 12 + 5, context->y + rowspan * 11 + 14, 0x00ffff7f, 0x00ffff7f);
	}

#if MENU_KEYBOARD_ROWS > 5
	gdl = menugfxDrawFilledRect(gdl, context->x + 4, context->y + 5 * 11 + 13,
			context->x + 5, context->y + 6 * 11 + 14, 0x00ffff7f, 0x00ffff7f);
	gdl = menugfxDrawFilledRect(gdl, context->x + 10 * 12 + 4, context->y + 5 * 11 + 13,
			context->x + 10 * 12 + 5, context->y + 6 * 11 + 14, 0x00ffff7f, 0x00ffff7f);
#endif

	gdl = text0f153628(gdl);

	x = context->x + 10;
	y = context->y + 2;

	for (col = 0; col < 10; col++) {
		for (row = 0; row < MENU_KEYBOARD_ROWS; row++) {
			if (context->dialog->transitionfrac < 0) {
				textcolour = g_MenuColours[context->dialog->type].item_unfocused;
			} else {
				textcolour = colourBlend(
						g_MenuColours[context->dialog->type2].item_unfocused,
						g_MenuColours[context->dialog->type].item_unfocused,
						context->dialog->colourweight);
			}

			if (context->dialog->dimmed) {
				textcolour = (colourBlend(textcolour, 0x00000000, 127) & 0xffffff00) | (textcolour & 0xff);
			}

			if (data->capseffective && col == 2 && row == 4) {
				// CAPS button - make it yellow
				textcolour = (textcolour & 0xff) | 0xffff0000;
			}

			// If this button is the focused one, set highlighted colour
			if (col == data->col && row == data->row) {
				alpha = menuGetSinOscFrac(40) * 255;

				if (context->dialog->transitionfrac < 0) {
					tmpcolour = g_MenuColours[context->dialog->type].item_focused_inner;
				} else {
					tmpcolour = colourBlend(
							g_MenuColours[context->dialog->type2].item_focused_inner,
							g_MenuColours[context->dialog->type].item_focused_inner,
							context->dialog->colourweight);
				}

				textcolour = colourBlend(colourBlend(textcolour, textcolour & 0xff, 127), tmpcolour, alpha);
			}

			x = context->x + col * 12 + 4;
			y = context->y + row * 11 + 15;

			if (row == 4) {
				u16 labels[] = {
					L_OPTIONS_314, // "DEL"
					L_OPTIONS_315, // "CAPS"
					L_OPTIONS_316, // "CANCEL"
					L_OPTIONS_317, // "OK"
				};

				s32 index = -1;

				if (col == 0) {
					index = 0;
				}

				if (col == 2) {
					index = 1;
				}

				if (col == 5) {
					index = 2;
				}

				if (col == 8) {
					index = 3;
				}

				if (index >= 0) {
					buttonwidth = 24;
					y++;

					if (index == 1 || index == 2) {
						buttonwidth = 36;
					}

					textMeasure(&textheight, &textwidth, langGet(labels[index]), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
					x = (buttonwidth - textwidth) / 2 + x;

					// Dim the OK button if string is empty
					if (index == 3 && menuitemKeyboardIsStringEmptyOrSpaces(data->string)) {
						if (context->dialog->transitionfrac < 0) {
							textcolour = g_MenuColours[context->dialog->type].item_disabled;
						} else {
							textcolour = colourBlend(
									g_MenuColours[context->dialog->type2].item_disabled,
									g_MenuColours[context->dialog->type].item_disabled,
									context->dialog->colourweight);
						}

						if (context->dialog->dimmed) {
							textcolour = (colourBlend(textcolour, 0, 127) & 0xffffff00) | (textcolour & 0xff);
						}

						textSetWaveColours(
								g_MenuWave2Colours[context->dialog->type].item_disabled,
								g_MenuWave1Colours[context->dialog->type].item_disabled);
					}

#ifndef PLATFORM_N64
					// Dim the Caps button if keyboard typing is enabled
					if (index == 1 && g_MenuKeyboardPlayer == g_MpPlayerNum) {
						textcolour = colourBlend(
								g_MenuColours[context->dialog->type2].item_disabled,
								g_MenuColours[context->dialog->type].item_disabled,
								context->dialog->colourweight);
						textSetWaveColours(
								g_MenuWave2Colours[context->dialog->type].item_disabled,
								g_MenuWave1Colours[context->dialog->type].item_disabled);
					}
#endif

					gdl = textRenderProjected(gdl, &x, &y, langGet(labels[index]), g_CharsHandelGothicXs, g_FontHandelGothicXs, textcolour, context->width, context->height, 0, 0);

					if (index == 3 && menuitemKeyboardIsStringEmptyOrSpaces(data->string)) {
						textSetWaveColours(
								g_MenuWave2Colours[context->dialog->type].item_unfocused,
								g_MenuWave1Colours[context->dialog->type].item_unfocused);
					}
				}
			} else if (row == 5) {
#ifndef PLATFORM_N64
				if (col == 0) {
					char *kbtext;
					// make the button yellow if it's active
					if (g_MenuKeyboardPlayer == g_MpPlayerNum) {
						textcolour = (textcolour & 0xff) | 0xffff0000;
						kbtext = (char *)"ESC: OK";
					} else {
						kbtext = (char *)"TYPE WITH KEYBOARD";
					}
					++y;
					textMeasure(&textheight, &textwidth, kbtext, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
					x = context->x + (context->width - textwidth) / 2;
					gdl = textRenderProjected(gdl, &x, &y, kbtext, g_CharsHandelGothicXs, g_FontHandelGothicXs, textcolour, context->width, context->height, 0, 0);
				}
#endif
			} else {
				// Alpha-numeric cell
				label[0] = g_KeyboardKeys[row][col];

				if (!data->capseffective && label[0] >= 'A' && label[0] <= 'Z') {
					// Make lowercase
					label[0] += 32;
				}

				textMeasure(&textheight, &textwidth, label, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
				x = (12 - textwidth) / 2 + x;
				gdl = textRenderProjected(gdl, &x, &y, label, g_CharsHandelGothicSm, g_FontHandelGothicSm, textcolour, context->width, context->height, 0, 0);
			}
		}
	}

	gdl = text0f153780(gdl);

	// Highlight border of focused button
	{
		s32 x1 = context->x + data->col * 12 + 4;
		s32 x2 = context->x + data->col * 12 + 16;
		s32 y1 = context->y + data->row * 11 + 13;
		s32 y2 = context->y + data->row * 11 + 24;

		if (data->row == 4) {
			if (data->col == 8) {
				x2 += 12;
			}

			if (data->col == 5) {
				x2 += 24;
			}

			if (data->col == 2) {
				x2 += 24;
			}

			if (data->col == 0) {
				x2 += 12;
			}
		} else if (data->row == 5) {
			x2 = context->x + 9 * 12 + 16;
		}

		gdl = menugfxDrawLine(gdl, x1, y1, x2, y1 + 1, -1, -1); // top
		gdl = menugfxDrawLine(gdl, x2, y1, x2 + 1, y2 + 1, -1, -1); // right
		gdl = menugfxDrawLine(gdl, x1, y2, x2, y2 + 1, -1, -1); // bottom
		gdl = menugfxDrawLine(gdl, x1, y1, x1 + 1, y2 + 1, -1, -1); // left
	}

	return gdl;
}

/**
 * Keyboard reference:
 * +---------------------------------------+
 * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
 * |---+---+---+---+---+---+---+---+---+---|
 * | a | b | c | d | e | f | g | h | i | j |
 * |---+---+---+---+---+---+---+---+---+---|
 * | k | l | m | n | o | p | q | r | s | t |
 * |---+---+---+---+---+---+---+---+---+---|
 * | u | v | w | x | y | z |   | ? | ! | . |
 * |-------+-----------+-----------+-------|
 * |  DEL  |    CAPS   |   CANCEL  |   OK  |
 * +-------+-----------+-----------+-------+
 */
bool menuitemKeyboardTick(struct menuitem *item, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	struct menuitemdata_keyboard *kb = &data->keyboard;
	union handlerdata handlerdata;
	s32 delete = false;

	if (tickflags & MENUTICKFLAG_ITEMISFOCUSED) {
		s16 prevcol = kb->col;
		s16 prevrow = kb->row;

#ifndef PLATFORM_N64
		// handle mouse
		struct menudialog *dialog = g_Menus[g_MpPlayerNum].curdialog;
		if (dialog && g_MenuUsingMouse && !inputs->leftright && !inputs->updown) {
			const s32 dleft = dialog->x + 4;
			const s32 dright = dleft + 12 * 10;
			const s32 dtop = menuitemGetTop(item, dialog) + 12;
			const s32 dbottom = dtop + 11 * 6;
			const s32 mx = inputs->mousex;
			const s32 my = inputs->mousey;
			if (mx > dleft && mx < dright && my > dtop && my < dbottom) {
				kb->row = (my - dtop) / 11;
				kb->col = (mx - dleft) / 12;
				if (kb->row == 4) {
					if (kb->col < 2) {
						kb->col = 0;
					} else if (kb->col < 5) {
						kb->col = 2;
					} else if (kb->col < 8) {
						kb->col = 5;
					} else {
						kb->col = 8;
					}
				} else if (kb->row == 5) {
					kb->col = 0;
				}
			}
		}
#endif

		// Handle left/right movement
		// In most cases the loop only runs once, but on row 4 the buttons span
		// across multiple columns so the loop will run again until the column
		// number is valid.
		if (inputs->leftright != 0) {
			do {
				kb->col += inputs->leftright;

				if (kb->col < 0) {
					kb->col = 9;
				}

				if (kb->col > 9) {
					kb->col = 0;
				}
			} while (kb->row == 4 && kb->col != 0 && kb->col != 2 && kb->col != 5 && kb->col != 8);
		}

		// Handle up/down movement
		if (inputs->updown) {
			kb->row += inputs->updown;

			if (kb->row < 0) {
				kb->row = MENU_KEYBOARD_ROWS - 1;
			}

			if (kb->row > MENU_KEYBOARD_ROWS - 1) {
				kb->row = 0;
			}

			// If moving onto row 4, bump column to a valid one
			if (kb->row == 4) {
				if (kb->col == 9) {
					kb->col = 8;
				}

				if (kb->col == 7 || kb->col == 6) {
					kb->col = 5;
				}

				if (kb->col == 3 || kb->col == 4) {
					kb->col = 2;
				}

				if (kb->col == 1) {
					kb->col = 0;
				}
			}
		}

		if (kb->row == 5) {
			kb->col = 0;
		}

		if (prevcol != kb->col || prevrow != kb->row) {
			menuPlaySound(MENUSOUND_KEYBOARDFOCUS);
		}

		if (inputs->back2) {
			delete = true;
		}

		if (inputs->start) {
			if (item->handler && !menuitemKeyboardIsStringEmptyOrSpaces(kb->string)) {
				menuPlaySound(MENUSOUND_SELECT);

				handlerdata.keyboard.string = kb->string;
				item->handler(MENUOP_SETTEXT, item, &handlerdata);

				menuPopDialog();

				item->handler(MENUOP_SET, item, &handlerdata);
			}

			inputs->start = false;
		}

		u8 maxlen = item->param == 0 ? 10 : item->param;
#ifndef PLATFORM_N64
		if (g_MenuKeyboardPlayer != g_MpPlayerNum && inputKeyJustPressed(VK_A + ('i' - 'a'))) {
			inputClearLastKey();
			inputClearLastTextChar();
			g_MenuKeyboardPlayer = g_MpPlayerNum;
			inputStartTextInput();
			menuPlaySound(MENUSOUND_SELECT);
		}

		if (g_MenuKeyboardPlayer == g_MpPlayerNum) {
			// match caps state to keyboard shift/caps if typing with keyboard
			const u32 kmod = inputGetKeyModState();
			kb->capslock = (kmod & KM_CAPS) != 0;
			if (kmod & KM_SHIFT) {
				kb->capslock = !kb->capslock;
			}
			// handle text input
			s32 prevpos = strlen(kb->string);
			s32 pos = prevpos;
			s32 result = inputTextHandler(kb->string, maxlen + 1, &pos, true);
			if (result == -1) {
				// cancel
				kb->row = 5;
				kb->col = 0;
				inputs->select = true;
			} else if (result == 1) {
				// accept
				kb->row = 4;
				kb->col = 8;
				inputs->select = true;
				g_MenuKeyboardPlayer = -1;
				inputStopTextInput();
			} else if (prevpos < pos) {
				// added char
				menuPlaySound(MENUSOUND_FOCUS);
			}
		}
#endif

		if (inputs->select) {
			if (kb->row == 4) {
				// DEL
				if (kb->col == 0) {
					delete = true;
				}

				// CAPS
				if (kb->col == 2) {
					kb->capslock = 1 - kb->capslock;
				}

				// OK
				if (kb->col == 8) {
					if (item->handler && !menuitemKeyboardIsStringEmptyOrSpaces(kb->string)) {
						handlerdata.keyboard.string = kb->string;
						item->handler(MENUOP_SETTEXT, item, &handlerdata);
					}
				}

				// OK or CANCEL
				if (kb->col == 8 || kb->col == 5) {
					s32 ok = (kb->col == 8);

					if (kb->col == 5 || !menuitemKeyboardIsStringEmptyOrSpaces(kb->string)) {
						menuPopDialog();

						if (ok) {
							item->handler(MENUOP_SET, item, &handlerdata);
							menuPlaySound(MENUSOUND_SELECT);
						} else {
							menuPlaySound(MENUSOUND_KEYBOARDCANCEL);
						}

						inputs->select = false;
					}
				}
			} else if (kb->row == 5) {
#ifndef PLATFORM_N64
				if (g_MenuKeyboardPlayer == -1) {
					g_MenuKeyboardPlayer = g_MpPlayerNum;
					inputClearLastTextChar();
					inputStartTextInput();
					menuPlaySound(MENUSOUND_SELECT);
				} else {
					g_MenuKeyboardPlayer = -1;
					inputStopTextInput();
					menuPlaySound(MENUSOUND_KEYBOARDCANCEL);
				}
#endif
			} else {
				// Pressed A on number or letter
				s32 appended = false;
				s32 i;
				s32 textwidth;
				s32 textheight;

				if (kb->string[maxlen-1] == '\0') {
					// String is not full
					i = 0;

					while (!appended) {
						if (kb->string[i] == '\0') {
							u8 key = g_KeyboardKeys[kb->row][kb->col];
							appended = true;

							if (kb->capseffective == 0 && key >= 'A' && key <= 'Z') {
								// Make lowercase
								key += 32;
							}

							kb->string[i] = key;
						}

						i++;
					}

					textMeasure(&textheight, &textwidth, kb->string, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

					if (item->param3 == 0 && textwidth > 58) {
						delete = true;
					}

					if (!delete) {
						menuPlaySound(MENUSOUND_FOCUS);
					}
				}
			}
		}

		// Handle deleting
		if (delete && kb->string[0] != '\0') {
			s32 deleted = false;
			s32 i = maxlen;

			menuPlaySound(MENUSOUND_FOCUS);

			while (!deleted) {
				if (kb->string[i] != '\0') {
					kb->string[i] = '\0';
					deleted = true;
				}

				i--;
			}
		}
	}

	// Update caps
	if (tickflags & MENUTICKFLAG_ITEMISFOCUSED) {
		u32 prev = kb->capseffective;
		kb->capseffective = kb->capslock;

		if (inputs->shoulder) {
			// Invert
			kb->capseffective = 1 - kb->capseffective;
		}

		if (kb->capseffective != prev) {
			if (kb->capseffective) {
				menuPlaySound(MENUSOUND_TOGGLEON);
			} else {
				menuPlaySound(MENUSOUND_TOGGLEOFF);
			}
		}
	}

	return true;
}

void menuitemKeyboardInit(struct menuitem *item, union menuitemdata *data)
{
	u32 stack;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(data->keyboard.string);) {
		data->keyboard.string[i++] = '\0';
	}

	if (item->handler) {
		union handlerdata handlerdata;
		handlerdata.keyboard.string = data->keyboard.string;
		item->handler(MENUOP_GETTEXT, item, &handlerdata);
	}

	data->keyboard.col = 0;
	data->keyboard.row = 4;
	data->keyboard.capseffective = 0;
	data->keyboard.capslock = 0;
}

Gfx *menuitemSeparatorRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 colour;

	if (context->dialog->transitionfrac < 0) {
		colour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		colour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	colour = (colour & 0xffffff00) | 0x3f;

#if VERSION >= VERSION_JPN_FINAL
	return menugfxDrawFilledRect(gdl, context->x, context->y, context->x + context->width, context->y + 1, colour, colour);
#else
	return menugfxDrawFilledRect(gdl, context->x, context->y + 2, context->x + context->width, context->y + 3, colour, colour);
#endif
}

u32 var800711e8 = 0x00000000;

Gfx *menuitemObjectivesRenderOne(Gfx *gdl, struct menudialog *dialog, s32 index, s32 position, s16 objx, s16 objy, s16 width, s16 height, bool withstatus, bool narrow)
{
	u32 sp12c;
	s32 x;
	s32 y;
	char *sp120;
	char buffer[80];
	char *spcc;
	u32 spc8;
	s32 textwidth;
	s32 textheight;
	s32 spbc;
	s32 tmp;
	s32 spb4;
	s32 spb0;
	s32 spac;
	s32 spa8;
	u32 stack3[2];
	s32 sp9c;
	s32 sp98;
	u32 s0;
	s32 stack[4];
	s32 sp80;
	s32 sp7c;
	s32 sp78;
	s32 sp74;
	s32 stack2;
	s32 sp6c;
	s32 sp58;
	s32 sp54;

	spbc = 0;

	if (narrow) {
#if VERSION == VERSION_JPN_FINAL
		spbc = 14;
#elif VERSION >= VERSION_PAL_FINAL
		spbc = 16;
#else
		spbc = 12;
#endif
	}

	mainOverrideVariable("brcol", &var800711e8);
	sp120 = langGet(g_Briefing.objectivenames[index]);
	y = objy;
	sp12c = MIXCOLOUR(dialog, item_unfocused);

	if (dialog->dimmed) {
		sp12c = (colourBlend(sp12c, 0, 0x2c) & 0xffffff00) | (sp12c & 0xff);
	}

	textSetWaveColours(g_MenuWave2Colours[dialog->type].item_unfocused, g_MenuWave1Colours[dialog->type].item_unfocused);
	buffer[0] = '\0';

	// Render objective number
	gdl = text0f153628(gdl);
	sprintf(buffer, "%d: ", position);
	textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
	x = objx - textwidth + 25;
	gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, sp12c, width, height, 0, 0);

	x = objx + 25;

	if (narrow) {
		textWrap(85, sp120, buffer, g_CharsHandelGothicXs, g_FontHandelGothicXs);
		gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicXs, g_FontHandelGothicXs, sp12c, width, height, 0, 0);
	} else {
		sprintf(buffer, "%s", sp120);
		gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, sp12c, width, height, 0, 0);
	}

	if (withstatus) {
		switch (objectiveCheck(index)) {
		case OBJECTIVE_INCOMPLETE:
			spcc = langGet(L_OPTIONS_001); // "Incomplete"
			spc8 = 0xffff00ff;
			break;
		case OBJECTIVE_COMPLETE:
			spcc = langGet(L_OPTIONS_000); // "Complete"
			spc8 = 0x00ff00ff;
			break;
		case OBJECTIVE_FAILED:
			spcc = langGet(L_OPTIONS_002); // "Failed"
			spc8 = 0xff4040ff;
			break;
		}

		if (dialog != g_Menus[g_MpPlayerNum].curdialog) {
			spc8 = g_MenuColours[0].item_unfocused;
		}

		textMeasure(&textheight, &textwidth, spcc, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		if (var800711e8 != 0) {
			spc8 = var800711e8;
		}

		x = objx + width - textwidth - 10;
#if VERSION == VERSION_JPN_FINAL
		y = objy + spbc;
		y += 10;
#else
		y = objy + spbc + 9;
#endif

		gdl = textRenderProjected(gdl, &x, &y, spcc, g_CharsHandelGothicXs, g_FontHandelGothicXs, spc8, width, height, 0, 0);

#if VERSION != VERSION_JPN_FINAL
		x = objx + width - textwidth - 10;
		y = objy + spbc + 9;

		gdl = textRenderProjected(gdl, &x, &y, spcc, g_CharsHandelGothicXs, g_FontHandelGothicXs, spc8 & 0xffffff7f, width, height, 0, 0);
#endif

		x = objx + width - textwidth - 13;
		y = objy + 9;

		gdl = text0f153780(gdl);

		spb4 = objx + 22;
		spb0 = objy - 2;
		spac = objy + 8;
		spa8 = y + spbc + 2;

		tmp = (objx * 3 + objx + 66) / 4;
		sp9c = x;
		sp98 = tmp + 19 + (sp9c - tmp - 24) * (position - 1) / 5;

		sp58 = (objx * 3 + objx + 66) / 4 - 1;
		sp54 = (objx * 3 + objx + 66) / 4 + 14;

#if VERSION == VERSION_JPN_FINAL
		spa8 += 3;
#endif

		gdl = menugfx0f0e2498(gdl);

		// Blue lines
		gdl = menugfxDrawProjectedLine(gdl, objx, spb0, spb4, spb0 + 1, sp12c & 0xffffff00, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, spb4, spb0, spb4 + 1, spac, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, objx, spac, sp58, spac + 1, sp12c & 0xffffff00, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 3, spac, spb4 + 1, spac + 1, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58, spac, sp58 + 1, spa8 + 2, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 2, spac, sp58 + 3, spa8, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		spb4 = (spc8 & 0xffffff00) | 0x3f;
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 2, spa8, sp54, spa8 + 1, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58, spa8 + 2, sp54, spa8 + 3, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp54, spa8, sp98, spa8 + 1, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp54, spa8 + 2, sp98, spa8 + 3, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);

		// Coloured lines
		gdl = menugfxDrawProjectedLine(gdl, sp98, spa8, sp98 + 1, spa8 + 3, spb4, spb4);
		gdl = menugfxDrawProjectedLine(gdl, sp98 + 1, spa8 + 1, sp9c - 4, spa8 + 2, spb4, spb4);
		gdl = menugfxDrawProjectedLine(gdl, sp9c - 4, spa8 + 1, sp9c, spa8 + 2, spb4, (spc8 & 0xffffff00) | 0xcf);
	} else {
		// Render lines without status
		sp80 = objx + 22;
		sp7c = objy - 2;
		sp78 = objy + 8;
		sp74 = y + 1;

		textMeasure(&textheight, &textwidth, sp120, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

#if VERSION >= VERSION_JPN_FINAL
		sp74 -= 2;
#endif

		sp6c = objx + textwidth + 25;
		sp58 = (objx * 3 + objx + 66) / 4 - 1;

		gdl = text0f153780(gdl);
		gdl = menugfx0f0e2498(gdl);

		gdl = menugfxDrawProjectedLine(gdl, objx, sp7c, sp80, sp7c + 1, sp12c & 0xffffff00, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp80, sp7c, sp80 + 1, sp78, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, objx, sp78, sp58, sp78 + 1, sp12c & 0xffffff00, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 3, sp78, sp80 + 1, sp78 + 1, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
#if VERSION == VERSION_JPN_FINAL
		gdl = menugfxDrawProjectedLine(gdl, sp58, sp78, sp58 + 1, sp74 + 3, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
#else
		gdl = menugfxDrawProjectedLine(gdl, sp58, sp78, sp58 + 1, sp74 + 2, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
#endif
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 2, sp78, sp58 + 3, sp74, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
		gdl = menugfxDrawProjectedLine(gdl, sp58 + 2, sp74, sp6c, sp74 + 1, (sp12c & 0xffffff00) | 0x3f, (sp12c & 0xffffff00) | 0x3f);
	}

	return gdl;
}

Gfx *menuitemObjectivesRender(Gfx *gdl, struct menurendercontext *context)
{
	s32 y = context->y + 5;
	s32 position = 1;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Briefing.objectivenames); i++) {
		if (g_Briefing.objectivenames[i] && g_Briefing.objectivedifficulties[i] & (1 << lvGetDifficulty())) {
			gdl = menuitemObjectivesRenderOne(gdl,
					context->dialog,
					i, position, context->x, y, context->width, context->height,
					context->item->param != 1,
					context->item->param == 2);

			position++;

#if VERSION >= VERSION_JPN_FINAL
			if (context->item->param == 0) {
				y += 24;
			} else if (context->item->param == 2) {
				y += 36;
			} else {
				y += 16;
			}
#else
			if (context->item->param == 0) {
				y += 18;
			} else if (context->item->param == 2) {
				y += VERSION >= VERSION_PAL_FINAL ? 34 : 30;
			} else {
				y += 14;
			}
#endif
		}
	}

	return gdl;
}

Gfx *menuitemModelRender(Gfx *gdl, struct menurendercontext *context)
{
	if (context->item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
		struct menuitemrenderdata renderdata;
		union handlerdata data;

		renderdata.x = context->x;
		renderdata.y = context->y;
		renderdata.width = context->width;

		if (context->dialog->transitionfrac < 0) {
			renderdata.colour = g_MenuColours[context->dialog->type].item_focused_inner;
		} else {
			renderdata.colour = colourBlend(
					g_MenuColours[context->dialog->type2].item_focused_inner,
					g_MenuColours[context->dialog->type].item_focused_inner,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			renderdata.colour = (colourBlend(renderdata.colour, 0, 127) & 0xffffff00) | (renderdata.colour & 0xff);
		}

		renderdata.unk10 = true;

		data.type19.gdl = gdl;
		data.type19.unk04 = (intptr_t)&renderdata;
		data.type19.renderdata2 = &renderdata;

		gdl = (Gfx *)context->item->handler(MENUOP_RENDER, context->item, &data);
	}

	return gdl;
}

Gfx *menuitemLabelRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 colour1;
	u32 colour2;
	char *text;
	s32 x;
	s32 y;
	struct menudfc *menudfc;
	u8 savedvalue = var8007fb9c;
	struct fontchar *font1 = g_CharsHandelGothicSm;
	struct font *font2 = g_FontHandelGothicSm;

	x = context->x + 10;
	y = context->y + 2;

	if (context->item->flags & MENUITEMFLAG_LESSLEFTPADDING) {
		x -= 6;
	}

	text = menuResolveParam2Text(context->item);

	if (!text) {
		return gdl;
	}

#if VERSION != VERSION_JPN_FINAL
	if (context->item->flags & MENUITEMFLAG_SMALLFONT) {
		font1 = g_CharsHandelGothicXs;
		font2 = g_FontHandelGothicXs;
		y -= 2;
	}
#endif

	if (context->item->flags & MENUITEMFLAG_SELECTABLE_CENTRE) {
		// Center
		s32 textheight;
		s32 textwidth;
		textMeasure(&textheight, &textwidth, text, font1, font2, 0);
		x = context->x + (context->width - textwidth) / 2;
	}

	if (context->item->flags & MENUITEMFLAG_LABEL_ALTCOLOUR) {
		if (context->dialog->transitionfrac < 0) {
			colour1 = g_MenuColours[context->dialog->type].checkbox_checked_unfocused;
		} else {
			colour1 = colourBlend(
					g_MenuColours[context->dialog->type2].checkbox_checked_unfocused,
					g_MenuColours[context->dialog->type].checkbox_checked_unfocused,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			colour1 = (colourBlend(colour1, 0, 127) & 0xffffff00) | (colour1 & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].checkbox_checked_unfocused,
				g_MenuWave1Colours[context->dialog->type].checkbox_checked_unfocused);
	} else {
		if (context->dialog->transitionfrac < 0) {
			colour1 = g_MenuColours[context->dialog->type].item_unfocused;
		} else {
			colour1 = colourBlend(
					g_MenuColours[context->dialog->type2].item_unfocused,
					g_MenuColours[context->dialog->type].item_unfocused,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			colour1 = (colourBlend(colour1, 0, 127) & 0xffffff00) | (colour1 & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_unfocused,
				g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	if (menuIsItemDisabled(context->item, context->dialog)) {
		if (context->dialog->transitionfrac < 0) {
			colour1 = g_MenuColours[context->dialog->type].item_disabled;
		} else {
			colour1 = colourBlend(
					g_MenuColours[context->dialog->type2].item_disabled,
					g_MenuColours[context->dialog->type].item_disabled,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			colour1 = (colourBlend(colour1, 0, 127) & 0xffffff00) | (colour1 & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_disabled,
				g_MenuWave1Colours[context->dialog->type].item_disabled);
	}

	menudfc = func0f0f1338(context->item);

	if (menudfc) {
		if (menudfc->unk04 < 0) {
			return gdl;
		}

		textBackupDiagonalBlendSettings();
		textSetDiagonalBlend(x, y, menudfc->unk04 * 300, 0);
		var8007fb9c = true;
	}

	colour2 = colour1;

	if (context->item->flags & MENUITEMFLAG_LABEL_CUSTOMCOLOUR) {
		union handlerdata data;
		data.label.colour2 = colour2;
		data.label.colour1 = colour1;

		if (context->item->handlervoid) {
			context->item->handlervoid(MENUOP_GETCOLOUR, context->item, &data);
		}

		if (context->item->handlervoid) {
		}

		colour2 = data.label.colour2;
		colour1 = data.label.colour1;
	}

	gdl = text0f153628(gdl);
	gdl = textRenderProjected(gdl, &x, &y, text,
			font1, font2, colour1, context->width, context->height, 0, 0);

	if ((context->item->flags & MENUITEMFLAG_LABEL_HASRIGHTTEXT) == 0) {
		// Right side text
		text = menuResolveText(context->item->param3, context->item);

		// This is not how you check if a string is empty...
		if (text != NULL && text != "") {
			s32 textheight;
			s32 textwidth;

			y = context->y + 2;

#if VERSION != VERSION_JPN_FINAL
			if (context->item->flags & MENUITEMFLAG_SMALLFONT) {
				y -= 2;
			}
#endif

			textMeasure(&textheight, &textwidth, text, font1, font2, 0);
			x = context->x + context->width - textwidth - 10;

			if (context->item->flags & MENUITEMFLAG_LESSLEFTPADDING) {
				x += 6;
			}

			gdl = textRenderProjected(gdl, &x, &y, text,
					font1, font2, colour2, context->width, context->height, 0, 0);
		}
	}

	gdl = text0f153780(gdl);

	if (menudfc) {
		if (context->width + 200 < menudfc->unk04 * 300 && context->dialog->redrawtimer < 0) {
			func0f0f13ec(context->item);
		}

		var8007fb9c = savedvalue;

		textRestoreDiagonalBlendSettings();
	}

	if (context->item->flags & MENUITEMFLAG_LIST_CUSTOMRENDER) {
		struct menuitemrenderdata renderdata;
		union handlerdata data;

		renderdata.x = context->x;
		renderdata.y = context->y;
		renderdata.width = context->width;
		renderdata.colour = colour1;
		renderdata.unk10 = false;

		data.type19.gdl = gdl;
		data.type19.unk04 = 0;
		data.type19.renderdata2 = &renderdata;

		gdl = (Gfx *) context->item->handler(MENUOP_RENDER, context->item, &data);
	}

	return gdl;
}

/**
 * Renders two yellow bars and an optional label. Suspected to be a resource
 * meter but with the calculations ifdeffed out, so the widths are static.
 */
Gfx *menuitemMeterRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 a = 9;
	char *text;
	s32 x;
	s32 y;
	u32 colour1;
	u32 colour2;
	u32 colour;
	s32 x1;
	s32 x2;
	s32 x3;

	if (context->dialog->transitionfrac < 0) {
		colour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		colour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 127) & 0xffffff00) | (colour & 0xff);
	}

	colour2 = 0xffff0000 | (colour & 0xff);
	colour1 = colourBlend(colour2, colour2 & 0xff, 127);

	x1 = context->x + 32;
	x2 = x1 + a;
	x3 = x2 + 6;

	gdl = textSetPrimColour(gdl, colour1);
	gDPFillRectangleScaled(gdl++, x1, context->y, x2, context->y + 5);
	gdl = text0f153838(gdl);

	gdl = textSetPrimColour(gdl, colour2);
	gDPFillRectangleScaled(gdl++, x2, context->y, x3, context->y + 5);
	gdl = text0f153838(gdl);

	text = menuResolveParam2Text(context->item);

	if (text) {
		gdl = text0f153628(gdl);
		x = context->x;
		y = context->y - 1;
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
				colour2 & 0xffffff7f, context->width, context->height, 0, 0);
		gdl = text0f153780(gdl);
	}

	return gdl;
}

#ifndef PLATFORM_N64

// Draws a colored box which fills the background of the menu item.
Gfx* menuitemColorBoxRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 width = context->width;
	u32 height = context->height;
	u32 colour1;
	s32 x1;
	s32 x2;

	x1 = context->x;
	x2 = x1 + width;

	union handlerdata data;
	if (context->item->handlervoid) {
		context->item->handlervoid(MENUOP_GETCOLOUR, context->item, &data);
	}

	colour1 = data.label.colour1;

	gdl = textSetPrimColour(gdl, colour1);
	gDPFillRectangleScaled(gdl++, x1, context->y, x2, context->y + height);
	gdl = text0f153838(gdl);

	return gdl;
}

#endif

Gfx *menuitemSelectableRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 leftcolour;
	u32 rightcolour;
	char *text;
	s32 x = context->x + 10;
	s32 y = context->y + 2;
	struct fontchar *font1 = g_CharsHandelGothicSm;
	struct font *font2 = g_FontHandelGothicSm;

	if (context->item->flags & MENUITEMFLAG_LESSLEFTPADDING) {
		x -= 6;
	}

	if (context->item->flags & MENUITEMFLAG_BIGFONT) {
		font1 = g_CharsHandelGothicMd;
		font2 = g_FontHandelGothicMd;
	}

	text = menuResolveParam2Text(context->item);

	if (context->dialog->transitionfrac < 0) {
		leftcolour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		leftcolour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		leftcolour = (colourBlend(leftcolour, 0, 127) & 0xffffff00) | (leftcolour & 0xff);
	}

	rightcolour = leftcolour;

	if (context->focused) {
		// Mismatch: The addiu and lui at eb030 and eb034 are swapped. The addiu
		// is adding the lower half of the address of g_MenuColours for
		// use in the else branch. The lui is loading a value into $at for the
		// float to u32 conversion for weight.
		u32 colour2;
		u32 weight = menuGetSinOscFrac(40) * 255; \
		if (context->dialog->transitionfrac < 0) { \
			colour2 = g_MenuColours[context->dialog->type].item_focused_inner; \
		} else { \
			colour2 = colourBlend(
					g_MenuColours[context->dialog->type2].item_focused_inner,
					g_MenuColours[context->dialog->type].item_focused_inner,
					context->dialog->colourweight);
		}

		leftcolour = colourBlend(colourBlend(leftcolour, leftcolour & 0x000000ff, 127), colour2, weight);

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_focused_inner,
				g_MenuWave1Colours[context->dialog->type].item_focused_inner);
	} else {
		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_unfocused,
				g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	if (menuIsItemDisabled(context->item, context->dialog)) {
		if (context->dialog->transitionfrac < 0) {
			leftcolour = g_MenuColours[context->dialog->type].item_disabled;
		} else {
			leftcolour = colourBlend(
					g_MenuColours[context->dialog->type2].item_disabled,
					g_MenuColours[context->dialog->type].item_disabled,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			leftcolour = (colourBlend(leftcolour, 0x00000000, 127) & 0xffffff00) | (leftcolour & 0xff);
		}

		rightcolour = leftcolour;

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_disabled,
				g_MenuWave1Colours[context->dialog->type].item_disabled);
	}

	if (context->item->flags & MENUITEMFLAG_SELECTABLE_CENTRE) {
		// Center text
		s32 textheight;
		s32 textwidth;
		textMeasure(&textheight, &textwidth, text, font1, font2, 0);
		x = context->x + (context->width - textwidth) / 2;
	}

	if (context->item->flags & MENUITEMFLAG_BIGFONT) {
		x += 35;
		y += 6;
	}

	gdl = text0f153628(gdl);
	gdl = textRenderProjected(gdl, &x, &y, text, font1, font2,
			leftcolour, context->width, context->height, 0, 0);

	if ((context->item->flags & (MENUITEMFLAG_LABEL_HASRIGHTTEXT | MENUITEMFLAG_BIGFONT)) == 0) {
		// Right side text
		text = menuResolveText(context->item->param3, context->item);

		// This is not how you check if a string is empty...
		if (text != NULL && text != "") {
			s32 textheight;
			s32 textwidth;

			y = context->y + 2;
			textMeasure(&textheight, &textwidth, text, font1, font2, 0);
			x = context->x + context->width - textwidth - 10;

			gdl = textRenderProjected(gdl, &x, &y, text, font1, font2,
					rightcolour, context->width, context->height, 0, 0);
		}
	}

	return text0f153780(gdl);
}

bool menuitemSelectableTick(struct menuitem *item, struct menuinputs *inputs, u32 tickflags)
{
	if ((tickflags & MENUTICKFLAG_ITEMISFOCUSED) && inputs->select) {
		menuPlaySound(MENUSOUND_SELECT);

		if (item->flags & MENUITEMFLAG_SELECTABLE_CLOSESDIALOG) {
			menuPopDialog();
		}

		if (item->flags & MENUITEMFLAG_SELECTABLE_OPENSDIALOG) {
			menuPushDialog((struct menudialogdef *)item->handler);
		} else if (item->handler) {
			union handlerdata data;
			item->handler(MENUOP_SET, item, &data);
		}
	}

	return true;
}

Gfx *menuitemSliderRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 colour;
	char *label;
	char buffer[16];
	s32 x;
	s32 y;
	s32 extray;
	s32 textwidth;
	s32 textheight;
	union handlerdata data;
	s32 slidervalue;
	s32 markerx;

	extray = 0;

	if (context->item->handler != NULL) {
		if ((context->item->flags & MENUITEMFLAG_SLIDER_DEFERRED) &&
			deferredindex != -1 &&
			context->dialog->dimmed &&
			context->focused) {
			slidervalue = (s16) deferredindex;
		} else {
			context->item->handler(MENUOP_GETSLIDER, context->item, &data);
			slidervalue = (s16) data.slider.value;
		}
	} else {
		slidervalue = 0;
	}

	if (context->dialog->unk6e != 0) {
		extray = 10;
	}

	if (context->item->flags & MENUITEMFLAG_SLIDER_ALTSIZE) {
		extray = 10;
	}

	x = context->x + 10;
	y = context->y + 2;

	if (context->item->flags & MENUITEMFLAG_LESSLEFTPADDING) {
		x -= 6;
	}

	label = menuResolveParam2Text(context->item);
	markerx = context->x + context->width + slidervalue * 75 / context->item->param3 - 82;

	colour = MIXCOLOUR(context->dialog, item_unfocused);

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
	}

	if (context->focused) {
		if (context->focused & 2) {
			u32 tmpcolour;
			u32 weight = menuGetSinOscFrac(40) * 255.0f;

			tmpcolour = MIXCOLOUR(context->dialog, item_focused_inner);

			colour = colourBlend(colour, colour & 0xff, 0x7f);
			colour = colourBlend(colour, tmpcolour, weight) | 0xff;
		}

		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_focused_inner, g_MenuWave1Colours[context->dialog->type].item_focused_inner);
	} else {
		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_unfocused, g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	gdl = menugfxRenderSlider(gdl, context->x + context->width - 82, context->y + extray + 5, context->x + context->width - 7, context->y + extray + 11, markerx, colour);

	colour = MIXCOLOUR(context->dialog, item_unfocused);

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
	}

	if (context->focused != 0) {
		if (context->focused & 2) {
			u32 tmpcolour;
			u32 weight = menuGetSinOscFrac(20) * 255.0f;
			tmpcolour = MIXCOLOUR(context->dialog, item_focused_inner);

			colour = colourBlend(colour, colour & 0xff, 0x7f);
			colour = colourBlend(colour, tmpcolour, weight);
		} else {
			u32 tmpcolour;
			u32 weight = menuGetSinOscFrac(40) * 255.0f;
			tmpcolour = MIXCOLOUR(context->dialog, item_focused_inner);

			colour = colourBlend(colour, colour & 0xff, 0x7f);
			colour = colourBlend(colour, tmpcolour, weight);
		}

		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_focused_inner, g_MenuWave1Colours[context->dialog->type].item_focused_inner);
	} else {
		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_unfocused, g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	gdl = text0f153628(gdl);
	gdl = textRenderProjected(gdl, &x, &y, label, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, context->width, context->height, 0, 0);

	if ((context->item->flags & MENUITEMFLAG_SLIDER_HIDEVALUE) == 0) {
		strcpy(buffer, "");
		sprintf(buffer, "%d\n", slidervalue);

		if (context->item->handler != NULL) {
			data.slider.value = slidervalue;
			data.slider.label = buffer;

			context->item->handler(MENUOP_GETSLIDERLABEL, context->item, &data);
		}

		textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		x = context->x + context->width - textwidth - 7;
		y = context->y + 2;

		colour = MIXCOLOUR(context->dialog, item_unfocused);

		if (context->dialog->dimmed) {
			colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
		}

		textSetWaveColours(g_MenuWave2Colours[context->dialog->type].item_unfocused, g_MenuWave1Colours[context->dialog->type].item_unfocused);

		colour = (colour & 0xffffff00) | ((colour & 0xff) >> 1);
		gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, context->width, context->height, 0, 0);
	}

	gdl = text0f153780(gdl);

	return gdl;
}

bool menuitemSliderTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	s32 index;
	union handlerdata handlerdata;
	s32 tmp;
	f32 f0;
	f32 f2;
	f32 f14;

	if ((tickflags & MENUTICKFLAG_ITEMISFOCUSED)) {
#ifndef PLATFORM_N64
		if (g_AllowMouseHeld && g_MenuUsingMouse && (inputs->select || inputs->mouseheld)) {
			// handle mouse
			struct menudialog *dialog = g_Menus[g_MpPlayerNum].curdialog;
			if (dialog) {
				const s32 left = dialog->x + dialog->width - 82;
				const s32 right = dialog->x + dialog->width - 7;
				const s32 size = right - left;
				const s32 delta = inputs->mousex - left;
				if (delta >= -8 && delta <= size + 8) {
					if ((tickflags & MENUTICKFLAG_DIALOGISDIMMED) == 0) {
						g_MouseDimmedMode = true;
					} else {
						index = (delta / (f32)size) * item->param3;
						if (index < 0) {
							index = 0;
						}
						if (index > item->param3) {
							index = item->param3;
						}
						if (item->handler) {
							if (item->flags & MENUITEMFLAG_SLIDER_DEFERRED) {
								deferredindex = index;
							} else {
								item->handler(MENUOP_GET, item, &handlerdata);
								handlerdata.slider.value = index;
								item->handler(MENUOP_SET, item, &handlerdata);
							}
						}
						return true;
					}
				}
			}
		}
#endif

		if (tickflags & MENUTICKFLAG_DIALOGISDIMMED) {
			if (item->handler) {
				if (item->flags & MENUITEMFLAG_SLIDER_DEFERRED) {
					if (deferredindex == -1) {
						item->handler(MENUOP_GETSLIDER, item, &handlerdata);
						deferredindex = (s16) handlerdata.slider.value;
					}
					index = deferredindex;
				} else {
					item->handler(MENUOP_GETSLIDER, item, &handlerdata);
					index = (s16) handlerdata.slider.value;
				}

			} else {
				index = 0;
			}

#ifndef PLATFORM_N64
			if (g_MenuUsingMouse && inputs->mousescroll) {
				if ((item->flags & MENUITEMFLAG_SLIDER_FAST) == 0) {
					index += -inputs->mousescroll;
				} else {
					s32 mult = item->param3 / 40;
					index += -inputs->mousescroll * (mult ? mult : 1);
				}
			}
#endif

			if ((item->flags & MENUITEMFLAG_SLIDER_FAST) == 0
					&& g_Menus[g_MpPlayerNum].xrepeatmode == MENUREPEATMODE_SLOW) {
				index = index + inputs->leftright;
			} else {
				f0 = data->slider.multiplier / 1000.0f;
				f0 = f0 * 100.0f / item->param3;
				f32 inputHeld;
				if (item->flags & MENUITEMFLAG_SLIDER_SLOW) {
					s32 holdTime = g_Menus[g_MpPlayerNum].xrepeattimer60;
					f32 scale;
					if (holdTime > TICKS(840)) {
						scale = 0.1f;  // 14s+: faster
					} else if (holdTime > TICKS(300)) {
						scale = 0.8f;  // 5-14s: medium
					} else {
						scale = 0.05f; // 0-5s: slow
					}
					inputHeld = inputs->leftrightheld * scale;
				} else {
					inputHeld = inputs->leftrightheld;
				}

#if VERSION >= VERSION_PAL_BETA
				f0 = f0 + inputHeld * g_Vars.diffframe60freal;
#else
				f0 = f0 + inputHeld * g_Vars.diffframe60;
#endif
				f0 = item->param3 * f0 / 100.0f;

				tmp = f0;
				f0 -= tmp;
				index += tmp;

				data->slider.multiplier = f0 * 1000.0f;
			}

			if (inputs->xaxis < 0) {
				f14 = inputs->xaxis;
				f2 = -f14;
			} else {
				f14 = inputs->xaxis;
				f2 = f14;
			}

			// Threshold for ramping: 40 for normal, 82 for SLIDER_SLOW
			s32 threshold = (item->flags & MENUITEMFLAG_SLIDER_SLOW) ? 82 : 40;

			if ((item->flags & MENUITEMFLAG_SLIDER_FAST) == 0 && f2 < threshold) {
				if (g_Menus[g_MpPlayerNum].xrepeatmode != MENUREPEATMODE_SLOW) {
					index = index + inputs->leftright;
				}
			} else {
				f0 = data->slider.multiplier / 1000.0f;
				f0 = f0 * 100.0f / item->param3;

				if (inputs->xaxis < 0) {
					f2 = -f14;
				} else {
					f2 = f14;
				}

				if (f2 > threshold) {
					f32 divisor = (item->flags & MENUITEMFLAG_SLIDER_SLOW) ? 40.0f : 16.0f;
					f2 = (f2 - threshold) / divisor;
#if VERSION >= VERSION_PAL_BETA
					f2 *= g_Vars.diffframe60freal;
#else
					f2 *= g_Vars.diffframe60f;
#endif

					if (inputs->xaxis < 0) {
						f0 -= f2;
					} else {
						f0 += f2;
					}
				}

				f0 = item->param3 * f0 / 100.0f;
				tmp = f0;
				f0 -= tmp;
				index += tmp;
				data->slider.multiplier = f0 * 1000.0f;
			}

			if (index < 0) {
				index = 0;
			}

			if (index > item->param3) {
				index = item->param3;
			}

			inputs->leftright = 0;
			handlerdata.slider.value = index;

			if (item->flags & MENUITEMFLAG_SLIDER_DEFERRED) {
				deferredindex = index;
			} else {
				if (item->handler) {
					item->handler(MENUOP_SET, item, &handlerdata);
				}
			}

			if (inputs->select || g_MouseEndDeferredSlider) {
				g_MouseEndDeferredSlider = false;
				if (item->flags & MENUITEMFLAG_SLIDER_DEFERRED) {
					deferredindex = -1;
					if (item->handler) {
						item->handler(MENUOP_SET, item, &handlerdata);
					}
				}
				dialog->dimmed = false;
			}
		} else {
			if (inputs->select) {
				dialog->dimmed = true;
			}
		}
	}

	return true;
}

void menuitemSliderInit(union menuitemdata *data)
{
	data->slider.multiplier = 0;
}

Gfx *menuitemCarouselRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 colour = 0xff0000ff;

	if (context->focused) {
		u32 colour1;
		u32 weight = menuGetSinOscFrac(40) * 255;

		if (context->dialog->transitionfrac < 0) {
			colour1 = g_MenuColours[context->dialog->type].item_focused_inner;
		} else {
			colour1 = colourBlend(
					g_MenuColours[context->dialog->type2].item_focused_inner,
					g_MenuColours[context->dialog->type].item_focused_inner,
					context->dialog->colourweight);
		}

		colour = colourBlend(colourBlend(colour, 0x000000ff, 127), colour1, weight);
	}

#ifdef PLATFORM_N64
	s16 chevronOffset = 0;
#else
	s16 chevronOffset = 3;
#endif

	// Left arrow
	gdl = menugfxDrawCarouselChevron(gdl, context->x + chevronOffset, context->y + context->height / 2, 8, 1, -1, colour);

	// Right arrow
	gdl = menugfxDrawCarouselChevron(gdl, context->x + context->width - chevronOffset, context->y + context->height / 2, 8, 3, -1, colour);

	// This part of the function is unused because param2 is always zero.
	// Setting it to 0x7b causes a crash.
	// 0x7c and 0x7d don't do anything with the option value, so it's probable
	// that the original source has a commented function call in that block.
	if (context->item->param2 == 0x7b && context->item->handler) {
		union handlerdata data;
		s32 headorbodynum = 0;

		context->item->handler(MENUOP_GETSELECTEDINDEX, context->item, &data);
		headorbodynum += data.carousel.value;

		gdl = func0f14f07c(gdl, headorbodynum,
				context->x + context->width / 2 - 32, context->y,
				context->x + context->width / 2 + 32, context->y + 64);
	} else if ((context->item->param2 == 0x7c || context->item->param2 == 0x7d) && context->item->handler) {
		union handlerdata data;
		context->item->handler(MENUOP_GETSELECTEDINDEX, context->item, &data);
	}

	return gdl;
}

bool menuitemCarouselTick(struct menuitem *item, struct menuinputs *inputs, u32 tickflags)
{
	union handlerdata data;
	s32 index;
	s32 numoptions;
	bool done;
	u32 stack;

	if (((tickflags & MENUTICKFLAG_ITEMISFOCUSED) || (item->flags & MENUITEMFLAG_CAROUSEL_04000000)) && item->handler) {
		if (inputs->leftright != 0) {
			if (mpIsPlayerLockedOut(g_MpPlayerNum) == 0 || (item->flags & MENUITEMFLAG_LOCKABLEMINOR) == 0) {
				done = false;

				item->handler(MENUOP_GETOPTIONCOUNT, item, &data);

				numoptions = data.carousel.value;
				item->handler(MENUOP_GETSELECTEDINDEX, item, &data);

				index = data.carousel.value;

				while (!done) {
					index = index + inputs->leftright;

					if (index >= numoptions) {
						index = 0;
					}

					if (index < 0) {
						index = numoptions - 1;
					}

					// Some kind of option-is-locked check?
					data.carousel.value = index;

					if (!item->handler(MENUOP_21, item, &data)) {
						done = true;
					}
				}

				data.carousel.value = index;
#if VERSION >= VERSION_NTSC_1_0
				data.carousel.unk04 = inputs->shoulder;
#endif

				item->handler(MENUOP_SET, item, &data);
			}
		}

		if (item->handler) {
			item->handler(MENUOP_11, item, &data);
		}
	}

	return true;
}

Gfx *menuitemCheckboxRender(Gfx *gdl, struct menurendercontext *context)
{
	u32 maincolour = 0x00000000;
	char *text;
	s32 x;
	s32 y;
	u8 data[3];
	bool checked = false;
	u32 fillcolour = 0xff002faf;
	struct font *font2 = g_FontHandelGothicSm;
	struct fontchar *font1 = g_CharsHandelGothicSm;

	if (context->item->flags & MENUITEMFLAG_SMALLFONT) {
		font2 = g_FontHandelGothicXs;
		font1 = g_CharsHandelGothicXs;
	}

	x = context->x;
	y = context->y;
	text = menuResolveParam2Text(context->item);
	data[0] = 0;

	if (context->item->handler
			&& context->item->handler(MENUOP_GET, context->item, (union handlerdata *)data) == true) {
		checked = true;

		if (context->dialog->transitionfrac < 0) {
			maincolour = g_MenuColours[context->dialog->type].checkbox_checked_unfocused;
		} else {
			maincolour = colourBlend(
					g_MenuColours[context->dialog->type2].checkbox_checked_unfocused,
					g_MenuColours[context->dialog->type].checkbox_checked_unfocused,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			maincolour = (colourBlend(maincolour, 0, 127) & 0xffffff00) | (maincolour & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].checkbox_checked_unfocused,
				g_MenuWave1Colours[context->dialog->type].checkbox_checked_unfocused);
	} else {
		if (context->dialog->transitionfrac < 0) {
			maincolour = g_MenuColours[context->dialog->type].item_unfocused;
		} else {
			maincolour = colourBlend(
					g_MenuColours[context->dialog->type2].item_unfocused,
					g_MenuColours[context->dialog->type].item_unfocused,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			maincolour = (colourBlend(maincolour, 0, 127) & 0xffffff00) | (maincolour & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_unfocused,
				g_MenuWave1Colours[context->dialog->type].item_unfocused);
	}

	gdl = text0f153628(gdl);

	if (context->focused) {
		// Mismatch: The addiu and lui at ec8fc and ec900 are swapped. The addiu
		// is adding the lower half of the address of g_MenuColours for
		// use in the else branch. The lui is loading a value into $at for the
		// float to u32 conversion for weight.
		u32 focuscolour;
		u32 weight = menuGetSinOscFrac(40) * 255; \
		if (context->dialog->transitionfrac < 0) { \
			focuscolour = g_MenuColours[context->dialog->type].item_focused_inner;
		} else {
			focuscolour = colourBlend(
					g_MenuColours[context->dialog->type2].item_focused_inner,
					g_MenuColours[context->dialog->type].item_focused_inner,
					context->dialog->colourweight);
		}

		maincolour = colourBlend(colourBlend(maincolour, maincolour & 0xff, 127), focuscolour, weight);

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_focused_inner,
				g_MenuWave1Colours[context->dialog->type].item_focused_inner);
	}

	if (menuIsItemDisabled(context->item, context->dialog)) {
		if (context->dialog->transitionfrac < 0) {
			maincolour = g_MenuColours[context->dialog->type].item_disabled;
		} else {
			maincolour = colourBlend(
					g_MenuColours[context->dialog->type2].item_disabled,
					g_MenuColours[context->dialog->type].item_disabled,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			maincolour = (colourBlend(maincolour, 0, 127) & 0xffffff00) | (maincolour & 0xff);
		}

		textSetWaveColours(
				g_MenuWave2Colours[context->dialog->type].item_disabled,
				g_MenuWave1Colours[context->dialog->type].item_disabled);

		fillcolour = 0x7f002faf;
	}

#if VERSION == VERSION_JPN_FINAL
	gdl = menugfxDrawCheckbox(gdl, context->x + context->width - 19, context->y + 2, 9, checked, maincolour, fillcolour);
#else
	gdl = menugfxDrawCheckbox(gdl, context->x + context->width - 16, context->y + 2, 6, checked, maincolour, fillcolour);
#endif

	x = context->x + 10;
	y = context->y + 2;
	gdl = textRenderProjected(gdl, &x, &y, text, font1, font2,
			maincolour, context->width, context->height, 0, 0);

	return text0f153780(gdl);
}

bool menuitemCheckboxTick(struct menuitem *item, struct menuinputs *inputs, u32 tickflags)
{
	union handlerdata data;

	if ((tickflags & MENUTICKFLAG_ITEMISFOCUSED) && inputs->select) {
		if (item->handler && item->handler(MENUOP_GET, item, &data) == 1) {
			data.checkbox.value = 0;
			menuPlaySound(MENUSOUND_TOGGLEOFF);
		} else {
			data.checkbox.value = 1;
			menuPlaySound(MENUSOUND_TOGGLEON);
		}

		if (item->handler) {
			item->handler(MENUOP_SET, item, &data);
		}
	}

	return true;
}

char *menuitemScrollableGetText(u32 type)
{
	switch (type) {
	case DESCRIPTION_MPCONFIG:
		return challengeGetConfigDescription(g_Menus[g_MpPlayerNum].training.mpconfig);
	case DESCRIPTION_MPCHALLENGE:
		if (!challengeIsLoaded()) {
			g_Menus[g_MpPlayerNum].menumodel.curparams = 0x4fac5ace;
			challengeLoadAndStoreCurrent(g_Menus[g_MpPlayerNum].menumodel.allocstart, g_Menus[g_MpPlayerNum].menumodel.alloclen);
		}
		return challengeGetCurrentDescription();
	case DESCRIPTION_CHRBIO:         return ciGetChrBioDescription();
	case DESCRIPTION_MISCBIO:        return ciGetMiscBioDescription();
	case DESCRIPTION_DEVICETRAINING: return dtGetDescription();
	case DESCRIPTION_FRWEAPON:       return frGetWeaponDescription();
	case DESCRIPTION_HANGARBIO:      return ciGetHangarBioDescription();
	case DESCRIPTION_HOLOTRAINING:   return htGetDescription();
	case DESCRIPTION_HOLOTIP1:       return htGetTip1();
	case DESCRIPTION_HOLOTIP2:       return htGetTip2();
	case DESCRIPTION_DEVICETIP1:     return dtGetTip1();
	case DESCRIPTION_DEVICETIP2:     return dtGetTip2();
	}

	return langGet(g_Briefing.briefingtextnum);
}

/**
 * Render a scrollable menu item, such as briefing paragraphs and challenge
 * descriptions.
 *
 * The text returned from menuitemScrollableGetText is first wrapped to fit the
 * width of the dialog. It is then separated into two strings:
 *
 * headings - which are started with the pipe character and ended with a
 * double line break
 * body text - which is anything else
 *
 * Headings are rendered in red, with a dropshadow and are outdented.
 */
Gfx *menuitemScrollableRender(Gfx *gdl, struct menurendercontext *context)
{
	char alltext[8000] = "";
	char headingtext[8000];
	char bodytext[8000];
	bool prevwaslinebreak;
	char *streams[2];
	char *inptr;
	s32 inheading;
	struct menuitemdata_scrollable *data = &context->data->scrollable;
	u32 colour;
	s32 x;
	s32 y;
	char *rawtext;
	s32 paddingright = 24;

	rawtext = menuitemScrollableGetText(context->item->param);

	if (menuIsScrollableUnscrollable(context->item)) {
		paddingright = 10;
	}

	if (rawtext) {
		textWrap(context->width - paddingright, rawtext, alltext, g_CharsHandelGothicSm, g_FontHandelGothicSm);
	}

	inptr = alltext;
	streams[1] = headingtext;
	streams[0] = bodytext;
	inheading = 0;
	prevwaslinebreak = false;

	while (*inptr != '\0') {
		if (*inptr == '|') {
			// Pipe character - don't append it to either string,
			// but switch the active string to the heading one
			inheading = 1;
		} else if (*inptr == '\n') {
			// Copy line breaks to both strings
			*streams[0] = *inptr;
			streams[0]++;
			*streams[1] = *inptr;
			streams[1]++;

			if (prevwaslinebreak) {
				// Got two line breaks in a row, so switch the active string
				// back to the body one
				inheading = 0;
			}

			prevwaslinebreak = true;
		} else {
			// Append to whichever string is the active one
			prevwaslinebreak = false;
			*streams[inheading] = *inptr;
			streams[inheading]++;
		}

		inptr++;
	}

	*streams[0] = '\0';
	*streams[1] = '\0';

	x = context->x + 2;
	y = context->y + 2;

	if (context->dialog->transitionfrac < 0) {
		colour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		colour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 0x7f) & 0xffffff00) | (colour & 0xff);
	}

	textSetWaveColours(
			g_MenuWave2Colours[context->dialog->type].item_unfocused,
			g_MenuWave1Colours[context->dialog->type].item_unfocused);

	gdl = text0f153628(gdl);

	// Heading text shadow
	x = context->x + 3;
	y = context->y + 3;
	gdl = textRenderProjected(gdl, &x, &y, headingtext, g_CharsHandelGothicSm, g_FontHandelGothicSm,
			0x000000ff, context->width - 4, context->height - 4, -data->scrolloffset, 0);

	// Heading text (red)
	x = context->x + 2;
	y = context->y + 2;
	gdl = textRenderProjected(gdl, &x, &y, headingtext, g_CharsHandelGothicSm, g_FontHandelGothicSm,
			0xff4444ff, context->width - 4, context->height - 4, -data->scrolloffset, 0);

	// Body text
	x = menuIsScrollableUnscrollable(context->item) ? context->x + 5 : context->x + 12;
	y = context->y + 2;

	gdl = textRenderProjected(gdl, &x, &y, bodytext, g_CharsHandelGothicSm, g_FontHandelGothicSm,
			colour, context->width - 4, context->height - 1, -data->scrolloffset, 0);

	return text0f153780(gdl);
}

bool menuitemScrollableTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	u32 stack;

#if VERSION >= VERSION_PAL_BETA
	if ((s16)dialog->height != data->scrollable.dialogheight || data->scrollable.language != g_LanguageId) {
#else
	if ((s16)dialog->height != data->scrollable.dialogheight) {
#endif
		char wrapped[8000] = "";
		char *rawtext;
		s32 width;
		s32 height;
		s32 rowindex;
		s32 colindex;
		s16 colwidth;
		s16 rowheight;

#if VERSION >= VERSION_PAL_BETA
		data->scrollable.language = g_LanguageId;
#endif

		dialogFindItem(dialog, item, &rowindex, &colindex);

		colwidth = g_Menus[g_MpPlayerNum].cols[colindex].width;
		rowheight = g_Menus[g_MpPlayerNum].rows[rowindex].height;

		if (menuIsScrollableUnscrollable(item)) {
			width = colwidth - 10;
		} else {
			width = colwidth - 24;
		}

		rawtext = menuitemScrollableGetText(item->param);

		if (rawtext) {
			textWrap(width, rawtext, wrapped, g_CharsHandelGothicSm, g_FontHandelGothicSm);
		}

		textMeasure(&height, &width, wrapped, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		data->scrollable.maxscrolloffset = height - rowheight + 5;

		if (data->scrollable.maxscrolloffset < -10) {
			data->scrollable.maxscrolloffset = -10;
		}

		data->scrollable.dialogheight = dialog->height;
	}

	if (menuIsScrollableUnscrollable(item)) {
		data->scrollable.scrolloffset = 0;
	} else if (tickflags & MENUTICKFLAG_ITEMISFOCUSED) {
		f32 floatval;
		s32 intval = 0;

		if (inputs->yaxis < 0) {
			floatval = -(f32)inputs->yaxis;
		} else {
			floatval = inputs->yaxis;
		}

		if (floatval > 20) {
			floatval = (floatval - 20) / 5;
#if VERSION >= VERSION_PAL_BETA
			floatval *= g_Vars.diffframe60freal;
#else
			floatval *= g_Vars.diffframe60f;
#endif

			if (inputs->yaxis < 0) {
				intval = floatval;
			} else {
				intval = -(s32)floatval;
			}
		}

#if VERSION >= VERSION_PAL_BETA
		intval = intval + (s32)(((f32)inputs->updownheld + (f32)inputs->updownheld) * g_Vars.diffframe60freal);
#else
		intval += inputs->updownheld * 2 * g_Vars.diffframe60;
#endif
#ifndef PLATFORM_N64
		intval += inputs->mousescroll * (LINEHEIGHT - 2) * 3;
#endif
		data->scrollable.scrolloffset += intval;

		if (data->scrollable.scrolloffset < -10) {
			data->scrollable.scrolloffset = -10;
		}

		if (data->scrollable.scrolloffset > data->scrollable.maxscrolloffset) {
			data->scrollable.scrolloffset = data->scrollable.maxscrolloffset;
		}
	}

	return true;
}

void menuitemScrollableInit(union menuitemdata *data)
{
	data->scrollable.dialogheight = -1;
	data->scrollable.scrolloffset = -10;
}

Gfx *menuitemMarqueeRender(Gfx *gdl, struct menurendercontext *context)
{
	struct menuitemdata_marquee *data = &context->data->marquee;
	char *text = menuResolveParam2Text(context->item);
	s32 x;
	s32 y;
	u32 colour;
	s32 i = 0;
	s32 textwidth;
	s32 textheight;
#if VERSION == VERSION_JPN_FINAL
	char string[] = {' ', '\0', '\0'};
#else
	char string[] = {' ', '\0'};
#endif
	bool hasmore;
	struct font *font2;
	struct fontchar *font1;
	s32 zero;

#if VERSION == VERSION_JPN_FINAL
	font1 = g_CharsHandelGothicMd;
	font2 = g_FontHandelGothicMd;
#else
	font2 = g_FontHandelGothicSm;
	font1 = g_CharsHandelGothicSm;
#endif
	zero = 0;

#if VERSION != VERSION_JPN_FINAL
	if (context->item->flags & MENUITEMFLAG_SMALLFONT) {
		font2 = g_FontHandelGothicXs;
		font1 = g_CharsHandelGothicXs;
	}
#endif

	if (!text) {
		return gdl;
	}

	if (context->dialog->transitionfrac < 0) {
		colour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		colour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		colour = (colourBlend(colour, 0, 127) & 0xffffff00) | (colour & 0xff);
	}

	x = context->x + context->width - data->totalmoved;
	y = context->y + 2;
	hasmore = true;

	while (hasmore == true) {
		if (text[i] == '\0') {
			hasmore = false;
		} else {
			string[0] = text[i];

#if VERSION == VERSION_JPN_FINAL
			if (string[0] < 0x80) {
				string[1] = '\0';
			} else {
				string[1] = text[i + 1];

				if ((string[0] & 0x7f) == 0x3e) {
					string[0]--;
				}

				if ((string[0] & 0x7f) == 0x04) {
					string[0]--;
				}
			}
#endif

			textMeasure(&textheight, &textwidth, string, font1, font2, 0);

			if (x + textwidth > context->x) {
				hasmore = false;
			} else {
				hasmore = true;
				x += textwidth;
				i++;

#if VERSION == VERSION_JPN_FINAL
				if (string[1] != '\0') {
					i++;
				}
#endif
			}
		}

		textwidth += zero;
	}

#if VERSION >= VERSION_NTSC_1_0
	g_ScissorX1 = context->x * g_ScaleX;
	g_ScissorX2 = (context->x + context->width) * g_ScaleX;
	g_ScissorY1 = context->y;
	g_ScissorY2 = context->y + context->height - 1;

	if (g_ScissorX1 < 0) {
		g_ScissorX1 = 0;
	}

	if (g_ScissorX2 < 0) {
		g_ScissorX2 = 0;
	}

	if (g_ScissorY1 < 0) {
		g_ScissorY1 = 0;
	}

	if (g_ScissorY2 < 0) {
		g_ScissorY2 = 0;
	}

	if (g_ScissorX1 > viGetBufWidth()) {
		g_ScissorX1 = viGetBufWidth();
	}

	if (g_ScissorX2 > viGetBufWidth()) {
		g_ScissorX2 = viGetBufWidth();
	}

	if (g_ScissorY1 > viGetBufHeight()) {
		g_ScissorY1 = viGetBufHeight();
	}

	if (g_ScissorY2 > viGetBufHeight()) {
		g_ScissorY2 = viGetBufHeight();
	}

	if (g_ScissorX2 < g_ScissorX1) {
		g_ScissorX2 = g_ScissorX1;
	}

	if (g_ScissorY2 < g_ScissorY1) {
		g_ScissorY2 = g_ScissorY1;
	}

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE,
			context->x * g_ScaleX, context->y,
			(context->x + context->width) * g_ScaleX, context->y + context->height - 1);
#endif

	textBackupAndResetBlends();

	if (context->item->flags & MENUITEMFLAG_MARQUEE_FADEBOTHSIDES) {
#if VERSION == VERSION_JPN_FINAL
		textSetHorizontalBlend(context->x, context->x + context->width, 28);
#else
		textSetHorizontalBlend(context->x, context->x + context->width, 14);
#endif
	} else {
		textSetHorizontalBlend(context->x, context->x, 14);
	}

	gdl = text0f153628(gdl);
	gdl = textRenderProjected(gdl, &x, &y, &text[i], font1, font2, colour,
			context->width + context->x - x, context->height, 0, 0);
	gdl = text0f153780(gdl);
	gdl = menuApplyScissor(gdl);

	textRestoreBlends();

	data->viewwidth = context->width;

	return gdl;
}

u32 var800711f0 = 0x00000002;

bool menuitemMarqueeTick(struct menuitem *item, union menuitemdata *data)
{
	s32 i;
	s32 textheight;
	s32 textwidth;
	struct fontchar *font1;
	struct font *font2;
	s32 increment;
	u16 texthash = 0;
	char *text = menuResolveParam2Text(item);
	s32 limit;

#if VERSION == VERSION_JPN_FINAL
	font1 = g_CharsHandelGothicMd;
	font2 = g_FontHandelGothicMd;
#else
	font2 = g_FontHandelGothicSm;
	font1 = g_CharsHandelGothicSm;

	if (item->flags & MENUITEMFLAG_SMALLFONT) {
		font2 = g_FontHandelGothicXs;
		font1 = g_CharsHandelGothicXs;
	}
#endif

	if (!text) {
		return true;
	}

	i = 0;

	while (text[i] != '\0') {
		texthash += text[i];
		i++;
	}

	if (data->marquee.texthash != texthash) {
		data->marquee.totalmoved = 0;
		data->marquee.texthash = texthash;
	}

	textMeasure(&textheight, &textwidth, text, font1, font2, 0);
	limit = data->marquee.viewwidth + textwidth;
	increment = g_Vars.diffframe60 / 2;

	if (increment <= 0) {
		increment = 1;
	}

	data->marquee.totalmoved += increment;

	if (data->marquee.totalmoved > limit) {
		data->marquee.totalmoved = 0;
	}

	return true;
}

void menuitemMarqueeInit(union menuitemdata *data)
{
	data->marquee.totalmoved = 0;
	data->marquee.viewwidth = 50;
	data->marquee.unk06 = 0;
}

Gfx *menuitem07Render(Gfx *gdl)
{
	return gdl;
}

Gfx *menuitemRankingRender(Gfx *gdl, struct menurendercontext *context)
{
	struct ranking rankings[MAX_MPCHRS];
	s32 numrows;
	u32 textcolour;
	s32 x;
	s32 y;
	u32 linecolour2; // ultimately unused
	s32 textwidth;
	s32 textheight;
	s32 team = false;
	u32 linecolour1;
	struct menuitemdata_ranking *data = &context->data->ranking;
	struct menudialog *dialog;
	s32 gap;
	s32 i;

	if (context->item->param2 == 1) {
		team = true;
		numrows = mpGetTeamRankings(rankings);
	} else {
		numrows = mpGetPlayerRankings(rankings);
	}

	gdl = text0f153628(gdl);

	// Gap from last item to bottom of dialog + header height
#if VERSION >= VERSION_JPN_FINAL
	gap = numrows * (LINEHEIGHT - 1) - context->height + 22;
#else
	gap = numrows * (LINEHEIGHT - 1) - context->height + 18;
#endif

	if (gap < 0) {
		gap = 0;
	}

	if (data->scrolloffset > gap) {
		data->scrolloffset = gap;
	}

	if (context->dialog->transitionfrac < 0) {
		textcolour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		textcolour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		textcolour = (colourBlend(textcolour, 0, 127) & 0xffffff00) | (textcolour & 0xff);
	}

	if (!team) {
		// "Deaths"
		textMeasure(&textheight, &textwidth, langGet(L_MPMENU_277), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		x = (context->x - textwidth) + 91;
		y = context->y + 1;
		gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_277), g_CharsHandelGothicXs, g_FontHandelGothicXs,
				textcolour, context->width, context->height, 0, 0);
	}

	// "Score"
	textMeasure(&textheight, &textwidth, langGet(L_MPMENU_278), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = (context->x - textwidth) + 120;
	y = context->y + 1;
	gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_278), g_CharsHandelGothicXs, g_FontHandelGothicXs,
			textcolour, context->width, context->height, 0, 0);
	gdl = text0f153780(gdl);

	dialog = context->dialog;

	if (dialog->transitionfrac < 0) {
		linecolour1 = g_MenuColours[dialog->type].dialog_border1;
	} else {
		linecolour1 = colourBlend(
				g_MenuColours[dialog->type2].dialog_border1,
				g_MenuColours[dialog->type].dialog_border1,
				dialog->colourweight);
	}

	if (dialog->dimmed) {
		linecolour1 = (colourBlend(linecolour1, 0, 44) & 0xffffff00) | (linecolour1 & 0xff);
	}

	if (dialog->transitionfrac < 0) {
		linecolour2 = g_MenuColours[dialog->type].dialog_border2;
	} else {
		linecolour2 = colourBlend(
				g_MenuColours[dialog->type2].dialog_border2,
				g_MenuColours[dialog->type].dialog_border2,
				dialog->colourweight);
	}

	if (dialog->dimmed) {
		linecolour2 = (colourBlend(linecolour2, 0, 44) & 0xffffff00) | (linecolour2 & 0xff);
	}

	linecolour1 = (textApplyProjectionColour(context->x, context->y + 2, -129) & 0xff) | (linecolour1 & 0xffffff00);
	linecolour2 = (textApplyProjectionColour(context->x + context->width, context->y + 2, -129) & 0xff) | (linecolour2 & 0xffffff00);

	// Horizontal line between header and body
#if VERSION == VERSION_JPN_FINAL
	gdl = menugfxDrawFilledRect(gdl, context->x, context->y + 13, context->x + context->width, context->y + 14, linecolour1, linecolour1);
#else
	gdl = menugfxDrawFilledRect(gdl, context->x, context->y + 9, context->x + context->width, context->y + 10, linecolour1, linecolour1);
#endif

	gDPPipeSync(gdl++);

#if VERSION >= VERSION_NTSC_1_0
	g_ScissorX1 = context->x * g_ScaleX;
	g_ScissorX2 = (context->x + context->width) * g_ScaleX;
	g_ScissorY1 = context->y + (VERSION == VERSION_JPN_FINAL ? 14 : 10);
	g_ScissorY2 = context->y + context->height - 1;

	if (g_ScissorX1 < 0) {
		g_ScissorX1 = 0;
	}

	if (g_ScissorX2 < 0) {
		g_ScissorX2 = 0;
	}

	if (g_ScissorY1 < 0) {
		g_ScissorY1 = 0;
	}

	if (g_ScissorY2 < 0) {
		g_ScissorY2 = 0;
	}

	if (g_ScissorX1 > viGetBufWidth()) {
		g_ScissorX1 = viGetBufWidth();
	}

	if (g_ScissorX2 > viGetBufWidth()) {
		g_ScissorX2 = viGetBufWidth();
	}

	if (g_ScissorY1 > viGetBufHeight()) {
		g_ScissorY1 = viGetBufHeight();
	}

	if (g_ScissorY2 > viGetBufHeight()) {
		g_ScissorY2 = viGetBufHeight();
	}

	if (g_ScissorX2 < g_ScissorX1) {
		g_ScissorX2 = g_ScissorX1;
	}

	if (g_ScissorY2 < g_ScissorY1) {
		g_ScissorY2 = g_ScissorY1;
	}

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE,
			context->x * g_ScaleX,
			context->y + 10,
			(context->x + context->width) * g_ScaleX,
			context->y + context->height - 1);
#endif

	gdl = text0f153628(gdl);

	for (i = 0; i < numrows; i++) {
		struct ranking *ranking = &rankings[i];
		u32 weight = 0;
		char valuebuffer[8];

#if VERSION < VERSION_NTSC_1_0
		u32 stack;
#endif

		if (numrows >= 2) {
			weight = i / (f32)(numrows - 1) * 255;
		}

		textcolour = colourBlend(0x008888ff, 0x00ffffff, weight);
		x = context->x + 5;
#if VERSION >= VERSION_JPN_FINAL
		y = context->y + i * 13 - data->scrolloffset + 18;
#else
		y = context->y + i * 10 - data->scrolloffset + 14;
#endif

		if (team) {
			gdl = textRenderProjected(gdl, &x, &y, g_BossFile.teamnames[ranking->teamnum],
					g_CharsHandelGothicSm, g_FontHandelGothicSm, textcolour, context->width, context->height, 0, 0);
		} else {
			gdl = textRenderProjected(gdl, &x, &y, ranking->mpchr->name,
					g_CharsHandelGothicSm, g_FontHandelGothicSm, textcolour, context->width, context->height, 0, 0);
		}

		if (!team) {
			// Deaths value (red)
			textcolour = colourBlend(0xcf0000ff, 0xff4040ff, weight);
			sprintf(valuebuffer, "%d\n", ranking->mpchr->numdeaths);
			textMeasure(&textheight, &textwidth, valuebuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
			x = context->x - textwidth + 91;
#if VERSION >= VERSION_JPN_FINAL
			y = context->y + i * 13 - data->scrolloffset + 18;
#else
			y = context->y + i * 10 - data->scrolloffset + 14;
#endif
			gdl = textRenderProjected(gdl, &x, &y, valuebuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
					textcolour, context->width, context->height, 0, 0);
		}

		// Score value (green)
		textcolour = colourBlend(0x009f00ff, 0x00ff00ff, weight);
		sprintf(valuebuffer, "%d\n", ranking->score);
		textMeasure(&textheight, &textwidth, valuebuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = context->x - textwidth + 120;
#if VERSION >= VERSION_JPN_FINAL
		y = context->y + i * 13 - data->scrolloffset + 18;
#else
		y = context->y + i * 10 - data->scrolloffset + 14;
#endif
		gdl = textRenderProjected(gdl, &x, &y, valuebuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				textcolour, context->width, context->height, 0, 0);
	}

	return text0f153780(gdl);
}

bool menuitemRankingTick(struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	f32 floatval;
	s32 intval;

	if (tickflags & MENUTICKFLAG_ITEMISFOCUSED) {
		intval = 0;
		floatval = inputs->yaxis < 0 ? -(f32)inputs->yaxis : inputs->yaxis;

		if (floatval > 20) {
			floatval = (floatval - 20) / 5;
#if VERSION >= VERSION_PAL_BETA
			floatval *= g_Vars.diffframe60freal;
#else
			floatval *= g_Vars.diffframe60f;
#endif

			intval = inputs->yaxis < 0 ? (s32)floatval : -(s32)floatval;
		}

#if VERSION >= VERSION_PAL_BETA
		intval = intval + (s32)(((f32)inputs->updownheld + (f32)inputs->updownheld) * g_Vars.diffframe60freal);
#else
		intval += inputs->updownheld * 2 * g_Vars.diffframe60;
#endif
#ifndef PLATFORM_N64
		intval += inputs->mousescroll * (LINEHEIGHT - 1);
#endif
		data->ranking.scrolloffset += intval;

		if (data->ranking.scrolloffset < 0) {
			data->ranking.scrolloffset = 0;
		}
	}

	return true;
}

void menuitemRankingInit(union menuitemdata *data)
{
	data->ranking.scrolloffset = 0;
}

Gfx *menuitemPlayerStatsRender(Gfx *gdl, struct menurendercontext *context)
{
	struct menuitemdata_dropdown *data = &context->data->dropdown;
	s32 x;
	s32 y;
	u32 maincolour;
	s32 playernum = g_MpSelectedPlayersForStats[g_MpPlayerNum];
	struct mpchrconfig *mpchr;
	u32 selectioncolour;
	s32 textheight;
	s32 textwidth;
	s32 numchrs = mpGetNumChrs();
	char buffer[12];
	s32 i;
	u32 weight;
	s32 gap;
	s32 ypos = 0;

	mpchr = MPCHR(playernum);

	gdl = text0f153628(gdl);

	// Write selected player's name
	weight = menuGetSinOscFrac(40) * 255;

	if (context->dialog->transitionfrac < 0) {
		selectioncolour = g_MenuColours[context->dialog->type].item_focused_inner;
	} else {
		selectioncolour = colourBlend(
				g_MenuColours[context->dialog->type2].item_focused_inner,
				g_MenuColours[context->dialog->type].item_focused_inner,
				context->dialog->colourweight);
	}

	selectioncolour = colourBlend(colourBlend(0xffffffff, 0x000000ff, 127), selectioncolour, weight);
	x = context->x + 2;
	y = context->y + 1;

	gdl = textRenderProjected(gdl, &x, &y, mpchr->name, g_CharsHandelGothicSm, g_FontHandelGothicSm,
			selectioncolour, context->width, context->height, 0, 0);

	// "Suicides" heading
	if (context->dialog->transitionfrac < 0) {
		maincolour = g_MenuColours[context->dialog->type].item_unfocused;
	} else {
		maincolour = colourBlend(
				g_MenuColours[context->dialog->type2].item_unfocused,
				g_MenuColours[context->dialog->type].item_unfocused,
				context->dialog->colourweight);
	}

	if (context->dialog->dimmed) {
		maincolour = (colourBlend(maincolour, 0x00000000, 127) & 0xffffff00) | (maincolour & 0xff);
	}

	textMeasure(&textheight, &textwidth, langGet(L_MPMENU_281), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
#if VERSION >= VERSION_NTSC_1_0
	x = context->x - textwidth + 121;
#else
	x = context->x + 56;
#endif
	y = context->y + 1;

	gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_281), g_CharsHandelGothicXs, g_FontHandelGothicXs,
			maincolour, context->width, context->height, 0, 0);

	// Num suicides
#if VERSION >= VERSION_NTSC_1_0
	x = context->x - textwidth + 119;
#endif
	sprintf(buffer, "%d\n", mpchr->killcounts[playernum]);
	textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
#if VERSION >= VERSION_NTSC_1_0
	x -= textwidth;
#else
	x = context->x + 95;
#endif
	y = context->y + 1;

#if VERSION == VERSION_JPN_FINAL
	y++;
#endif

	gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
			0xffff00ff, context->width, context->height, 0, 0);

	// Move ypos past top row
#if VERSION >= VERSION_JPN_FINAL
	ypos += 14;
#else
	ypos += 12;
#endif

	if (mpGetNumChrs() >= 2) {
		if (context->dialog->transitionfrac < 0) {
			maincolour = g_MenuColours[context->dialog->type].item_unfocused;
		} else {
			maincolour = colourBlend(
					g_MenuColours[context->dialog->type2].item_unfocused,
					g_MenuColours[context->dialog->type].item_unfocused,
					context->dialog->colourweight);
		}

		if (context->dialog->dimmed) {
			maincolour = (colourBlend(maincolour, 0, 127) & 0xffffff00) | (maincolour & 0xff);
		}

		// "Deaths" heading
		textMeasure(&textheight, &textwidth, langGet(L_MPMENU_282), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		x = context->x - textwidth + 120;
		y = context->y + ypos;
		gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_282), g_CharsHandelGothicXs, g_FontHandelGothicXs,
				maincolour, context->width, context->height, 0, 0);

		// "Kills" heading
		textMeasure(&textheight, &textwidth, langGet(L_MPMENU_283), g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

#if VERSION >= VERSION_JPN_FINAL
		x = context->x + 4;
#else
		x = context->x - textwidth + 25;
#endif

		y = context->y + ypos;
		gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPMENU_283),  g_CharsHandelGothicXs, g_FontHandelGothicXs,
				maincolour, context->width, context->height, 0, 0);

		// Move ypos past heading row
#if VERSION >= VERSION_JPN_FINAL
		ypos += 12;
#else
		ypos += 7;
#endif

		// Prepare scissor for table
		gap = numchrs * (LINEHEIGHT - 1) - context->height + ypos;
#if VERSION >= VERSION_JPN_FINAL
		gap -= 13;
#else
		gap -= 10;
#endif

		if (gap < 0) {
			gap = 0;
		}

		if (data->scrolloffset > gap) {
			data->scrolloffset = gap;
		}

		gDPPipeSync(gdl++);

#if VERSION >= VERSION_NTSC_1_0
		g_ScissorX1 = context->x * g_ScaleX;
		g_ScissorX2 = (context->x + context->width) * g_ScaleX;
		g_ScissorY1 = context->y + ypos;
		g_ScissorY2 = context->y + context->height;

		if (g_ScissorX1 < 0) {
			g_ScissorX1 = 0;
		}

		if (g_ScissorX2 < 0) {
			g_ScissorX2 = 0;
		}

		if (g_ScissorY1 < 0) {
			g_ScissorY1 = 0;
		}

		if (g_ScissorY2 < 0) {
			g_ScissorY2 = 0;
		}

		if (g_ScissorX1 > viGetBufWidth()) {
			g_ScissorX1 = viGetBufWidth();
		}

		if (g_ScissorX2 > viGetBufWidth()) {
			g_ScissorX2 = viGetBufWidth();
		}

		if (g_ScissorY1 > viGetBufHeight()) {
			g_ScissorY1 = viGetBufHeight();
		}

		if (g_ScissorY2 > viGetBufHeight()) {
			g_ScissorY2 = viGetBufHeight();
		}

		if (g_ScissorX2 < g_ScissorX1) {
			g_ScissorX2 = g_ScissorX1;
		}

		if (g_ScissorY2 < g_ScissorY1) {
			g_ScissorY2 = g_ScissorY1;
		}

		gDPSetScissor(gdl++, G_SC_NON_INTERLACE, g_ScissorX1, g_ScissorY1, g_ScissorX2, g_ScissorY2);
#else
		gDPSetScissor(gdl++, G_SC_NON_INTERLACE,
				context->x * g_ScaleX,
				context->y + ypos,
				(context->x + context->width) * g_ScaleX,
				context->y + context->height);
#endif

		ypos -= data->scrolloffset;

		for (i = 0; i < MAX_MPCHRS; i++) {
			if (g_MpSetup.chrslots & (1 << i)) {
				struct mpchrconfig *loopmpchr = MPCHR(i);

				if (i != playernum) {
					// Name
					x = context->x + 29;
					y = context->y + ypos;
					gdl = textRenderProjected(gdl, &x, &y, loopmpchr->name, g_CharsHandelGothicSm, g_FontHandelGothicSm,
							0x00ffffff, context->width, context->height, 0, 0);

					// Num deaths
					sprintf(buffer, "%d\n", loopmpchr->killcounts[playernum]);
					textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
					x = context->x - textwidth + 120;
					y = context->y + ypos;
					gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
							0xff4040ff, context->width, context->height, 0, 0);

					// Num kills
					sprintf(buffer, "%d\n", mpchr->killcounts[i]);
					textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

#if VERSION == VERSION_JPN_FINAL
					x = context->x + 4;
#else
					x = context->x - textwidth + 25;
#endif

					y = context->y + ypos;
					gdl = textRenderProjected(gdl, &x, &y, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
							0x00ff00ff, context->width, context->height, 0, 0);

#if VERSION == VERSION_JPN_FINAL
					ypos += 13;
#else
					ypos += 10;
#endif
				}
			}
		}
	}

	return text0f153780(gdl);
}

bool menuitemPlayerStatsTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	f32 floatval;
	s32 intval;

	if ((tickflags & MENUTICKFLAG_ITEMISFOCUSED) && !dialog->dimmed) {
		intval = 0;
		floatval = inputs->yaxis < 0 ? -(f32)inputs->yaxis : inputs->yaxis;

		if (floatval > 20) {
			floatval = (floatval - 20) / 5;
#if VERSION >= VERSION_PAL_BETA
			floatval *= g_Vars.diffframe60freal;
#else
			floatval *= g_Vars.diffframe60f;
#endif

			intval = inputs->yaxis < 0 ? (s32)floatval : -(s32)floatval;
		}

#if VERSION >= VERSION_PAL_BETA
		intval = intval + (s32)(((f32)inputs->updownheld + (f32)inputs->updownheld) * g_Vars.diffframe60freal);
#else
		intval += inputs->updownheld * 2 * g_Vars.diffframe60;
#endif
#ifndef PLATFORM_N64
		intval += inputs->mousescroll * (LINEHEIGHT - 1);
#endif
		data->dropdown.scrolloffset += intval;

		if (data->dropdown.scrolloffset < 0) {
			data->dropdown.scrolloffset = 0;
		}
	}

	return menuitemDropdownTick(item, dialog, inputs, tickflags, data);
}

Gfx *menuitemPlayerStatsOverlay(Gfx *gdl, s16 x, s16 y, s16 x2, s16 y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data)
{
#if VERSION >= VERSION_NTSC_1_0
	return menuitemDropdownOverlay(gdl, x + 1, y, -1, y2, item, dialog, data);
#else
	return menuitemDropdownOverlay(gdl, x, y, 56, y2, item, dialog, data);
#endif
}

void menuitemPlayerStatsInit(struct menuitem *item, union menuitemdata *data)
{
	data->dropdown.scrolloffset = 0;
	g_MpSelectedPlayersForStats[g_MpPlayerNum] = g_MpPlayerNum;

	menuitemDropdownInit(item, data);
}

Gfx *menuitemControllerRenderLine(Gfx *gdl, s32 speed, s32 x1, s32 y1, s32 x2, s32 y2)
{
	speed = speed + (x1 % 4);

	gSPTextureRectangle(gdl++,
			x1 * 4 * g_ScaleX, y1 * 4,
			x2 * 4 * g_ScaleX, y2 * 4,
			G_TX_RENDERTILE,
			speed * 32,
			(y1 % 4) * 32,
			1024 / g_ScaleX, 1024);

	return gdl;
}

Gfx *menuitemControllerRenderTexture(Gfx *gdl, s32 x, s32 y, s32 texturenum, u32 alpha)
{
	gDPPipeSync(gdl++);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);

	texSelect(&gdl, &g_TexGeneralConfigs[texturenum], 2, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);

	gDPSetCombineLERP(gdl++,
			TEXEL0, 0, ENVIRONMENT, 0,
			TEXEL0, 0, ENVIRONMENT, 0,
			TEXEL0, 0, ENVIRONMENT, 0,
			TEXEL0, 0, ENVIRONMENT, 0);

	gDPSetColor(gdl++, G_SETENVCOLOR, 0xffffff00 | alpha);

	gSPTextureRectangle(gdl++,
			(x << 2) * g_ScaleX, y << 2,
			((x + 32) << 2) * g_ScaleX, (y + 32) << 2,
			0, 16, 1008, 1024 / g_ScaleX, 0xfc00);

	return gdl;
}

struct lineconfig {
	s32 x1;
	s32 y1;
	s32 x2;
	s32 y2;
};

Gfx *menuitemControllerRenderLines(Gfx *gdl, struct menurendercontext *context, s32 firstindex, s32 lastindex, s32 padx, s32 pady, u32 alpha)
{
	s32 speed;
	s32 i;

	static struct lineconfig linecfgs[] = {
#if VERSION == VERSION_JPN_FINAL
		/* 0*/ { 12, 4,   -1, 31  },
		/* 1*/ { 51, 4,   -1, 31  },
		/* 2*/ { 12, 4,   75, -1  },
		/* 3*/ { 55, 40,  64, -1  },
		/* 4*/ { 63, 40,  -1, 16  },
		/* 5*/ { 63, 16,  75, -1  },
		/* 6*/ { 51, 44,  55, -1  },
		/* 7*/ { 59, 44,  68, -1  },
		/* 8*/ { 67, 44,  -1, 28  },
		/* 9*/ { 67, 28,  75, -1  },
		/*10*/ { 55, 48,  72, -1  },
		/*11*/ { 71, 48,  -1, 40  },
		/*12*/ { 71, 40,  75, -1  },
		/*13*/ { 49, 52,  75, -1  },
		/*14*/ { 43, 49,  -1, 64  },
		/*15*/ { 43, 64,  75, -1  },
		/*16*/ { 30, 56,  37, -1  },
		/*17*/ { 37, 56,  -1, 76  },
		/*18*/ { 37, 76,  75, -1  },
		/*19*/ { 34, 73,  34, 88  },
		/*20*/ { 34, 88,  75, -1  },
		/*21*/ { 12, 51,  -1, 100 },
		/*22*/ { 12, 100, 75, -1  },
#elif PAL
		/* 0*/ { 12, -1, -1, 10 },
		/* 1*/ { 51, -1, -1, 10 },
		/* 2*/ { 12, -1, 75, -1 },
		/* 3*/ { 55, 19, 64, -1 },
		/* 4*/ { 63, 19, -1, 7  },
		/* 5*/ { 63, 7,  75, -1 },
		/* 6*/ { 51, 23, 55, -1 },
		/* 7*/ { 59, 23, 68, -1 },
		/* 8*/ { 67, 23, -1, 15 },
		/* 9*/ { 67, 15, 75, -1 },
		/*10*/ { 55, 27, 72, -1 },
		/*11*/ { 71, 27, -1, 23 },
		/*12*/ { 71, 23, 75, -1 },
		/*13*/ { 49, 31, 75, -1 },
		/*14*/ { 43, 28, -1, 39 },
		/*15*/ { 43, 39, 75, -1 },
		/*16*/ { 30, 35, 37, -1 },
		/*17*/ { 37, 35, -1, 47 },
		/*18*/ { 37, 47, 75, -1 },
		/*19*/ { 34, 55, 75, -1 },
		/*20*/ { 12, 30, -1, 63 },
		/*21*/ { 12, 63, 75, -1 },
#else
		/* 0*/ { 12, 10, -1, 3  },
		/* 1*/ { 51, 10, -1, 3  },
		/* 2*/ { 12, 3,  75, -1 },
		/* 3*/ { 55, 19, 64, -1 },
		/* 4*/ { 63, 19, -1, 10 },
		/* 5*/ { 63, 10, 75, -1 },
		/* 6*/ { 51, 23, 55, -1 },
		/* 7*/ { 59, 23, 68, -1 },
		/* 8*/ { 67, 23, -1, 17 },
		/* 9*/ { 67, 17, 75, -1 },
		/*10*/ { 55, 27, 72, -1 },
		/*11*/ { 71, 27, -1, 24 },
		/*12*/ { 71, 24, 75, -1 },
		/*13*/ { 49, 31, 75, -1 },
		/*14*/ { 43, 28, -1, 38 },
		/*15*/ { 43, 38, 75, -1 },
		/*16*/ { 30, 35, 37, -1 },
		/*17*/ { 37, 35, -1, 45 },
		/*18*/ { 37, 45, 75, -1 },
		/*19*/ { 34, 52, 75, -1 },
		/*20*/ { 12, 30, -1, 59 },
		/*21*/ { 12, 59, 75, -1 },
#endif
	};

	speed = (s32)(-g_20SecIntervalFrac * 4.0f * 50.0f) % 4;
	alpha = alpha >> 1;

	gDPPipeSync(gdl++);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);

	texSelect(&gdl, &g_TexGeneralConfigs[55], 2, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetCombineLERP(gdl++,
			TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
			TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
	gDPSetEnvColorViaWord(gdl++, 0xffffff00 | alpha);

	for (i = firstindex; i <= lastindex; i++) {
		s32 thisspeed = speed;
		s32 tmp;

		if (linecfgs[i].y2 == -1) {
			linecfgs[i].y2 = linecfgs[i].y1 + 1;
		}

		if (linecfgs[i].x2 == -1) {
			linecfgs[i].x2 = linecfgs[i].x1 + 1;
		}

		if (linecfgs[i].x2 == linecfgs[i].x1) {
			linecfgs[i].x2 = linecfgs[i].x1 + 1;
		}

		if (linecfgs[i].y2 == linecfgs[i].y1) {
			linecfgs[i].y2 = linecfgs[i].y1 + 1;
		}

		if (linecfgs[i].x2 < linecfgs[i].x1) {
			tmp = linecfgs[i].x1;
			linecfgs[i].x1 = linecfgs[i].x2;
			linecfgs[i].x2 = tmp;
		}

		if (linecfgs[i].y2 < linecfgs[i].y1) {
			tmp = linecfgs[i].y1;
			linecfgs[i].y1 = linecfgs[i].y2;
			linecfgs[i].y2 = tmp;
		}

		switch (i) {
		case 14:
		case 17:
#if VERSION >= VERSION_JPN_FINAL
		case 19:
		case 21:
#else
		case 20:
#endif
			thisspeed = -thisspeed;
			break;
		}

		gdl = menuitemControllerRenderLine(gdl, thisspeed,
				linecfgs[i].x1 + context->x + padx, linecfgs[i].y1 + context->y + pady,
				linecfgs[i].x2 + context->x + padx, linecfgs[i].y2 + context->y + pady);
	}

	return gdl;
}

// These are the action names, like "AIM", and "LOOK UP"
u16 var80071354[][9] = {
	/*0*/  { L_MPWEAPONS_194, L_MPWEAPONS_196, L_MPWEAPONS_202, L_MPWEAPONS_197, L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_206, L_MPWEAPONS_205, L_MPWEAPONS_210 },
	/*1*/  { L_MPWEAPONS_194, L_MPWEAPONS_200, L_MPWEAPONS_202, L_MPWEAPONS_201, L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_207, L_MPWEAPONS_205, L_MPWEAPONS_210 },
	/*2*/  { L_MPWEAPONS_203, L_MPWEAPONS_196, L_MPWEAPONS_202, L_MPWEAPONS_197, L_MPWEAPONS_205, L_MPWEAPONS_204, L_MPWEAPONS_206, L_MPWEAPONS_194, L_MPWEAPONS_210 },
	/*3*/  { L_MPWEAPONS_203, L_MPWEAPONS_200, L_MPWEAPONS_202, L_MPWEAPONS_201, L_MPWEAPONS_205, L_MPWEAPONS_204, L_MPWEAPONS_207, L_MPWEAPONS_194, L_MPWEAPONS_210 },
	/*4*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_211, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_206, L_MPWEAPONS_205, L_OPTIONS_003   },
	/*5*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_211, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_207, L_MPWEAPONS_205, L_OPTIONS_003   },
	/*6*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_211, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_206, L_MPWEAPONS_194, L_OPTIONS_003   },
	/*7*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_211, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_207, L_MPWEAPONS_194, L_OPTIONS_003   },
	/*8*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_212, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_209, L_MPWEAPONS_194, L_OPTIONS_003   },
	/*9*/  { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_212, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_208, L_MPWEAPONS_194, L_OPTIONS_003   },
	/*10*/ { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_212, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_209, L_MPWEAPONS_205, L_OPTIONS_003   },
	/*11*/ { L_OPTIONS_003,   L_OPTIONS_003,   L_MPWEAPONS_212, L_OPTIONS_003,   L_MPWEAPONS_203, L_MPWEAPONS_204, L_MPWEAPONS_208, L_MPWEAPONS_205, L_OPTIONS_003   },
};

u16 menuitemControllerGetButtonAction(s32 mode, s32 buttonnum)
{
	u32 textid = var80071354[mode][buttonnum];

	if (textid == L_MPWEAPONS_194 // "AIM"
			&& optionsGetAimControl(g_Menus[g_MpPlayerNum].main.mpindex) == AIMCONTROL_TOGGLE) {
		textid = L_MPWEAPONS_195; // "AIM TOGGLE"
	}

	if (textid == L_MPWEAPONS_196 // "LOOK UP"
			&& optionsGetForwardPitch(g_Menus[g_MpPlayerNum].main.mpindex) == false) {
		textid = L_MPWEAPONS_198; // "LOOK DOWN"
	}

	if (textid == L_MPWEAPONS_197 // "LOOK DOWN"
			&& optionsGetForwardPitch(g_Menus[g_MpPlayerNum].main.mpindex) == false) {
		textid = L_MPWEAPONS_199; // "LOOK UP"
	}

	return textid;
}

/**
 * Renders the button labels and actions for the control style dialog.
 *
 * When switching modes (control styles), a fade effect is used to fade in the
 * action names, but only if they differ from the previous mode.
 *
 * Note that the valuecolour argument is mostly unused - only the alpha channel
 * is used because the rest is bitwise or'ed to white.
 */
Gfx *menuitemControllerRenderText(Gfx *gdl, s32 curmode, struct menurendercontext *context, s32 padx, s32 pady, u32 valuecolour, u32 labelcolour, s8 prevmode)
{
	s32 rx;
	s32 ry;
	u16 textnum;
	u32 colour;

	u16 labels[] = {
		/*0*/ L_MPWEAPONS_185, // "L/R BUTTONS:"
		/*1*/ L_MPWEAPONS_186, // "UP C BUTTON:"
		/*2*/ L_MPWEAPONS_187, // "LEFT/RIGHT C BUTTONS:"
		/*3*/ L_MPWEAPONS_188, // "DOWN C BUTTON:"
		/*4*/ L_MPWEAPONS_189, // "A BUTTON:"
		/*5*/ L_MPWEAPONS_190, // "B BUTTON:"
		/*6*/ L_MPWEAPONS_191, // "CONTROL STICK:"
		/*7*/ L_MPWEAPONS_192, // "Z BUTTON:"
		/*8*/ L_MPWEAPONS_193, // "+ CONTROL PAD:"
	};

	s32 i;

	gdl = text0f153628(gdl);

	for (i = 0; i < ARRAYCOUNT(labels); i++) {
#if VERSION == VERSION_JPN_FINAL
		ry = i * 12 + context->y + pady + 1;
#elif VERSION >= VERSION_PAL_BETA
		ry = i * 8 + context->y + pady - 4;
#elif VERSION >= VERSION_NTSC_1_0
		ry = i * 7 + context->y + pady;
#endif

		// For the 2.x styles, only labels 4-7 are shown
		if ((curmode < CONTROLMODE_21 || curmode == CONTROLMODE_PC) || (i >= 4 && i <= 7)) {
			// Rendering a label such as "L/R BUTTONS:"
			rx = context->x + padx + 76;
#if VERSION < VERSION_NTSC_1_0
			ry = i * 7 + context->y + pady;
#endif
			gdl = textRenderProjected(gdl, &rx, &ry, langGet(labels[i]),
					g_CharsHandelGothicXs, g_FontHandelGothicXs, labelcolour, viGetWidth(), viGetHeight(), 0, 0);
		}

		textnum = menuitemControllerGetButtonAction(curmode, i);
		colour = valuecolour;

		// If there's a prevmode, get the text ID that was in this position for
		// prevmode. If it's the same text as curmode, don't fade the text.
		if (prevmode >= 0) {
			// I don't see how curmode can ever be > CONTROLMODE_24. Perhaps
			// during development the second player in the 2.x styles had to
			// choose their control style separately to player 1, in which case
			// there would have been 2.5, 2.6, 2.7 and 2.8 for player 2.
			if (curmode > CONTROLMODE_24 && curmode != CONTROLMODE_PC) {
				if (textnum == menuitemControllerGetButtonAction(prevmode + 4, i)) {
					colour = labelcolour;
				}
			} else {
				if (textnum == menuitemControllerGetButtonAction(prevmode, i)) {
					colour = labelcolour;
				}
			}
		}

		if (curmode >= CONTROLMODE_21 && curmode < CONTROLMODE_PC && i == 2) {
			// Rendering a "CONTROLLER 1" or "CONTROLLER 2" heading
			rx = context->x + padx + 63;
			colour = labelcolour;
		} else {
			// Rendering a value such as "WALK/TURN"
			// Make it white but preserve alpha
			colour |= 0xffffff00;
		}

		gdl = textRenderProjected(gdl, &rx, &ry, langGet(textnum),
				g_CharsHandelGothicXs, g_FontHandelGothicXs, colour, viGetWidth(), viGetHeight(), 0, 0);
	}

	return text0f153780(gdl);
}

Gfx *menuitemControllerRenderPad(Gfx *gdl, struct menurendercontext *context, s32 padx, s32 pady, s32 curmode, u32 alpha, u32 colour1, u32 colour2, s8 prevmode)
{
	s32 rx = context->x + padx;
	s32 ry = context->y + pady + (VERSION == VERSION_JPN_FINAL ? 25 : 4);

	// The controller graphic is split into 4 textures
	gdl = menuitemControllerRenderTexture(gdl, rx, ry, 0x33, alpha);
	gdl = menuitemControllerRenderTexture(gdl, rx + 32, ry, 0x34, alpha);
	gdl = menuitemControllerRenderTexture(gdl, rx, ry + 32, 0x35, alpha);
	gdl = menuitemControllerRenderTexture(gdl, rx + 32, ry + 32, 0x36, alpha);

#if VERSION >= VERSION_JPN_FINAL
	if (curmode >= CONTROLMODE_21 && curmode < CONTROLMODE_PC) {
		gdl = menuitemControllerRenderLines(gdl, context, 13, 20, padx, pady, alpha);
	} else {
		gdl = menuitemControllerRenderLines(gdl, context, 0, 22, padx, pady, alpha);
	}
#else
	if (curmode >= CONTROLMODE_21 && curmode < CONTROLMODE_PC) {
		gdl = menuitemControllerRenderLines(gdl, context, 13, 19, padx, pady, alpha);
	} else {
		gdl = menuitemControllerRenderLines(gdl, context, 0, 21, padx, pady, alpha);
	}
#endif

	return menuitemControllerRenderText(gdl, curmode, context, padx, pady, colour1, colour2, prevmode);
}

Gfx *menuitemControllerRender(Gfx *gdl, struct menurendercontext *context)
{
	struct menuitemdata_controller *data = &context->data->controller;
	u32 colour;
	u32 textcolour;
	s32 x;
	s32 y;
	char text[32];
	struct menudialog *dialog = context->dialog;
	u32 contalpha;
	u32 textalpha;

	textBackupAndResetBlends();

	// If changing modes within the same group (eg. within 1.x)
	if (data->curmode != (g_Menus[g_MpPlayerNum].main.controlmode & 0xff)) {
		data->prevmode = data->curmode;
		data->curmode = g_Menus[g_MpPlayerNum].main.controlmode;
		data->textfadetimer = 0;
	}

	// If changing control group (eg. 1.4 -> 2.1)
	if (data->controlgroup == 1) {
		if (g_Menus[g_MpPlayerNum].main.controlmode < CONTROLMODE_21 || g_Menus[g_MpPlayerNum].main.controlmode == CONTROLMODE_PC) {
			data->controlgroup = 2;
			data->contfadetimer = 0;
			data->prevmode = -1;
		}
	} else {
		if (g_Menus[g_MpPlayerNum].main.controlmode >= CONTROLMODE_21 && g_Menus[g_MpPlayerNum].main.controlmode != CONTROLMODE_PC) {
			data->controlgroup = 1;
			data->contfadetimer = 0;
			data->prevmode = -1;
		}
	}

	// If text is fading in (eg. recently changed 1.1 -> 1.2)
	if (data->textfadetimer < 255) {
		s32 value = data->textfadetimer + g_Vars.diffframe240 * 2;

		if (value > 255) {
			value = 255;
		}

		data->textfadetimer = value;
	}

	// If controller is fading in (eg. recently changed 1.4 -> 2.1)
	if (data->contfadetimer < 255) {
		s32 value = data->contfadetimer + g_Vars.diffframe240 * 2;

		if (value > 255) {
			value = 255;
		}

		data->contfadetimer = value;
	}

	textalpha = data->textfadetimer;
	contalpha = data->contfadetimer;
	gdl = text0f153628(gdl);

	if (dialog->transitionfrac < 0) {
		colour = g_MenuColours[dialog->type].item_unfocused;
	} else {
		colour = colourBlend(
				g_MenuColours[dialog->type2].item_unfocused,
				g_MenuColours[dialog->type].item_unfocused,
				dialog->colourweight);
	}

	if (dialog->dimmed) {
		colour = (colourBlend(colour, 0, 44) & 0xffffff00) | (colour & 0xff);
	}

	textSetWaveColours(
			g_MenuWave2Colours[dialog->type].item_unfocused,
			g_MenuWave1Colours[dialog->type].item_unfocused);

	if (g_Menus[g_MpPlayerNum].main.controlmode == CONTROLMODE_PC) {
		sprintf(text, langGet(L_MPWEAPONS_213), // "Control Style %s %s"
				"Ext",
				langGet(L_MPWEAPONS_215)); // "(Two-Handed)"
	} else if (g_Menus[g_MpPlayerNum].main.controlmode >= CONTROLMODE_21) {
		sprintf(text, langGet(L_MPWEAPONS_213), // "Control Style %s %s"
				langGet(g_ControlStyleOptions[g_Menus[g_MpPlayerNum].main.controlmode]),
				langGet(L_MPWEAPONS_215)); // "(Two-Handed)"
	} else {
		sprintf(text, langGet(L_MPWEAPONS_213), // "Control Style %s %s"
				langGet(g_ControlStyleOptions[g_Menus[g_MpPlayerNum].main.controlmode]),
				langGet(L_MPWEAPONS_214)); // "(One-Handed)"
	}

	x = context->x + 2;
	y = context->y + 2;
	gdl = textRenderProjected(gdl, &x, &y, text,
			g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, viGetWidth(), viGetHeight(), 0, 0);
	gdl = text0f153780(gdl);

	textcolour = colourBlend(colour, colour & 0xffffff00, textalpha);
	colour = colourBlend(colour, colour & 0xffffff00, contalpha);

	if (g_Menus[g_MpPlayerNum].main.controlmode == CONTROLMODE_PC) {
		x = context->x;
		y = context->y + 92;
#if VERSION == VERSION_JPN_FINAL
		y += 34;
#endif
		gdl = text0f153628(gdl);
		gdl = textRenderProjected(gdl, &x, &y,
				"PC Port control scheme.\n"
				"Only change to another control scheme\n"
				"if you want to use original N64 controls.\n\n"
				"See Extended Key Bindings for more info.",
				g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);
	} else if (g_Menus[g_MpPlayerNum].main.controlmode >= CONTROLMODE_21) {
		gdl = menuitemControllerRenderPad(gdl, context, 0, VERSION == VERSION_JPN_FINAL ? -4 : 12,
				g_Menus[g_MpPlayerNum].main.controlmode,
				contalpha, textcolour, colour, data->prevmode);
		gdl = menuitemControllerRenderPad(gdl, context, 0, VERSION == VERSION_JPN_FINAL ? 74 : 80,
				g_Menus[g_MpPlayerNum].main.controlmode + 4,
				contalpha, textcolour, colour, data->prevmode);
	} else {
		gdl = menuitemControllerRenderPad(gdl, context, 0, PAL ? 19 : 15,
				g_Menus[g_MpPlayerNum].main.controlmode,
				contalpha, textcolour, colour, data->prevmode);

		x = context->x;
		y = context->y + 92;

#if VERSION == VERSION_JPN_FINAL
		y += 34;
#endif

		gdl = text0f153628(gdl);
		gdl = textRenderProjected(gdl, &x, &y, langGet(L_MPWEAPONS_216), // "Hold weapon button for ..."
				g_CharsHandelGothicSm, g_FontHandelGothicSm, colour, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);
	}

	textRestoreBlends();

	return gdl;
}

void menuitemControllerInit(union menuitemdata *data)
{
	data->controller.textfadetimer = 0;
	data->controller.contfadetimer = 0;
	data->controller.curmode = 255;
	data->controller.controlgroup = 255;
	data->controller.prevmode = -1;
}

Gfx *menuitemRender(Gfx *gdl, struct menurendercontext *context)
{
	switch (context->item->type) {
	case MENUITEMTYPE_LIST:        return menuitemListRender(gdl, context);
	case MENUITEMTYPE_SELECTABLE:  return menuitemSelectableRender(gdl, context);
	case MENUITEMTYPE_SLIDER:      return menuitemSliderRender(gdl, context);
	case MENUITEMTYPE_CHECKBOX:    return menuitemCheckboxRender(gdl, context);
	case MENUITEMTYPE_SCROLLABLE:  return menuitemScrollableRender(gdl, context);
	case MENUITEMTYPE_MARQUEE:     return menuitemMarqueeRender(gdl, context);
	case MENUITEMTYPE_LABEL:       return menuitemLabelRender(gdl, context);
	case MENUITEMTYPE_METER:       return menuitemMeterRender(gdl, context);
	case MENUITEMTYPE_SEPARATOR:   return menuitemSeparatorRender(gdl, context);
	case MENUITEMTYPE_OBJECTIVES:  return menuitemObjectivesRender(gdl, context);
	case MENUITEMTYPE_07:          return menuitem07Render(gdl);
	case MENUITEMTYPE_DROPDOWN:    return menuitemDropdownRender(gdl, context);
	case MENUITEMTYPE_KEYBOARD:    return menuitemKeyboardRender(gdl, context);
	case MENUITEMTYPE_RANKING:     return menuitemRankingRender(gdl, context);
	case MENUITEMTYPE_PLAYERSTATS: return menuitemPlayerStatsRender(gdl, context);
	case MENUITEMTYPE_CAROUSEL:    return menuitemCarouselRender(gdl, context);
	case MENUITEMTYPE_MODEL:       return menuitemModelRender(gdl, context);
	case MENUITEMTYPE_CONTROLLER:  return menuitemControllerRender(gdl, context);

#ifndef PLATFORM_N64
	case MENUITEMTYPE_COLORBOX:    return menuitemColorBoxRender(gdl, context);
#endif

	}

	return gdl;
}

/**
 * Return true if default up/down/left/right/back behaviour should be used.
 */
bool menuitemTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags, union menuitemdata *data)
{
	switch (item->type) {
	case MENUITEMTYPE_LIST:        return menuitemListTick(item, inputs, tickflags, data);
	case MENUITEMTYPE_SELECTABLE:  return menuitemSelectableTick(item, inputs, tickflags);
	case MENUITEMTYPE_SLIDER:      return menuitemSliderTick(item, dialog, inputs, tickflags, data);
	case MENUITEMTYPE_CHECKBOX:    return menuitemCheckboxTick(item, inputs, tickflags);
	case MENUITEMTYPE_SCROLLABLE:  return menuitemScrollableTick(item, dialog, inputs, tickflags, data);
	case MENUITEMTYPE_MARQUEE:     return menuitemMarqueeTick(item, data);
	case MENUITEMTYPE_RANKING:     return menuitemRankingTick(inputs, tickflags, data);
	case MENUITEMTYPE_DROPDOWN:    return menuitemDropdownTick(item, dialog, inputs, tickflags, data);
	case MENUITEMTYPE_KEYBOARD:    return menuitemKeyboardTick(item, inputs, tickflags, data);
	case MENUITEMTYPE_CAROUSEL:    return menuitemCarouselTick(item, inputs, tickflags);
	case MENUITEMTYPE_PLAYERSTATS: return menuitemPlayerStatsTick(item, dialog, inputs, tickflags, data);
	}

	return true;
}

void menuitemInit(struct menuitem *item, union menuitemdata *data)
{
	switch (item->type) {
	case MENUITEMTYPE_LIST:
	case MENUITEMTYPE_DROPDOWN:
		menuitemDropdownInit(item, data);
		break;
	case MENUITEMTYPE_SCROLLABLE:
		menuitemScrollableInit(data);
		break;
	case MENUITEMTYPE_MARQUEE:
		menuitemMarqueeInit(data);
		break;
	case MENUITEMTYPE_RANKING:
		menuitemRankingInit(data);
		break;
	case MENUITEMTYPE_SLIDER:
		menuitemSliderInit(data);
		break;
	case MENUITEMTYPE_PLAYERSTATS:
		menuitemPlayerStatsInit(item, data);
		break;
	case MENUITEMTYPE_KEYBOARD:
		menuitemKeyboardInit(item, data);
		break;
	case MENUITEMTYPE_CONTROLLER:
		menuitemControllerInit(data);
		break;
	}
}

Gfx *menuitemOverlay(Gfx *gdl, s16 x, s16 y, s16 x2, s16 y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data)
{
	switch (item->type) {
	case MENUITEMTYPE_LIST:        return menuitemListOverlay(gdl, x, y, x2, y2);
	case MENUITEMTYPE_DROPDOWN:    return menuitemDropdownOverlay(gdl, x, y, x2, y2, item, dialog, data);
	case MENUITEMTYPE_PLAYERSTATS: return menuitemPlayerStatsOverlay(gdl, x, y, x2, y2, item, dialog, data);
	}

	return gdl;
}

#ifndef PLATFORM_N64

s32 menuitemGetTop(struct menuitem *item, struct menudialog *dialog)
{
	struct menu *menu = &g_Menus[g_MpPlayerNum];
	s32 dtop = dialog->y + LINEHEIGHT + 1;

	for (s32 i = 0; i < dialog->numcols; ++i) {
		const s32 colindex = i + dialog->colstart;

		for (s32 j = 0; j < menu->cols[colindex].numrows; ++j) {
			const s32 rowindex = j + menu->cols[colindex].rowstart;
			struct menuitem *pitem = &dialog->definition->items[menu->rows[rowindex].itemindex];
			if (pitem == item) {
				return dtop;
			}
			dtop += menu->rows[rowindex].height;
		}
	}

	return dtop;
}

#endif
