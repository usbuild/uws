#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_http.h"
#include "uws_config.h"
#include "uws_mime.h"

extern struct nv_pair** uws_configs;

static struct response header_body;

static char* mime;


int
comparestr(const void *p1, const void *p2)
{
    return strcmp(* (char * const *)p1, * (char * const *) p2);
}
void
printdir(const char *fpath) {//打印目录项排序
    DIR *dp = opendir(fpath);
    struct dirent *dir_entry;
    int dir_len = 0;
    char **entries;

    while((dir_entry = readdir(dp)) != NULL)
        dir_len++;

    header_body.content_len = dir_len * 64;//Max filename Length
    header_body.content = (char*) calloc (sizeof(char), header_body.content_len);

    rewinddir(dp);
    entries = (char**) malloc ((dir_len + 1) * sizeof(char*));
    dir_len = 0;
    while((dir_entry = readdir(dp)) != NULL)
        entries[dir_len++] = dir_entry->d_name;
    entries[dir_len] = NULL;
    qsort(entries, dir_len, sizeof(char*), comparestr);


    while(*(entries)!= NULL) {
        strcat(header_body.content, "<a href=\"./");
        strcat(header_body.content, *entries);
        strcat(header_body.content, "\">");
        strcat(header_body.content, *(entries++));
        strcat(header_body.content, "</a><br/>\n");
    }
    header_body.content_len = strlen(header_body.content);
}
void
printfile(const char *path)
{
    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    header_body.content_len = ftell(file);

    rewind(file);

    header_body.content = (char*) calloc (sizeof(char), header_body.content_len);
    fread(header_body.content, sizeof(char), header_body.content_len, file);
    fclose(file);
}


static char*
get_time_string() {
    struct tm *cur_time;
    char* buff = (char*) malloc(sizeof(char) * 40);
    time_t tt;
    time(&tt);
    cur_time = localtime(&tt);
    strftime(buff, 40, "%a, %e %b %Y %T %Z", cur_time);
    return buff;
}

static char*
get_mime(const char* path)
{
    int i = 0;
    i = strlen(path);
    i--;
    while(*(path + i) != '.') {
        if(i == 0) {
            return "text/html";
            break;
        }
        i--;
    }
    if(i != 0){
        return mimebyext(path + i + 1);
    }
    return "text/html";
}
static void
set_header() {
    header_body.header = (char*) calloc(sizeof(char), HEADER_LEN);

    if(mime != NULL) {
        sprintf(header_body.header,    "HTTP/1.1 200 OK\n"
            "Cache-Control: private\n"
            "Connection: Keep-Alive\n"
            "Server: UWS/0.001\n"
            "Date: %s\n"
            "Content-Length: %d\n"
            "Content-Type: %s;charset=utf-8\n"
            "\n"\
            , get_time_string(), header_body.content_len, mime);
    }
    else {
        sprintf(header_body.header,    "HTTP/1.1 404 Not Found\n"
            "Cache-Control: private\n"
            "Connection: Keep-Alive\n"
            "Server: UWS/0.001\n"
            "Date: %s\n"
            "\n"\
            , get_time_string());
    }
    header_body.header_len = strlen(header_body.header);
}
int
http_router(int sockfd, const struct http_header* header) 
{
    char path[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    strcpy(path, header->path);
    while(path[i] != 0) {
        if(path[i] == '?' || path[i] == '#') {
            path[i] = 0;
            break;
        }
        i++;
    }
    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            mime = "text/html";
            printdir(path);
        }
        else
        {
            mime = get_mime(path);
            printfile(path);
        }
    }
    else {
        mime = NULL;
    }
    set_header();
    write(sockfd, header_body.header, header_body.header_len);
    write(sockfd, header_body.content, header_body.content_len);
    return 0;
}
