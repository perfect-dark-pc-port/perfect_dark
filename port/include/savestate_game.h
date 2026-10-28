#ifndef _IN_SAVESTATE_GAME_H
#define _IN_SAVESTATE_GAME_H

// Force-included into every game (src/) translation unit by CMakeLists.txt.
//
// The game's few direct heap allocations (room geometry) are routed into the checkpoint
// arena (port/src/savestate_arena.c), so the pointers the game's globals hold into them
// stay valid when a checkpoint is restored.

// On macOS the linker regroups zero-initialised data, so link order cannot bound the game's
// .bss there (see CMakeLists.txt); clang's section pragma places it explicitly instead.
#if defined(__APPLE__) && defined(__clang__) && !defined(__cplusplus)
#pragma clang section data="__DATA,__pd_gdata" bss="__DATA,__pd_gbss"
#endif

#define sysMemAlloc savestateArenaAlloc
#define sysMemZeroAlloc savestateArenaZeroAlloc
#define sysMemRealloc savestateArenaRealloc
#define sysMemFree savestateArenaFree

#endif
