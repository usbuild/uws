#ifndef __UWS_HEADER_H__
#define __UWS_HEADER_H__
#include "uws.h"

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

char* get_header_param(char*, struct http_header *request_header);
void add_header_param(char*, struct http_header *request_header);
void free_header_params(struct http_header *request_header);

#endif

