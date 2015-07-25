#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	pid_t pid;

	printf("process Creation Study");
	printf("\npid = %d\n\n",(int)pid);
	pid = fork();
	printf("fpid = %d\n\n",(int)pid);
	switch( pid){
	case 0:
		printf("child process is running ,CurPid is %d , ParentPid is %d\n", pid ,getppid());
		break;
	case -1:
		perror("process Creation failed\n ");
		break;
	default:
		printf("parent process is running ,childpid is %d ,parentpid :%d \n", pid ,getpid() );
		break;
	}
	exit (0);	
}
