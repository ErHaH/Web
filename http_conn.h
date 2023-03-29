#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

class http_conn {
public:
    http_conn() {};
    ~http_conn() {};
    void init(int &connfd, struct sockaddr_in &addr);
    void close_conn();
    void process();
    bool read();
    bool write();


public:
    static int m_epollfd;
    static int m_user_count;
private:
    int m_sockfd;
    sockaddr_in m_addr;
};

#endif