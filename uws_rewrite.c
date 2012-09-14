#include "uws_rewrite.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_header.h"
int rewrite_router(int sockfd) {
    char **tttt = running_server->rewrite.rules.array;
    while(*tttt != NULL) {puts(*tttt); tttt++;}
    return 1;
}

