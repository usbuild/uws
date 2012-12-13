#include "uws.h"
#include <malloc.h>
#include "uws_memory.h"
#include "usmem.h"
//#define TRACE

#ifdef USE_POOL
static size_t
round_up(size_t size) {
    return (size + 7) & ~7;
}
#endif

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

#ifdef TRACE
static count = 0;
static void *pool[2000] = {NULL};
#endif

#ifdef USE_POOL
static us_allocator obj = NULL;
#endif

void* uws_malloc(size_t size){
#ifdef USE_POOL
    if(obj == NULL) {
        obj = new_us_allocator();
    }
    size_t real_size = round_up(size + sizeof(size_t));
    void *p = us_alloc(obj, real_size);
    size_t* sp = (size_t*)p;
    *sp = real_size;
    return sp + 1;
#endif

#ifndef USE_POOL
    void* p =  malloc(size);
#ifdef TRACE
    pool[count++] = p;
#endif
    return p;
#endif
}
void uws_free(void *ptr){
#ifdef USE_POOL
    if(ptr == NULL) return;
    size_t *p = (size_t*) ptr - 1;
    us_dealloc(obj, p, *p);
#endif

#ifndef USE_POOL

#ifdef TRACE
    int find = 0;
    int j;
    for(j = 0; j < count; j++) {
        if(pool[j] == ptr)  {
            find = 1;
            break;
        }
    }
    if(find == 0) {
        puts("Error");
    }
    pool[j] = NULL;
#endif

    free(ptr);
#endif
}
