#include "uws.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "uws_socket.h"
#include "uws_mime.h"
#include "uws_config.h"
#include "uws_fileio.h"

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
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int res;
    //---
    signal(SIGINT, sig_int);
    //---
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

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
             path[PATH_LEN],
             type[10],
             httpver[10];
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        FILE *input_file = fdopen(client_sockfd, "r+"); 

        fgets(line, BUFF_LEN, input_file);
        printf("%s", line);
        sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, path, httpver);


        while(fgets(line, BUFF_LEN, input_file) != NULL) {
            printf("%s", line);
            if(strlen(line) == 2) break;
        }

        pathrouter(path, input_file);
        fflush(input_file);
        close(client_sockfd);
    }

}
