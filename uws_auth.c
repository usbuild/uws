#include "uws_auth.h"
#include "uws_config.h"
#include "uws_header.h"
#define BUFF_LEN 64
bool validate(char *auth_str, char *file){
    FILE *f = fopen(file, "r");
    char line[BUFF_LEN];
    char username[BUFF_LEN];
    char password[BUFF_LEN];
    while(fgets(line, BUFF_LEN, f)) {
        bzero(username, BUFF_LEN);
        bzero(password, BUFF_LEN);
        sscanf(line, "%[^:]:%s", username, password);
    }
    fclose(f);
    return 1;
}
int auth_router(int sockfd) {
    if(running_server->auth_basic == NULL) return 1;
    char *auth_str = get_header_param("Authorization", request_header);
    if(auth_str == NULL) {
        char value[PATH_LEN] = {0};
        sprintf(value, "Basic realm=\"%s\"", running_server->auth_basic);
        add_header_param("WWW-Authenticate", value, response_header);
        send_error_response(sockfd, 401, true);
    } else {
        if(validate(auth_str, running_server->auth_basic_user_file)) {

            return 1;
        } else {
            send_error_response(sockfd, 401, true);
        }
    }
}
