#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

    request_header = (struct http_header*) calloc(1, sizeof(struct http_header));
    request_content = (memory_t *) calloc(1, sizeof(memory_t));
    response_header = (struct http_header*) calloc(1, sizeof(struct http_header));

    fgets(line, BUFF_LEN, input_file);

    request_header->url = (char*)calloc(PATH_LEN, sizeof(char));
    request_header->path = (char*)calloc(PATH_LEN, sizeof(char));
    request_header->params = NULL;
    request_header->request_params = (char*)calloc(PATH_LEN, sizeof(char));

    sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \r]", type, request_header->url, httpver);
    strcpy(request_header->path, request_header->url);

    while(request_header->url[i] != 0) {
        if(request_header->url[i] == '?' || request_header->url[i] == '#') {
            request_header->url[i] = 0;
            i++;
            break;
        }
        i++;
    }
    strcpy(request_header->request_params, request_header->url + i);
    request_header->method = type;
    request_header->http_ver = httpver;
    i = 0;

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
    //get content body
    for(; ;) {
        if(feof(input_file) || ferror(input_file)) break;
        size_t read_num = fread(line, sizeof(char), LINE_LEN, input_file);
        append_mem_t(request_content, line, read_num);//add to content
    }
    request_content->mem[request_content->len] = 0;

    char* host = get_header_param("Host", request_header);
    if(host != NULL) 
    {
        char host_with_port[LINE_LEN] = {0};
        while(uws_config.http.servers[i] != NULL) {
            strcpy(host_with_port, uws_config.http.servers[i]->server_name);
            strcat(host_with_port, ":");
            char *port_no = itoa(uws_config.http.servers[i]->listen);
            strcat(host_with_port, port_no);
            free(port_no);

            if(wildcmp(host_with_port, host) == 1) {
                //We've got a file regiestered in the config file;
                running_server = uws_config.http.servers[i];
                break;
            }
            i++;
        }
        if(running_server != NULL) {
            if(setjmp(error_jmp_buf) == 0) {
                //deal with client and server ip thing
                struct sockaddr_in peeraddr;
                socklen_t peerlen;
                do{
                    getpeername(client_sockfd, (struct sockaddr *)&peeraddr, &peerlen);
                }while(ntohs(peeraddr.sin_port) == 0);
                client_ip = strdup(inet_ntoa(peeraddr.sin_addr));
                if(running_server->facade) {
                    add_header_param("X-Forwarded-For", client_ip, request_header);
                    add_header_param("Client-IP", client_ip, request_header);
                    char *client_port = itoa(ntohs(peeraddr.sin_port));
                    add_header_param("Client-Port", client_port, request_header);
                } else {
                    char *old_ip = get_header_param("X-Forwarded-For", request_header);
                    char *proxy_ip = (char*) calloc(strlen(old_ip) + strlen(client_ip) + 5, sizeof(char));
                    strcpy(proxy_ip, old_ip);
                    strcpy(proxy_ip, ",");
                    strcat(proxy_ip, client_ip);
                    add_header_param("X-Forwarded-For", proxy_ip, request_header);
                    free(proxy_ip);
                }
                getsockname(client_sockfd, (struct sockaddr *)&peeraddr, &peerlen);
                strcpy(server_ip, inet_ntoa(peeraddr.sin_addr));
                pathrouter(client_sockfd);
                free(client_ip);
            }
        }
    }

    fclose(input_file);//if we don't close file, will cause memory leak
    close(client_sockfd);
    free(request_content->mem);
    free(request_content);
    free(request_header->url);
    free(request_header->path);
    free(request_header->request_params);
    free_header_params(request_header);
}
void handle_client_fd(int client_sockfd) {
    deal_client_fd(client_sockfd);
}
