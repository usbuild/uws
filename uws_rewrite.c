#include "uws_memory.h"
#include <sys/stat.h>
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_header.h"
#include "uws_error.h"
#include "uws_status.h"
#include "uws_http.h"
#include "uws_router.h"
static void 
split_string(char *src, char **type, char **regexp, char **patch) {
    int len = strlen(src);
    *type = (char*)uws_calloc(len, sizeof(char));
    *regexp = (char*)uws_calloc(len, sizeof(char));
    *patch = (char*)uws_calloc(len, sizeof(char));
    sscanf(src, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]", *type, *regexp, *patch);
}
void rewrite_router(pConnInfo conn_info) {
    if(!conn_info->running_server->rewrite.engine || conn_info->running_server->rewrite.rules.total == 0)  {
        apply_next_router(conn_info);
        return;
    }
    char **rules = conn_info->running_server->rewrite.rules.array;

    bool apply_access = false;
    bool apply_rewrite = false;

    char *type, *regexp, *patch;
    char *url = conn_info->request_header->url;
    while(*rules != NULL) {
        //apply a rule, when 
        split_string(*rules, &type, &regexp, &patch);
        if(conn_info->running_server->rewrite.exist) {
            char *path = strlcat(conn_info->running_server->root, conn_info->request_header->url);

            struct stat stat_buff;
            if(lstat(path, &stat_buff) != -1) {
                apply_rewrite = true;
            }
            uws_free(path);
        }
        if(!apply_access) {
            if(strcmp(type, "allow") == 0) {
                if(preg_match(url, regexp)) { //then apply allow rule
                    if(wildcmp(patch, conn_info->client_ip)) {
                        apply_access = true;
                    }
                }
            } else if(strcmp(type, "deny") == 0) {
                if(preg_match(url, regexp)) { //then apply deny rule
                    if(wildcmp(patch, conn_info->client_ip)) {
                        send_error_response(conn_info, 403, true);
                    }
                }
            }
        }
        if(!apply_rewrite) {
            if(strcmp(type, "dispatch") == 0) {
                if(preg_match(url, regexp)) { //then apply dispatch rule
                    char *new_url = preg_replace(url, regexp, patch);
                    uws_free(conn_info->request_header->path);
                    conn_info->request_header->path = new_url;
                    apply_rewrite = true;
                }
            } else if(strcmp(type, "redirect-t") == 0) {
                if(preg_match(url, regexp)) { //then apply redirect-t rule
                    char *new_url = preg_replace(url, regexp, patch);
                    add_header_param("Location", new_url, conn_info->response_header);
                    uws_free(new_url);
                    apply_rewrite = true;
                    apply_access = true;
                    send_error_response(conn_info, 302, false);
                }
            } else if(strcmp(type, "redirect-p") == 0){
                if(preg_match(url, regexp)) { //then apply redirect-p rule
                    char *new_url = preg_replace(url, regexp, patch);
                    add_header_param("Location", new_url, conn_info->response_header);
                    uws_free(new_url);
                    apply_rewrite = true;
                    apply_access = true;
                    send_error_response(conn_info, 301, false);

                }
            } else{}
        }
        uws_free(type); uws_free(regexp); uws_free(patch);
        rules++;
    }
}
