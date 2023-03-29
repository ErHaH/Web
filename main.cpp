#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
//#include NODEBUG
#include <assert.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65536
#define MAX_EVENT_NUM 10000

extern void addfd(int epollfd, int fd, bool one_shot);

void addsig(int sig, void(handler)(int)) {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sigfillset(&sa.sa_mask);
	sa.sa_handler = handler;
	assert(sigaction(sig, &sa, NULL) != 1);
}

int main(int argc, char* argv[]) {
	if(argc <= 1) {
		printf("please putin:./%s port\n", basename(argv[0]));
		return -1;
	}

	int port = atoi(argv[1]);
	addsig(SIGPIPE, SIG_IGN);

	threadpool<http_conn>* pool = NULL;
	try {
		pool = new threadpool<http_conn>(8, 10000);
	}
	catch(...) {
		return -1;
	}

	http_conn* users = new http_conn[MAX_FD];

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	int reuse = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
	listen(listenfd, 5);
	
	epoll_event events[MAX_EVENT_NUM];
	int epollfd = epoll_create(5);
	addfd(epollfd, listenfd, false);
	http_conn::m_epollfd = epollfd;

	while(true) {
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
		if((number < 0) && (errno != EINTR)) {
			printf("epoll failure\n");
			break;
		}

		for(int i = 0; i < number; i++) {
			int sockfd = events[i].data.fd;
			if(sockfd == listenfd) {
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
				if(connfd < 0) {
					printf("errno is : %d\n", errno);
					continue;
				}

				if(http_conn::m_user_count > MAX_FD) {
					printf("sorry, please wait a min\n");
					close(connfd);
					continue;
				}
				users[connfd].init(connfd, client_addr);
			}
			else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				users[sockfd].close_conn();
			}
			else if(events[i].events & EPOLLIN) {
				if(users[sockfd].read()) {
					pool->append(users + sockfd);
				}
				else {
					users[sockfd].close_conn();
				}
			}
			else if(events[i].events & EPOLLOUT) {
				if(!users[sockfd].write()) {
					users[sockfd].close_conn();					
				}
			}
		}

	}
	
	close(epollfd);
	close(listenfd);
	delete [] users;
	delete pool;
	return 0;
}