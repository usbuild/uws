#include "uws.h"
#include <malloc.h>
#include "uws_memory.h"
#define INIT_OBJS 20

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
    size_t size;
    size_t fixCount;
    size_t fixSize;
} ObjAllocator, *pObjAllocator;

typedef struct uws_allocator {
    pObjAllocator objs;
    size_t objCount;
    size_t size;
} Allocator, *pAllocator;

static Allocator allocator = {NULL, 0, 0};

static bool
add_new_obj(pObjAllocator newObj) {

    if(allocator.size == 0) {
        allocator.size = INIT_OBJS;
        allocator.objs = (pObjAllocator) malloc(allocator.size * sizeof(ObjAllocator) );
        if(!allocator.objs) {
            return false;
        }
    } else if(allocator.size == allocator.objCount) {
        allocator.size += INIT_OBJS;
        allocator.objs = (pObjAllocator) realloc(allocator.objs, allocator.size);
        if(!allocator.objs) {
            return false;
        }
    }
    memcpy(&allocator.objs[allocator.objCount++], newObj, sizeof(ObjAllocator));
    return true;
}
static bool 
add_new_fixed(pFixedAllocator fixObj) {
    pObjAllocator cur_obj = &allocator.objs[allocator.objCount - 1];
    if(cur_obj->size == 0) {
        cur_obj->size = INIT_OBJS; 
        cur_obj->pool = (pFixedAllocator) malloc(sizeof(FixedAllocator) * cur_obj->size);
        if(!cur_obj->pool) {
            return false;
        }
    } else if(cur_obj->size == cur_obj->fixCount) {
        cur_obj->size += INIT_OBJS;
        cur_obj->pool = (pFixedAllocator) malloc(sizeof(FixedAllocator) * cur_obj->size);
        if(!cur_obj->pool) {
            return false;
        }
    }
    memcpy(&cur_obj->pool[cur_obj->fixCount++], fixObj, sizeof(FixedAllocator));
    return true;
}



static size_t
round_up(size_t size) {
    return (size + 8) & ~8;
}

void* uws_malloc(size_t size){
    void* p =  malloc(size + sizeof(size_t));
    size_t* sp = (size_t*)p;
    *sp = size;
    printf("\t%d", size);
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
