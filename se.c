#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<signal.h>
#include<time.h>		 
#include<ctype.h>    //for ispunct()
 


#define QUEUELEN   	100//客户端最大数
#define BUF_SIZE	1024//最大字节数
#define IP		"127.0.0.1"//ｉｐ地址
#define PORT		1234	//端口号
#define FILENAME	"users_info"//存储保存用户信息的文件名
#define CLIENT_ID	1000//客户端的ID

/*		功能定义*/
#define	PUBLIC_CHAT				1//群聊
#define PRIVATE_CHAT			2//私聊
#define CLIENT_LOAD				3//登陆
#define CLIENT_REGISTER			4//注册
#define CLIENT_ONLINE			5//在线用户查询
#define CLIENT_EXIT				6//退出


/*　　用户端的定义　*/
#define NORMAL_USER_LOAD_FAILED			0//登陆失败
#define NORMAL_USER_LOAD_SUCCESS		1//登陆成功
#define NORMAL_USER_LOAD_FAILED_ONLINE  2//用户已经在线
#define NORMAL_USER_LOAD_PASSWD_ERROR	3//用户登陆密码错误


#define REGIST_FALIED	0//注册失败
#define REGIST_EXITED   1//注册用户已经存在

#define NORMAL_USER	0//用户



/*客户信息的结构体*/
typedef struct {
		pthread_t tid ;				//线程
		int sock_fd;				//accept的返回的客户端的新的套接字描述符
		char client_name[32];		//用户名
		char client_passwd[32];		//密码
		int client_id;				//用户ID
		int is_online;				//在线状态：１为在线，０为不在线
}client_info;
client_info clients[QUEUELEN];

/*客户发送的数据结构*/
typedef struct send_info{
		int type ;					//类型，eg:群聊，私聊
		char id[32];					//客户端的id
		char buf[BUF_SIZE];				//发送的内容
		char name[25];					//用户的名字
		char passwd[32];				//用户的密码
}send_info;
/* 存储命令的结构体
定义一个数据结构，其中包括一个数字和一个函数指针（void fun()）类型，下面是具体的填充内容，
给出一个数字，去判断数字是否在定义的fun_flag中，如果在，就调用相对应的函数*/
typedef struct  {
		int fun_flag;
		void (*fun)();
}proto;


/*公用的自定义函数处理系统*/
int my_err(const char *err_string ,int line )	{
		fprintf(stderr ,"line : %d " ,line);
		perror( err_string);
		exit(EXIT_FAILURE);
}

//  =============se.c======================

void fun();//判断数字是否在定义的fun_flag中，如果在，就fun()调用相对应的函数
int check_online (char *t[]);//判断用户是否在线
int init_clents(char *t[]);//初始化用户数组结构
void register_new_client( send_info *send ,int newfd);//注册新用户
void server_check_login( send_info *send ,int newfd);//验证登陆
void client_exit(send_info *send , int newfd);//客户端的退出处理
void get_all_online_clients(send_info *send, int newfd);//查看所有在线用户
int get_sock_fd(char dest[]);//获取用户的sockid(accept的返回值)
void parse_buf(send_info *send,int newfd);//解析客户端传过来的字符串数据结构中的type 成员来匹配调用不同的函数
void format_buf(char *dest,char *content,int newfd);//格式化字符串 (加上时间，用户昵称和ID等)
void private_chat ( send_info *send,int newfd);//私聊
void public_chat (send_info *send ,int newfd);//群聊（不包含自己）
void *pthreads(void *arg);// 线程读函数
int system_init();//初始化连接
void  connect_to_client(int sock_fd ) ;//等待客户端连接
void client_exit(send_info *send , int exit_client_socket_fd);//客户端的退出处理

proto p[]={
  {PUBLIC_CHAT,      public_chat}, 					 //群聊
  {PRIVATE_CHAT,     private_chat},					 //私聊
  {CLIENT_LOAD,     server_check_login},				 //登陆验证
  {CLIENT_REGISTER,  register_new_client},				 //注册
  {CLIENT_ONLINE,    get_all_online_clients},				 //获取所有的在线用户
  {CLIENT_EXIT,      client_exit},					 //客户退出
  {0,0}
};

pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;	//互斥锁初始化

static int id=CLIENT_ID; //初始化用户id

int main(int argc ,char *argv[])
{
	int sock_fd,newfd;
	sock_fd=system_init();//服务器初始化
	connect_to_client(sock_fd);//和客户端进行连接
	return 0;
}
void fun()
{ 
	int i=0;
	for(;i< QUEUELEN;i++)
	 {
	  	if(clients[i].tid!=0)	{//如果客户存在线程id
				fprintf( stderr , "----id : %lu\n" ,clients[i].tid );				 //线程ＩＤ
				fprintf( stderr , "----sock_fd : %d\n" ,clients[i].sock_fd);			 //accept连接情况
				fprintf( stderr , "----name : %s\n" ,clients[i].client_name );		 //客户端的用户名
				fprintf( stderr , "----passwd : %s\n" ,clients[i].client_passwd );    //客户端的密码
				fprintf( stderr , "----id : %d" ,clients[i].client_id );				 //客户端的ＩＤ
				fprintf( stderr , "----online : %d" ,clients[i].is_online);				 //客户端是否在线
		}
  	}
}
int check_online (char *t[])//判断用户是否在线
{
		int i = 0;
		for (i = 0; i< QUEUELEN ;i++)
		{
				if((clients[i].client_id == atoi (t[2] )) && clients[i].is_online == 1)
						return 1; //返回１为在线
		}
		//检查完所有在线用户后，如果用户没有在线则返回０
		return 0;//不在线
}



int init_clents(char *t[])//初始化用户数组结构
{
		pthread_mutex_lock (&mutex);//加互斥锁
		int i = 0 ;
		if(check_online (t ))//在线返回１　，未在线返回０
				return 1;
		for(i = 0 ;i < QUEUELEN ;i++)
		{
				if(clients[i].client_id == 0 && clients[i].is_online != 1)//不在线的情况下
				{
						strcpy( clients[i].client_name ,t[0]);   //将用户名存入
						strcpy( clients[i].client_passwd ,t[1]); //将密码存入
						clients[i].client_id = atoi(t[2]);		//将ｉｄ整形化存入
						clients[i].is_online = 1;				//在线
						break;
				}
		}
		pthread_mutex_unlock (&mutex);//解锁
		//fun;
		return 0;
}

void register_new_client( send_info *send ,int newfd)//注册新用户
{
		pthread_mutex_lock (&mutex);//加锁
		int ret = 0;//保存状态eg:客户退出
		char all_buf[BUF_SIZE];//将用户名，密码，ｉｄ，类型保存
		char read_buf[BUF_SIZE];//读写位置的位移数
		int fd , n ,m ;
		if((fd = open (FILENAME , O_CREAT |O_RDWR |O_APPEND ,0644)) == -1 )//打开文件，没有则创建
				my_err("open ",__LINE__);

		int len = 0;//文件的字节数
		int is_account_exited_flag = 0;
		int j = 0;

		char *t[5];
		while(1)
		{
				lseek(fd ,len ,SEEK_SET);//文件读写指针移动
				if(read(fd , read_buf ,BUF_SIZE) == 0)//读取文件成功
				{
						id++;
						break;
				}
				else {
						j = 0;
						char string[BUF_SIZE] = {0};//设置一个字符串用来存储读取的数据
						strcpy( string ,read_buf );
						t[j++] = strtok(read_buf ,":");//将文件中:后的内容存入ｔ中
						while( t[j++] = strtok(NULL ,":"));//遇见：便将：后的内容存入t中
						if(strcmp (t[0] ,send->name ) == 0)//将客户端的用户名存入t[0];
						{
								is_account_exited_flag = REGIST_EXITED;//注册用户用户已经存在
								break;
						}
						id = atoi(t[2]);	//获得id号
						len += strlen(string)+2;
						bzero( string, sizeof(string));//初始化string
				}
		}
		if(is_account_exited_flag == REGIST_EXITED)//如果注册用户已经存在
		{
				ret = REGIST_EXITED;	//退出
				write( newfd , &ret ,sizeof(int ));//将内容写入
		}
		else //如果用户未注册
		{
				sprintf( all_buf ,"%s :%s :%d :%d",send->name ,send->passwd, id ,NORMAL_USER);//将用户名，密码，ｉｄ，类型写入
				n = write(fd ,all_buf ,strlen(all_buf)+1 );//n为成功写入的字数
				m = write(fd ,"\n",1);//成功写入'\n'
				if( m != 0 && n != 0){
					ret = id;
					write (newfd ,&ret ,sizeof(int ));
				}
				else{
						ret = REGIST_FALIED;//错误
						write(newfd ,&ret ,sizeof(int));
				}
		}
		pthread_mutex_unlock(&mutex);
		close(fd);
}


