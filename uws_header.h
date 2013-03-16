#ifndef __UWS_HEADER_H__
#define __UWS_HEADER_H__
#include "uws.h"
#define HEADER_LEN  4096
#define INIT_PARAMS_NUM  20
#define HEADER_SEP  "\r\n"

struct http_header{
    char* method;
    char* url;
    char* request_url;
    char* path;
    char* http_ver;
    int status_code;
    char* status;
    char* request_params;
    int used_len;
    int max_len;
    Http_Param* params;
};

char* get_header_param(char*, struct http_header*);
void add_header_param(char*, char*, struct http_header*);
void push_header_param(char*, char*, struct http_header*);
void free_header_params(struct http_header*);
char* str_response_header(struct http_header *header);
char* str_request_header(struct http_header *header);

#endif

