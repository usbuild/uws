#ifndef __FILEIO_H_
#define __FILEIO_H_
#define PATH_LEN    512
#define BUFF_LEN    4096

void pathrouter(const char* arg);
static char* get_mime(const char* path);

#endif
