#include "uws.h"
#include "uws_memory.h"
#include "uws_header.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_status.h"
#include "uws_router.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

static void 
split_string(char *src, char **host, int *port, char **regexp) {
    int len = strlen(src);
    *host = (char*)uws_calloc(len, sizeof(char));
    char aport[10];
    *regexp = (char*)uws_calloc(len, sizeof(char));
    sscanf(src, "server%*[ ]%[^:]:%[^ ]%*[ ]%[^;]", *host, aport, *regexp);
    *port = atoi(aport);

}
static int
handle_proxy(pConnInfo conn_info, const char *host, int port) {
    int sockfd, n;
    int client_fd = conn_info->clientfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return 1;
    server = gethostbyname(host);
    if (server == NULL)  return 1;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(  (char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  return 1;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    char *request_str = str_request_header(conn_info->request_header);
    n = write(sockfd, request_str, strlen(request_str));
    if (n < 0) return 1;
    n = write(sockfd, "\r\n", strlen("\r\n"));
    do{
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        write(client_fd, buffer, n);
    } while (n > 0);
    return 0;
}
void proxy_router(pConnInfo conn_info)
{
    int i = 0;
    if(!conn_info->running_server->proxy) {
        apply_next_router(conn_info);
        return;
    }
    char *host, *regexp;
    int port;
    for(i = 0; i < conn_info->running_server->upstream.len; i ++) {
        split_string(conn_info->running_server->upstream.array[i], &host, &port, &regexp);
        if(preg_match(conn_info->request_header->url, regexp)) {
            int res = handle_proxy(conn_info, host, port);
            uws_free(host);
            uws_free(regexp);
            if(!res) {
                apply_next_router(conn_info);
                return;
            }
            else return;
        }
        uws_free(host);
        uws_free(regexp);
    }
    return;
}
