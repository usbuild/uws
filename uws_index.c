#include <dirent.h>
#include <sys/stat.h>
#include "uws.h"
#include "uws_config.h"

extern struct nv_pair** uws_configs;

int
dir_router(int sockfd, struct http_header* header) 
{
    char path[PATH_LEN];
    char path2[PATH_LEN];
    struct stat stat_buff;
    int i = 0; 
    getcwd(path, PATH_LEN);

    while(header->url[i] != 0) {
        if(header->url[i] == '?' || header->url[i] == '#') {
            header->url[i] = 0;
            i++;
            break;
        }
        i++;
    }

    header->request_params = (char*) calloc(sizeof(char), PATH_LEN);
    strcpy(header->request_params, header->url + i);

    strcat(path, header->url);
    strcpy(header->path,  path);

    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            char *index;
            if((index = get_opt("index")) != NULL) {
                strcpy(path2, path);
                strcat(path2, "/");
                strcat(path2, index);
                if(lstat(path2, &stat_buff) != -1) {
                    strcpy(header->path,  path2);

                    strcpy(path2, header->url);
                    strcat(path2, "/");
                    strcat(path2, index);
                    header->url = (char*) realloc(header->url, strlen(path2));
                    strcpy(header->url, path2);
                }
            }
        }
    }
    puts(header->url);
    puts(header->path);
    puts(header->request_params);
    return 1;
}
