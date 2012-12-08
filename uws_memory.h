#include <sys/types.h>
void *uws_malloc(size_t size);
void uws_free(void *ptr);
void *uws_calloc(size_t nmemb, size_t size);
void *uws_realloc(void *ptr, size_t old, size_t size);
