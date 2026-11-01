#ifndef _IN_VERSIONS_H
#define _IN_VERSIONS_H

#define VERSION_NTSC_BETA  0
#define VERSION_NTSC_1_0   1
#define VERSION_NTSC_FINAL 2
#define VERSION_PAL_BETA   3
#define VERSION_PAL_FINAL  4
#define VERSION_JPN_FINAL  5

/**
 * PERFECTHEAD restores the cut PerfectHead feature (Game Boy Camera faces).
 * It is off by default so that matching builds of the original ROMs are
 * unchanged. Enable it with -DPERFECTHEAD=1. See docs/perfecthead.md.
 */
#ifndef PERFECTHEAD
#define PERFECTHEAD 0
#endif

#endif