void server_check_login( send_info *send ,int newfd)//验证登陆
{
		pthread_mutex_lock(&mutex);
		int fd;
		char buf[BUF_SIZE];
		if((fd = open(FILENAME , O_RDONLY ,0644)) == -1)
		{
				my_err("open",__LINE__);
		}
		int len = 0;
		int ret = 0;
		char *t[5];
		int j = 0;
		while (1){
				lseek(fd ,len ,SEEK_SET);
				if( read(fd ,buf ,BUF_SIZE ) == 0)//读文件
						break;
				len += strlen(buf)+2;//算出所占的字节
				j = 0;//将ｂｕｆ中内容储存至ｔ［］中
				t[j++] = strtok(buf ,":");
				while( t[j++] = strtok(buf ,":"));
				if( strcmp(t[0] ,send->name) == 0)
				{
						if( strcmp(t[1] ,send->passwd) == 0)//密码正确
						{
								bzero(&ret ,sizeof(ret ));
								pthread_mutex_unlock(&mutex); //加锁
								if( init_clents(t)) // ret = 2:在线
										ret = NORMAL_USER_LOAD_FAILED_ONLINE ;//已经在线
								else
								{
										ret = NORMAL_USER_LOAD_SUCCESS; //登陆成功
								}break;

						}
						else //密码错误
						{
								bzero(&ret ,sizeof(ret ));
								ret = NORMAL_USER_LOAD_PASSWD_ERROR;//passwd error
								break;
						}
				}else {//没有此帐号
						bzero(&ret ,sizeof(ret));
						ret = NORMAL_USER_LOAD_FAILED;
				}
				memset(buf ,0 ,sizeof(buf));
		}
		write(newfd ,&ret ,sizeof(ret)); //1:success 0:failed 2:online 3:passwd error
		pthread_mutex_unlock (&mutex) ;//解锁
		close( fd);
}


void client_exit(send_info *send , int exit_client_socket_fd)//客户端的退出处理
{
		int i = 0;
		for(i = 0; i < QUEUELEN; i++)
		{
				if(clients[i].sock_fd == exit_client_socket_fd)
						break;
		}
		//把该用户相关信息置为空
		fprintf(stderr ,"client name :%s \t id:%d \t exited \n",clients[i].client_name ,clients[i].client_id);
		memset(&clients[i] ,0 ,sizeof(clients[i]));
		close(exit_client_socket_fd);
}

void get_all_online_clients(send_info *send, int newfd)//查看所有在线用户
{
		int i = 0;
		char buf[BUF_SIZE];
		char str[BUF_SIZE];
		char no_client_online[40] = {"只有你一个人在线 !\n"};
		for(i = 0; i< QUEUELEN ;i++)
		{//查看在线不包括自己
				if ((clients[i].is_online == 1)&&(clients[i].sock_fd != newfd))
				{
						sprintf( buf ,"%s \t",clients[i].client_name);
						strcat(str , buf);
				}
		}
		if(strcmp(str,"") == 0)//没有用户在线
				write(newfd , no_client_online ,strlen(no_client_online)+1);
		else
				write(newfd , str, strlen(str)+1);
}

int get_sock_fd(char dest[]) //获取用户的sockid(accept的返回值)
{
		int i = 0;
		for (i = 0; i < QUEUELEN; i++)
		{
				if(clients[i].client_id == atoi(dest) || strcmp(clients[i].client_name ,dest ) == 0)
				{
						return clients[i].sock_fd;
				}
		}
		return 0;
}



