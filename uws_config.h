#ifndef __UWS_CONFIG_H__
#define __UWS_CONFIG_H__
#define bool    int
#define true    1
#define false   0
#define CONFIG_FILE "uws.conf"

//config struct defined here
typedef struct{
    char* root;
    bool autoindex;
    char** error_page;
    char** index;
    char* server_name;
    char* fastcgi_pass;
} server_cfg_t;

struct events_cfg{
    int worker_connections;
};

struct http_cfg{
    char* mimefile;
    bool sendfile;
    bool tcp_nopush;
    bool tcp_nodelay;
    int keepalive_timeout;
    int types_hash_max_size;
    bool server_tokens;
    int server_names_hash_bucket_size;
    char* access_log;
    char* error_log;
    bool gzip;
    char* gzip_disable;
    bool gzip_vary;
    char* gzip_proxied;
    int     gzip_comp_level;
    char*   gzip_http_version;
    char**  gzip_types;
    server_cfg_t** servers;
};

typedef struct{
    char* user;
    int worker_processes;
    char* pid;
    struct events_cfg events;
    struct http_cfg http;
} uws_config_t;
//config definition end here
uws_config_t uws_config;//Main Configuration
server_cfg_t* running_server;//Current Server Profile Used
void init_config();

#endif

