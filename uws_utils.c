#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <zlib.h>
#include "uws_utils.h"
int wildcmp(const char* wild, const char* string){
    const char* cp = NULL, *mp = NULL;
    while((*string) && (*wild != '*'))
    {
        if((*wild != *string) && (*wild != '?')) 
        {
            return 0;
        }
        wild++;
        string++;
    }
    while(*string)
    {
        if(*wild == '*')
        {
            if(!*++wild){
                return 1;
            }
            mp = wild;
            cp = string + 1;
        } 
        else if((*wild == *string) || (*wild == '?'))
        {
            wild++;
            string++;
        } else {
            wild = mp;
            string = cp++;
        }
    }
    while(*wild == '*')
    {
        wild++;
    }
    return !*wild;
}
void setnonblocking(int sock)
{
    int opts = fcntl(sock, F_GETFL);
    if (opts < 0) exit_err("fcntl(F_GETFL)");

    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) exit_err("fcntl(F_SETFL)");
    return;
}
char* strdup(const char *s){
    char *r;
    if(s == 0 || *s == 0)
        return NULL;
    r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}
char *strlcat(const char *s1, const char *s2) {
    char *new_str = (char*) calloc(strlen(s1) + strlen(s2) + 1, sizeof(char));
    strcpy(new_str, s1);
    strcat(new_str, s2);
    return new_str;
}
char *itoa(const size_t data) {
    size_t length = (size_t) pow(data, 0.1) + 2;
    char *str = (char*) calloc(length, sizeof(char));
    sprintf(str, "%u", data);
    return str;
}
char* get_time_string(time_t *tt) {
    struct tm *cur_time;
    time_t t;
    char* buff = (char*) malloc(sizeof(char) * 60);
    if(tt == NULL) {
        t = time(NULL);
        tt = &t;
    }
    cur_time = gmtime(tt);
    strftime(buff, 60, "%a, %d %b %Y %H:%M:%S GMT", cur_time);
    return buff;
}
time_t parse_time_string(char *time_str) {
    struct tm cur_time;
    char buff[60];
    strptime(time_str, "%a, %d %b %Y %H:%M:%S GMT", &cur_time);

    return mktime(&cur_time);
}
int in_int_array(int array[], int needle, int length) {
    int i;
    for(i = 0; i < length; i++) {
        if(array[i] == needle) {
            return i;
        }
    }
    return -1;
}

int gzcompress(char **zdata, size_t *nzdata, char *data, size_t ndata)/*{{{*/
{
    z_stream c_stream;
    int err = 0;
    if(data && ndata > 0)
    {
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        if(
           deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK

            ) 
            return -1;
        *nzdata = deflateBound(&c_stream, ndata);
        *zdata = (char*) calloc(*nzdata + 1, sizeof(char));

        c_stream.next_out = *zdata;
        c_stream.avail_out  = *nzdata;
        if(Z_STREAM_END != deflate(&c_stream, Z_FINISH)) 
            return -1;
        if(deflateEnd(&c_stream) != Z_OK) return -1;
        return 0;
    }
    return -1;
}/*}}}*/
int deflatecompress(char **zdata, size_t *nzdata, char *data, size_t ndata) {/*{{{*/
    z_stream c_stream;
    int err = 0;
    if(data && ndata > 0)
    {
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        if(
           deflateInit(&c_stream, Z_DEFAULT_COMPRESSION) != Z_OK

            ) 
            return -1;
        *nzdata = deflateBound(&c_stream, ndata);
        *zdata = (char*) calloc(*nzdata + 1, sizeof(char));

        c_stream.next_out = *zdata;
        c_stream.avail_out  = *nzdata;
        if(Z_STREAM_END != deflate(&c_stream, Z_FINISH)) 
            return -1;
        if(deflateEnd(&c_stream) != Z_OK) return -1;
        return 0;
    }
    return -1;
}/*}}}*/
int in_str_array(char **array, char *needle) {
    int i = 0;
    while(array[i] != NULL) {
        if(strcmp(array[i], needle) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}
char *get_file_time(const char *path) {
    struct stat buff;
    lstat(path, &buff);
    return get_time_string(&buff.st_mtime);
}

bool is_expire(char *time1, char *time2) {
    // time1 after time2 return false
    time_t t1 = parse_time_string(time1);
    time_t t2 = parse_time_string(time2);
    return t1 < t2;
}
int writen(int fd, char *buff, size_t len) {
    size_t already = 0;
    long res = 0;
    while(already < len) {
        res = write(fd, buff + already, len - already);
        if(res == -1) return -1;
        already += res;
    }
    return already;
}
int readn(int fd, char *buff, size_t len) {
    size_t already = 0;
    long res = 0;
    while(already < len) {
        res = read(fd, buff + already, len - already);
        if(res == -1) return -1;
        already += res;
    }
    return already;
}
char *nullstring(char *str) {
    if(str == NULL) return "";
    return str;
}
