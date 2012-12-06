#include "uws.h"
#include "uws_memory.h"
static int m = 0;
static int f = 0;
static void *t[100] = {NULL};
void* uws_malloc(size_t size){
    m++;
    printf("malloc: %d, free: %d\n", m, f);
    void* p =  malloc(size + 1);
    t[m] = p;
    return p;
}
void* uws_free(void *ptr){
    f++;
    int i = 0;
    int found = 0;
    for(i; i < 100; i++) {
        if(ptr == t[i]) {
            t[i] = NULL;
            found = 1;
            break;
        }
    }
    if(!found) {
        printf("Not hit->%x", ptr);
    }
    printf("malloc: %d, free: %d\n", m, f);

    if(f == 43) {
        puts("request finished");
    }

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
