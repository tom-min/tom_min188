#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>


#define SER_PORT 	6666
#define SER_IP   	"192.168.0.222"


void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}


int main(void)
{
	int cli_fd = -1;
	int ret;

	cli_fd = socket(AF_INET,SOCK_STREAM,0);
	if(cli_fd < 0)
		sys_error("socket");
	
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SER_PORT);
	//ser.sin_addr.s_addr = inet_addr(SER_IP);
	inet_aton(SER_IP,&(ser.sin_addr.s_addr));

	ret = connect(cli_fd,(struct sockaddr *)&ser,sizeof(ser));
	if(ret < 0)
		sys_error("bind");
		

	printf("connect ok\n");

	char buf[128];

	while(1)
	{
		memset(buf,0,128);
		fgets(buf,127,stdin);
		if(!strncmp(buf,"quit",4))
		{
			printf("bye bye client\n");
			break;
		}
		buf[strlen(buf)-1] = '\0';
		ret = write(cli_fd,buf,strlen(buf));
		if(ret < 0)
			sys_error("read");
		printf("client senk ok\n");
	}
	
	close(cli_fd);

	return 0;
}






































