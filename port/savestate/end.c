// Linked immediately after the game's objects (see CMakeLists.txt), so these mark where
// the game's globals end in .data and .bss. Used by port/src/savestate.c.
// macOS uses explicit sections instead (savestate_game.h).

#ifndef __APPLE__
unsigned char g_SavestateDataEnd[16] = { 1 };
unsigned char g_SavestateBssEnd[16];
#endif
