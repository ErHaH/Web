#include "http_conn.h"

int http_conn::m_epollfd = -1;
int http_conn::m_user_count = 0;

void setnonblocking(int& fd) {
    int oldflag = fcntl(fd, F_GETFL);
    int newflag = oldflag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
}

void addfd(int epollfd, int fd, bool one_shot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    if(one_shot == true) {
        event.events |= EPOLLONESHOT;
    }
    setnonblocking(fd);    
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

void http_conn::init(int &connfd, struct sockaddr_in &addr) {
    this->m_sockfd = connfd;
    this->m_addr = addr;

    int reuse = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addfd(m_epollfd, m_sockfd, true);
}

void http_conn::close_conn() {
    if(m_sockfd != -1) {
        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_sockfd, 0);
        close(m_sockfd);
        m_sockfd == -1;
        m_user_count--;
    }
}

bool http_conn::read() {
    printf("一次性读完啦...\n");
    return true;
}

bool http_conn::write() {
    return true;
}

void http_conn::process() {
    printf("process!\n");
}