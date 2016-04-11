#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int globVar = 5;
int main()
{
	pid_t pid;
	int var = 1,i;
	
	printf("fork is diffirent with vfork \n");

	/*pid =fork();*/
	pid = vfork(); 
	printf("\n pid = %d\n",pid);
	switch (pid){
		case 0:
			i=3;
			while(i-- >0)
			{
				printf("Child process is running \n");
				globVar++;
				var++;
				sleep(1);
			}
			printf("Child's gloVar= %d ,var =% d",globVar ,var);
			break;
		case -1:
			perror("Process creation failed \n");
			exit(0);
		default:
			i=5;
			while (i-- >0 )
			{
				printf("parent process is running\n ");
				globVar++;
				var++;
				sleep(1);
			}
			printf("Parent's globVar =%d ,var = %d\n",globVar ,var);
			exit(0);
		}
}
