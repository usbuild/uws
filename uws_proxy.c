#include "uws.h"
#include "uws_header.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_proxy.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

static void 
split_string(char *src, char **host, int *port, char **regexp) {
    int len = strlen(src);
    *host = (char*)calloc(len, sizeof(char));
    char aport[10];
    *regexp = (char*)calloc(len, sizeof(char));
    sscanf(src, "server%*[ ]%[^:]:%[^ ]%*[ ]%[^;]", *host, aport, *regexp);
    *port = atoi(aport);

}
static int
handle_proxy(int client_fd, const char *host, int port) {
    int sockfd, n;
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
    char *request_str = str_request_header(request_header);
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
int proxy_router(int serverfd)
{
    int i = 0;
    if(running_server->upstream.total == 0) return 1;
    char *host, *regexp;
    int port;
    for(i = 0; i < running_server->upstream.len; i ++) {
        split_string(running_server->upstream.array[i], &host, &port, &regexp);
        if(preg_match(request_header->url, regexp)) {
            int res = handle_proxy(serverfd, host, port);
            free(host);
            free(regexp);
            return res;
        }
        free(host);
        free(regexp);
    }
    return 0;
}
