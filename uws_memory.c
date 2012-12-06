#include "uws.h"
#include <malloc.h>
#include "uws_memory.h"
#define INIT_OBJS

typedef struct uws_chunk_allocator{
    unsigned char* pData;
    unsigned char firstAvailableBlock;
    unsigned char blocksAvailable;
} Chunk, *pChunk;

typedef struct uws_fixed_allocator{
    size_t blockSize;
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
    size_t chunkSize;
    size_t maxObjectSize;
}ObjAllocator, *pObjAllocator;

static size_t
round_up(size_t size) {
    return (size + 8) & ~8;
}

void* uws_malloc(size_t size){
    void* p =  malloc(size + sizeof(size_t));
    size_t* sp = (size_t*)p;
    *sp = size;
    return sp + 1;
}
void* uws_free(void *ptr){
    free((size_t*)ptr - 1);
}
void *uws_calloc(size_t nmemb, size_t size) {
    int s = nmemb * size;
    void *p = uws_malloc(s);
    bzero(p, s);
    return p;
}
void *uws_realloc(void *ptr, size_t old,  size_t size) {
    void *d = uws_malloc(size);
    memcpy(d, ptr, old);
    uws_free(ptr);
    return d;
}
