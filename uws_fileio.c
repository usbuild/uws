#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_fileio.h"
#include "uws_config.h"
#include "uws_mime.h"
extern struct nv_pair** uws_configs;
static char*
get_time_string();
int
comparestr(const void *p1, const void *p2)
{
    return strcmp(* (char * const *)p1, * (char * const *) p2);
}
void
printdir(const char *fpath, FILE* stream) {//打印目录项排序
    DIR *dp = opendir(fpath);
    struct dirent *dir_entry;
    int dir_len = 0;
    char **entries;

    while((dir_entry = readdir(dp)) != NULL)
        dir_len++;
    rewinddir(dp);
    entries = (char**) malloc ((dir_len + 1) * sizeof(char*));
    dir_len = 0;
    while((dir_entry = readdir(dp)) != NULL)
        entries[dir_len++] = dir_entry->d_name;
    entries[dir_len] = NULL;
    qsort(entries, dir_len, sizeof(char*), comparestr);

    while(*(entries)!= NULL) {
        fprintf(stream, "%s", "<a href=\"./");
        fprintf(stream, "%s", *entries);
        fprintf(stream, "%s", "\">");
        fprintf(stream, "%s", *(entries++));
        fprintf(stream, "%s\n", "</a><br/>");

    }
}
void
printfile(const char *path, FILE* stream)
{
    FILE* file = fopen(path, "r");
    char buff[BUFF_LEN];
    int nread;
    while( (nread = fread(buff, sizeof(char), BUFF_LEN, file)) > 0)
        fwrite(buff, sizeof(char), nread, stream);
    fclose(file);
}
void
pathrouter(const char* arg, FILE* stream)
{
    char path[PATH_LEN];
    char path2[PATH_LEN];
    struct stat stat_buff;

    getcwd(path, PATH_LEN);
    strcat(path, arg);
    if(lstat(path, &stat_buff) != -1) {
        fputs("HTTP/1.1 200 OK\n", stream);
        fputs("Cache-Control: private\n", stream);
        fputs("Connection: Keep-Alive\n", stream);
        fputs("Server: UWS/0.001\n", stream);
        fprintf(stream, "Date: %s\n", get_time_string());
        if( S_ISDIR(stat_buff.st_mode) ) {
            char *index;
            if((index = get_opt("index")) != NULL)
            {
                strcpy(path2, path);
                strcat(path2, "/");
                strcat(path2, index);
                if(lstat(path2, &stat_buff) != -1) {
                    fprintf(stream, "Content-Type: %s; charset=UTF-8\n\n", get_mime(path2));
                    printfile(path2, stream);
                    return;
                }
            }
            fprintf(stream, "Content-Type: %s; charset=UTF-8\n\n", "text/html");
            printdir(path, stream);
        }
        else
        {
            fprintf(stream, "Content-Type: %s; charset=UTF-8\n\n", get_mime(path));
            printfile(path, stream);
        }
    }
    else {
        fputs("HTTP/1.1 404 Not Found\n", stream);
    }

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
}
