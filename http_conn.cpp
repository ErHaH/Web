#include "http_conn.h"

int setNoBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addEpoll(int epollfd, int fd, bool one_shut) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if(one_shut) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNoBlocking(fd);
}

void removefd(int epollfd, int sockfd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
    close(sockfd);
}

http_conn::http_conn() = default;
http_conn::~http_conn() = default;

void http_conn::init(int sockfd, const sockaddr_in& addr) {
    m_sockfd_ = sockfd;
    m_sockaddr_ = addr;

    int reuse = 1;
    setsockopt(m_sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addEpoll(m_epollfd_, m_sockfd_, true);
    m_user_count_++;
    //http_init();
}

bool http_conn::CloseConn() {
    if(m_sockfd_ != -1) {
        removefd(m_epollfd_, m_sockfd_);
        m_sockfd_ = -1;
        m_user_count_--;
    }
}

bool http_conn::read() {
    //TODO

    return true;
}

bool http_conn::write() {
    //TODO

    return true;
}

int http_conn::m_epollfd_ = -1;
int http_conn::m_user_count_ = 0;