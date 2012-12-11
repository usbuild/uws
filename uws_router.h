#ifndef __UWS_ROUTER_H__
#define __UWS_ROUTER_H__
#define BUFF_LEN    4096
#include <sys/types.h>
#include "uws_header.h"
#include "uws_status.h"

typedef struct {
    char* preg;
    int (*func)(pConnInfo);
} Router;
void pathrouter(pConnInfo);
void init_routers();

#endif
