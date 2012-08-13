#ifndef __UWS_HEADER_H__
#define __UWS_HEADER_H__
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "uws.h"

struct http_header{
    char* method;
    char* url;
    char* path;
    char* http_ver;
    char* request_params;
    Http_Param* params;
};
struct http_header request_header;

#endif

