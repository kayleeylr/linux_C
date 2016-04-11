
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
#include<ctype.h>    
 

#define QUEUELEN     100  		//最大客户数
#define BUF_SIZE     256  
#define IP        "127.0.0.1"
#define PORT         1234             // 端口号
#define FILENAME     "userinfo"       // 储存用户信息的文件名
#define CLIENT_ID    1000    	      // 客户端的id 
/*功能*/
#define PUBLIC_CHAT        			1  //群聊
#define PRIVATE_CHAT       			2  //私聊
#define CLIENT_LOAD       			3  //登陆验证
#define CLIENT_REGISTER    			4  //注册
#define CLIENT_ONLINE      			5  //所用的在线用户
#define CLIENT_EXIT        			6  //退出

/*登陆*/
#define NORMAL_USER_LOAD_FAILED  			 0 //普通用户登录失败
#define NORMAL_USER_LOAD_SUCCESS 			 1 //普通用户登录成功
#define NORMAL_USER_LOAD_FAILED_ONLINE  2 //普通用户已经在线

#define NORMAL_USER_LOAD_PASSWD_ERROR   4 //普通用户登录密码错误

#define REGIST_FALIED   0   //注册失败
#define REGIST_EXITED   1   //注册的用户已经存在
#define NORMAL_USER     0   //普通用户

#define CHAT_DEBUG
#ifdef  CHAT_DEBUG
#define DEBUG(message...) fprintf(stderr, message)
#else
#define DEBUG(message...)
#endif


/*客户端*/
typedef struct{
	pthread_t tid;					//线程的描述符,unsigned long int ,printf用%lu
	int  sock_fd;  					//accept的返回的客户端的新的套接字描述符
	char client_name[25]; 	// 账号
	char client_passwd[25]; //密码
	int client_id;					//用户ID
  	int is_online;					// 在线状态 1 在线 0 不在线
  	int admin;              //用户权限，0为普通用户
}client_info;
client_info clients[QUEUELEN];

/*客户发送的数据结构*/
 typedef struct send_info{
	int  type;				//类型
	char id[25];  				//对方id
	char buf[BUF_SIZE]; 			//内容
	char name[25];				//用户名（昵称）
	char passwd[25];			//密码
	}send_info;


int my_err( const char *err_string ,int line)
{
		fprintf(stderr ,"line :%d",line);
		perror(err_string);
		exit(1);
}

/*客户端函数*/
void pthread_load(void *arg);//接受数据
void inits();//建立套接字，连接，初始化客户端
void show_menu();//显示菜单
void reg_log();//登陆或者注册
void is_check( char *string );//判断输入的用户名是否存在非法字符
void regiser_client();//注册新用户
void parse_input_buf( char *p ,send_info *send);//解析命令
void write_data();//发送数据
void login();//登陆界面
/*se.c*/


int check_online (char *t[]);//判断用户是否在线
int init_clents(char *t[]);//初始化用户数组结构
void register_new_client( send_info *send ,int newfd);//注册新用户
void server_check_login( send_info *send ,int newfd);//验证登陆
void client_exit(send_info *send , int newfd);//客户端的退出处理
void get_all_online_clients(send_info *send, int newfd);//查看所有在线用户
int get_sock_fd(char dest[]);//获取用户的sockid(accept的返回值)
void parse_buf(send_info *send,int newfd);//解析客户端传过来的字符串数据结构中的type 成员来匹配调用不同的函数
void format_buf_public(char *dest,char *content,int newfd);//格式化字符串 (加上时间，用户昵称和ID等)专供群聊使用
void format_buf_private(char *dest,char *content,int newfd ,send_info *send);//格式化字符串 (加上时间，用户昵称和ID等)专供私聊使用
void private_chat ( send_info *send,int newfd);//私聊
void public_chat (send_info *send ,int newfd);//群聊（不包含自己）
void *pthreads(void *arg);// 线程读函数
int system_init();//初始化连接
void  connect_to_client(int sock_fd ) ;//等待客户端连接
void client_exit(send_info *send , int exit_client_socket_fd);//客户端的退出处理



pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;	//互斥锁初始化

static int id = CLIENT_ID; //初始化用户id

int main(int argc ,char *argv[])
{
	int sock_fd,newfd;
	sock_fd=system_init();//服务器初始化
	connect_to_client(sock_fd);//和客户端进行连接
	return 0;
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
						clients[i].admin = NORMAL_USER;
						break;
				}
		}
		pthread_mutex_unlock (&mutex);//解锁
		return 0;
}

void register_new_client( send_info *send ,int newfd)//验证注册新用户
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
				write( newfd , &ret ,sizeof(int ));//将内容传给客户端
		}
		else //如果用户未注册
		{
				sprintf( all_buf ,"%s:%s:%d:%d",send->name ,send->passwd, id ,NORMAL_USER);//将用户名，密码，ｉｄ，类型写入
				n = write(fd ,all_buf ,strlen(all_buf)+1 );//n为成功写入的字数
				m = write(fd ,"\n",1);//成功写入'\n'
				if( m != 0 && n != 0){
					ret = id;
					write (newfd ,&ret ,sizeof(int ));//将内容传给客户端
				}
				else{
						ret = REGIST_FALIED;//错误
						write(newfd ,&ret ,sizeof(int));//将内容传给客户端
				}
		}
		pthread_mutex_unlock(&mutex);
		close(fd);
}


