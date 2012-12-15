#include "uws_memory.h"
#include "uws_config.h"
#include "uws_header.h"
#include "uws_status.h"
#include "uws_utils.h"
#include "uws_router.h"

#define AUTH_LEN 128
bool validate(char *raw_str, char *file){
    FILE *f = fopen(file, "r");
    char line[AUTH_LEN];
    sscanf(raw_str, "%*[^ ]%*[ ] %s", raw_str);//By now, only basic auth can be applied
    while(fgets(line, AUTH_LEN, f)) {
        line[strlen(line) - 1] = 0;
        char *enc = (char*)base64(line);
        if(strcmp(enc, raw_str) == 0) return 1;
        uws_free(enc);
    }
    fclose(f);
    return 0;
}
void auth_router(pConnInfo conn_info) {
    if(conn_info->running_server->auth_basic == NULL) {
        apply_next_router(conn_info);
        return;
    }
    char *auth_str = get_header_param("Authorization", conn_info->request_header);
    char value[PATH_LEN] = {0};
    sprintf(value, "Basic realm=\"%s\"", conn_info->running_server->auth_basic);
    if(auth_str == NULL) {
        add_header_param("WWW-Authenticate", value, conn_info->response_header);
        conn_info->status = 401;
        apply_next_router(conn_info);
        return;
    } else {
        if(validate(auth_str, conn_info->running_server->auth_basic_user_file)) {
            return;
        } else {
            add_header_param("WWW-Authenticate", value, conn_info->response_header);
            conn_info->status = 401;
            apply_next_router(conn_info);
            return;
        }
    }
}
