#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess/common.h"

#include "constants.h"

struct n64_header {
	s32 num_pads;
	s32 num_covers;
	u32 ptr_waypoints;
	u32 ptr_waygroups;
	u32 ptr_cover;
};

struct host_header {
	s32 num_pads;
	s32 num_covers;
	uintptr_t ptr_waypoints;
	uintptr_t ptr_waygroups;
	uintptr_t ptr_cover;
};

struct padheader {
	u32 flags : 18;
	u32 room : 10;
	u32 liftnum : 4;
};

struct n64_waypoint {
	s32 padnum;
	u32 ptr_neighbours;
	s32 groupnum;
	s32 step;
};

struct n64_waygroup {
	u32 ptr_neighbours;
	u32 ptr_waypoints;
	s32 step;
};

static u32 convertPads(u8 *dst, u32 dstpos, u8 *src, u32 srcpos, int num_pads)
{
	u16 *src_offsets = (u16 *) &src[srcpos];
	u16 *dst_offsets = (u16 *) &dst[dstpos];

	dstpos += num_pads * sizeof(u16);

	for (int i = 0; i < num_pads; i++) {
		srcpos = PD_BE16(src_offsets[i]);

		dst_offsets[i] = (dstpos);

		// Header
		u32 n64_padheader = PD_BE32(*(u32 *) &src[srcpos]);
		struct padheader *host_padheader = (struct padheader *) &dst[dstpos];
		u32 flags = (n64_padheader >> 14) & 0x3ffff;

		*(u32*)host_padheader = (n64_padheader);

		srcpos += sizeof(struct padheader);
		dstpos += sizeof(struct padheader);

		// Position
		if (flags & PADFLAG_INTPOS) {
			s16 *srcptr = (s16 *) &src[srcpos];
			s16 *dstptr = (s16 *) &dst[dstpos];

			dstptr[0] = PD_BE16(srcptr[0]);
			dstptr[1] = PD_BE16(srcptr[1]);
			dstptr[2] = PD_BE16(srcptr[2]);

			srcpos += 8;
			dstpos += 8;
		} else {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Up
		if ((flags & (PADFLAG_UPALIGNTOX | PADFLAG_UPALIGNTOY | PADFLAG_UPALIGNTOZ)) == 0) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Look
		if ((flags & (PADFLAG_LOOKALIGNTOX | PADFLAG_LOOKALIGNTOY | PADFLAG_LOOKALIGNTOZ)) == 0) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Bbox
		if (flags & PADFLAG_HASBBOXDATA) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);
			dstptr[3] = PD_BE32(srcptr[3]);
			dstptr[4] = PD_BE32(srcptr[4]);
			dstptr[5] = PD_BE32(srcptr[5]);

			srcpos += 4 * 6;
			dstpos += 4 * 6;
		}
	}

	return dstpos;
}

