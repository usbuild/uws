#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "uws.h"
#include "uws_router.h"
#include "uws_header.h"
#include "uws_fileio.h"
#include "uws_config.h"
#include "uws_mime.h"
static char*
get_time_string();
struct response header_body;
void
pathrouter(const char* arg)
{
    char path[PATH_LEN];
    char path2[PATH_LEN];
    struct stat stat_buff;
    char* mime;

    getcwd(path, PATH_LEN);
    strcat(path, arg);
    if(lstat(path, &stat_buff) != -1) {
        if( S_ISDIR(stat_buff.st_mode) ) {
            char *index;
            if((index = get_opt("index")) != NULL)
            {
                strcpy(path2, path);
                strcat(path2, "/");
                strcat(path2, index);
                if(lstat(path2, &stat_buff) != -1) {
                    mime = get_mime(path2);
                    printfile(path2);
                    return;
                }
            }
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
    set_header(mime);
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
