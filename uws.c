#include "uws.h"
#include "uws_socket.h"
extern int errno;
void
exit_err(const char* str) 
{
    printf("%s: %s\n", str, strerror(errno));
    exit(errno);
}

int
main(int argc, const char *argv[])
{
    init_config();
    start_server();
    return 0;
}

