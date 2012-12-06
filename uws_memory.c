#include "uws.h"
#include "uws_memory.h"
void* uws_malloc(size_t size){
    //printf("mallocated %ld\n", size);
    void* p =  malloc(size);
    return p;
}
void* uws_free(void *ptr){
    //printf("free memory %x\n", ptr);
    free(ptr);
}
void *uws_calloc(size_t nmemb, size_t size) {
    //printf("callocated %ld\n", size * nmemb);
    int s = nmemb * size;
    void *p = uws_malloc(s);
    bzero(p, s);
    return p;
}
void *uws_realloc(void *ptr, size_t old,  size_t size) {
    //printf("reallocated %ld\n", size);
    void *d = uws_malloc(size);
    memcpy(d, ptr, old);
    uws_free(ptr);
    return d;
}
