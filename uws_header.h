#ifndef __UWS_HEADER_H__
#define __UWS_HEADER_H__
#include "uws.h"
#define INIT_PARAM_NUM

struct http_header{
    char* method;
    char* url;
    char* path;
    char* http_ver;
    char* request_params;
    int used_len;
    int max_len;
    Http_Param* params;
};
struct http_header request_header;

char* get_header_param(char*);
void add_header_param(char*);

#endif

