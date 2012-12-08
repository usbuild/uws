#include "uws.h"
#include <malloc.h>
#include "uws_memory.h"
#define USE_POOL
#define INIT_OBJS       20
#define MAX_CHUNKS      255
#define CHUNK_TOP       2048

void *
Malloc(size_t size) {
    void *p;
    if((p = malloc(size)) == NULL) {
        puts("Out of Memory");
        exit(0);
    }
    return p;
}

void *
Realloc(void *p, size_t size) {
    void *t;
    if((t = realloc(p, size)) == NULL) {
        puts("Out of Memory");
        exit(0);
    }
    return t;
}
typedef struct uws_chunk_allocator{
    unsigned char* pData;
    unsigned char firstAvailableBlock;
    unsigned char blocksAvailable;
} Chunk, *pChunk;

typedef struct uws_fixed_allocator{/*{{{*/
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

static bool 
add_new_fixed(pFixedAllocator fixObj, pObjAllocator objAlloc) {
    if(objAlloc->size == 0) {
        objAlloc->size = INIT_OBJS; 
        objAlloc->pool = (pFixedAllocator) Malloc(sizeof(FixedAllocator) * objAlloc->size);
    } else if(objAlloc->size <= objAlloc->fixCount) {
        objAlloc->size += INIT_OBJS;
        objAlloc->pool = (pFixedAllocator) Realloc(objAlloc->pool, sizeof(FixedAllocator) * objAlloc->size);
    }
    memcpy(&objAlloc->pool[objAlloc->fixCount++], fixObj, sizeof(FixedAllocator));
    return true;
}

static bool 
add_new_chunk(pFixedAllocator fixObj, pChunk chunk) {
    if(fixObj->size == 0) {
        fixObj->size = INIT_OBJS;
        fixObj->chunks = (pChunk) Malloc(sizeof(Chunk) * fixObj->size);
    } else if(fixObj->size == fixObj->chunkCount) {
        fixObj->size += INIT_OBJS;
        fixObj->chunks= (pChunk) Realloc(fixObj->chunks, sizeof(Chunk) * fixObj->size);
    }
    memcpy(&fixObj->chunks[fixObj->chunkCount++], chunk, sizeof(Chunk));
    return true;
}/*}}}*/

static pChunk/*{{{*/
init_chunk(size_t size){
    pChunk chunk = (pChunk) Malloc(sizeof(Chunk));
    int blocks = MAX_CHUNKS;
    if(size * blocks > CHUNK_TOP) {
        blocks = CHUNK_TOP / size;
    }
    blocks = (blocks == 0 ? 1 : blocks);
    chunk->pData = (unsigned char *) Malloc(sizeof(unsigned char) * size * blocks);
    chunk->firstAvailableBlock = 0;
    chunk->blocksAvailable = blocks;
    int i;
    unsigned char* tmp = chunk->pData;
    for(i = 0; i < blocks; ++i) {
        *tmp = i + 1;
        tmp += sizeof(unsigned char) * size;
    }
    return chunk;
}/*}}}*/

static void*
chunk_malloc(pChunk chunk, size_t size) {
    if(chunk->blocksAvailable == 0) return NULL;
    unsigned char *p = chunk->pData + chunk->firstAvailableBlock * size;
    chunk->firstAvailableBlock  = *p;
    --chunk->blocksAvailable;
    return p;
}

static void
chunk_free(pChunk chunk, void *p, size_t size) {//to free p, ensure p is in chunk
    *(unsigned char*)p = chunk->firstAvailableBlock;
    chunk->firstAvailableBlock = ((unsigned char *)p - chunk->pData) / size;
    ++chunk->blocksAvailable;
}/*}}}*/

static void *
fix_allocate(pFixedAllocator fixObj) {
    int i;
    for(i = 0; i < fixObj->chunkCount; ++i) {
        pChunk cur_chunk = &fixObj->chunks[i];
        if(cur_chunk->blocksAvailable != 0) {
            return chunk_malloc(cur_chunk, fixObj->blockSize);
        }
    }
    pChunk chunk = init_chunk(fixObj->blockSize);
    add_new_chunk(fixObj, chunk);
    free(chunk);
    pChunk cur_chunk = &fixObj->chunks[fixObj->chunkCount - 1];
    return chunk_malloc(cur_chunk, fixObj->blockSize);
}

static void/*{{{*/
fix_deallocate(pFixedAllocator fixObj, void *p) {
    int i;
    size_t range;
    int blocks = MAX_CHUNKS;
    if(fixObj->blockSize * blocks > CHUNK_TOP) {
        blocks = CHUNK_TOP / fixObj->blockSize;
    }
    blocks = (blocks == 0 ? 1 : blocks);
    for( i = 0; i < fixObj->chunkCount; ++i) {
        pChunk cur_chunk = &fixObj->chunks[i];
        if(cur_chunk->pData <= (unsigned char *)p && (unsigned char*)p < cur_chunk->pData + blocks * fixObj->blockSize) {
            chunk_free(cur_chunk, p, fixObj->blockSize);
        }
    }

}/*}}}*/

static void * /*{{{*/
obj_allocate(pObjAllocator objAlloc, size_t size) {
    int i = 0;
    pFixedAllocator found = NULL;
    for(i = 0; i < objAlloc->fixCount; ++i) {
        pFixedAllocator fix = &objAlloc->pool[i];
        if(fix->blockSize == size) {
            found = fix;
            break;
        }
    }
    if(found == NULL) {
        pFixedAllocator fix = (pFixedAllocator) calloc(1, sizeof(FixedAllocator));
        fix->blockSize = size;
        add_new_fixed(fix, objAlloc);
        free(fix);
        found = &objAlloc->pool[objAlloc->fixCount - 1];
    }
    return fix_allocate(found);
}/*}}}*/
static void
obj_deallocate(pObjAllocator objAlloc, void *p, size_t size) {
    int i = 0;
    pFixedAllocator found = NULL;
    for(i = 0; i < objAlloc->fixCount; ++i) {
        pFixedAllocator fix = &objAlloc->pool[i];
        if(fix->blockSize == size) {
            found = fix;
            break;
        }
    }
    if(found) {
        fix_deallocate(found, p);
        return;
    }
}

static pObjAllocator obj = NULL;

static size_t
round_up(size_t size) {
    return (size + 7) & ~7;
}

void* uws_malloc(size_t size){
#ifdef USE_POOL
    if(obj == NULL) {
        obj = (pObjAllocator) calloc(1, sizeof(ObjAllocator));
    }
    size_t real_size = round_up(size + sizeof(size_t));
    void *p = obj_allocate(obj, real_size);
    size_t* sp = (size_t*)p;
    *sp = real_size;
    return sp + 1;
#endif
#ifndef USE_POOL
    size_t real_size = round_up(size);
    void* p =  Malloc(real_size);
    return p;
#endif
}
void uws_free(void *ptr){
#ifdef USE_POOL
    if(ptr == NULL) return;
    size_t *p = (size_t*) ptr - 1;
    size_t real_size = *p;
    obj_deallocate(obj, ptr, real_size);
#endif

#ifndef USE_POOL
    free(ptr);
#endif
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
