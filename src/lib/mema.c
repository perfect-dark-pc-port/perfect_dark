#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "game/debug.h"
#include "lib/debughud.h"
#include "lib/mema.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

/**
 * mema - memory (ad hoc) allocation system.
 */

struct memorypool* g_MemaPool;

void memaDefrag(void)
{
	memDefrag(g_MemaPool);
}

void memaInit(void)
{
	g_MemaPool = &g_MemPools[MEMPOOL_ADHOC];
}

void memaReset(void)
{
	memReset(g_MemaPool);
}

void memaPrint(void)
{
	memDefrag(g_MemaPool);
}

/*
 * Expands the size of the allocation tracker.
 * On failure, will defrag the allocation tracker.
 * Returns true on success, false on failure.
 */
bool memaExpand(void)
{
	return memExpand(g_MemaPool);
}

#if MEMORY_DEBUG
void *_memaAlloc(size_t size, char* label)
{
	return _memAlloc(g_MemaPool, size, label);
}
#else
void *memaAlloc(size_t size)
{
	return memAlloc(g_MemaPool, size);
}
#endif

void memaFree(void *addr, size_t size)
{
	memFree(g_MemaPool, addr, size);
}
