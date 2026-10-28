#ifndef _IN_LIB_MEMP_H
#define _IN_LIB_MEMP_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

struct allocslot {
	void* addr;
	size_t size;
#if MEMORY_DEBUG
	char* label;
#endif
};

struct memorypool {
	u32 nextAlloc;
	size_t slots;
	struct allocslot* allocations;
	char* label;
};

extern struct memorypool g_MemPools[10];

bool memDefragPass(struct memorypool *pool);
void memDefrag(struct memorypool *pool);
void memInit(struct memorypool *pool, size_t initialCapacity, char* label);
void memReset(struct memorypool *pool);
bool memExpand(struct memorypool *pool);
void memFree(struct memorypool *pool, void *addr, size_t size);
void memInfo(void *addr, char* buffer, size_t bufferCapacity);

#if MEMORY_DEBUG
void *_memAlloc(struct memorypool *pool, size_t size, char* label);
#define MEM_LINETOSTR2(line)	#line
#define MEM_LINETOSTR(line)		MEM_LINETOSTR2(line)
#define MEM_GET_LABEL()			__FILE__ ":" MEM_LINETOSTR(__LINE__)
#define memAlloc(pool, size)	_memAlloc(pool, size, MEM_GET_LABEL())
#else
void *memAlloc(struct memorypool *pool, size_t size);
#endif

void mempInit(void);
#define mempGetStageFree()
u32 mempGetPoolFree(u8 poolnum, u32 bank);
void mempResetPool(u8 pool);
#define mempGetStageFree()
#if MEMORY_DEBUG
void *_mempGetNextStageAllocation(char* label);
#define mempGetNextStageAllocation()	_mempGetNextStageAllocation(MEM_GET_LABEL())
void *_mempAlloc(u32 len, u8 pool, char* label);
#define mempAlloc(len, pool)			_mempAlloc(len, pool, MEM_GET_LABEL())
#else
void *mempGetNextStageAllocation(void);
void *mempAlloc(u32 len, u8 pool);
#endif
s32 mempRealloc(void *allocation, s32 newsize, u8 poolnum);
#if MEMORY_DEBUG
void *_mempAllocFromRight(u32 len, u8 pool, char* label);
#define mempAllocFromRight(len, pool)	_mempAllocFromRight(len, pool, MEM_GET_LABEL())
#else
void *mempAllocFromRight(u32 len, u8 pool);
#endif
#define mempDisablePool(pool)

#endif
