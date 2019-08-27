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


#if 0
int main(void)
{
	int 	ser_fd = -1;
	int 	ret;
	char buf[128];
	int 	cli_fd = -1;
	int maxfd = -1;
	fd_set r_set;
	//int fd_arr[128] = {0};
	cli_pack fd_arr[128] = {0};
	int i,cnt = 0;

	ser_fd = socket(AF_INET,SOCK_STREAM,0);
	if(ser_fd < 0)
		sys_error("socket");
	
	int on = 1;
	setsockopt(ser_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SER_PORT);
	ser.sin_addr.s_addr = inet_addr(SER_IP);

	ret = bind(ser_fd,(struct sockaddr *)&ser,sizeof(ser));
	if(ret < 0)
		sys_error("bind");
		
	ret = listen(ser_fd,5);
	if(ret < 0)
		sys_error("listen");

	printf("wait connect\n");

	struct sockaddr_in cli;
	int cli_len = 0;
	memset(&cli,0,sizeof(cli));

	FD_ZERO(&r_set);
	while(1)
	{
		FD_SET(0,&r_set);
		FD_SET(ser_fd,&r_set);
		maxfd = ser_fd;

#if 0
		if(cli_fd > 0)
		{
			FD_SET(cli_fd,&r_set);
			if(cli_fd > maxfd)
				maxfd = cli_fd;
		}
#else
		for(i=0;i<cnt;i++)
		{
			if(fd_arr[i].cli_fd > 0)
			{
				FD_SET(fd_arr[i].cli_fd,&r_set);
				if(fd_arr[i].cli_fd> maxfd)
					maxfd = fd_arr[i].cli_fd;
			}
		}
		
#endif

		printf("select wait\n");

		ret = select(maxfd+1,&r_set,NULL,NULL,NULL);
		if(ret < 0)
			sys_error("select");
		
		if(ret > 0)
		{
			if(FD_ISSET(0,&r_set))
			{
				memset(buf,0,128);
				fgets(buf,127,stdin);
				printf("tom read: %s\n",buf);
			}
			
			if(FD_ISSET(ser_fd,&r_set))
			{
				cli_fd = accept(ser_fd,(struct sockaddr *)&cli,&cli_len);
				if(cli_fd < 0)
					sys_error("accept");
				fd_arr[cnt++].cli_fd = cli_fd;
printf("client addr: %s, client port: %d\n",inet_ntoa(cli.sin_addr.s_addr),ntohs(cli.sin_port));
				fd_arr[cnt].cli_port = ntohs(cli.sin_port);
			}

			for(i=0;i<cnt;i++)
			{
				if(FD_ISSET(fd_arr[i].cli_fd,&r_set))
				{
					memset(buf,0,128);
					ret = read(fd_arr[i].cli_fd,buf,127);
					if(ret < 0)
						sys_error("read");
					else if(ret == 0)
					{
						printf("client exit\n");
						FD_CLR(fd_arr[i].cli_fd,&r_set);
						close(fd_arr[i].cli_fd);
						fd_arr[i].cli_fd = -1;
						continue;
					}

					printf("[%s] server recv buf: %s\n",__TIME__,buf);
				}
			}
		}
	}

	close(ser_fd);

	return 0;
}


#else

/*
EPOLLIN:表示对应的文件描述符可以读
EPOLLOUT:表示对应的文件描述符可以写
EPOLLET: 将EPOLL设为边缘触发(Edge Triggered)模式
*/

int main(void)
{
	int i,epfd,nfds;
	
	struct epoll_event ev,events[5];
	epfd = epoll_create(1);
	ev.data.fd = STDIN_FILENO;
	ev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&ev);

	for(;;)
	{
		nfds = epoll_wait(epfd,events,5,-1);
		for(i=0;i<nfds;i++)
		{
			if(events[i].data.fd == STDIN_FILENO)
				printf("welcome to epoll's world!\n");
		}
	}
	
	return 0;
}

#endif