void server_check_login( send_info *send ,int newfd)//验证登陆
{
		pthread_mutex_lock(&mutex);
		int fd;
		char buf[BUF_SIZE] = {0};
		if((fd = open(FILENAME , O_RDONLY ,0644)) == -1)
		{
				my_err("open",__LINE__);
		}
		int len = 0;
		int ret = 0;
		char *t[5];//存储客户端信息
		int j = 0;
		while (1){
				lseek(fd ,len ,SEEK_SET);
				if( read(fd ,buf ,BUF_SIZE ) == 0)//读文件
						break;
				len += strlen(buf)+2;//算出所占的字节
				j = 0;//将ｂｕｆ中内容储存至ｔ［］中
				t[j++] = strtok(buf ,":");
				while( t[j++] = strtok(NULL ,":"));
				if( strcmp(t[0] ,send->name) == 0)
				{
						if( strcmp(t[1] ,send->passwd) == 0)//密码正确
						{
								bzero(&ret ,sizeof(ret ));
								pthread_mutex_unlock(&mutex); //减锁
								if( init_clents(t)) // ret = １:在线，ret = 0不在线
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
		write(newfd ,&ret ,sizeof(ret)); ////将内容传给客户端1:登陆成功 0:登陆失败 2:在线 3:密码错误
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
		fprintf(stderr ,"用户名 :%s \t id:%d \t 退出 \n",clients[i].client_name ,clients[i].client_id);
		memset(&clients[i] ,0 ,sizeof(clients[i]));
		close(exit_client_socket_fd);
}

void get_all_online_clients(send_info *send, int newfd)//查看所有在线用户
{
		int i = 0;
		char buf[BUF_SIZE] = {'\0'};//存储某个在线用户的名字
		char str[BUF_SIZE] = {'\0'};//储存所有在线用户的名字
		char no_client_online[40] = {"只有你一个人在线 !(￣ˇ￣)\n"};
		for(i = 0; i< QUEUELEN ;i++)//查看在线不包括自己
		{
				if ((clients[i].is_online == 1)&&(clients[i].sock_fd != newfd))
				{
						sprintf( buf ,"%s \t",clients[i].client_name);
						strcat(str , buf);
				}
		}
		if(strcmp(str,"") == 0)//没有用户在线
				write(newfd , no_client_online ,strlen(no_client_online) +1);//把数据传给客户端
		else
				write(newfd , str, strlen(str)+1);//把数据传给客户端
}

int get_sock_fd(char dest[]) //获取用户的sockid(accept的返回值)
{
		int i = 0;
		for (i = 0; i < QUEUELEN; i++)
		{
				if(clients[i].client_id == atoi(dest) || strcmp(clients[i].client_name ,dest ) == 0)
				{
						return clients[i].sock_fd;//获得用户的accept返回值				
				}
		}
		return 0;
}



void parse_buf(send_info *send ,int newfd)//解析客户端传过来的字符串数据结构中的type 成员来匹配调用不同的函数。
{  

	                        if(send->type == PUBLIC_CHAT)//群聊
  			        {
        				    public_chat(send,newfd);	
       				}
 				if(send->type == PRIVATE_CHAT)//私聊
  			        {
        				   private_chat(send,newfd);	
       				}
				if(send->type == CLIENT_LOAD)//登陆验证
  			        {
        				   server_check_login(send,newfd);	
       				}
				if(send->type == CLIENT_REGISTER)//注册
  			        {
        				   register_new_client(send,newfd);	
       				}
				if(send->type == CLIENT_ONLINE)//获取所有在线用户
  			        {
        				   get_all_online_clients(send,newfd);	
       				}
				if(send->type == CLIENT_EXIT)//客户退出
  			        {
        				   client_exit(send,newfd);	
       				}
				   
}	
       		
   

void format_buf_public(char *dest,char *content,int newfd)//格式化字符串 (加上时间，用户昵称和ID等)
{  
		    int i=0;
    	  	    time_t  timep;
     		    for(;i<QUEUELEN;i++)
		    { 
	    	  	    if(clients[i].sock_fd == newfd)
					break;
     		    }
      		    time(&timep);
                    sprintf(dest,"%s\t\t%s说：%s\n",ctime(&timep),clients[i].client_name,content);//将聊天用户名称，时间，内容发送 存入dest 
} 
void format_buf_private(char *dest,char *content,int newfd ,send_info *send)//格式化字符串 (加上时间，用户昵称和ID等)
{  
		
		    int i=0;
    	  	    time_t  timep;
     		    for(;i<QUEUELEN;i++)
		    { 
	    	  	    if(clients[i].sock_fd == newfd)
					break;
     		    }
      		    time(&timep);
                    sprintf(dest,"%s\t\t%s对%s说：%s\n",ctime(&timep),clients[i].client_name,send->id,content);//将聊天用户名称，时间，内容发送 存入dest 
} 

void private_chat ( send_info *send,int newfd)//私聊
{
			char dest[BUF_SIZE]={'\0'};
			int tag_id = 0;//储存客户端appept 套接字的返回值
			tag_id = get_sock_fd(send->id);
			if( (tag_id != 0 ) && (tag_id != newfd))
     			{
 	      			format_buf_private (dest ,send->buf ,newfd,send);
				printf("\n私聊 %s" ,dest);//在服务器显示私聊内容
 				write(tag_id ,dest ,strlen(dest)+1); //通过tag_id传给客户端 
 			}
			else if( tag_id == newfd)
			{	
				sprintf(dest,"私聊对象不能是自己 (￣▽￣＃) = ﹏﹏\n\n" );
 				write(tag_id ,dest ,strlen(dest)+1); //通过tag_id传给客户端
				DEBUG("私聊对象不能是自己\n\n"); 
			}
 			else if(tag_id == 0 ){
				sprintf(dest,"没有此用户，请核对! Σ( ° △ °|||)︴\n\n" );
 				write(newfd ,dest ,strlen(dest)+1); //通过tag_id传给客户端
 				DEBUG("没有此用户\n\n");
			}

}

void public_chat (send_info *send ,int newfd)//群聊（不包含自己）
{
   		 int j=0;
    		 char str[BUF_SIZE]={'\0'};
		 printf("\n群聊:");
		 for( j = 0; j < QUEUELEN ;j++)
		 {
			 if(clients[j].sock_fd == 0 )//套接字不为０
				continue;
			 else
			 {
			  	    format_buf_public (str,send->buf,newfd);
				    write(clients[j].sock_fd ,str ,strlen(str)+1);//传给所有客户端
			 }
		 }
		printf(" %s" ,str);//在服务器显示私聊内容
}
void *pthreads(void *arg)// 线程读函数
{
	int newfd = *((int *)arg);
	char buf[BUF_SIZE] = {0};
	int length = 0;
	send_info   send, *p;
  	p = &send;
 	while(1)
	{
			length = read(newfd ,&send ,sizeof(send));//读取字节的长度
			if(length <= 0)//读取服务器失败或者服务器为空
			{
				printf("一个客户端退出！\n");
				break;	
			}
			parse_buf(p, newfd);//解析客户端传过来的字符串数据结构中的type 成员来调用函数。
			memset(&send ,0 ,sizeof(send));
	}
        client_exit(p, newfd);
}



int system_init()//初始化连接
{
    	int sock_fd;
	struct sockaddr_in  serv_addr ;
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0)) == -1)//创建一个ＴＣＰ套接字
	{
        my_err("socket error\n",__LINE__);
      	} 

    	int opt,optlen=sizeof(optlen);
	if( (setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&opt,optlen))==-1)//设置该套接字可以使之重新绑定端口
		my_err(" server setsockopt()  error ",__LINE__);
	//初始化服务器地址结构
	bzero(&serv_addr ,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	inet_pton(AF_INET ,IP ,&serv_addr.sin_addr);
	//将套接字绑定至端口
	if((bind(sock_fd ,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr) )) == -1)
	{
       		 my_err("bind error",__LINE__);
    	}
	//将套接字转化为监听套接字
	if( (listen(sock_fd,QUEUELEN)) == -1)
	{
        	my_err("listen error",__LINE__);
        }
		fprintf(stderr,"等待客户端连接....\n");
	
	return sock_fd ;// 返回新的socket 描述符 
 }