void parse_buf(send_info *send ,int newfd)//解析客户端传过来的字符串数据结构中的type 成员来匹配调用不同的函数。
{  
		  int i;
		  for(i= 0; i < p[i].fun_flag != 0 ;i++)
		  {
			        if(send->type == p[i].fun_flag)//如果客户端传过来的类型和已定义相同，则调用相对应的函数
  			        {
        				    p[i].fun(send,newfd);	
        				    break; 
       				}
     		  }   
}	
       		
       		 


void format_buf(char *dest,char *content,int newfd)//格式化字符串 (加上时间，用户昵称和ID等)
{  
		    int i=0;
    	  	    time_t  timep;
     		    for(;i<QUEUELEN;i++)
		    { 
	    	  	    if(clients[i].sock_fd ==newfd)break;
     		    }
      		    time(&timep);
                    sprintf(dest,"%s\t%s \n %s",clients[i].client_name,ctime(&timep),content);  
} 

void private_chat ( send_info *send,int newfd)//私聊
{
			char dest[BUF_SIZE]={'\0'};
			int tag_id = 0;
			if((tag_id = get_sock_fd(send->id))!=0)
     			{
 	      			format_buf (dest,send->buf,newfd);
 				write(tag_id,dest,strlen(dest)+1);  
 			}
 			else
 				my_err("没有此用户，请核对!\n",__LINE__);

}

void public_chat (send_info *send ,int newfd)//群聊（不包含自己）
{
   		 int j=0;
    		 char str[BUF_SIZE]={'\0'};
		 for( j = 0; j < QUEUELEN ;j++)
		 {
			 if(clients[j].sock_fd == 0 || clients[j].sock_fd == newfd)//套接字不为０，且和本身的套接字不同
				continue;
			 else
			 {
			  	  format_buf (str,send->buf,newfd);
				    write(clients[j].sock_fd,str,strlen(str)+1);
			 }
		 }
}


void *pthreads(void *arg)// 线程读函数
{
	int newfd = *((int *)arg);
	char buf[BUF_SIZE]={0};
	int length=0;
	send_info   send, *p;
  	p = &send;
 	while(1)
	{
			length=read(newfd,&send,sizeof(send));
			if(length <= 0)
			{
				printf("服务器接收数据是空的！\n");
				break;	
			}
			parse_buf(p, newfd);
			memset(&send ,0 ,sizeof(send));
	}
     client_exit(p, newfd);
}



int system_init()//初始化连接
{
    	int sock_fd;
	struct sockaddr_in  server_addr ;
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
        my_err("socket error\n",__LINE__);
      	} 

    	int opt,optlen=sizeof(optlen);
	if( (setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&opt,optlen))==-1)
		my_err(" server setsockopt()  error ",__LINE__);

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT);
	inet_pton(AF_INET,IP,&server_addr.sin_addr);

	if((bind(sock_fd ,(struct sockaddr *)&server_addr, sizeof(struct sockaddr) )) == -1)
	{
       		 my_err("bind error",__LINE__);
    	}

	if( (listen(sock_fd,QUEUELEN))==-1)
	{
        	my_err("listen error",__LINE__);
        }
		fprintf(stderr,"waiting client connection....\n");
	
	return sock_fd ;// return 新的socket 描述符 
 }

void  connect_to_client(int sock_fd ) //等待客户端连接
{
	int newfd;
   	struct sockaddr_in   acc_addr;
	socklen_t  len=sizeof(struct sockaddr_in);
	int i=0;
	while(1)
	{
    
		if((newfd=accept(sock_fd,(struct sockaddr *)&acc_addr,&len))==-1)
			my_err("server accept()   error",__LINE__);  
		printf("   %s请求连接。\n",inet_ntoa(acc_addr.sin_addr));	
		           
		for(i=0;i < QUEUELEN;i++)
		{
			if(clients[i].sock_fd==0)
			{
				clients[i].sock_fd=newfd;
			  	printf("clients[i]sock_fd=%d\n",clients[i].sock_fd);
				break;
			}
		} 
			
		if((pthread_create(&clients[i].tid,NULL,( void* )pthreads,&clients[i].sock_fd))==-1)
			my_err("server pthread_create() 2  error",__LINE__); 
	}
	close(sock_fd);
	close(newfd);
}



