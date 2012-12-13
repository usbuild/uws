#ifndef __UWS_STATUS_H_
#define __UWS_STATUS_H_
#include "uws.h"
#include "uws_header.h"
#include "uws_config.h"
#include <stdio.h>
#include <setjmp.h>
#define STATUS_SUM  8
#define RW_BUFF_LEN 2048

typedef int (*DataReader)(unsigned char*);
typedef struct {
    unsigned char ptr[RW_BUFF_LEN];
    size_t len;  
} RW_BUFF;

enum conn_status {//define some useful request handler statuses
    CS_WAIT,
    CS_ACCEPT,
    CS_REQUEST_READ,
    CS_UPSTREAM_WRITE,
    CS_UPSTREAM_READ,
    CS_FILE_READ,
    CS_RESPONSE_WRITE,
    CS_CLOSE
};

typedef struct {
    enum                conn_status status;     //the place of this position
    int                 clientfd;               //incoming socket fd
    int                 serverfd;               //for upstream
    FILE                *input_file;            //clientfd fdopen file
    DataReader          readData;               //function to get more response data
    RW_BUFF             rBuff;                  //save unsyncronized read data
    RW_BUFF             wBuff;
    struct http_header  *request_header;
    struct http_header  *response_header;
    jmp_buf             error_jmp_buf;          //to quick jump out of error response
    server_cfg_t*       running_server;         //Current Server Profile Used
    char                server_ip[20];
    char                *client_ip;
} ConnInfo, *pConnInfo;
#endif
