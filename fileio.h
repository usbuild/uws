#ifndef __FILEIO_H_
#define __FILEIO_H_

#define PATH_LEN    512
#define BUFF_LEN    4096

int comparestr(const void *p1, const void *p2);
void printdir(const char *fpath, FILE *stream);
void printfile(const char *path, FILE *stream);
void dir_or_file(const char* arg, FILE *stream);

#endif
