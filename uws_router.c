#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_router.h"
#include "uws_config.h"
#include "uws_utils.h"
#include "uws_mime.h"
#include "uws_header.h"
#include "uws_status.h"
#define MAP_LEN 20      //we design max router null


static Router
map[MAP_LEN] = {{NULL, NULL}};
//extern router handlers
extern void dir_router(pConnInfo);
extern void http_router(pConnInfo);
extern void fastcgi_router(pConnInfo);
extern void rewrite_router(pConnInfo);
extern void auth_router(pConnInfo);
extern void proxy_router(pConnInfo);
//end extern router handler
void add_router(Router router) {
    int i = 0;
    while(map[i].preg != NULL) i++;
    map[i] = router;
}
void init_routers(){
    Router proxyrt;
    proxyrt.preg = ".*";
    proxyrt.func = proxy_router;
    add_router(proxyrt);


    Router rewritert;
    rewritert.preg = ".*";
    rewritert.func = rewrite_router;
    add_router(rewritert);


    Router authrt;
    authrt.preg = ".*";
    authrt.func = auth_router;
    add_router(authrt);

    Router dirrt;
    dirrt.preg = ".*";
    dirrt.func = dir_router;
    add_router(dirrt);


    Router fastcgirt;
    fastcgirt.preg = "/([^/]+/)*[^/]+\\.php";
    fastcgirt.func = fastcgi_router;
    add_router(fastcgirt);

    //--- FILO
    Router httprt;
    httprt.preg = ".*";
    httprt.func = http_router;
    add_router(httprt);
    //---
}

/*
void pathrouter(pConnInfo conn_info) {
    int i = 0;
    while(map[i].preg != NULL) i++; //最先添加的最后执行
    i--;
    for(; i >= 0; i--) {
        if(preg_match(conn_info->request_header->path, map[i].preg)) {
            if(!map[i].func(conn_info)) return;//返回值为0则停止冒泡
        }
    }
}
*/

void apply_next_router(pConnInfo conn_info) {
    int i = conn_info->request_id;
    if(map[i].preg == NULL) return;
    ++conn_info->request_id;
    if(preg_match(conn_info->request_header->path, map[i].preg)) {
        map[i].func(conn_info);
    } else {
        apply_next_router(conn_info);
    }
}
