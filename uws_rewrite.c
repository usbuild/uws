#include "uws_rewrite.h"
#include <sys/stat.h>
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_header.h"
#include "uws_error.h"
static void 
split_string(char *src, char **type, char **regexp, char **patch) {
    int len = strlen(src);
    *type = (char*)uws_calloc(len, sizeof(char));
    *regexp = (char*)uws_calloc(len, sizeof(char));
    *patch = (char*)uws_calloc(len, sizeof(char));
    sscanf(src, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]", *type, *regexp, *patch);
}
int rewrite_router(int sockfd) {
    if(!running_server->rewrite.engine || running_server->rewrite.rules.total == 0) return 1;
    char **rules = running_server->rewrite.rules.array;

    bool apply_access = false;
    bool apply_rewrite = false;

    char *type, *regexp, *patch;
    char *url = request_header->url;
    while(*rules != NULL) {
        //apply a rule, when 
        split_string(*rules, &type, &regexp, &patch);
        if(running_server->rewrite.exist) {
            char *path = strlcat(running_server->root, request_header->url);

            struct stat stat_buff;
            if(lstat(path, &stat_buff) != -1) {
                apply_rewrite = true;
            }
            uws_free(path);
        }
        if(!apply_access) {
            if(strcmp(type, "allow") == 0) {
                if(preg_match(url, regexp)) { //then apply allow rule
                    if(wildcmp(patch, client_ip)) {
                        apply_access = true;
                    }
                }
            } else if(strcmp(type, "deny") == 0) {
                if(preg_match(url, regexp)) { //then apply deny rule
                    if(wildcmp(patch, client_ip)) {
                        send_error_response(sockfd, 403, true);
                    }
                }
            }
        }
        if(!apply_rewrite) {
            if(strcmp(type, "dispatch") == 0) {
                if(preg_match(url, regexp)) { //then apply dispatch rule
                    char *new_url = preg_replace(url, regexp, patch);
                    uws_free(request_header->path);
                    request_header->path = new_url;
                    apply_rewrite = true;
                }
            } else if(strcmp(type, "redirect-t") == 0) {
                if(preg_match(url, regexp)) { //then apply redirect-t rule
                    char *new_url = preg_replace(url, regexp, patch);
                    add_header_param("Location", new_url, response_header);
                    uws_free(new_url);
                    apply_rewrite = true;
                    apply_access = true;
                    send_error_response(sockfd, 302, false);
                }
            } else if(strcmp(type, "redirect-p") == 0){
                if(preg_match(url, regexp)) { //then apply redirect-p rule
                    char *new_url = preg_replace(url, regexp, patch);
                    add_header_param("Location", new_url, response_header);
                    uws_free(new_url);
                    apply_rewrite = true;
                    apply_access = true;
                    send_error_response(sockfd, 301, false);

                }
            } else{}
        }
        uws_free(type); uws_free(regexp); uws_free(patch);
        rules++;
    }
    return 1;
}
