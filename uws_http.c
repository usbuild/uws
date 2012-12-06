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
set_header() {
    char *time_string = get_time_string(NULL);
    response_header->http_ver = "HTTP/1.1";
    response_header->status_code = 200;
    response_header->status = "OK";
    add_header_param("Cache-Control", "private", response_header);
    add_header_param("Connection", "Keep-Alive", response_header);
    add_header_param("Vary", "Accept-Encoding", response_header);
    add_header_param("Server", UWS_SERVER, response_header);
    add_header_param("Date", time_string, response_header);

    char *len = itoa(header_body.content_len);
    add_header_param("Content-Length", len, response_header);
    uws_free(len);

    add_header_param("Content-Type", mime, response_header);
    header_body.header = response_header;
    uws_free(time_string);
}

int
comparestr(const void *p1, const void *p2)
{
    return strcmp(* (char * const *)p1, * (char * const *) p2);
}
static void
printdir(const char *fpath, int client_fd) {//打印目录项排序
    if(!running_server->autoindex) {
        send_error_response(client_fd, 403, true);
        return;
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
printfile(const char *path, int client_fd)
{
    char *mod_time_str ;
    char *file_mod_time = get_file_time(path);
    if((mod_time_str = get_header_param("If-Modified-Since", request_header))) {
        if(!is_expire(mod_time_str, file_mod_time)) {
            send_error_response(client_fd, 304, false);
            return;
        }
    }

    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    header_body.content_len = ftell(file);
    rewind(file);


    header_body.content = (char*) uws_malloc (header_body.content_len * sizeof(char));
    int res = fread(header_body.content, sizeof(char), header_body.content_len, file);
    fclose(file);

    add_header_param("Last-Modified", file_mod_time, response_header);
    uws_free(file_mod_time);
}

int
http_router(int sockfd) 
{
    char path[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    strcpy(path, request_header->path);
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
            printdir(path, sockfd);
        }
        else
        {
            mime = get_mime(path);
            printfile(path, sockfd);
        }
    }
    else {
        send_error_response(sockfd, 404, true);
        return 0;
    }

    set_header();
    write_response(sockfd, &header_body);
    free_header_params(response_header);
    uws_free(header_body.header);
    uws_free(header_body.content);
    return 0;
}
int write_response(int sockfd, struct response* header_body) {
    int res;
    char *accept_encoding; 
    //compress--start--

    if((header_body->content_len > 0) &&
        uws_config.http.gzip && 
        in_str_array(uws_config.http.gzip_types, get_header_param("Content-Type", header_body->header)) >= 0 &&
        (accept_encoding = get_header_param("Accept-Encoding", header_body->header)) &&
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

    res = write(sockfd, header_str, header_len);
    uws_free(header_str);

    if(res == -1) return -1;
    res = write(sockfd, HEADER_SEP, strlen(HEADER_SEP));
    if(res == -1) return -1;
    res = writen(sockfd, header_body->content, header_body->content_len);
    if(res == -1) {return -1;}
    return 0;
}
