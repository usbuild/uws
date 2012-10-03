#include "uws.h"
#include "uws_header.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_proxy.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int proxy_router(int serverfd)
{
    int i = 0;
    if(running_server->upstream.total == 0) return 1;
    /*
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    portno = atoi("80");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    bzero(buffer,256);
    sprintf(buffer, "GET / HTTP/1.1 \r\nHost: localhost\r\n\r\n");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) error("ERROR writing to socket");
    do{
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        printf("%s",buffer);
    }while(n > 0);
    return 0;
    */
    return 0;
}
