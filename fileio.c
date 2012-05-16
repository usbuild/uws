#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"
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
        fprintf(stream, "%s", "<a href=\"");
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
dir_or_file(const char* arg, FILE* stream)
{
    char path[PATH_LEN];
    struct stat stat_buff;
    getcwd(path, PATH_LEN);
    strcat(path, arg);
    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) )
            printdir(path, stream);
        else
            printfile(path, stream);
    }

}
