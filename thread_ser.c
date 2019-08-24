#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>


#define SER_PORT 	8888
#define SER_IP   	"192.168.0.222"


void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}


//明天加入信号量玩玩，这几天多玩玩linux的应用编程，tcp这块
void * fun_client(void* arg)
{	
	char buf[128];
	int ret;
	int cli_fd = *((int *)arg);
	//int cli_fd = 1;
	
	printf("welcome to [%s]\n",__func__);
	
	while(1)
	{
		memset(buf,0,128);
		ret = read(cli_fd,buf,127);
		if(ret < 0)
		  sys_error("read");
		else if(ret == 0)
		{
			printf("client exit\n");
			close(cli_fd);
		}
		
		printf("[%d] read buf: %s\n",cli_fd,buf);
	}
}



int main(void)
{
	int ser_fd = -1;
	int ret;
	char buf[128];
	pid_t pid;
	int cli_port = 0;
	pthread_t tid;

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

	int new_fd = -1;

	printf("wait connect\n");

	struct sockaddr_in cli;
	memset(&ser,0,sizeof(ser));
	int len = sizeof(cli);


	while(1)
	{
		   new_fd = accept(ser_fd,(struct sockaddr *)&cli,&len);
		   if(new_fd < 0)
				 sys_error("accept");

		   printf("connect new client ok\n");
printf("client addr: %s, client port: %d\n",inet_ntoa(cli.sin_addr.s_addr),ntohs(cli.sin_port));
		   cli_port = ntohs(cli.sin_port);

		if(pthread_create(&tid,NULL,fun_client,((void *)&new_fd)) != 0)	
		//if(pthread_create(&tid,NULL,fun_client,NULL) != 0)	
		{			
			perror("pthread_create failed\n");			
			exit(1);		
		}

		//这个函数的问题，以及select，epoll的使用，还有ubuntu环境
		//pthread_join(tid,NULL);
	}


	close(ser_fd);

	return 0;
}





























