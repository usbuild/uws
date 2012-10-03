#include "uws.h"
#include "uws_memory.h"
void* uws_malloc(size_t size){
    return malloc(size);
}
void* uws_free(void *ptr){
    free(ptr);
}
