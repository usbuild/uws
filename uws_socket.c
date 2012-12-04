#include "uws.h"
#include "uws_memory.h"
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include "uws_socket.h"
#include "uws_mime.h"
#include "uws_config.h"
#include "uws_utils.h"
#include "uws_fdhandler.h"
#include "uws_header.h"
#define MAX_EVENTS  10
//#define DEBUG


struct epoll_event events[MAX_EVENTS];
int epollfd, nfds;
int start_server()
{
    int res;
    int reuse = 1;
    int worker_processes = uws_config.worker_processes;
    int worker_count = 0;
    int server_port_num = 0;
    int i = 0;
    int ports_num = 0;
    int servers_num = 0;
    pid_t self_pid;

    signal(SIGPIPE, SIG_IGN);

    //here we got all servers count
    while(uws_config.http.servers[servers_num++] != NULL);
    servers_num--;

    int *servers_port = (int*)malloc(sizeof(int) * server_port_num);
    for(i = 0; i < servers_num; i++) {
        int listen_port = uws_config.http.servers[i]->listen;
        if(in_int_array(servers_port, listen_port, ports_num) == -1) {
            servers_port[ports_num++] = listen_port;
        }
    }
    //ports_num now stores the distinct port num
    int *listen_fds = (int*) malloc(sizeof(int) * ports_num);

    for(i = 0; i < ports_num; i++) {
        socklen_t server_len;
        struct sockaddr_in server_address;
        int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(servers_port[i]);

        res = setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
        if(res < 0) exit_err("Set Socket Option Fail");
        server_len = sizeof(server_address);
        res = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
        if(res < 0) exit_err("Bind Error");
        res = listen(server_sockfd, 500);
        if(res < 0) exit_err("Listen Error");
        listen_fds[i] = server_sockfd;
    }
    uws_free(servers_port);

    //prefork here
#ifndef DEBUG
    self_pid = getpid();
    for(worker_count = 0; worker_count < worker_processes; worker_count++ ){
        pid_t pid = fork();
        if(pid < 0) exit_err("Fork Worker Error");
        if(pid == 0) break;//Master continuing fork
    }
    if(getpid() == self_pid)//this is master process
    {
        int statloc;
        pid_t child_pid;
        while((child_pid = wait(&statloc)) != -1)
        {
            printf("Child process %d exit with %d\n", child_pid, statloc);
        }
        return 0;
    }
#endif

    //epoll init here
    epollfd = epoll_create(MAX_EVENTS);//create
    if(epollfd == -1) exit_err("Epoll create");

    for(i = 0; i < ports_num; i++) {
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = listen_fds[i];
        if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fds[i], &ev) == -1)
            exit_err("epoll_ctl: listen_sock");
    }

    //epoll here end
    for( ; ; ) { 
        int n;
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if(nfds == -1) exit_err("epoll_wait");

        for(n = 0; n < nfds; n++) {
            if(in_int_array(listen_fds, events[n].data.fd, ports_num) != -1) {
                int pos = in_int_array(listen_fds, events[n].data.fd, ports_num);
                struct sockaddr_in client_address;
                int server_sockfd, client_sockfd; 
                server_sockfd = events[n].data.fd;
                socklen_t client_len = sizeof(client_address);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                if(client_sockfd == -1) exit_err("accept_error");
                setnonblocking(client_sockfd);
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
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
