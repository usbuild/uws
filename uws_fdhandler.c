#include <pthread.h>
#include "uws_config.h"
#include "uws_router.h"
#include "uws_header.h"
#include "uws_fdhandler.h"
struct thread_info{
    int client_sockfd;
};

void *thread_unit(void *arg)
{
    struct thread_info *info = (struct thread_info*)arg;
    int client_sockfd = info->client_sockfd;
    char line[BUFF_LEN] = "",
         type[10],
         httpver[10];
    int i = 0;

    FILE *input_file = fdopen(client_sockfd, "r+"); 

    fgets(line, BUFF_LEN, input_file);
    struct http_header request_header;
    request_header.path = (char*)calloc(PATH_LEN, sizeof(char));
    sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, request_header.path, httpver);
    request_header.method = type;
    request_header.url = (char*) calloc(strlen(request_header.path) + 1, sizeof(char)); //max index filename length
    strcpy(request_header.url, request_header.path);
    request_header.http_ver = httpver;

    while(fgets(line, BUFF_LEN, input_file) != NULL) {
        if(strcmp(line, "\r\n") != 0) {
            add_header_param(line, &request_header);
        }
        else {
            break;
        }
    }
    char* host = get_header_param("Host", &request_header);

    if(host != NULL) 
    {
        i = 0;
        while(uws_config.http.servers[i] != NULL) {
            if(wildcmp(uws_config.http.servers[i]->server_name, host) == 1) {
                //We've got a file regiestered in the config file;
                running_server = uws_config.http.servers[i];
                break;
            }
            i++;
        }
        if(running_server != NULL) {
            pathrouter(client_sockfd, &request_header);
        }
    }
    //
    close(client_sockfd);
    free(request_header.url);
    free(request_header.path);
    free_header_params(&request_header);
    return NULL;
}
void handle_client_fd(int client_sockfd) {
    int err;
    pthread_t ntid;
    struct thread_info *info = (struct thread_info*)calloc(1, sizeof(struct thread_info));
    info->client_sockfd = client_sockfd;
    /*
     * e, it is not a good idea to use multi-thread, benchmark down, on my vmware 256M ubuntu
     */
    //err = pthread_create(&ntid, NULL, thread_unit, info);
    //if(err != 0) exit_err("Fdhandler Thread:");
    thread_unit(info);//single thread
    //printf("original client_fd:%d\n", client_sockfd);
    free(info);
    return;
}

