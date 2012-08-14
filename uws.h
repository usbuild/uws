#ifndef __UWS_H__
#define  __UWS_H__
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define LINE_LEN    256
#define OPT_LEN     20
#define VLU_LEN     50
#define PATH_LEN    512
#define bool        int
#define true        1
#define false       0


void exit_err(const char* str);

typedef struct nv_pair {
    char* name;
    char* value;
}Http_Param, Param_Value;

struct response {
    char    *header;
    int     header_len;
    char    *content;
    int     content_len;
};
char* find_value(char* key);

#endif

