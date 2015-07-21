


#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

//自定义错误处理函数
void my_err(const char* err_string ,int line)
{
	fprintf(stderr ,"line :%d ",line );
	perror (err_string);
//	exit(1);
}


int main()
{
	int  ret;
	int  access_mode;
	int  fd;
	if( (fd = open ("example_64", O_CREAT |O_TRUNC |O_RDWR ,S_IRWXU)) == -1)
	{
	my_err( "open ",__LINE__);
	}
	//设置文件打开方式
	if  ((ret = fcntl (fd ,F_SETFL ,O_APPEND)) <0 ){
		my_err("fcntl ",__LINE__);

	}
printf("F_SETFL =%d \n",F_SETFL );
printf("fd=%d \n",fd);
	printf("F_GETFL=%d \n",F_GETFL);
	printf("O_RDONLY=%d \n",O_RDONLY);
	printf("O_WRONLY=%d \n", O_WRONLY);
	printf("O_RDWR=%d \n",  O_RDWR);
	printf("O_APPEND=%d \n", O_APPEND);
	printf("O_ACCMODE=%d \n",O_ACCMODE);
	printf("ret=%d \n",ret);
	//获取文件打方式
	if ((ret = fcntl (fd ,F_GETFL ,0)) <0){
		my_err("fcnrl " ,__LINE__);
	}
printf("ret=%d \n",ret);
	access_mode = ret & O_ACCMODE;
printf("access_mode= %d \n",access_mode);

printf("ret=%d \n",ret);
	if (access_mode == O_RDONLY){

		printf("example_64 access mode : read only ");
	}
	else if (access_mode == O_WRONLY ){
		printf("example_64 access mode : write only ");
	}	
	else if (access_mode == O_RDWR){
		printf("example_64 access mode : write + read ");
	}
	if (ret & O_APPEND)
	{
		printf(" , append");	
	}
	if (ret & O_NONBLOCK){
		printf(", nonblock");
	}
	if (ret & O_ASYNC){
		printf(" , async");
	}
printf("ret=%d \n",ret);
	printf("\n");
	return 0;
}
