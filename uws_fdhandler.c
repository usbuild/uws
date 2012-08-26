#include "uws_config.h"
#include "uws_router.h"
#include "uws_header.h"
#include "uws_fdhandler.h"
#include "uws_utils.h"
#include "uws_datatype.h"
#include "uws_error.h"

void deal_client_fd(client_sockfd)
{
    char line[BUFF_LEN] = "",
         type[10],
         httpver[10];
    int i = 0;

    FILE *input_file = fdopen(client_sockfd, "r+"); 

    fgets(line, BUFF_LEN, input_file);
    request_header = (struct http_header*) calloc(1, sizeof(struct http_header));
    response_header = (struct http_header*) calloc(1, sizeof(struct http_header));

    request_header->path = (char*)calloc(PATH_LEN, sizeof(char));
    request_header->params = NULL;
    request_header->request_params = (char*)calloc(PATH_LEN, sizeof(char));

    sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, request_header->path, httpver);
    request_header->method = type;
    request_header->url = (char*) calloc(strlen(request_header->path) + 1, sizeof(char)); //max index filename length
    strcpy(request_header->url, request_header->path);
    request_header->http_ver = httpver;

    char key[BUFF_LEN];
    char value[BUFF_LEN];
    while(fgets(line, BUFF_LEN, input_file) != NULL) {
        if(strcmp(line, "\r\n") != 0) {
            sscanf(line, "%[^:]: %[^\r\n]", key, value);
            add_header_param(key, value, request_header);
        }
        else {
            break;
        }
    }
    char* host = get_header_param("Host", request_header);

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
            pathrouter(client_sockfd);
        }
    }
    //
    //send_error_response(client_sockfd, 404);

    fclose(input_file);//if we don't close file, will cause memory leak
    close(client_sockfd);
    free(request_header->url);
    free(request_header->path);
    free(request_header->request_params);
    free_header_params(request_header);
}
void handle_client_fd(int client_sockfd) {
    deal_client_fd(client_sockfd);
}
