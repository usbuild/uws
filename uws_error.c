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
        error_pages ++;
    }
    error_file_path  = strdup("/etc/hostname"); //just for test
    if(error_path != NULL) {
        if(access(error_file_path, F_OK)) {
            free(error_file_path);
            error_file_path = strlcat(running_server->root, error_path);
            free(error_path);
        }
    }
}
char* find_file_by_status(int status_code) {
    return NULL;
}
