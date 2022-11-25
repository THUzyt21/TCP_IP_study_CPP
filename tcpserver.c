#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <pthread.h>
#define MAXLNE  4096
#define POLL_SIZE	1024
//8m * 4G = 128 , 512
//C10k
void *client_routine(void *arg) { //
	int connfd = *(int *)arg;
	char buff[MAXLNE];
	while (1) {
		int n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);
	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
			break;
        }
	}
	return NULL;
}
int main(int argc, char **argv) 
{
    int listenfd, connfd, n;
    struct sockaddr_in servaddr; //存储地址和端口号等信息
    char buff[MAXLNE];
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //创建套接字并开启监听
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9999);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;  //原型：bind(int socketfd, const struct sockaddr* ,socklen_t)
    }//bind到端口，迎宾的门口
    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;//listen(socketfd, backlog)
    }//listen迎宾的小姐姐,得到listenfd
 #if 0
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }//accept,与客户端进行连接，得到connfd，即在包厢里面服务的小姐姐
    //sockfd, struct addr*, *addrlen
    printf("========waiting for client's request========\n");
    while (1) {//收发的一个过程

        n = recv(connfd, buff, MAXLNE, 0);//sockfd, void* buff, size_t size,
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0


    printf("========waiting for client's request========\n");
    while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

        n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0

	while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

		pthread_t threadid;
		pthread_create(&threadid, NULL, client_routine, (void*)&connfd);

    }

#elif 0

	// select
	fd_set rfds, rset, wfds, wset;

	FD_ZERO(&rfds);//初始化select读文件描述符
	FD_SET(listenfd, &rfds);//增加监听
	FD_ZERO(&wfds);//初始化+添加listen fd
	int max_fd = listenfd;
	while (1) {
		rset = rfds;
		wset = wfds;
		int nready = select(max_fd+1, &rset, &wset, NULL, NULL);//轮询中检测
		if (FD_ISSET(listenfd, &rset)) { //测试listenfd在不在rset有没有数据
            //接受客户端的连接
			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;//开始连接的文件描述符
		    }//accept 用于面向连接服务器，参数addr 和addrlen 存放客户方的地址信息，调用前，参数addr 指向一个初始值为
//空的地址结构，而addrlen 初始化为0。调用accept 后，服务器从编号为参数s 表示的套
//接字上接受用户连接请求，连接请求是由客户方的connet 调用发出的，当有连接请求到达
//时，accept 调用将请求连接队列上的第一个客户方套接字地址及长度放入addr 和addrlen
//中，并创建一个与参数s 有相同属性的新套接字。

			FD_SET(connfd, &rfds);//添加客户端的文件描述符
			if (connfd > max_fd) max_fd = connfd;//更新最大fd
			if (--nready == 0) continue;
		}
		int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++) { //通信的文件描述符，从listenfd+1开始进行通信，1个监听的,n个通信的fd
			if (FD_ISSET(i, &rset)) { //如果i位置需要收东西
				n = recv(i, buff, MAXLNE, 0);//从客户端收
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);
					FD_SET(i, &wfds);
					//reactor
					//send(i, buff, n, 0);
		        } else if (n == 0) { //收到0表示客户端断开连接
					FD_CLR(i, &rfds);
					//printf("disconnect\n");
		            close(i);
		        }
				if (--nready == 0) break;
			} else if (FD_ISSET(i, &wset)) {//如果i位置要写东西
				send(i, buff, n, 0);
				FD_SET(i, &rfds);
			}
		}
	}

#elif 0


	struct pollfd fds[POLL_SIZE] = {0};
	fds[0].fd = listenfd;
	fds[0].events = POLLIN;

	int max_fd = listenfd;
	int i = 0;
	for (i = 1;i < POLL_SIZE;i ++) {
		fds[i].fd = -1;
	}
	while (1) {
		int nready = poll(fds, max_fd+1, -1);
		if (fds[0].revents & POLLIN) {
			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;
		    }
			printf("accept \n");
			fds[connfd].fd = connfd;
			fds[connfd].events = POLLIN;
			if (connfd > max_fd) max_fd = connfd;
			if (--nready == 0) continue;
		}
		//int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++)  {
			if (fds[i].revents & POLLIN) {
				n = recv(i, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);
					send(i, buff, n, 0);
		        } else if (n == 0) { //
					fds[i].fd = -1;
		            close(i);
		        }
				if (--nready == 0) break;
			}

		}

	}

#else
	//poll/select --> 
	// epoll_create 
	// epoll_ctl(ADD, DEL, MOD)
	// epoll_wait
	int epfd = epoll_create(1); //创建epoll实例，通过epfd来操作它，size必须大于0
	struct epoll_event events[POLL_SIZE] = {0};//首先放置用于监听的文件描述符，listenfd
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;//初始化一个epollin的事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);//把listen事件加到epoll树上
	while (1) {
		int nready = epoll_wait(epfd, events, POLL_SIZE, 5);//使用epoll_wait检测树上的东西
        //返回就绪的fd数量，并写入到events数组里面
		if (nready == -1) {
			continue;
		}
		int i = 0;
		for (i = 0;i < nready;i ++) {
			int clientfd =  events[i].data.fd;//寻找events数组的文件描述符
			if (clientfd == listenfd) {//如果是用于监听的，就进行客户端的连接
				struct sockaddr_in client;
			    socklen_t len = sizeof(client);
			    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
			        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			        return 0;
			    }
				printf("accept\n");
				ev.events = EPOLLIN;
				ev.data.fd = connfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);//连接完了以后把connfd加到树上
			} else if (events[i].events & EPOLLIN) {//如果是用于通信的文件描述符，就进行通信
				n = recv(clientfd, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);
					send(clientfd, buff, n, 0);
		        } else if (n == 0) { //
					ev.events = EPOLLIN;
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
		            close(clientfd);
		        }

			}

		}

	}
	

#endif
 
    close(listenfd);
    return 0;
}

