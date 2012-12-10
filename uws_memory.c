#include "uws.h"
#include <malloc.h>
#include "uws_memory.h"
#include "usmem.h"

static size_t
round_up(size_t size) {
    return (size + 7) & ~7;
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

static us_allocator obj = NULL;

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
    free(ptr);
#endif
}
