#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>



int main(void)
{
	int fd,ret;

	umask(0);
	fd = open("177.txt",O_RDWR|O_CREAT,0666);
	if(fd < 0)
	{
		perror("open failed\n");
		exit(1);
	}
	char buf[128];
	while(1)
	{
		   memset(buf,0,128);
		   fgets(buf,127,stdin);
		   //buf[strlen(buf)-1] = 0;
		   buf[strlen(buf)-1] = '\0';
		   //if(!strncmp(buf,"quit",4))
		   if(!strcmp(buf,"quit"))
		   {
				 printf("see you later\n ");
				 break;
		   }
		   printf("recv buf: %s\n",buf);
		   ret = write(fd,buf,strlen(buf));
		   if(ret < 0)
		   {
				 perror("write failed\n");
				 exit(1);
		   }
		   printf("write ok\n");
	}


	return 0;
}