void  connect_to_client(int sock_fd ) //等待客户端连接
{
	int newfd;
   	struct sockaddr_in   wait_addr;//等待连接的客户端
	socklen_t  len = sizeof(struct sockaddr_in);
	int  i = 0;
	while(1)
	{
    
		if((newfd = accept(sock_fd ,(struct sockaddr *)&wait_addr ,&len)) == -1) {//newfd储存接收客户端连接请求所产生的套接字
			my_err("server accept()   error",__LINE__);  
		}
		printf("   %s 请求连接。\n",inet_ntoa(wait_addr.sin_addr));//网络字节顺序的二进制ＩＰ地址	
		           
		for(i=0;i < QUEUELEN;i++)
		{
			if(clients[i].sock_fd == 0)
			{
				clients[i].sock_fd = newfd;//将接收客户端的套接字放入专门存储客户端套接字的结构体之中
			  	printf("目前连接客户端对应的套接字 ： %d \n", clients[i].sock_fd);//打印目前连接客户端的套接字
				break;
			}
		} 
			
		if((pthread_create(&clients[i].tid ,NULL ,( void* )pthreads ,&clients[i].sock_fd)) == -1)//创建线程,不同的线程id对应不同的
			my_err("pthread_create() ",__LINE__); 
	}
	close(sock_fd);//关闭套接字
	close(newfd);//关闭连接的套接字
}



