#include "uws_memory.h"
#include "uws_auth.h"
#include "uws_config.h"
#include "uws_header.h"
#define BUFF_LEN 64
bool validate(char *raw_str, char *file){
    FILE *f = fopen(file, "r");
    char line[BUFF_LEN];
    char auth_str[BUFF_LEN];
    sscanf(raw_str, "%*[^ ]%*[ ] %s", raw_str);//By now, only basic auth can be applied
    while(fgets(line, BUFF_LEN, f)) {
        line[strlen(line) - 1] = 0;
        char *enc = (char*)base64(line);
        if(strcmp(enc, raw_str) == 0) return 1;
        uws_free(enc);
    }
    fclose(f);
    return 0;
}
int auth_router(int sockfd) {
    if(running_server->auth_basic == NULL) return 1;
    char *auth_str = get_header_param("Authorization", request_header);
    char value[PATH_LEN] = {0};
    sprintf(value, "Basic realm=\"%s\"", running_server->auth_basic);
    if(auth_str == NULL) {
        add_header_param("WWW-Authenticate", value, response_header);
        send_error_response(sockfd, 401, true);
    } else {
        if(validate(auth_str, running_server->auth_basic_user_file)) {
            return 1;
        } else {
            add_header_param("WWW-Authenticate", value, response_header);
            send_error_response(sockfd, 401, true);
        }
    }
}
