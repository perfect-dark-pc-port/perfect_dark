#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/boot.h"
#include "lib/crash.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "system.h"
#include <string.h>

/**
 * memshared - functions usable by both memp and mema.
 */

#if MEMORY_DEBUG
char* lastAttemptedFree = NULL;
#endif

bool memDefragPass(struct memorypool *pool)
{
	size_t freeSpace;
	bool foundFreeSpace = false;

	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == NULL)
		{
			freeSpace = i;
			foundFreeSpace = true;
			break;
		}
	}
	if (!foundFreeSpace)
	{
		return false;
	}
	for (size_t i = freeSpace; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr != NULL)
		{
#if MEMORY_DEBUG
			pool->allocations[freeSpace].label = pool->allocations[i].label;
			pool->allocations[i].label = NULL;
#endif
			pool->allocations[freeSpace].addr = pool->allocations[i].addr;
			pool->allocations[i].addr = NULL;
			pool->allocations[freeSpace].size = pool->allocations[i].size;
			pool->allocations[i].size = 0;
			return true;
		}
	}
	return false;
}

void memDefrag(struct memorypool *pool)
{
	while (memDefragPass(pool));
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == NULL)
		{
			pool->nextAlloc = i;
			return;
		}
	}
}

void memInit(struct memorypool *pool, size_t initialCapacity, char* label)
{
	pool->nextAlloc = 0;
	pool->slots = initialCapacity;
	pool->label = label;
	size_t allocationSize = pool->slots * sizeof(*pool->allocations);
	void* allocations = sysMemZeroAlloc(allocationSize);
	if (allocations != NULL)
	{
		pool->allocations = allocations;
	}
	else
	{
		sysFatalError("Could not allocate %zu bytes for memory pool %s.", allocationSize, label);
	}
}

void memFreeIndex(struct memorypool *pool, size_t index)
{
#if MEMORY_DEBUG
	lastAttemptedFree = pool->allocations[index].label;
#endif
	sysMemFree(pool->allocations[index].addr);
#if MEMORY_DEBUG
	pool->allocations[index].label = NULL;
#endif
	pool->allocations[index].addr = NULL;
	pool->allocations[index].size = 0;
}

void memReset(struct memorypool *pool)
{
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr != NULL)
		{
			memFreeIndex(pool, i);
		}
	}
	pool->nextAlloc = 0;
}

/*
 * Expands the size of the allocation tracker.
 * On failure, will call memDefrag.
 * Returns true on success, false on failure.
 */
bool memExpand(struct memorypool *pool)
{
	size_t oldSize = pool->slots;
	size_t newSize = pool->slots * 2;
	void* tmp = sysMemRealloc(pool->allocations, newSize * sizeof(*pool->allocations));
	if (tmp == NULL)
	{
		memDefrag(pool);
		return false;
	}
	pool->allocations = (struct allocslot*)tmp;
	pool->slots = newSize;
	memset(tmp + (oldSize * sizeof(*pool->allocations)), 0, (newSize - oldSize) * sizeof(*pool->allocations));
	return true;
}

#if MEMORY_DEBUG
void *_memAlloc(struct memorypool *pool, size_t size, char* label)
#else
void *memAlloc(struct memorypool *pool, size_t size)
#endif
{
	void *allocation;

	if (pool->nextAlloc >= pool->slots)
	{
		memExpand(pool);
		// memorypool may have been expanded (pool->slots is now higher)
		// or defragmented (pool->nextAlloc is now lower)
		if (pool->nextAlloc >= pool->slots)
		{
#if MEMORY_DEBUG
			printf("memAlloc: %s: Failed to expand %s and allocate %zu bytes\n", label, pool->label, size);
#else
			printf("memAlloc: Failed to expand %s and allocate %zu bytes\n", pool->label, size);
#endif
			return NULL;
		}
	}

	allocation = sysMemZeroAlloc(size);
	if (allocation == NULL)
	{
#if MEMORY_DEBUG
		printf("memAlloc: %s: OS failed to allocate %zu bytes in %s\n", label, size, pool->label);
#else
		printf("memAlloc: OS failed to allocate %zu bytes in %s\n", size, pool->label);
#endif
		return NULL;
	}
#if MEMORY_DEBUG
	pool->allocations[pool->nextAlloc].label = label;
#endif
	pool->allocations[pool->nextAlloc].addr = allocation;
	pool->allocations[pool->nextAlloc].size = size;
	pool->nextAlloc++;
#if MEMORY_DEBUG > 1
	printf("memAlloc: %s: Allocated %zu bytes in %s\n", label, size, pool->label);
#endif
	return allocation;
}

