#include "uws.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "uws_socket.h"
#include "uws_mime.h"
#include "uws_config.h"
#include "uws_router.h"
#include "uws_header.h"

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
    signal(SIGINT, sig_int);
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
    while(1) { char line[BUFF_LEN] = "",
             type[10],
             httpver[10];
        int i = 0;
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        pid_t pid = fork();
        if(pid == 0) {

            FILE *input_file = fdopen(client_sockfd, "r+"); 
            fgets(line, BUFF_LEN, input_file);
            request_header.path = (char*)calloc(PATH_LEN, sizeof(char));

            sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, request_header.path, httpver);
            request_header.method = type;
            request_header.url = (char*) calloc(strlen(request_header.path) + 1, sizeof(char)); //max index filename length
            strcpy(request_header.url, request_header.path);
            request_header.http_ver = httpver;

            while(fgets(line, BUFF_LEN, input_file) != NULL) {
                if(strcmp(line, "\r\n") != 0) {
                    add_header_param(line);
                }
                else {
                    break;
                }
            }
            char* host = get_header_param("Host");

            if(host == NULL) exit(0);

            int i = 0;
            while(uws_config.http.servers[i] != NULL) {
                if(strcmp(host, uws_config.http.servers[i]->server_name) == 0) {
                    //We've got a file regiestered in the config file;
                    running_server = uws_config.http.servers[0];
                    break;
                }
                i++;
            }
            if(running_server == NULL) exit(0);
            //
            pathrouter(client_sockfd);
            close(client_sockfd);
            free(request_header.url);
            free(request_header.path);
            free(request_header.request_params);
            exit(0);
        }
    close(client_sockfd);
    }

}
