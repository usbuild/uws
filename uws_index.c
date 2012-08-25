#include <dirent.h>
#include <sys/stat.h>
#include "uws.h"
#include "uws_config.h"
#include "uws_header.h"

int
dir_router(int sockfd, struct http_header *request_header) 
{
    char path[PATH_LEN];
    char path2[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    strcpy(path, running_server->root);

    while(request_header->url[i] != 0) {
        if(request_header->url[i] == '?' || request_header->url[i] == '#') {
            request_header->url[i] = 0;
            i++;
            break;
        }
        i++;
    }

    strcpy(request_header->request_params, request_header->url + i);

    strcat(path, request_header->url);
    strcpy(request_header->path,  path);

    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            if(!running_server->autoindex) {
                close(sockfd);// we should dealwith nonindex
            }
            char *index;
            int i = 0;
            while((index = running_server->index[i++]) != NULL) {
                strcpy(path2, path);
                strcat(path2, "/");
                strcat(path2, index);
                if(lstat(path2, &stat_buff) != -1) {
                    strcpy(request_header->path,  path2);
                    strcpy(path2, request_header->url);
                    strcat(path2, "/");
                    strcat(path2, index);
                    request_header->url = (char*) realloc(request_header->url, strlen(path2));
                    strcpy(request_header->url, path2);
                    break;
                }
            }
        }
    }
    /*
    puts(request_header->url);
    puts(request_header->path);
    puts(request_header->request_params);
    */
    return 1;
}
