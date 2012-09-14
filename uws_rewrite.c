#include "uws_rewrite.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_header.h"
#include "uws_error.h"
void split_string(char *src, char **type, char **regexp, char **patch) {
    int len = strlen(src);
    *type = (char*)calloc(len, sizeof(char));
    *regexp = (char*)calloc(len, sizeof(char));
    *patch = (char*)calloc(len, sizeof(char));
    sscanf(src, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]", *type, *regexp, *patch);
}
int rewrite_router(int sockfd) {
    char **rules = running_server->rewrite.rules.array;

    bool apply_access = false;
    bool apply_rewrite = false;

    char *type, *regexp, *patch;
    char *url = request_header->url;
    while(*rules != NULL) {
        //apply a rule, when 
        split_string(*rules, &type, &regexp, &patch);
        if(!apply_access) {
            if(strcmp(type, "allow") == 0) {
                if(preg_match(url, regexp)) { //then apply allow rule
                    if(wildcmp(patch, client_ip)) {
                        apply_access = true;
                    }
                }
            } else if(strcmp(type, "deny") == 0) {
                if(preg_match(url, regexp)) { //then apply allow rule
                    if(wildcmp(patch, client_ip)) {
                        send_error_response(sockfd, 403, true);
                    }
                }
            }
        }
        if(!apply_rewrite) {
            if(strcmp(type, "dispatch") == 0) {
                if(preg_match(url, regexp)) { //then apply allow rule
                    char *new_url = preg_replace(url, regexp, patch);
                    free(request_header->url);
                    request_header->url = new_url;
                    apply_rewrite = true;
                }
            }
        }
        free(type); free(regexp); free(patch);
        rules++;
    }
    return 1;
}
