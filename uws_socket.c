#include "uws.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include "uws_socket.h"
#include "uws_mime.h"
#include "uws_config.h"
#include "uws_utils.h"
#include "uws_fdhandler.h"
#define MAX_EVENTS  100


int server_sockfd, client_sockfd;
static void
sig_int(int signo)
{
    close(server_sockfd);
    exit(0);
}
int start_server()
{
    socklen_t server_len/*, client_len*/;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int res;
    int reuse = 1;
    int worker_processes = uws_config.worker_processes;
    int worker_count = 0;
    signal(SIGINT, sig_int);
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    res = setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if(res < 0) {
        exit_err("Set Socket Option Fail");
    }

    server_len = sizeof(server_address);
    res = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    if(res < 0) {
        exit_err("Bind Error");
    }

    res = listen(server_sockfd, 500);
    if(res < 0) {
        exit_err("Listen Error");
    }
    printf("Server Listening On: %d\n", PORT);
    //prefork here
    for(worker_count = 0; worker_count < worker_processes; worker_count++ ){
        pid_t pid = fork();
        if(pid < 0)
            exit_err("Fork Worker Error");
        if(pid == 0) break;//Master continuing fork
    }
    //epoll init here
    struct epoll_event ev,events[MAX_EVENTS];
    int nfds,epollfd;
    epollfd = epoll_create(100);//create
    if(epollfd == -1){
        exit_err("Epoll create");
    }
    ev.events = EPOLLIN;
    ev.data.fd = server_sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sockfd, &ev) == -1)
    {
        exit_err("epoll_ctl: listen_sock");
    }

    //epoll here end
    while(1) { 
        int n;
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if(nfds == -1) exit_err("epoll_wait");

        for(n = 0; n < nfds; n++) {
            if(events[n].data.fd == server_sockfd){
                socklen_t client_len;
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                if(client_sockfd == -1)
                    exit_err("accept_error");
                setnonblocking(client_sockfd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_sockfd;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1)
                    exit_err("epoll_ctl:conn_sock");
                //fd here
            } else {
                handle_client_fd(events[n].data.fd);
            }
        }
    }
}

