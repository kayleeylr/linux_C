#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define normal		0 //一般命令
#define out_redirect	1 //输出重定向
#define in_redirect	2 //输入重定向
#define have_pipe	3 //命令中有管道



void print_prompt()
{
	printf("myshell~$ ");				//打印提示符
}


//获取用户输入
void get_input(char *buf)
{
	int len = 0;
	int ch;

	ch = getchar();
	while(len <256 && ch!= '\n'){
		buf[len++] = ch;
		ch = getchar();
	}
	if(len == 256) {
		printf("command is too long \n");
		exit(-1); //输入的命令过长则退出程序
	}
	buf[len] = '\n';
	len ++;
	buf[len] = '\0';
}
	
//解析buf中的命令，将结果存在arglist中，命令以回车符号\n结束
//如输入命令为"ls -l /tmp"，则arglist[1],arglsit[2]分别为ls -l 和/tmp

void explain_input(char *buf ,int *argcount, char arglist[100][256])
{
	char *p = buf ;
	char *q = buf ;
	int number = 0;
	
	while(1) {
		if(p[0] == '\n')
		break;
		if(p[0] == ' ')
		p++;
		else{
			q = p;
			number = 0;
			while((q[0] != ' ') && (q[0] != '\n' )) {
			number++;
			q++;
			}
			strncpy( arglist[*argcount ], p, number+1 );
			arglist[*argcount][number] = '\0';
			*argcount = *argcount +1;
			p=q;
			}
	}
}


void printf_prompt()
{
	printf("myshell$$ ");

}

/*判断命令是否存在于以下目录*/
int find_command(char *command)
{
	DIR 		*dp;	
	struct dirent*	dirp;
	char* path[] = {"./","/bin","/usr/bin",NULL};
	int i=0;

	if(strncmp(command, "./",2) == 0)			//如当前命令可执行,取出命令
		command = command + 2;						

	while(path[i] != NULL) {						
		if( (dp = opendir(path[i])) == NULL ) { 	//打开目录
			printf("can not open the mulu\n");
			return 0;
		}
		else{	
			while((dirp = readdir(dp)) != NULL) {	//读取目录流
				if(strcmp(dirp->d_name, command) == 0) {	
					closedir(dp);	
					return 1;		//若path 环境中找到该命令,关闭目录流,返回1
				}
			}
			closedir(dp);
		}
		i++;
	}
	return 0;						//未找到返回0
}	
		


