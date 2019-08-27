#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>


#define SER_PORT 	6666
#define SER_IP   	"192.168.0.222"


void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}


void * fun_client(void* arg)
{	
	char buf[128];
	int ret;
	int cli_fd = *((int *)arg);
	
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
		if(!strncmp(buf,"quit",4))
		{
			printf("bye bye thread client\n");
			break;
		}
		
		printf("[%d] read buf: %s\n",cli_fd,buf);
	}
}



int main(void)
{
	int i,cli_fd = -1;
	int ret;
	pthread_t tid;
	char buf[128];
	
#if 0
	cli_fd = socket(AF_INET,SOCK_STREAM,0);
	if(cli_fd < 0)
		sys_error("socket");
#endif
	
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SER_PORT);
	//ser.sin_addr.s_addr = inet_addr(SER_IP);
	inet_aton(SER_IP,&(ser.sin_addr.s_addr));

	for(i=0;i<50;i++)
	{
		cli_fd = socket(AF_INET,SOCK_STREAM,0);
		if(cli_fd < 0)
			sys_error("socket");
		
		ret = connect(cli_fd,(struct sockaddr *)&ser,sizeof(ser));
		if(ret < 0)
			sys_error("connect");
		//sleep(1);
		//usleep(100000);
	}

		

	printf("connect ok\n");

	if(pthread_create(&tid,NULL,fun_client,((void *)&cli_fd)) != 0)	
	{			
		perror("pthread_create failed\n");			
		exit(1);		
	}



	while(1)
	{
		memset(buf,0,128);
		fgets(buf,127,stdin);
#if 0		
		if(!strncmp(buf,"quit",4))
		{
			printf("bye bye client\n");
			break;
		}
#endif
		buf[strlen(buf)-1] = '\0';
		ret = write(cli_fd,buf,strlen(buf));
		if(ret < 0)
			sys_error("read");
		printf("client senk ok\n");
		if(!strncmp(buf,"quit",4))
		{
			printf("bye bye client\n");
			break;
		}
	}

	pthread_join(tid,NULL);
	close(cli_fd);

	return 0;
}






































