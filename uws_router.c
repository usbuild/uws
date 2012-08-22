#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_router.h"
#include "uws_config.h"
#include "uws_mime.h"
#include "uws_fastcgi.h"
#include "uws_header.h"
#define MAP_LEN 20


static Router
map[MAP_LEN] = {{NULL, NULL}};
//extern router handlers
extern int dir_router(int sockfd, struct http_header *request_header);
extern int http_router(int sockfd, struct http_header *request_header);
extern int fastcgi_router(int sockfd, struct http_header *request_header);
//end extern router handler
void add_router(Router router) {
    int i = 0;
    while(map[i].preg != NULL) i++;
    map[i] = router;
}
void init_routers(){
    //---
    Router httprt;
    httprt.preg = ".*";
    httprt.func = http_router;
    add_router(httprt);
    //---

    Router fastcgirt;
    fastcgirt.preg = "/([^/]+/)*[^/]+\\.php";
    fastcgirt.func = fastcgi_router;
    add_router(fastcgirt);

    Router dirrt;
    dirrt.preg = ".*";
    dirrt.func = dir_router;
    add_router(dirrt);
}

void pathrouter(int sockfd, struct http_header *request_header) {
    char* path = request_header->url;
    int i = 0;
    while(map[i].preg != NULL) i++; //最先添加的最后执行
    i--;
    for(; i >= 0; i--) {
        int res;
        regex_t reg;

        if((res = regcomp(&reg, map[i].preg, REG_EXTENDED | REG_ICASE)) != 0) {
            puts("Compile regex error");
            exit(1);
        };
        if(regexec(&reg, path, 0, NULL, 0) == 0) {
            regfree(&reg);
            if(!map[i].func(sockfd, request_header)) return;//返回值为0则停止冒泡
        }
    }
}
