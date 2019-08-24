#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>


#define SER_PORT 	8888
#define SER_IP   	"192.168.0.222"


void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}


int main(void)
{
	int ser_fd = -1;
	int ret;

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


	new_fd = accept(ser_fd,NULL,NULL);
	if(new_fd < 0)
		sys_error("accept");

	printf("connect new client ok\n");

	char buf[128];

	while(1)
	{
		memset(buf,0,128);
		ret = read(new_fd,buf,127);
		if(ret < 0)
			sys_error("read");
		else if(ret == 0)
		{
			printf("client exit\n");
			close(new_fd);
			break;
		}
		printf("read buf: %s\n",buf);
	}
	
	close(ser_fd);

	return 0;
}





























