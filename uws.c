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
#include <pwd.h>
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

    setenv("MALLOC_TRACE", "output", 1);
    //mtrace();
    init_config();

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


    //lockfile
    fd = open(uws_config.pid, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd < 0) exit_err("open lockfile");
    if(lockfile(fd) < 0)  {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd); 
        }
        exit_err("Lockfile");
    }


    //set uid
    struct passwd* pd = getpwnam(uws_config.user);
    if(pd == NULL) exit_err("No such user");
    int res = setgid(pd->pw_gid);
    if(res != 0) exit_err("Set gid");
    res = setuid(pd->pw_uid);
    if(res != 0) exit_err("Set uid");
#endif

    read_mime();
    init_routers();
    start_server();
    return 0;
}