static u32 convertWayPoints(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_waypoint *n64_waypoints = (struct n64_waypoint *) &src[srcpos];
	struct waypoint *host_waypoints = (struct waypoint *) &dst[dstpos];
	int num_waypoints;

	for (num_waypoints = 0; n64_waypoints[num_waypoints].padnum != -1; num_waypoints++);

	dstpos += (num_waypoints + 1) * sizeof(struct waypoint);

	u32 *host_neighbours = (u32 *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waypoints; i++) {
		host_waypoints[i].padnum = PD_BE32(n64_waypoints[i].padnum);
		host_waypoints[i].neighbours = (void *)(uintptr_t)dstpos;
		host_waypoints[i].groupnum = PD_BE32(n64_waypoints[i].groupnum);
		host_waypoints[i].step = 0;

		u32 *n64_neighbours = (u32 *) &src[PD_BE32(n64_waypoints[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = PD_BE32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waypoints[num_waypoints].padnum = 0xffffffff;
	host_waypoints[num_waypoints].neighbours = NULL;
	host_waypoints[num_waypoints].groupnum = 0;
	host_waypoints[num_waypoints].step = 0;

	return dstpos;
}

static u32 convertWayGroups(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_waygroup *n64_waygroups = (struct n64_waygroup *) &src[srcpos];
	struct waygroup *host_waygroups = (struct waygroup *) &dst[dstpos];
	int num_waygroups;

	for (num_waygroups = 0; n64_waygroups[num_waygroups].ptr_neighbours != 0; num_waygroups++);

	dstpos += (num_waygroups + 1) * sizeof(struct waygroup);

	// Waygroups and child waypoints
	u32 *host_waypoints = (u32 *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].waypoints = (void *)(uintptr_t)dstpos;
		host_waygroups[i].step = 0;

		u32 *n64_waypoints = (u32 *) &src[PD_BE32(n64_waygroups[i].ptr_waypoints)];

		for (int j = 0; n64_waypoints[j] != 0xffffffff; j++) {
			host_waypoints[n++] = PD_BE32(n64_waypoints[j]);
			dstpos += 4;
		}

		host_waypoints[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waygroups[num_waygroups].neighbours = NULL;
	host_waygroups[num_waygroups].waypoints = NULL;
	host_waygroups[num_waygroups].step = 0;

	// Waygroup neighbours
	u32 *host_neighbours = (u32 *) &dst[dstpos];
	n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].neighbours = (void *)(uintptr_t)dstpos;

		u32 *n64_neighbours = (u32 *) &src[PD_BE32(n64_waygroups[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = PD_BE32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	return dstpos;
}

static u32 convertCover(u8 *dst, u32 dstpos, u8 *src, u32 srcpos, int num_covers)
{
	struct coverdefinition *n64_covers = (struct coverdefinition *) &src[srcpos];
	struct coverdefinition *host_covers = (struct coverdefinition *) &dst[dstpos];

	for (int i = 0; i < num_covers; i++) {
		host_covers[i].pos = PD_SWAPPED_VAL(n64_covers[i].pos);
		host_covers[i].look = PD_SWAPPED_VAL(n64_covers[i].look);
		host_covers[i].flags = PD_BE16(n64_covers[i].flags);
	}

	dstpos += num_covers * sizeof(struct coverdefinition);

	return dstpos;
}

static u32 convertPadsFile(u8 *dst, u8 *src)
{
	u32 dstpos = 0;
	struct n64_header *n64_header = (struct n64_header *) src;
	struct host_header *host_header = (struct host_header *) dst;

	int num_pads = PD_BE32(n64_header->num_pads);
	int num_covers = PD_BE32(n64_header->num_covers);
	host_header->num_pads = (num_pads);
	host_header->num_covers = (num_covers);

	dstpos += sizeof(struct host_header);

	// Pads
	dstpos = convertPads(dst, dstpos, src, sizeof(struct n64_header), num_pads);

	// Waypoints
	host_header->ptr_waypoints = (dstpos);
	dstpos = convertWayPoints(dst, dstpos, src, PD_BE32(n64_header->ptr_waypoints));

	// Waygroups
	host_header->ptr_waygroups = (dstpos);
	dstpos = convertWayGroups(dst, dstpos, src, PD_BE32(n64_header->ptr_waygroups));

    // On 32bit ARM the covers MUST be 32bit aligned for float access, otherwise SIGBUS
    // This fixes crash when starting level 3
#if PLATFORM_ARM == 7
    dstpos = (dstpos + 3) & ~3u;
#endif

	// Cover
	host_header->ptr_cover = (dstpos);
	dstpos = convertCover(dst, dstpos, src, PD_BE32(n64_header->ptr_cover), num_covers);

	return dstpos;
}

u8* preprocessPadsFile(u8 *data, u32 size, u32 *outSize) {
	u32 newSizeEstimated = romdataFileGetEstimatedSize(size, LOADTYPE_PADS);
	u8* dst = sysMemZeroAlloc(newSizeEstimated);

	u32 newSize = convertPadsFile(dst, data);

	if (newSize > newSizeEstimated) {
		sysFatalError("overflow when trying to preprocess a pads file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
