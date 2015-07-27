#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	pid_t pid;
	char * msg;
	int k;

	printf("\n\npid = %d",pid);
	printf("process Creation Study\n");
	pid = fork();
	printf("\n\nfpid = %d",pid);
	switch(pid ) {
		case 0:
			msg="Child process is running ";
			k=3;
			break;
		case -1:
			perror("Process Creation failed \n");
			break;
		default:
			msg="Parent process is running ";
			k = 5;
			break;
		}
	while(k >0)
	{
		puts(msg);
		sleep(1);
		k--;
	}
	exit (0);

}
