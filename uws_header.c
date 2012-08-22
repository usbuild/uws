#include "uws_header.h"
#define INIT_PARAMS_NUM  20
char* get_header_param(char* key, struct http_header *request_header){
    int i = 0;
    if(request_header->params == NULL)
        return NULL;
    while(i < request_header->used_len){
        if(strcmp(key, request_header->params[i].name) == 0) {
            return request_header->params[i].value;
        }
        i++;
    }
    return NULL;
}

void add_header_param(char* kvstring, struct http_header *request_header){
    int i = 0;
    int str_len = strlen(kvstring);
    if(request_header->params == NULL) 
    {
        request_header->params = (Http_Param*) calloc(INIT_PARAMS_NUM, sizeof(Http_Param));
        request_header->used_len = 0;
        request_header->max_len = INIT_PARAMS_NUM;
    }
    if(request_header->used_len == request_header->max_len - 1)
    {
        request_header->max_len *= 2;
        Http_Param* tmp = request_header->params;
        request_header->params = (Http_Param*)calloc(request_header->max_len, sizeof(Http_Param));
        memcpy(request_header->params, tmp, request_header->used_len);
        free(tmp);
    }
    char* key = (char*) calloc(str_len, sizeof(char));
    char* value = (char*) calloc(str_len, sizeof(char));
    sscanf(kvstring, "%[^:]: %[^\r\n]", key,value);
    Http_Param* new_param = &request_header->params[request_header->used_len];
    new_param->name = key;
    new_param->value = value;
    request_header->used_len++;
}
