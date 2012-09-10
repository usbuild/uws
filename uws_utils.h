#ifndef __UWS_UTILS_H__
#define __UWS_UTILS_H__
#include "uws.h"
int wildcmp(const char*, const char*);
void setnonblocking(int sock);
char* strdup(const char *s);
char* strlcat(const char *s1, const char *s2);
char *itoa(const size_t data);
char* get_time_string(time_t *tt);
time_t parse_time_string(char *);
int in_int_array(int array[], int needle, int length);
int gzcompress(char **zdata, size_t *nzdata, char *data, size_t ndata);
int deflatecompress(char **zdata, size_t *nzdata, char *data, size_t ndata);
int in_str_array(char **array, char *needle);
char *get_file_time(const char *path);
bool is_expire(char *time1, char *time2);
int writen(int fd, char *buff, size_t len);
int readn(int fd, char *buff, size_t len);
char *nullstring(char *str);
void append_mem(memory_t *smem, char *start, size_t len);
#endif
