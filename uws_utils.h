#ifndef __UWS_UTILS_H__
#define __UWS_UTILS_H__
#include "uws.h"
int wildcmp(const char*, const char*);
void setnonblocking(int sock);
char* strdup(const char *s);
char* strlcat(const char *s1, const char *s2);
#endif
