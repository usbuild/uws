#ifndef __US_MEM_H__
#define __US_MEM_H__
#include <stddef.h>
typedef struct uws_chunk_allocator{
    unsigned char* pData;
    unsigned char firstAvailableBlock;
    unsigned char blocksAvailable;
} Chunk, *pChunk;

typedef struct uws_fixed_allocator{
    size_t blockSize;
    size_t size;
    size_t chunkCount;
    unsigned char numBlocks;
    pChunk chunks;
    pChunk allocChunk;
    pChunk deallocChunk;
    struct uws_fixed_allocator *prev;
    struct uws_fixed_allocator *next;
} FixedAllocator, *pFixedAllocator;

typedef struct uws_obj_allocator {
    pFixedAllocator pool;
    pFixedAllocator pLastAlloc;
    pFixedAllocator pLastDealloc;
    size_t size;
    size_t fixCount;
    size_t fixSize;
} ObjAllocator, *pObjAllocator;

typedef pObjAllocator us_allocator;

extern void * us_alloc(us_allocator, size_t);
extern void us_dealloc(us_allocator, void*, size_t);
extern us_allocator new_us_allocator();

#endif
