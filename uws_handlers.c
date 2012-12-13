#include "uws.h"
#include "uws_status.h"
#include "uws_utils.h"
#include "uws_memory.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

void add_accept(int epollfd, pConnInfo conn_info) {
    struct sockaddr_in client_address;
    int client_sockfd;
    socklen_t client_len = sizeof(client_address);
    client_sockfd = accept(conn_info->clientfd, (struct sockaddr *)&client_address, &client_len);
    if(client_sockfd == -1) exit_err("accept_error");
    setnonblocking(client_sockfd);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    pConnInfo info = (pConnInfo) uws_malloc(sizeof(ConnInfo));
    info->clientfd = client_sockfd;
    info->status = CS_ACCEPT;
    ev.data.ptr = info;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1)
        exit_err("epoll_ctl");
}

void read_request_header(int epollfd, pConnInfo conn_info) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    conn_info->status = CS_REQUEST_READ;
    ev.data.ptr = conn_info;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_info->clientfd, &ev) == -1)
        exit_err("epoll_ctl");
}
