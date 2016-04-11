#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main( int argc ,char *argv[])
{
	struct stat buf;
	//檢查參數的個數
	if(argc != 2){
		printf("Usage :my_stat <filename> \n");
		exit (0);
	}
	//獲取文件屬性
	if( stat (argv[1] ,&buf ) == -1 ) {
		perror ("stat ");
		exit (1);
	}
	//打印文件的屬性
	printf ("device is : %d\n" ,(int)(buf.st_dev));
 	printf ("inode is : %d \n " ,(int)buf.st_ino);
	printf ("mode is : %o \n" ,buf.st_mode);
	printf ("number of hard links is : %d \n",(int) buf.st_nlink);
	printf ("user ID of owner is : %d\n ",(int)buf.st_uid );
	printf ("group ID of owner is : %d \n",(int)buf.st_gid);
	printf ("device type (if inode device ) is : %d\n",(int ) buf.st_rdev);

	printf ("total size ,in bytes is :%d \n ",(int)buf.st_size );
	printf ("blocksize for filessystem I/O is : %d \n ",(int)buf.st_blksize);
	printf ("number of blocks allocated is :%d \n ",(int) buf.st_blocks);
	
	printf ("time of last access is :%s  ", ctime( &buf.st_atime));
	printf ("time of last modeification is: %s ", ctime( &buf.st_mtime));
	printf ("time of last change is : %s ", ctime( &buf.st_ctime));

	return 0;
}
