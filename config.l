%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
char** getArray(char* s);
bool getBoolean(char* s);
char** getArray(char* s);

enum value_type{
    LOOK_VAL,
    STRING_VAL,
    INT_VAL,
    BOOL_VAL,
    ARRAY_VAL
};
uws_config_t config;
enum value_type state = LOOK_VAL;
char* options;
char** val_string;
int* val_int;
bool* val_bool;
char*** val_array;
int server_num = 10;
int current_server = 0;

server_cfg_t* server;

%}

space   [ \t\n]
%s      HTTP
%s      EVENTS
%s      SERVER

%%
#.*\n               ;

events{space}*\{                {BEGIN EVENTS;}
http{space}*\{                  {BEGIN HTTP; 
                                    config.http.servers = (server_cfg_t**) calloc(server_num, sizeof(server_cfg_t*));
                                }
<HTTP>server{space}*\{          {BEGIN SERVER;
                                    server = (server_cfg_t*)malloc(sizeof(server_cfg_t));
                                }
<EVENTS>\}                      {BEGIN 0;}
<HTTP>\}                        {BEGIN 0;}
<SERVER>\}                      {BEGIN HTTP;
                                    if(current_server >= server_num)
                                    {
                                        server_num *= 2;
                                        server_cfg_t** tmp = config.http.servers;
                                        config.http.servers = (server_cfg_t**) calloc(server_num, sizeof(server_cfg_t*));
                                        memcpy(config.http.servers, tmp, sizeof(tmp));//Any Questions?
                                        free(tmp);
                                    }
                                    config.http.servers[current_server] = server;
                                    current_server++;
                                }

user                {state = STRING_VAL; val_string = &config.user;}
pid                 {state = STRING_VAL; val_string = &config.pid;}
worker_processes    {state = INT_VAL; val_int = &config.worker_processes;}

<HTTP>sendfile      {state = BOOL_VAL;val_bool = &config.http.sendfile; }
<HTTP>tcp_nopush    {state = BOOL_VAL;val_bool = &config.http.tcp_nopush; }
<HTTP>tcp_nodelay   {state = BOOL_VAL;val_bool = &config.http.tcp_nodelay; }
<HTTP>keepalive_timeout   {state = INT_VAL;val_int = &config.http.keepalive_timeout; }
<HTTP>types_hash_max_size   {state = INT_VAL;val_int = &config.http.types_hash_max_size; }
<HTTP>server_tokens  {state = BOOL_VAL;val_bool = &config.http.server_tokens; }
<HTTP>server_names_hash_bucket_size   {state = INT_VAL;val_int = &config.http.server_names_hash_bucket_size; }
<HTTP>access_log     {state = STRING_VAL; val_string = &config.http.access_log;}
<HTTP>error_log      {state = STRING_VAL; val_string = &config.http.error_log;}
<HTTP>gzip           {state = BOOL_VAL;val_bool = &config.http.gzip; }
<HTTP>gzip_disable   {state = STRING_VAL; val_string = &config.http.gzip_disable;}
<HTTP>gzip_vary      {state = BOOL_VAL;val_bool = &config.http.gzip_vary; }
<HTTP>gzip_proxied   {state = STRING_VAL; val_string = &config.http.gzip_proxied;}
<HTTP>gzip_comp_level {state = INT_VAL;val_int = &config.http.gzip_comp_level; }
<HTTP>gzip_http_version      {state = STRING_VAL; val_string = &config.http.gzip_http_version;}
<HTTP>gzip_types      {state = ARRAY_VAL; val_array = &config.http.gzip_types;}

<SERVER>root         {state = STRING_VAL; val_string= &server->root;}
<SERVER>index        {state = ARRAY_VAL; val_array= &server->index;}
[^ \t\n\t;]+       {
                    switch(state){
                        case STRING_VAL:
                                *val_string = strdup(yytext);
                                break;
                        case INT_VAL:
                                *val_int = atoi(yytext);
                                break;
                        case BOOL_VAL:
                                *val_bool = getBoolean(yytext);
                                break;
                        case ARRAY_VAL:
                                *val_array = getArray(yytext);
                        default:
                                break;
                            }
                    } 


;                               {state = LOOK_VAL;}
.|\n                            ;
%%

int main(){

    yylex();
    printf("user: %s\n", config.user);
    printf("worker_processes: %d\n", config.worker_processes);
    printf("http.sendfile: %d\n", config.http.sendfile);
    printf("http.server.root: %s\n", config.http.servers[0]->root);
    char** index = config.http.servers[0]->index;
    while(*index != NULL){
        printf("http.servers.index:%s\n",*index);
        index++;
    }
    return 0;
}
int yywrap(){
    return 1;
}
char* strdup(const char *s){
    char *r;
    if(s == 0 || *s == 0)
        return NULL;
    r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}
bool getBoolean(char* s){
    if(!strcmp("on", yytext)){
        return true;
    } else{
        return false;
    }
}
char** getArray(char* s){
    int items = 1;
    int i = 0;
    for(i = 0; i < strlen(s); i++){
        if(s[i] == ',') items++;
    }
    char** array = (char**)calloc(items + 1, sizeof(char*));
    for(i = 0; i < items; i++){
        array[i] = (char*)calloc(strlen(s) + 1, sizeof(char*));
    }
    items = 0;
    int tmp = 0;

    for(i = 0; i < strlen(s); i++){
       if(s[i] == ' ' || s[i] == '\t' || s[i]  == '\n') {
            continue;
       }
       if(s[i] == ','){
           items++;
           tmp = 0;
           continue;
       }
       array[items][tmp++] = s[i];
    }
    return array;
}
