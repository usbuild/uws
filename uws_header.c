#include <time.h>
#include "uws.h"
#include "uws_header.h"
#include "uws_config.h"
#include "uws_mime.h"
static char*
get_time_string();
extern struct
response header_body;
void
set_header(const char* mime)
{
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
