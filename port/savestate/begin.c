// Linked immediately before the game's objects (see CMakeLists.txt), so these mark where
// the game's globals start in .data and .bss. Used by port/src/savestate.c.
// macOS uses explicit sections instead (savestate_game.h).

#ifndef __APPLE__
unsigned char g_SavestateDataBegin[16] = { 1 };
unsigned char g_SavestateBssBegin[16];
#endif
