#include "uws_rewrite.h"
#include "uws_utils.h"
#include "uws_config.h"
#include "uws_header.h"
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
                        puts("allow it");
                    }
                }
            } else if(strcmp(type, "deny") == 0) {
            }
        }

        free(type); free(regexp); free(patch);
        rules++;
    }
    return 1;
}

