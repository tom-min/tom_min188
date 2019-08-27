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


#define SER_PORT 	6667
#define SER_IP   	"192.168.3.199"


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
		
	if(listen(servaddr,listenq));
		sys_error("listen");

	printf("welcome to epoll_create\n");

	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = listenfd;

	
			
	
	




	
	return 0;
}


