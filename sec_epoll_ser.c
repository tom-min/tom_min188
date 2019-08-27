#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>


#include <sys/time.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>


#define SER_PORT 	6666
#define SER_IP   	"192.168.0.222"


typedef struct cli_1{
	int cli_fd;
	int cli_port;
}cli_pack,*p_cli;



void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}

#define MAXEPOLLSIZE 	10000
#define MAXLINE 		10240
int handle(int coonfd);

int setnonblocking(int sockfd)
{
	if(fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK) == -1)
		return -1;
	return 0;
}


int main(void)
{
	int servport = 6888;	
	int listenq = 1024;
	
	int listenfd,connfd,kdpfd,nfds,n,nread,curfds,acceptCount = 0;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t socklen = sizeof(struct sockaddr_in);
	struct epoll_event ev;
	struct epoll_event events[MAXEPOLLSIZE];
	struct rlimit rt;
	char buf[MAXLINE];
	time_t tm;

	rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
	if((setrlimit(RLIMIT_NOFILE,&rt) == -1))
	{
		perror("setrlimit error");
		return -1;
	}
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SER_PORT);
	
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd < 0)
		sys_error("socket");
	
	int on = 1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)))
	{
		sys_error("bind");
	}
		
	if(listen(listenfd,listenq) < 0)
		sys_error("listen188");

	printf("welcome to epoll_create\n");

	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = listenfd;

	if(epoll_ctl(kdpfd,EPOLL_CTL_ADD,listenfd,&ev) < 0)
	{
		fprintf(stderr,"epoll set insertion error: fd=%d\n",listenfd);
		return -1;
	}

	curfds = 1;

	printf("epollserver startup,port %d, max connection is %d, backlog is %d\n", SER_PORT, MAXEPOLLSIZE, listenq);

	while(1)
	{
		nfds = epoll_wait(kdpfd,events,curfds,-1);
		if(nfds < 0)
			sys_error("epoll_wait");	

		for(n = 0;n < nfds;n++)
		{
			//printf("tom1-----\r\n");
			if(events[n].data.fd == listenfd)
			{
				//printf("tom2-----\r\n");
				connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&socklen);
				if(connfd < 0)
					sys_error("accept");

				time(&tm);
                	sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
				printf("%s %d:%s", ctime(&tm),++acceptCount, buf);
			

				if(curfds > MAXEPOLLSIZE)
				{
					fprintf(stderr,"too many connection, more than %d\n",curfds);
					close(connfd);
					continue;
				}
				if(setnonblocking(connfd) < 0)
					perror("setnonblocking error");
				
				ev.events = EPOLLIN|EPOLLET; 
				ev.data.fd = connfd;

				if(epoll_ctl(kdpfd,EPOLL_CTL_ADD,connfd,&ev) < 0)
				{
					fprintf(stderr, "add socket '%d' to epoll failed: %s\n", connfd, strerror(errno));
					return -1;
				}
				
				curfds++;
				continue;
			}
			if(handle(events[n].data.fd) < 0)
			{
				epoll_ctl(kdpfd,EPOLL_CTL_DEL,events[n].data.fd,&ev);
				curfds--;
			}
			//printf("tom3-----\r\n");
		}
	
	}
	
	close(listenfd);
	
	return 0;
}


int handle(int connfd)
{
	int ret;
	char buf[MAXLINE];

	printf("welcome to [%s]\n",__func__);
	ret = read(connfd,buf,MAXLINE);
	if(ret == 0)
	{
		printf("client close the connection\n");
		close(connfd);
		return -1;
	}
	else if(ret < 0)
	{
		perror("read\n");
		close(connfd);
		return -1;
	}

	printf("server read buf: %s\r\n",buf);
	write(connfd,buf,strlen(buf));

	return 0;
}








