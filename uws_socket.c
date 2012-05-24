#include "uws.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "uws_socket.h"
#include "uws_mime.h"
#include "uws_config.h"
#include "uws_router.h"

int server_sockfd, client_sockfd;
static void
sig_int(int signo)
{
    close(server_sockfd);
    printf("\nUser terminated\n");
    exit(0);
}
int start_server()
{
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int res;
    int reuse = 1;
    //---
    signal(SIGINT, sig_int);
    //---
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    res = setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if(res < 0) {
        exit_err("Set Socket Option Fail");
    }

    server_len = sizeof(server_address);
    res = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    if(res < 0) {
        exit_err("Bind Error");
    }

    res = listen(server_sockfd, 500);
    if(res < 0) {
        exit_err("Listen Error");
    }
    printf("Server Listening On: %d\n", PORT);
    while(1) {
        char line[BUFF_LEN] = "",
             type[10],
             httpver[10];
        int i = 0;
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        pid_t pid = fork();
        if(pid == 0) {

            struct http_header header;

            FILE *input_file = fdopen(client_sockfd, "r+"); 
            fgets(line, BUFF_LEN, input_file);
            header.path = (char*)calloc(sizeof(char), PATH_LEN);
            sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, header.path, httpver);
            header.method = type;
            header.url = (char*) calloc(sizeof(char), strlen(header.path) + 1); //max index filename length
            strcpy(header.url, header.path);
            header.http_ver = httpver;
            header.params = (Http_Param *) calloc(sizeof(Http_Param), MAX_HEADER);

            while(fgets(line, BUFF_LEN, input_file) != NULL) {
                Http_Param param;
                if(strcmp(line, "\r\n") != 0) {
                    param.name = (char*) calloc(sizeof(char), HEADER_OPT);
                    param.value = (char*) calloc(sizeof(char), strlen(line));
                    sscanf(line, "%[^:]: %[^\r\n]", param.name, param.value);
                    header.params[i++] = param;
                }
                else {
                    param.name = NULL;
                    param.value = NULL;
                    header.params[i] = param;
                    break;
                }
            }
            pathrouter(client_sockfd, &header);
            close(client_sockfd);
            free(header.url);
            free(header.path);
            free(header.request_params);
            exit(0);
        }
    close(client_sockfd);
    }

}
