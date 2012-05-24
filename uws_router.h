#ifndef __UWS_ROUTER_H__
#define __UWS_ROUTER_H__
#define PATH_LEN    512
#define BUFF_LEN    4096
#include <sys/types.h>
#include <regex.h>


typedef struct {
    char* preg;
    int (*func)(int, struct http_header*);
} Router;
void
pathrouter( int sockfd, struct http_header* header);
void
init_routers();


#endif
