#include "uws.h"
#include <mcheck.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include "uws_socket.h"
#include "uws_config.h"
#include "uws_router.h"
#include "uws_mime.h"
#include "uws_utils.h"
extern int
errno;
void
exit_err(const char* str) 
{
    printf("%s: %s\n", str, strerror(errno));
    exit(errno);
}

int
main(int argc, const char *argv[])
{
    #ifndef DEBUG
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    if(fork() != 0) exit(0);
    if(setsid() < 0) exit(0);
    if(fork() != 0) exit(0);
    int fd, fdtablesize;
    for (fd = 0, fdtablesize = getdtablesize(); fd < fdtablesize; fd++)
        close(fd);
    umask(0);
#endif

    setenv("MALLOC_TRACE", "output", 1);
    //mtrace();
    init_config();
    read_mime();
    init_routers();
    start_server();
    return 0;
}

