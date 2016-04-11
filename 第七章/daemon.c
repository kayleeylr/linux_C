#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>


int init_daemon()
{
	int pid ;
	int i;

	/*忽略终端Ｉ／Ｏ信号，ＳＴＯＰ信号*/
	signal(SIGTTOU ,SIG_IGN);
	signal(SIGTTIN ,SIG_IGN);
	signal(SIGTSTP ,SIG_IGN);
	signal(SIGHUP ,SIG_IGN);

	pid = fork();
	if(pid >0 ){
		exit (0);//结束父进程，是子进程成为后台程序
	}
	else if(pid < 0){
		return -1;
	}
	//建立一个新进程组，在这个新进程里，子程序成为这个进程组的首进程，以使该进程脱离所有终端
	setsid();
	
	//再次新建一个子进程，退出父进程，保证该进程不是进程组长，同时让该进程无法在打开一个新的终端
	pid =fork();
	if( pid > 0) {
		exit(0);
	}
	else if ( pid <0 ) {
		return -1;
	}
	//关闭所有从父进程继承的不再需要的文件描述符
	
	for (i =0 ; i<NOFILE ;close(i++ ));
	//改变工作目录，使进程不与任何文件系统联系
	chdir("/");
	//将文件屏蔽字设置为０
	umask(0);

	//忽略SIGHLD信号
	signal(SIGCHLD,SIG_IGN);

	return 0;
}
	

int main()
{
	time_t now;
	init_daemon();
	syslog(LOG_USER | LOG_INFO ,"测试守护进程！\n");
	while (1){
		sleep(8);
		time(&now );
		syslog(LOG_USER | LOG_INFO ,"系统时间:\t%s\t\t\n",ctime(&now));
	}
} 

