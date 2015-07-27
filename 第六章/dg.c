#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#define MAX_PATH_LEN 512
int count =0;

void list (char *dirname)
{
 	 
  	 struct stat s;
	 struct dirent *filename;
	 char path[MAX_PATH_LEN ];
	 DIR *dir; 
	 int c=0;
	
	 if( (dir = opendir(dirname))== NULL)
	 {
	  printf("open dir %s error!\n",dirname);
	  exit(1);
	 } 
	 while((filename = readdir(dir)) != NULL)  {    
	 	if(!strcmp(filename->d_name,".")||!strcmp(filename->d_name,"..")) 
	  	 continue;
			 
	 	sprintf(path,"%s/%s",dirname,filename->d_name);
 
  		lstat(path,&s);
 
  		if(S_ISDIR(s.st_mode)) {
		/*	getcwd(filename->d_name,MAX_PATH_LEN );
_		getcwd(path,_MAX_PATH);*/
		
   			list(path);//递归调用
  		}
 		 else {
		printf("\n\n当前工作目录:\n%s\n\n",path);
   		printf("%d. %s\n",++c,filename->d_name);
		count++;
  		}
 	}
 closedir(dir);
}


int main(int argc, char **argv)
{

char dirPath[MAX_PATH_LEN];

 if(argc != 2)
 {
  printf("one dir required!(for eample: ./a.out /home/桌面)\n");
  exit(1);
 }
 strcpy(dirPath,argv[1]);
 list(dirPath);
 printf("total files:%d\n",count);
 return 0;
}
