#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "uws_socket.h"
#include "fileio.h"
int start_server()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 5);
    printf("Server Listening On: %d\n", PORT);
    while(1) {
        int i;
        char line[BUFF_LEN] = "";
        char path[PATH_LEN];
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        i = 0;

        FILE *input_file = fdopen(client_sockfd, "r+"); 

        while(++i) {
            fgets(line, BUFF_LEN, input_file);
            printf("%s", line);
            strtok(line, " ");
            if(i == 1) {
                strcpy(path, strtok(NULL, " "));
            }
            if(strlen(line) == 2) break;
        }

        printf("%s\n", path);
        dir_or_file(path, input_file);
        fflush(input_file);
        close(client_sockfd);
    }

}
