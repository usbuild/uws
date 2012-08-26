#include <fcntl.h>
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
