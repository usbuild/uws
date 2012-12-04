#include "uws.h"
#include "uws_memory.h"
void* uws_malloc(size_t size){
    //printf("mallocated %ld\n", size);
    return malloc(size);
}
void* uws_free(void *ptr){
    free(ptr);
}
void *uws_calloc(size_t nmemb, size_t size) {
    //printf("callocated %ld\n", size * nmemb);
    return calloc(nmemb, size);
}
void *uws_realloc(void *ptr, size_t size) {
    //printf("reallocated %ld\n", size);
    return realloc(ptr, size);
}
