#include "uws.h"
#include "uws_memory.h"
#include "uws_cgi.h"
#include "uws_config.h"
extern struct response header_body;
void
cgi_handler(const char* path, int fd)
{
    char* cmd = (char*) uws_calloc(sizeof(char), strlen(path) + 10);
    FILE* dest = fdopen(fd, "r+");
    FILE* src;
    char buff[LINE_LEN];
    int nread;
    strcat(cmd, "php-cgi -f ");
    strcat(cmd, path);
    src = popen(cmd, "r");
    while(!feof(src)) {
        nread = fread(buff, sizeof(char), LINE_LEN, src);
        fwrite(buff, sizeof(char), nread, dest);
    }
    pclose(src);
}