int do_cmd(int *argcount, char arglist[100][256])
{
	
	
	int i, len, j, p=0;
	int how=0, background=0 ;
	int	pid, fd;
	char *argnext[256];
	int status;
	char*	arg[256];

			
	int	flag=0;
	for(i=0; i<*argcount; i++) {
	/*判断后台运行符*/
		if(strcmp(arglist[i], "&")== 0)  {
			if(i ==(*argcount - 1)) {		
				background = 1;			//如果命令中存在&,且在命令尾,将background置为1
				arglist[i][0] = '\0';		//将此行字符串清空,即清除换行符,只保留可执行命令
				}
			else {
				printf("the command is wrong!\n");//存在&却不在命令尾时,提示命令错误退出.
				return 0;
			}
		}
	/*判断输出重定向符>, 只能出现一次,且不可位于命令首尾处*/
		if(strcmp(arglist[i], ">") == 0) {		
			flag++;					
			how = out_redirect;				//存在 > 时,flag加1,out_redirect置为1
			if(i == (*argcount -1))																flag++;					   //若 > 在最后一个,flag再加1
			if(i == 0)					
			     flag++;					//若 > 在命令头, flag 再加1

		}

	/*判断输入重定向符<, 只能出现一次,且不可位于命令首尾处*/	
		if(strcmp(arglist[i],"<") == 0) {
			flag++;
			how = in_redirect;				//存在 < 时,flag加1 ,in_redirect置为1
			if(i == 0)
			  	flag++;					//若 < 在第一个,flag加1
			if(i == (*argcount - 1)) 			
				flag++;					//若 < 在最后一个,flag加1

		}
	
	/*判断管道符 | ,只能出现一次,且不可位于命令首尾处*/
		if(strcmp(arglist[i], "|") == 0) {		
			flag++;					
			how = have_pipe;				//存在 | 时,flag加1,how置为have_pipe
			if(i == (*argcount -1))																flag++;					   //若 | 在最后一个,flag加1
			if(i == 0)
			     flag++;					//若 | 在命令第一个,flag加1
		}
	}
			
	if(flag > 1) {							//若flag大于1,即存在以上命令符但不符合要求时,则命令出错
		printf("the command is wrong!\n");
		return 0;
	}	
	
	char	*file;
	if(how == out_redirect) {					//命令中只有输出重定向符 >
			i = 0;						
			while(strcmp(arglist[i], ">") != 0) { 
				
				arg[i] = arglist[i];			//将 > 之前的命令存入arg 中,arg 存可执行命令
				i++;					
			}
			file = arglist[i+1];				//>后面为文件名,存入file
			arg[i] = NULL;					//可执行命令尾置为空
		}

	if(how == in_redirect) {					//命令中只有输出重定向符 >
			i = 0;
			while(strcmp(arglist[i], "<") != 0) { 
				
				arg[i] = arglist[i];			//将 < 之前的命令存入arg 中,arg 存入可执行命令
				i++;					
			}
			strcpy(file,arglist[i+1]);			//<后面为文件名,存入file
			arg[i] = NULL;
		}



	if(how == have_pipe) {						//命令中只有管道符 |
			i = 0;
			while(strcmp(arglist[i], "|") != 0) {
				arg[i] = arglist[i];			//将管道符之前的命令存入arg,
				i++;
			}
			arg[i] = NULL;					//命令尾置为NULL

			for(j=i+1; j<*argcount; j++) {
				argnext[p] = arglist[j];		//将管道符后的命令存入argnext
				p++;
			}
			argnext[p] = NULL;				//命令尾置为NULL
	}
		
	if(how == 0) {							//命令中不存在任何符
			for(i=0; arglist[i][0] != '\0'; i++)
				arg[i] = arglist[i];			//直接将之前解析后的命令存入可执行命令中	
			arg[i] = NULL;
	}

	if ((pid = fork()) != -1){					//fork一个子进程
	if (pid == 0){							//若为子进程
	switch(how) {							//判断how值,how值表示的为上述的各种符
	
	case 0:								//无 <, >, &, |的纯命令
		if((find_command(arg[0]))==1) {				//调用find_command()函数查看命令是否可执行
				execvp(arg[0],arg );			//执行命令
		}
		else{
			printf("can not found the command\n");		//命令未发现
		}
		break;


	case 1:								//此时存在输出重定向符 >
		if((find_command(arg[0])) == 1) {			//调用find_command()函数查看命令是否可执行
			fd = open(file, O_RDWR|O_CREAT|O_TRUNC, 0644);	//打开文件,文件描述符存入fd 
			dup2(fd, 1);					//将标准输出重定向到文件
			execvp(arg[0], arg);				//调用execvp()函数在子进程执行另一个程序
		}
		else {
			printf("can not found the command!\n");		//目录中找不到该命令
		}
		break;
 
		
			
	case 2:								//此时存在输入重定向符 <
		if((find_command(arg[0])) == 1) {			//调用find_command()函数查看命令是否可执行
				fd = open(file, O_RDONLY);		//只读方式打开文件,
				dup2(fd, 0);				//将标准输入的从键盘移到文件,即从文件获取信息执行命令
				execvp(arg[0], arg);			//调用execvp()函数在子进程执行另一个程序
		}
		else {
			printf("can not found the command!\n");		//目录中找不到该文件
		}
		break;

	case 3:								//此时存在管道符 |
		{
		int status2;
		if((find_command(arg[0])) == 1) {			//调用find_command()函数查看命令是否可执行
			pid = fork();					//fork() 一个子进程,因为 | 时有两个命令
			if(pid == -1) {			
				printf("fork error");
				return 0;				//fork失败,退出
			}
			if(pid == 0) {
			fd = open("/tmp/yunting", O_RDWR|O_CREAT|O_TRUNC, 0644);//fork() 成功,打开文件
			dup2(fd, 1);					//输出重定向到文件
			execvp(arg[0],arg);				//调用execvp()函数在子进程执行管道符前的命令
			}
		} else {
			printf("%s can not found\n",arg[0]);
			exit(0);
		}		
		if(waitpid(pid, &status2, 0) == -1) 			//父进程等待子进程结束,pid为要等待的子进程ID,status存放子进程退出码
			printf("wait for child process error管道\n");
			
		if((find_command(argnext[0])) == 1) {
			fd = open("/tmp/yunting", O_RDONLY);		//管道符之后的命令可执时打开文件
			dup2(fd, 0);					//将标准输入重定向到文件
			execvp(argnext[0], argnext);			//在子进程执行该命令

		} else {
			printf("not found the command!\n");		//命令未找到
			return 0;
		}
		}
		break;
			
		default: break;
	}
	}
	else{
		if(background == 1) {
			printf("process id  is %d\n",pid);		//有后台运行&,输出子进程ID
			return 1;
		}
		else{
			wait(NULL);	return 0;			//若不是,等待子进程结束
		}
		
	}
	}
	else
	{
		perror("ForkError");	return 0;			//与switch外的fork()对应,fork()失败,返回
	}
	
}


int main(int argc ,char **argv) 
{
	int i;
	int argcount = 0;
	char arglist[100][256];
	char **arg =NULL;
	char *buf = NULL;
	
	buf =(char *)malloc (256);
	if(buf == NULL){
		perror("malloc failed ");
		exit(-1 );
	}
	while(1){
	//将buf所指向的空间清零
	memset(buf ,0 , 256);
	printf_prompt();
	get_input( buf);
	//若输入的命令为exit或logout则退出本程序
	if(strcmp(buf , "exit\n") == 0 || strcmp(buf , "logout\n") == 0)
		break ;
	for ( i=0 ; i <100 ;i++ )
	{
		arglist[i][0] = '\0';
	}

	argcount = 0;
	explain_input(buf,&argcount,arglist);
	do_cmd(&argcount ,arglist );
	}

	if(buf != NULL ) {
		free(buf );
		buf = NULL;
	}
	
	exit(0);
}

