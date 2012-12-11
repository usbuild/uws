#include "uws_error.h"
#include "uws_memory.h"
#include "uws_utils.h"
#include "uws_header.h"
#include "uws_config.h"
#include "uws_http.h"
void send_error_response(pConnInfo conn_info, const int status_code, const bool with_page) {
    char** error_pages = conn_info->running_server->error_page;
    int i;
    char *error_path = NULL;
    char *error_file_path;
    while(*error_pages != NULL) {
        for(i = 0; i < strlen(*error_pages); i++) {
            if((*error_pages)[i] == '=') {
                (*error_pages)[i] = '\0';
                if(atoi(*error_pages) == status_code) {
                    error_path = uws_strdup(*error_pages + i + 1);
                    (*error_pages)[i] = '=';
                    break;
                }
                (*error_pages)[i] = '=';
            }
        }
        if(error_path != NULL) break;
        error_pages++;
    }
    int content_len;
    char *content;
    if(with_page) {
        error_file_path  = uws_strdup("/dev/null"); //just for test
        if(error_path != NULL) {
            char *tmp_path = strlcat(conn_info->running_server->root, error_path);
            if(access(tmp_path, F_OK) == 0) {
                uws_free(error_file_path);
                error_file_path = tmp_path;
                uws_free(error_path);
            } else {
                uws_free(tmp_path);
            }
        }
        FILE* file = fopen(error_file_path, "r");
        fseek(file, 0, SEEK_END);
        content_len = ftell(file);
        rewind(file);
        content = (char*) uws_malloc (content_len * sizeof(char));

        fread(content, sizeof(char), content_len, file);
        fclose(file);
    } else {
        content_len = 0;
        content = uws_strdup("");
    }

    //go here
    char *time_string = get_time_string(NULL);

    conn_info->response_header->http_ver = "HTTP/1.1";
    conn_info->response_header->status_code = status_code;
    conn_info->response_header->status = get_by_code(status_code);
    add_header_param("Cache-Control", "private", conn_info->response_header);
    add_header_param("Connection", "Keep-Alive", conn_info->response_header);
    add_header_param("Server", UWS_SERVER, conn_info->response_header);
    add_header_param("Date", time_string, conn_info->response_header);
    add_header_param("Content-Type", "text/html", conn_info->response_header);


    if(with_page) {
        char *content_len_str = itoa(content_len);
        add_header_param("Content-Length", content_len_str, conn_info->response_header);
        uws_free(content_len_str);
    }


    struct response header_body;

    header_body.header = conn_info->response_header;
    header_body.content = content;
    header_body.content_len = content_len;

    uws_free(time_string);
    write_response(conn_info->clientfd, &header_body);
    free_header_params(header_body.header);
    //uws_free(header_body.header); don't free this!!
    uws_free(header_body.content);
    longjmp(conn_info->error_jmp_buf, 1);
}
char *get_by_code(int code) {
    int i = 0;
    while(http_status[i].code != -1) {
        if(http_status[i].code == code) return http_status[i].message;
        i++;
    }
    return http_status[i].message;
}
