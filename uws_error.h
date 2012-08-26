#ifndef __UWS_ERROR_H__
#define __UWS_ERROR_H__
#include "uws.h"
void send_error_response(int client_fd, int status_code);
#endif
