#include "uws.h"
enum conn_status {//define some useful request handler statuses
    CS_ACCEPT,
    CS_REQUEST_READ,
    CS_UPSTREAM_WRITE,
    CS_UPSTREAM_READ,
    CS_RESPONSE_WRITE,
    CS_CLOSE
};

typedef struct {
    enum conn_status status;        //the place of this position
    int clientfd;                   //incoming socket fd
    int serverfd;                   //for upstream
    int readData(unsigned char*);   //function to get more response data
} ConnInfo, *pConnInfo;
