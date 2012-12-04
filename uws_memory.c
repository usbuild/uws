#include "uws.h"
#include "uws_memory.h"
void* uws_malloc(size_t size){
    //printf("mallocated %ld\n", size);
    return malloc(size);
}
void* uws_free(void *ptr){
    //printf("free memory %x\n", ptr);
    free(ptr);
}
void *uws_calloc(size_t nmemb, size_t size) {
    //printf("callocated %ld\n", size * nmemb);
    //return calloc(nmemb, size);
    return calloc(nmemb, size);
}
void *uws_realloc(void *ptr, size_t size) {
    //printf("reallocated %ld\n", size);
    return realloc(ptr, size);
}

/*
static struct var_array{
    size_t len;
    struct {
        void *addr;
        size_t size;
    } p[1024];
} used_ptr = {0, {{NULL, 0}}}, free_ptr = {0, {NULL}}; 

void *free_all() {
    
}
*/
