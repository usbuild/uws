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

void exit_err(const char* str);

struct nv_pair {
    char* name;
    char* value;
};

#endif

