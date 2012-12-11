#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "uws_memory.h"
#include "uws_config.h"
#include "uws_router.h"
#include "uws_header.h"
#include "uws_fdhandler.h"
#include "uws_utils.h"
#include "uws_datatype.h"
#include "uws_error.h"
#include "uws_status.h"

void deal_client_fd(client_sockfd)
{
    char line[BUFF_LEN] = "",
         type[10],
         httpver[10];
    int i = 0;
    conn_info = (pConnInfo) calloc(1, sizeof(ConnInfo));

    conn_info->input_file = fdopen(client_sockfd, "r+"); 

    conn_info->request_header = (struct http_header*) uws_calloc(1, sizeof(struct http_header));
    conn_info->response_header = (struct http_header*) uws_calloc(1, sizeof(struct http_header));

    fgets(line, BUFF_LEN, conn_info->input_file);

    conn_info->request_header->url = (char*)uws_calloc(PATH_LEN, sizeof(char));
    conn_info->request_header->path = (char*)uws_malloc(PATH_LEN * sizeof(char));
    conn_info->request_header->params = NULL;
    conn_info->request_header->request_params = (char*)uws_malloc(PATH_LEN * sizeof(char));

    sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \r]", type, conn_info->request_header->url, httpver);
    strcpy(conn_info->request_header->path, conn_info->request_header->url);

    while(conn_info->request_header->url[i] != 0) {
        if(conn_info->request_header->url[i] == '?' || conn_info->request_header->url[i] == '#') {
            conn_info->request_header->url[i] = 0;
            i++;
            break;
        }
        i++;
    }
    strcpy(conn_info->request_header->request_params, conn_info->request_header->url + i);
    conn_info->request_header->method = type;
    conn_info->request_header->http_ver = httpver;
    i = 0;

    char key[BUFF_LEN];
    char value[BUFF_LEN];
    while(fgets(line, BUFF_LEN, conn_info->input_file) != NULL) {
        if(strcmp(line, "\r\n") != 0) {
            sscanf(line, "%[^:]: %[^\r\n]", key, value);
            add_header_param(key, value, conn_info->request_header);
        }
        else {
            break;
        }
    }

    char* host = get_header_param("Host", conn_info->request_header);
    if(host != NULL) 
    {
        char host_with_port[LINE_LEN] = {0};
        while(uws_config.http.servers[i] != NULL) {
            strcpy(host_with_port, uws_config.http.servers[i]->server_name);
            strcat(host_with_port, ":");
            char *port_no = itoa(uws_config.http.servers[i]->listen);
            strcat(host_with_port, port_no);
            uws_free(port_no);

            if(wildcmp(host_with_port, host) == 1) {
                //We've got a file regiestered in the config file;
                conn_info->running_server = uws_config.http.servers[i];
                break;
            }
            i++;
        }
        if(conn_info->running_server != NULL) {
            if(setjmp(conn_info->error_jmp_buf) == 0) {
                //deal with client and server ip thing
                struct sockaddr_in peeraddr;
                socklen_t peerlen;
                do{
                    getpeername(client_sockfd, (struct sockaddr *)&peeraddr, &peerlen);
                }while(ntohs(peeraddr.sin_port) == 0);
                conn_info->client_ip = uws_strdup(inet_ntoa(peeraddr.sin_addr));
                if(conn_info->running_server->facade) {
                    add_header_param("X-Forwarded-For", conn_info->client_ip, conn_info->request_header);
                    add_header_param("Client-IP", conn_info->client_ip, conn_info->request_header);
                    char *client_port = itoa(ntohs(peeraddr.sin_port));
                    add_header_param("Client-Port", client_port, conn_info->request_header);
                    uws_free(client_port);
                } else {
                    char *old_ip = get_header_param("X-Forwarded-For", conn_info->request_header);
                    char *proxy_ip = (char*) uws_malloc((strlen(old_ip) + strlen(conn_info->client_ip) + 5) * sizeof(char));
                    strcpy(proxy_ip, old_ip);
                    strcpy(proxy_ip, ",");
                    strcat(proxy_ip, conn_info->client_ip);
                    add_header_param("X-Forwarded-For", proxy_ip, conn_info->request_header);
                    uws_free(proxy_ip);
                }
                getsockname(client_sockfd, (struct sockaddr *)&peeraddr, &peerlen);
                strcpy(conn_info->server_ip, inet_ntoa(peeraddr.sin_addr));
                pathrouter(client_sockfd);
                uws_free(conn_info->client_ip);
            }
        }
    }

    fclose(conn_info->input_file);//if we don't close file, will cause memory leak
    close(client_sockfd);
    uws_free(conn_info->request_header->url);
    uws_free(conn_info->request_header->path);
    uws_free(conn_info->request_header->request_params);
    free_header_params(conn_info->request_header);
    uws_free(conn_info->request_header);
    uws_free(conn_info->response_header);
}
void handle_client_fd(int client_sockfd) {
    deal_client_fd(client_sockfd);
}
