#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


/*自定义的错误处理函数*/


void my_err(const char * err_string ,int line)
{
	fprintf(stderr ,"line :%d ",line);
	perror(err_string);
//	exit(1);
}

int my_read (int fd)
{
	int len ;
	int ret;
	int i;
	char read_buf[64];
	/*获取文件长度并保持文件的读写指针在文件开始处*/
	if(lseek(fd ,0,SEEK_END ) == -1)
	{
		my_err ("lseek ",__LINE__);
	}
	if ((len = lseek(fd ,0 ,SEEK_CUR))==-1){
		my_err ("lseek ",__LINE__);
	}
	if ((lseek (fd ,0 ,SEEK_SET)) == -1) {
		my_err ("lseed " ,__LINE__);
	}
	printf(" len :%d \n",__LINE__);

	/*读数据*/
	if((ret = read(fd ,read_buf ,len )) < 0 ){
		my_err ("read  ",__LINE__);
	}
	//da yi shu ju
	for (i=0 ;i<len ;i++){
		printf("%c " ,read_buf[i] );
	}
	printf("\n ");

	return ret ;
}
	

int main ()
{
	int fd ;
	char write_buf[32] ="hello world! ";
	//
//


	if( (fd =open ("example_63.c", O_RDWR |O_CREAT |O_TRUNC  ,S_IRWXU )) == -1) {
		my_err ("write ", __LINE__);
	} else {
		printf("creat file success \n ");
	}



	//
	if (write (fd ,write_buf ,strlen(write_buf )) != strlen(write_buf )){
		my_err("write ",__LINE__);
	}
	my_read(fd);


	//
	printf("/*----------------------------*/\n");
	if (lseek (fd ,10 ,SEEK_END ) == -1 ){
		my_err ("lseek ",__LINE__);
	}
	if (write (fd ,write_buf ,strlen(write_buf )) != strlen (write_buf)) {
		my_err ("write " ,__LINE__);
	}
	my_read(fd );


	close (fd );
	return 0;

}
