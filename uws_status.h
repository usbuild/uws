#include "uws.h"
typedef int (*DataReader)(unsigned char*);
enum conn_status {//define some useful request handler statuses
    CS_ACCEPT,
    CS_REQUEST_READ,
    CS_UPSTREAM_WRITE,
    CS_UPSTREAM_READ,
    CS_FILE_READ,
    CS_RESPONSE_WRITE,
    CS_CLOSE
};

typedef struct {
    enum conn_status status;        //the place of this position
    int clientfd;                   //incoming socket fd
    int serverfd;                   //for upstream
    DataReader readData;            //function to get more response data
    unsigned char *buff;            //save unsyncronized data
} ConnInfo, *pConnInfo;