void memFree(struct memorypool *pool, void *addr, size_t size)
{
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == addr)
		{
			memFreeIndex(pool, i);
			return;
		}
	}
}

void memInfo(void *addr, char* infoBuffer, size_t infoBufferCapacity)
{
	for (size_t i = 0; i < ARRAYCOUNT(g_MemPools); i++)
	{
		struct memorypool* pool = &g_MemPools[i];
		for (size_t j = 0; j < pool->nextAlloc; j++)
		{
			if (pool->allocations[j].addr == addr)
			{
#if MEMORY_DEBUG
				snprintf(infoBuffer, infoBufferCapacity, "%p: %s: %s: %zu bytes", addr, pool->label, pool->allocations[j].label, pool->allocations[j].size);
#else
				snprintf(infoBuffer, infoBufferCapacity, "%p: %s: %zu bytes", addr, pool->label, pool->allocations[j].size);
#endif
				return;
			}
		}
	}
	snprintf(infoBuffer, infoBufferCapacity, "%p: Non-Managed Pointer", addr);
	return;
}

/**
 * memp - memory pool allocation system.
 *
 * Memp is the main memory allocation system in the game.
 *
 * MEMPOOL_STAGE is for general data and is cleared on stage load.
 * MEMPOOL_5 probably had something to do with file loading at some point.
 * MEMPOOL_7 is reset alongside MEMPOOL_STAGE.
 * MEMPOOL_8 has something to do with utils.
 * MEMPOOL_PERMANENT is for permanent data and is never cleared.
 * MEMPOOL_ADHOC is used for mema allocations.
 */

struct memorypool g_MemPools[10];

void mempInit(void)
{
	for (int i = 0; i < ARRAYCOUNT(g_MemPools); i++) {
		switch(i) {
			case MEMPOOL_STAGE:
				memInit(&g_MemPools[i], 2048, "MEMPOOL_STAGE");
				break;
			case MEMPOOL_PERMANENT:
				memInit(&g_MemPools[i], 32, "MEMPOOL_PERMANENT");
				break;
			case MEMPOOL_ADHOC:
				memInit(&g_MemPools[i], 128, "MEMPOOL_ADHOC");
				break;
			default:
				char* label = sysMemZeroAlloc(12);
				sprintf(label, "MEMPOOL_%u", i);
				memInit(&g_MemPools[i], 1, label);
				break;
		}
	}
}

#if MEMORY_DEBUG
void *_mempGetNextStageAllocation(char* label)
{
	return _memAlloc(&g_MemPools[MEMPOOL_STAGE], 4 * 1024 * 1024, label); // Give it 4MB so I don't have to figure out the right amount to allocate right now.
}
#else
void *mempGetNextStageAllocation(void)
{
	return memAlloc(&g_MemPools[MEMPOOL_STAGE], 4 * 1024 * 1024); // Give it 4MB so I don't have to figure out the right amount to allocate right now.
}
#endif

#if MEMORY_DEBUG
void *_mempAlloc(u32 len, u8 pool, char* label)
{
	return _memAlloc(&g_MemPools[pool], len, label);
}
#else
void *mempAlloc(u32 len, u8 pool)
{
	return memAlloc(&g_MemPools[pool], len);
}
#endif

#if MEMORY_DEBUG
void *_mempAllocFromRight(u32 len, u8 pool, char* label)
{
	return _memAlloc(&g_MemPools[pool], len, label);
}
#else
void *mempAllocFromRight(u32 len, u8 pool)
{
	return memAlloc(&g_MemPools[pool], len);
}
#endif

/**
 * Reallocate the given allocation in the given pool.
 * The pointer will remain unchanged.
 *
 * newsize must be less than (or equal to) the original size.
 */
s32 mempRealloc(void *allocation, s32 newsize, u8 poolnum)
{
	struct memorypool* pool = &g_MemPools[poolnum];

	if (newsize == 0)
	{
		memFree(pool, allocation, newsize);
		return 1;
	}

	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == allocation)
		{
			if (newsize <= pool->allocations[i].size)
			{
				pool->allocations[i].size = newsize;
			}
			return 1;
		}
	}

	return 0;
}

/**
 * Return the amount of free space in the given pool and bank.
 */
u32 mempGetPoolFree(u8 poolnum, u32 bank)
{
	return 4300; // Just lie and say we have enough for texLoad.
}

void mempResetPool(u8 pool)
{
	memReset(&g_MemPools[pool]);
}
