#ifndef __UWS_ROUTER_H__
#define __UWS_ROUTER_H__
#include <sys/types.h>
#include "uws_header.h"
#include "uws_status.h"

typedef struct {
    char* preg;
    void (*func)(pConnInfo);
} Router;
void init_routers();
void apply_next_router(pConnInfo conn_info);
#endif
