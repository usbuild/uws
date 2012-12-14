#include "uws.h"
#include "uws_status.h"
#include "uws_utils.h"
#include "uws_memory.h"

#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>

void add_accept(pConnInfo conn_info) {
    struct sockaddr_in client_address;
    int client_sockfd;
    socklen_t client_len = sizeof(client_address);
    client_sockfd = accept(conn_info->clientfd, (struct sockaddr *)&client_address, &client_len);
    if(client_sockfd == -1) exit_err("accept_error");
    setnonblocking(client_sockfd);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    pConnInfo info = (pConnInfo) uws_calloc(1, sizeof(ConnInfo));
    info->clientfd = client_sockfd;
    info->status = CS_ACCEPT;
    info->epollfd = conn_info->epollfd;
    ev.data.ptr = info;

    if(epoll_ctl(info->epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1)
        exit_err("epoll_ctl");
}

void read_request_header(pConnInfo conn_info) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    conn_info->status = CS_REQUEST_READ;
    ev.data.ptr = conn_info;
    if(epoll_ctl(conn_info->epollfd, EPOLL_CTL_ADD, conn_info->clientfd, &ev) == -1)
        exit_err("epoll_ctl");
}
