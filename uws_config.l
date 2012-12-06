%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "uws_config.h"

bool getBoolean(char* s);
char** getArray(char* s);
char* append_str_array(str_array_t *array_t, char *string);

enum value_type{
    LOOK_VAL,
    STRING_VAL,
    INT_VAL,
    BOOL_VAL,
    ARRAY_VAL
};

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
%s      REWRITE
%s      UPSTREAM

%%
#.*\n               ;

events{space}*\{                {BEGIN EVENTS;}
http{space}*\{                  {BEGIN HTTP; 
                                    uws_config.http.servers = (server_cfg_t**) calloc(server_num, sizeof(server_cfg_t*));
                                }
<HTTP>server{space}*\{          {BEGIN SERVER;
                                    server = (server_cfg_t*)malloc(sizeof(server_cfg_t));
                                    server->autoindex = false;
                                    server->facade = true;
                                    server->listen = 80;
                                    server->rewrite.engine = true;
                                    server->rewrite.exist = false;
                                    server->rewrite.rules.len = 0;
                                    server->auth_basic = NULL;
                                    server->rewrite.rules.total = 0;
                                    server->proxy = false;
                                    server->upstream.total = 0;
                                    server->upstream.len = 0;
                                }
<SERVER>rewrite{space}*\{       {BEGIN REWRITE;}
<SERVER>upstream{space}*\{      {BEGIN UPSTREAM;}
<EVENTS>\}                      {BEGIN 0;}
<HTTP>\}                        {BEGIN 0;}
<REWRITE>\}                     {BEGIN SERVER;}
<UPSTREAM>{space}*\}                    {BEGIN SERVER;}
<SERVER>\}                      {BEGIN HTTP;
                                    if(current_server >= server_num)
                                    {
                                        server_num *= 2;
                                        server_cfg_t** tmp = uws_config.http.servers;
                                        uws_config.http.servers = (server_cfg_t**) calloc(server_num, sizeof(server_cfg_t*));
                                        int tmp_size = sizeof(tmp);
                                        memcpy(uws_config.http.servers, tmp, tmp_size);//Any Questions?
                                        free(tmp);
                                    }
                                    uws_config.http.servers[current_server] = server;
                                    current_server++;
                                }

user                {state = STRING_VAL; val_string = &uws_config.user;}
pid                 {state = STRING_VAL; val_string = &uws_config.pid;}
worker_processes    {state = INT_VAL; val_int = &uws_config.worker_processes;}

<HTTP>sendfile      {state = BOOL_VAL;val_bool = &uws_config.http.sendfile; }
<HTTP>tcp_nopush    {state = BOOL_VAL;val_bool = &uws_config.http.tcp_nopush; }
<HTTP>tcp_nodelay   {state = BOOL_VAL;val_bool = &uws_config.http.tcp_nodelay; }
<HTTP>keepalive_timeout   {state = INT_VAL;val_int = &uws_config.http.keepalive_timeout; }
<HTTP>types_hash_max_size   {state = INT_VAL;val_int = &uws_config.http.types_hash_max_size; }
<HTTP>server_tokens  {state = BOOL_VAL;val_bool = &uws_config.http.server_tokens; }
<HTTP>server_names_hash_bucket_size   {state = INT_VAL;val_int = &uws_config.http.server_names_hash_bucket_size; }
<HTTP>access_log     {state = STRING_VAL; val_string = &uws_config.http.access_log;}
<HTTP>error_log      {state = STRING_VAL; val_string = &uws_config.http.error_log;}
<HTTP>default_type   {state = STRING_VAL; val_string = &uws_config.http.default_type;}
<HTTP>gzip           {state = BOOL_VAL;val_bool = &uws_config.http.gzip; }
<HTTP>gzip_disable   {state = STRING_VAL; val_string = &uws_config.http.gzip_disable;}
<HTTP>gzip_vary      {state = BOOL_VAL;val_bool = &uws_config.http.gzip_vary; }
<HTTP>gzip_proxied   {state = STRING_VAL; val_string = &uws_config.http.gzip_proxied;}
<HTTP>gzip_comp_level {state = INT_VAL;val_int = &uws_config.http.gzip_comp_level; }
<HTTP>gzip_http_version      {state = STRING_VAL; val_string = &uws_config.http.gzip_http_version;}
<HTTP>gzip_types      {state = ARRAY_VAL; val_array = &uws_config.http.gzip_types;}

<SERVER>root         {state = STRING_VAL; val_string= &server->root;}
<SERVER>autoindex    {state = BOOL_VAL; val_bool = &server->autoindex;}
<SERVER>proxy       {state = BOOL_VAL; val_bool = &server->proxy;}
<SERVER>facade       {state = BOOL_VAL; val_bool = &server->facade;}
<SERVER>server_name  {state = STRING_VAL; val_string = &server->server_name;}
<SERVER>index        {state = ARRAY_VAL; val_array = &server->index;}
<SERVER>error_page   {state = ARRAY_VAL; val_array = &server->error_page;}
<SERVER>fastcgi_pass {state = STRING_VAL; val_string = &server->fastcgi_pass;}
<SERVER>listen       {state = INT_VAL; val_int = &server->listen;}
<SERVER>auth_basic   {state = STRING_VAL; val_string = &server->auth_basic;}
<SERVER>auth_basic_user_file   {state = STRING_VAL; 
                     val_string = &server->auth_basic_user_file;}

<REWRITE>engine      {state = BOOL_VAL; val_bool = &server->rewrite.engine;}
<REWRITE>exist       {state = BOOL_VAL; val_bool = &server->rewrite.exist;}
<REWRITE>(allow|deny|dispatch|redirect-t|redirect-p)[^;]+ {
                        append_str_array(&server->rewrite.rules, yytext);
                     }
<UPSTREAM>server[^;]+;   { 
                        append_str_array(&server->upstream, yytext);
                     }

[^ \t\n\t;]+         {
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


;                     {state = LOOK_VAL;}
.|\n                  ;
%%

int yywrap(){
    return 1;
}

void set_default_config() {
    uws_config.user = "www-data";
    uws_config.worker_processes = 4;
    uws_config.pid = "/var/run/uws.pid";
    uws_config.events.worker_connections = 768;
    uws_config.http.mimefile = "mime.types";
    uws_config.http.sendfile = true;
    uws_config.http.tcp_nopush = true;
    uws_config.http.tcp_nodelay = true;
    uws_config.http.gzip = false;
    //lot of configs
}
void init_config(){
    set_default_config();
    yyin = fopen(CONFIG_FILE, "r");
    yylex();
    fclose(yyin);
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

char* append_str_array(str_array_t *array_t, char *string){
    if(array_t->total == 0) {
        array_t->total = INIT_ARR_LEN;
        array_t->array = (char **)calloc(array_t->total, sizeof(char*));
    }
    if(array_t->total == array_t->len + 1) {
        array_t->total *= 2;
        array_t->array = (char **)realloc(array_t->array, sizeof(char*) * array_t->total);
        bzero(array_t->array + array_t->len, array_t->total - array_t->len);
    }
    char **tmp = array_t->array;
    while(*tmp != NULL) {
        *tmp++;
    }
    *tmp = strdup(string);
    array_t->len++;
}
