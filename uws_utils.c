#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <zlib.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "uws_utils.h"
#include "uws_memory.h"
#define OVECCOUNT   30
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
char *strlcat(const char *s1, const char *s2) {
    char *new_str = (char*) uws_malloc((strlen(s1) + strlen(s2) + 1) * sizeof(char));
    strcpy(new_str, s1);
    strcat(new_str, s2);
    return new_str;
}
char *itoa(const size_t data) {
    size_t length = (size_t) pow(data, 0.1) + 2;
    char *str = (char*) uws_malloc(length * sizeof(char));
    sprintf(str, "%u", data);
    return str;
}
char* get_time_string(time_t *tt) {
    struct tm *cur_time;
    time_t t;
    char* buff = (char*) uws_malloc(sizeof(char) * 60);
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
        *zdata = (char*) uws_malloc((*nzdata + 1) * sizeof(char));

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
        *zdata = (char*) uws_malloc((*nzdata + 1) * sizeof(char));

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
int writen(int fd, void *buff, size_t len) {
    size_t already = 0;
    long res = 0;
    while(already < len) {
        res = write(fd, buff + already, len - already);
        if(res == -1) return -1;
        already += res;
    }
    return already;
}
int readn(int fd, void *buff, size_t len) {
    size_t already = 0;
    long res = 0;
    while(already < len) {
        res = read(fd, buff + already, len - already);
        if(res == -1) return -1;
        already += res;
    }
    return already;
}
inline char *
nullstring(char *str) {
    if(str == NULL) return "";
    return str;
}

void append_mem_t(memory_t *smem, void *start, size_t len) {
    if(smem->len == 0) {
        smem->mem = (unsigned char *)uws_malloc(len * sizeof(unsigned char));
        smem->total = len;
        memcpy(smem->mem, start, smem->total);
        smem->len = len;
    } else {
        while(smem->len + len > smem->total) {
            smem->total *= 2;
        }
        smem->mem = (unsigned char*) uws_realloc(smem->mem, smem->len, smem->total);
        memcpy(smem->mem + smem->len, start, len);
        smem->len += len;
    }
}
inline void
free_mem_t(memory_t *smem) {
    uws_free(smem->mem);
    smem->mem = NULL;
    smem->len = 0;
    smem->total = 0;
}
inline int 
str_count(char *haystack, char *needle) {
    char *found = haystack;
    int needle_len = strlen(needle);
    int count = 0;
    while(found = strstr(found, needle)) {
        count++;
        found += needle_len;
    }
    return count;
}
char* str_replace(char *haystack, char *search, char *replace) {
    char *pos = strstr(haystack, search);
    int count = str_count(haystack, search);
    if(pos = NULL) {
        return uws_strdup(haystack);
    }
    int search_len = strlen(search);
    int replace_len = strlen(replace);
    char *new_str = (char*) uws_malloc( (strlen(haystack) + count * (replace_len - search_len) + 2) * sizeof(char));
    new_str[0] = 0;
    char *found = haystack;

    pos = haystack;
    while(found = strstr(found, search)) {
        strncat(new_str, pos, found - pos);
        strcat(new_str, replace);
        found += search_len;
        pos = found;
    }
    strcat(new_str, pos);
    return new_str;
}

char* preg_replace( char *src, const char *pattern, const char *replace) {
    pcre *re;
    int ovector[OVECCOUNT];
    int rc, i;
    re = get_pcre(pattern);
    if(re == NULL) {
        return NULL;
    }
    rc = pcre_exec(re, NULL, src, strlen(src), 0, 0, ovector, OVECCOUNT);
    if(rc < 0) {
        return NULL;
    }

    char *str = uws_strdup(replace);
    char flag[] = {'$', 0, 0};
    for(i = 0; i < rc; i++) {
        char *substring_start = src + ovector[2 * i];  
        int substring_length = ovector[2 * i + 1] - ovector[ 2 * i ];  
        char *new_str = (char*) uws_malloc(substring_length + 1*sizeof(char));
        strncpy(new_str, substring_start, substring_length);
        new_str[substring_length] = 0;
        flag[1] = i + '0';
        char *tmp = str_replace(str, flag, new_str);
        uws_free(str);
        str = tmp;
        uws_free(new_str);
    }
    return str;
}

bool preg_match(char *src, const char *pattern) {
    pcre *re;
    int ovector[OVECCOUNT];
    int rc, i;
    //re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    re = get_pcre(pattern);
    if(re == NULL) {
        return false;
    }
    rc = pcre_exec(re, NULL, src, strlen(src), 0, 0, ovector, OVECCOUNT);
    if(rc < 0) {
        return false;
    }
    return true;
}

char* base64(char *input) {
    int length = strlen(input);
    BIO *b64 = NULL;
    BIO * bmem = NULL;
    BUF_MEM *bptr = NULL;
    char * output = NULL;
    b64 = BIO_new((BIO_METHOD *)BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    output = (char *) uws_malloc ((bptr->length + 1) * sizeof(char));
    memcpy(output, bptr->data, bptr->length);
    BIO_free_all(b64);
    return output;
}

/*
 * Because of once regexp constructed, it will remain in memory, so we don't put it into the memory pool
 */
pcre* 
get_pcre(const char *src) {
    static regex_map_t *regex_map;
    static len = 0;
    static total = 0;
    int i = 0;
    for(i = 0; i < len; i++) {
        if(strcmp(src, regex_map[i].src) == 0) {
            return regex_map[i].re;
        }
    }
    if(total == 0)   {
        total = INIT_ARR_LEN;
        regex_map = (regex_map_t*) malloc(total * sizeof(regex_map_t));
    }
    if(len >= total - 1) {
        total *= 2;
        regex_map = (regex_map_t*) realloc(regex_map, total);
    }
    const char *error;
    int erroffset;
    pcre *re = pcre_compile(src, 0, &error, &erroffset, NULL);
    if(re == NULL) return NULL;
    regex_map[len].src = strdup(src); 
    regex_map[len].re = re;
    len++;
    return re;
}
inline char *
uws_strdup(const char *s) {
    char *t = uws_malloc(strlen(s) + 1);
    strcpy(t, s);
    return t;
}
