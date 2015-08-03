#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//对互斥锁进行初始化

int lock_var;		//锁
time_t end_time;	//时间
void pthread1(void *arg);	//线程１
void pthread2(void *arg);	//线程２

int main(int argc, char *argv[])
{
    pthread_t id1,id2;
    pthread_t mon_th_id;
    int ret;
    end_time = time(NULL)+10;
   /*互斥锁初始化*/
     pthread_mutex_init(&mutex,NULL);
   /*创建两个线程*/
     ret=pthread_create(&id1,NULL,(void *)pthread1, NULL);
     if(ret!=0)
     perror("pthread cread1");//判断线程是否创建成功，函数返回０，则线程创建成功，非０则失败
     ret=pthread_create(&id2,NULL,(void *)pthread2, NULL);
     if(ret!=0)
     perror("pthread cread2");//判断线程是否创建成功，函数返回０，则线程创建成功，非０则失败
     pthread_join(id1,NULL);	//回收资源，释放内存                                
     pthread_join(id2,NULL);      //回收资源，释放内存                             
     exit(0);                                               
}
                                                        
void pthread1(void *arg)                               
{                                                      
    int i;                                                 
    while(time(NULL) < end_time)
    {                          
    /*互斥锁上锁*/                                         
     if(pthread_mutex_lock(&mutex)!=0)
     {                     
      perror("pthread_mutex_lock");                          
     }                                                      
     else                                                   
     printf("pthread1:pthread1 lock the variable\n");       
     for(i=0;i<2;i++){                                      
     sleep(1);                                              
     lock_var++;                                            
     }                                                      
   /*互斥锁接锁*/                                         
     if(pthread_mutex_unlock(&mutex)!=0){                   
     perror("pthread_mutex_unlock");                        
     }                                                      
     else                                                   
     printf("pthread1:pthread1 unlock the variable\n");     
     sleep(1);                                              
    }                                                      
}      
   

                                             
void pthread2(void *arg)                               
{                                                      
    int nolock=0;                                          
    int ret;                                               
    while(time(NULL) < end_time)
    {                          
     /*测试互斥锁*/                                         
     ret=pthread_mutex_trylock(&mutex);  //测试锁                   
     if(ret==EBUSY)	//资源正在使用不能共享 ，说明mutex已经被加锁，无法再获得一个锁                                       
     printf("pthread2:the variable is locked by pthread1\n");
     else
     {  
	if(ret != 0)   	/*pthread_mutex_trylock() 在成功获得了一个mutex的锁后返回０*/
      {                                                                                      
        perror("pthread_mutex_trylock");                 
        exit(1);                                         
       }                                                
       else                                             
       printf("pthread2:pthread2 got lock.The variable is%d\n",lock_var);                                 
        /*互斥锁接锁*/                                   
       if(pthread_mutex_unlock(&mutex)!=0) //解锁失败
       {             
        perror("pthread_mutex_unlock");                  
       }                                                
       else                                             
       printf("pthread2:pthread2 unlock the variable\n");
      }                                                
     sleep(3);                                        
     }                                                
} 
