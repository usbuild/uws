#include "uws_error.h"
#include "uws_utils.h"
#include "uws_header.h"
#include "uws_config.h"
void send_error_response(int client_fd, const int status_code) {
    char** error_pages = running_server->error_page;
    int i;
    char *error_path = NULL;
    char *error_file_path;
    while(*error_pages != NULL) {
        for(i = 0; i < strlen(*error_pages); i++) {
            if((*error_pages)[i] == '=') {
                (*error_pages)[i] = '\0';
                if(atoi(*error_pages) == status_code) {
                    error_path = strdup(*error_pages + i + 1);
                    (*error_pages)[i] = '=';
                    break;
                }
                (*error_pages)[i] = '=';
            }
        }
        if(error_path != NULL) break;
        error_pages++;
    }
    error_file_path  = strdup("/etc/hostname"); //just for test
    if(error_path != NULL) {
        if(access(error_file_path, F_OK)) {
            free(error_file_path);
            error_file_path = strlcat(running_server->root, error_path);
            free(error_path);
        }
    }
    FILE* file = fopen(error_file_path, "r");
    fseek(file, 0, SEEK_END);
    int content_len = ftell(file);
    rewind(file);
    char *content = (char*) calloc (content_len, sizeof(char));
    fread(content, sizeof(char), content_len, file);
    fclose(file);

    //
    char *time_string = get_time_string();
    response_header->http_ver = "HTTP/1.1";
    response_header->status_code = status_code;
    response_header->status = "Error";
    add_header_param("Cache-Control", "private", response_header);
    add_header_param("Connection", "Keep-Alive", response_header);
    add_header_param("Server", "UWS/0.001", response_header);
    add_header_param("Date", time_string, response_header);
    add_header_param("Content-Length", itoa(content_len), response_header);
    add_header_param("Content-Type", "text/html", response_header);
    char *header = str_response_header(response_header);
    int header_len = strlen(header);
    free(time_string);
    //
    write(client_fd, header, header_len);
    write(client_fd, HEADER_SEP, strlen(HEADER_SEP));
    write(client_fd, content, content_len);
    free(header);
    free(content);
    free_header_params(response_header);
}
