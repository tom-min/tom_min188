#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/socket.h>

#include<netinet/in.h>
#include<netinet/ip.h>

#include<unistd.h>
#include<sys/select.h>
#include<sys/time.h>


//
void sys_error(char *ch)
{
	perror(ch);
	exit(1);
}
#define SER_PORT   6666
#define SER_IP   	"192.168.3.199"


typedef struct cli_1{
	int cli_fd;
	int cli_port;
}cli_pack,*p_cli;



int main(void)
{
	int ret = -1;
	int serfd = -1;
	//int fd_arr[128] = {0};
	cli_pack fd_arr[128] = {0};
	int i,cnt = 0;

	//1.建立socket
	serfd = socket(AF_INET,SOCK_STREAM,0);
	if(serfd<0)
		sys_error("socket");

	int on = 1;
	setsockopt(serfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	//2.绑定本地IP和端口
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));

	ser.sin_family = AF_INET;
	ser.sin_port = htons(SER_PORT);
	//ser.sin_addr.s_addr = htonl(INADDR_ANY);
	ser.sin_addr.s_addr = inet_addr(SER_IP);

	ret = bind(serfd,(struct sockaddr*)&ser,sizeof(ser));


	//3.监听套接字
	ret = listen(serfd,5);

	int clifd = -1;
	struct sockaddr_in cli;
	bzero(&cli,sizeof(cli));
	int len = sizeof(cli);

	printf("listen\n");
	
	fd_set r_set;
       	//1.清空读集合
	FD_ZERO(&r_set); 
	int maxfd =-1;

	while(1){
		
		//2.添加要监听的文件描述符到读集合
		FD_SET(0,&r_set);	//添加标准输入到读集合
		FD_SET(serfd,&r_set);   //添加serfd到读集合
		maxfd = serfd;     
		
		for(i=0;i<cnt;i++)
		{
			if(fd_arr[i].cli_fd> 0)
			{
				FD_SET(fd_arr[i].cli_fd,&r_set);
				if(fd_arr[i].cli_fd> maxfd)
					maxfd = fd_arr[i].cli_fd;
			}
		}

		//3.阻塞等待读集合响应
		printf("select wait\n");
		ret = select(maxfd+1,&r_set,NULL,NULL,NULL);
		if(ret <0){
			perror("select");
			exit(1);
		}
		
		//4.ret >0  select集合中有响应
		if(ret >0){

			//5.二次判断哪一个文件描述符有响应


			//5.1 判断是否有键盘输入
			if(FD_ISSET(0,&r_set)){
				char rbuf[128]={0};

				fgets(rbuf,127,stdin);
				printf("rbuf:%s\n",rbuf);

			}

			//5.2 新客户端连接
			if(FD_ISSET(serfd,&r_set)){
					
				fd_arr[cnt].cli_fd= accept(serfd,(struct sockaddr*)&cli,&len);  //非阻塞
				printf("client ip=%s port=%d\n",inet_ntoa(cli.sin_addr.s_addr),ntohs(cli.sin_port));
				fd_arr[cnt].cli_port = ntohs(cli.sin_port);
				cnt++;
			}

			for(i=0;i<cnt;i++)
			{
				//5.3 客户端有数据
				if(FD_ISSET(fd_arr[i].cli_fd,&r_set))
				{

					char buf[128]={0};

					ret = read(fd_arr[i].cli_fd,buf,127);   //read非阻塞
					if(ret <0){

						perror("read");
						exit(1);
					}
					if(ret == 0){   
						printf("[%d] client exit\n",fd_arr[i].cli_port);
						FD_CLR(fd_arr[i].cli_fd,&r_set);	
						close(fd_arr[i].cli_fd);
						fd_arr[i].cli_fd= -1;
						continue;
					}
					printf("[%d] recv:%s\n",fd_arr[i].cli_port,buf);
				}
			}
		}

	}
	


	//6.关闭
	close(serfd);

	return 0;
}
