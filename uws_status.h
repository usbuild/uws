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
    enum conn_status status;
} ConnInfo, *pConnInfo;
