#include <iostream>
#include <memory>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>

#include "http_conn.h"
#include "threadpool.hpp"

const int MAX_FD = 65536;
const int MAX_EVENT_NUM = 10000;

extern void addEpoll(int epollfd, int fd, bool one_shut);

void addSignal(int sig, void (handler)(int)) {
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = handler;
    sigfillset(&act.sa_mask);
    sigaction(sig, &act, NULL);
}

int main(int argc, char* argv[]) {
    if(argc <= 1) {
        std::cout << "You should input:" << argv[0] << " port number.Such as ./a.out 10000" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    if(port < 0 || port > 65535) {
        std::cout << "Port number should greater than 0 and less than 65536" << std::endl;
        return 1;
    }

    addSignal(SIGPIPE, SIG_IGN);
    ThreadPool<http_conn>* pool = NULL;
    //std::unique_ptr<http_conn> pool2(new ThreadPool<http_conn>);
    try {
        pool = new ThreadPool<http_conn>;
    }
    catch(...) {
        return 10;
    }

    http_conn* users = new http_conn[MAX_FD];

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        perror("socket");
        exit(-1);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(listenfd, (sockaddr*)&address, sizeof(address));
    listen(listenfd, 5);

    int epollfd = epoll_create(5);
    epoll_event events[MAX_EVENT_NUM];
    addEpoll(epollfd, listenfd, true);
    http_conn::m_epollfd_ = epollfd;

    while(true) {
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
        if((num < 0) && (errno != EINTR)) {
            std::cout << "Epoll fail..." << std::endl;
            break;
        }
        
        for(int i = 0; i < num; ++i) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
                
                if(client_fd < 0) {
                    std::cout << "accept fail..." << std::endl;
                    continue;
                }

                if(http_conn::m_user_count_ >= MAX_FD) {
                    close(client_fd);
                    continue;
                }
                users[client_fd].init(client_fd, client_addr);
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                users[sockfd].CloseConn();
            }
            else if(events[i].events & (EPOLLIN)) {
                if(users[sockfd].read()) {
                    pool->AppendRequest(users + sockfd);
                }
                else {
                    users[sockfd].CloseConn();
                }
            }     
            else if(events[i].events & (EPOLLOUT)) {
                if(users[sockfd].write()) {
                    std::cout << "SUEECSS WRITE..." << std::endl;
                }
                else{
                    users[sockfd].CloseConn();
                }
            }                        
        }
    }

    close(epollfd);
    close(listenfd);
    delete[] users;
    delete pool;
    return 0;
}