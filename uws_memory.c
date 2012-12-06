#include "uws.h"
#include "uws_memory.h"
#define MAX_CHUNKS 200
#define TRACE_MEM

#ifdef TRACE_MEM
static int m = 0;
static int f = 0;
static void *t[MAX_CHUNKS] = {NULL};
#endif
void* uws_malloc(size_t size){

    void* p =  malloc(size + 1);

#ifdef TRACE_MEM
    m++;
    printf("malloc: %d, free: %d\n", m, f);
    t[m] = p;
#endif

    return p;
}
void* uws_free(void *ptr){

#ifdef TRACE_MEM
    f++;
    int i = 0;
    int found = 0;
    for(i; i < MAX_CHUNKS; i++) {
        if(ptr == t[i]) {
            t[i] = NULL;
            found = 1;
            break;
        }
    }
    if(!found) {
        printf("Not hit->%x", (unsigned int)ptr);
    }
    printf("malloc: %d, free: %d\n", m, f);

    if(f == 97) {
        puts("request finished");
    }
#endif

    free(ptr);
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
