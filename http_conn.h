#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

class http_conn {
public:
    http_conn();
    ~http_conn();

    void init(int sockfd, const sockaddr_in& addr);
    bool CloseConn();
    bool read();
    bool write();

public:
    static int m_epollfd_;
    static int m_user_count_;

private:
    int m_sockfd_;
    sockaddr_in m_sockaddr_;
};

#endif