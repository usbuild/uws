#include <dirent.h>
#include <sys/stat.h>
#include "uws.h"
#include "uws_memory.h"
#include "uws_config.h"
#include "uws_header.h"
#include "uws_status.h"

int
dir_router(int sockfd) 
{
    char path[PATH_LEN];
    char path2[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    strcpy(path, conn_info->running_server->root);

    char *tmp = conn_info->request_header->path;
    conn_info->request_header->path = (char *)uws_malloc((strlen(path) + PATH_LEN) * sizeof(char));
    strcpy(path, conn_info->running_server->root);
    strcat(path, tmp);
    strcpy(conn_info->request_header->path,  path);
    uws_free(tmp);

    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            char *index;
            int i = 0;
            while((index = conn_info->running_server->index[i++]) != NULL) {
                strcpy(path2, path);
                strcat(path2, index);
                if(lstat(path2, &stat_buff) != -1) {
                    strcpy(conn_info->request_header->path,  path2);
                    strcpy(path2, conn_info->request_header->url);
                    strcat(path2, index);
                    conn_info->request_header->url = (char*) uws_realloc(conn_info->request_header->url,strlen(conn_info->request_header->url), strlen(path2));
                    strcpy(conn_info->request_header->url, path2);
                    break;
                }
            } }
    }
    /*
    puts(conn_info->request_header->path);
    puts(conn_info->request_header->url);
    puts(conn_info->request_header->request_params);
    */
    return 1;
}
