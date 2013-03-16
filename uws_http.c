#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_memory.h"
#include "uws_http.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_mime.h"
#include "uws_header.h"
#include "uws_status.h"
#include "uws_router.h"

struct response header_body;
static char* mime;

static char*
get_mime(const char* path)
{
    int i = 0;
    i = strlen(path);
    i--;
    while(*(path + i) != '.') {
        if(i == 0) {
            return uws_strdup("text/html");
            break;
        }
        i--;
    }
    if(i != 0){
        return mimebyext(path + i + 1);
    }
    return uws_strdup("text/html");
}
static void
set_header(pConnInfo conn_info) {
    char *time_string = get_time_string(NULL);
    conn_info->response_header->http_ver = "HTTP/1.1";
    conn_info->response_header->status_code = 200;
    conn_info->response_header->status = "OK";
    add_header_param("Cache-Control", "private", conn_info->response_header);
    add_header_param("Connection", "Keep-Alive", conn_info->response_header);
    add_header_param("Vary", "Accept-Encoding", conn_info->response_header);
    add_header_param("Server", UWS_SERVER, conn_info->response_header);
    add_header_param("Date", time_string, conn_info->response_header);

    char *len = itoa(header_body.content_len);
    add_header_param("Content-Length", len, conn_info->response_header);
    uws_free(len);

    add_header_param("Content-Type", mime, conn_info->response_header);
    header_body.header = conn_info->response_header;
    uws_free(time_string);
}

int
comparestr(const void *p1, const void *p2)
{
    return strcmp(* (char * const *)p1, * (char * const *) p2);
}
static void
printdir(const char *fpath, pConnInfo conn_info) {//打印目录项排序
    if(!conn_info->running_server->autoindex) {
        /*
         * error response
        conn_info->status_code = 403;
        apply_next_router(conn_info);
        return;
        */
    }

    DIR *dp = opendir(fpath);
    struct dirent *dir_entry;
    int dir_len = 0;
    char **entries;
    struct stat stat_buff;

    while((dir_entry = readdir(dp)) != NULL)
        dir_len++;

    header_body.content_len = dir_len * 64;//Max filename Length
    header_body.content = (char*) uws_malloc (header_body.content_len * sizeof(char));
    header_body.content[0] = 0;

    rewinddir(dp);
    entries = (char**) uws_malloc ((dir_len + 5) * sizeof(char*));
    dir_len = 0;
    //
    //判断是否是目录
    //
    
    while((dir_entry = readdir(dp)) != NULL) {
        char *newpath = (char*) uws_malloc(PATH_LEN * sizeof(char));//max filename length
        strcpy(newpath, fpath);

        entries[dir_len++] = dir_entry->d_name;

        //strcat(newpath, "/");
        strcat(newpath, dir_entry->d_name);
        lstat(newpath, &stat_buff);
        if(S_ISDIR(stat_buff.st_mode)) {
            strcat(entries[dir_len - 1], "/");
        }
        uws_free(newpath);
    }
    entries[dir_len] = NULL;
    qsort(entries, dir_len, sizeof(char*), comparestr);

    while(*(entries)!= NULL) {
        strcat(header_body.content, "<a href=\"");
        strcat(header_body.content, *entries);
        strcat(header_body.content, "\">");
        strcat(header_body.content, *(entries++));
        strcat(header_body.content, "</a><br/>\n");
    }
    header_body.content_len = strlen(header_body.content);
    closedir(dp);
}
static void
printfile(const char *path, pConnInfo conn_info)
{
    char *mod_time_str ;
    char *file_mod_time = get_file_time(path);
    if((mod_time_str = get_header_param("If-Modified-Since", conn_info->request_header))) {
        if(!is_expire(mod_time_str, file_mod_time)) {
            conn_info->status_code = 304;
            send_error_response(conn_info);
            return;
        }
    }

    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    header_body.content_len = ftell(file);
    rewind(file);

    add_header_param("Last-Modified", file_mod_time, conn_info->response_header);
    uws_free(file_mod_time);

    header_body.content = (char*) uws_malloc (header_body.content_len * sizeof(char));
    size_t read_size = fread(header_body.content, sizeof(char), header_body.content_len, file);
    fclose(file);

}

int
http_router(pConnInfo conn_info) 
{
    if(conn_info->status_code != 0) send_error_response(conn_info);
    int sockfd = conn_info->clientfd;
    char path[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    strcpy(path, conn_info->request_header->path);
    while(path[i] != 0) {
        if(path[i] == '?' || path[i] == '#') {
            path[i] = 0;
            break;
        }
        i++;
    }
    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            mime = uws_strdup("text/html");
            printdir(path, conn_info);
        }
        else
        {
            mime = get_mime(path);
            printfile(path, conn_info);
        }
    }
    else {
        conn_info->status_code = 404;
        send_error_response(conn_info);
        return;
    }

    set_header(conn_info);
    write_response(conn_info, &header_body);
    //uws_free(header_body.header);  sorry, reponse_header will be freed at the end of request
    uws_free(header_body.content);
    free_header_params(conn_info->response_header);
}


void send_error_response(pConnInfo conn_info) {
    int status_code = conn_info->status_code;
    bool with_page;
    int return_page_status[4] = {404, 502, 500, 403};
    if(in_int_array(return_page_status, status_code, 4) != -1) {
        with_page = true;
    } else {
        with_page = false;
    }
    char** error_pages = conn_info->running_server->error_page;
    int i;
    char *error_path = NULL;
    char *error_file_path;
    conn_info->status_code = status_code;
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

        size_t read_size = fread(content, sizeof(char), content_len, file);
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
    write_response(conn_info, &header_body);
    free_header_params(header_body.header);
    //uws_free(header_body.header); don't free this!!
    uws_free(header_body.content);
    //longjmp(conn_info->error_jmp_buf, 1);
}

char *get_by_code(int code) {
    int i = 0;
    for(; http_status[i].code != -1; ++i) {
        if(http_status[i].code == code) return http_status[i].message;
    }
    return http_status[i].message;
}

int write_response(pConnInfo conn_info, struct response* header_body) {/*{{{*/
    int res;
    char *accept_encoding; 
    setblocking(conn_info->clientfd);
    //compress--start--

    if((header_body->content_len > 0) &&
        uws_config.http.gzip && 
        in_str_array(uws_config.http.gzip_types, get_header_param("Content-Type", header_body->header)) >= 0 &&
        (accept_encoding = get_header_param("Accept-Encoding", conn_info->request_header)) &&
        strstr(accept_encoding, "gzip") != NULL
        ) {
        size_t src_len = header_body->content_len;
        size_t dst_len;
        char *dst_buff;
        gzcompress(&dst_buff, &dst_len, header_body->content, src_len);
        char *content_len = itoa(dst_len);
        add_header_param("Content-Length", content_len, header_body->header);
        uws_free(content_len);
        add_header_param("Content-Encoding", "gzip", header_body->header);
        uws_free(header_body->content);
        header_body->content = dst_buff;
        header_body->content_len = dst_len;
    }

    char* header_str = str_response_header(header_body->header);
    size_t header_len = strlen(header_str);

    res = write(conn_info->clientfd, header_str, header_len);
    uws_free(header_str);

    if(res == -1) return -1;
    res = write(conn_info->clientfd, HEADER_SEP, strlen(HEADER_SEP));
    if(res == -1) return -1;
    res = writen(conn_info->clientfd, header_body->content, header_body->content_len);
    if(res == -1) {return -1;}
    return 0;
}/*}}}*/
