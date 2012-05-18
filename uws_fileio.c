#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_fileio.h"
#include "uws_config.h"
#include "uws_mime.h"
extern struct nv_pair** uws_configs;
extern struct response header_body;
static char*
get_time_string();
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

    printf("%d\n", header_body.content_len);
    header_body.content = (char*) calloc (sizeof(char), header_body.content_len);
    fread(header_body.content, sizeof(char), header_body.content_len, file);
    fclose(file);
}
