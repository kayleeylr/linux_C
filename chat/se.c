
/*　　　头文件　　*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>		 
#include <ctype.h>
#include <termios.h>    
#include <unistd.h> 



#define QUEUELEN     100			  //最大客户数
#define BUF_SIZE     256			  //最大字节数
#define IP        "127.0.0.1"		  //ip地址
#define PORT         1234             // 端口号
#define CLIENT_ID    1000    	      // 客户端的id 
#define FILENAME     "userinfo"       // 储存用户信息的文件名
#define C_HISTORY "save_history.odt"  //储存聊天记录的文件名
#define SERVER_LOG "server_log.odt"   //储存服务器日志的文件名


/*功能*/
#define PUBLIC_CHAT        			1  //群聊
#define PRIVATE_CHAT       			2  //私聊
#define CLIENT_LOAD       			3  //登陆验证
#define CLIENT_REGISTER    			4  //注册
#define CLIENT_ONLINE      			5  //所用的在线用户
#define CHAT_HISTORY				6  //聊天记录
#define CLIENT_EXIT        			7  //退出
#define FIND_FRIEND					8  //好友列表获取
#define ADD_FRIEND					9  //添加好友
#define ACCEPT_FRIEND				10 //同意添加好友
#define DISACCEPT_FRIEND			11 //拒绝添加好友


/*好友*/
#define FRIEND_AGREE	 			 1   //对方同意加好友
#define FRIEND_DISAGREE  			 2	 //对方不同意加好友
#define FRIEND_FALIED				 3   //加好友失败
#define FRIEND_LOAD_FALIED_EXIST 	 4   //对方已经是你的好友
#define FRIEND_LOAD_FALIED_UNONLINE  5   //对方不存在或者不在线
 
 
 
/*登陆*/
#define NORMAL_USER_LOAD_FAILED  			 0 //用户登录失败
#define NORMAL_USER_LOAD_SUCCESS 			 1 //用户登录成功
#define NORMAL_USER_LOAD_FAILED_ONLINE  	 2 //用户已经在线
#define NORMAL_USER_LOAD_PASSWD_ERROR   	 3 //用户登录密码错误

#define REGIST_FALIED   0   //注册失败
#define REGIST_EXITED   1   //注册的用户已经存在


#define CHAT_DEBUG
#ifdef  CHAT_DEBUG
#define DEBUG(message...) fprintf(stderr, message)
#else
#define DEBUG(message...)
#endif


/*客户端*/
typedef struct{
	pthread_t tid;					//线程的描述符
	int  sock_fd;  					//accept的返回的客户端的新的套接字描述符
	char client_name[25]; 			// 账号
	char client_passwd[25]; 		//密码
	int	 client_id;					//用户ID
  	int  is_online;					// 在线状态 1 在线 0 不在线
  	char friend[100];				//好友存储
}client_info;
client_info clients[ QUEUELEN];


/*客户发送的数据结构*/
 typedef struct send_info{
	int  type;					//类型
	char id[25];  				//对方用户名
	char buf[BUF_SIZE]; 		//内容
	char name[25];				//用户名（昵称）
	char passwd[25];			//密码
	}send_info;

/*自定义错误函数处理*/
int my_err( const char *err_string ,int line)
{
		fprintf(stderr ,"line :%d",line);
		perror(err_string	);
		exit(1);
}




/*全局变量：用于获取时间*/
time_t  timep;


/*cl.c*/
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
int  check_online (char *t[]);//判断用户是否在线
int  init_clents(char *t[]);//初始化用户数组结构
void register_new_client( send_info *send ,int newfd);//注册新用户
void server_check_login( send_info *send ,int newfd);//验证登陆
void client_exit(send_info *send , int newfd);//客户端的退出处理
void get_all_online_clients(send_info *send, int newfd);//查看所有在线用户
int  get_sock_fd(char dest[]);//获取用户的sockid(accept的返回值)
void parse_buf(send_info *send,int newfd);//解析客户端传过来的字符串数据结构中的type 成员来匹配调用不同的函数
void format_buf_public(char *dest,char *content,int newfd);//格式化字符串 (加上时间，用户昵称和ID等)专供群聊使用
void format_buf_private(char *dest,char *content,int newfd ,send_info *send);//格式化字符串 (加上时间，用户昵称和ID等)专供私聊使用
void private_chat ( send_info *send,int newfd);//私聊
void public_chat (send_info *send ,int newfd);//群聊（不包含自己）
void *pthreads(void *arg);// 线程读函数
int  system_init();//初始化连接
void connect_to_client(int sock_fd ) ;//等待客户端连接
void client_exit(send_info *send , int exit_client_socket_fd);//客户端的退出处理
int  my_read(int fd , int newfd);//读取文件中数据内容
void chat_history(int newfd);//聊天记录查询
void save_history_log( char *dest );//服务器日志保存
void save_history_chat( char *dest );//聊天记录保存
void save_friend(char *dest ,int newfd);//建立或打开一个以用户名命名的文档保存好友信息
void server_check_add( send_info *send ,int newfd);//验证加好友
int  my_read_friend(int fd , int newfd);//读取文件中数据内容(查看好友专用)
void add_friend( send_info *send ,int newfd);//验证要加的好友是否存在或者在线，如果存在并且在线的话，
void find_friend();//从服务器获取好友列表
void accept_friend(send_info *send ,int newfd );//添加好友成功
void disaccept_friend(send_info *send ,int newfd );//添加好友拒绝

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
						clients[i].client_id = atoi(t[2]);		//将id整形化存入
						clients[i].is_online = 1;				//在线
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
		char save[BUF_SIZE] = {0};//存储服务器日志内容
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
						if(strcmp (t[0] ,send->name ) == 0)//将客户端的用户名与文件中储存的用户名进行对比，如果相同;
						{
								is_account_exited_flag = REGIST_EXITED;//注册用户用户已经存在
								break;
						}
						id = atoi(t[2]);	//获得id号
						len += strlen(string)+2;
						bzero( string, sizeof(string));//初始化string
						time(&timep);
						sprintf( save ,"时间： %s \n用户名 :%s  \t 注册成功 \n",ctime(&timep) ,send->name );//存入save中
						save_history_log( save); // 调用函数存入服务器日志
				}
		}
		if(is_account_exited_flag == REGIST_EXITED)//如果注册用户已经存在
		{
				ret = REGIST_EXITED;	//退出
				write( newfd , &ret ,sizeof(int ));//将内容传给客户端
				time(&timep);
				sprintf( save ,"时间： %s \n  注册失败 \n失败原因：用户已存在\n",ctime(&timep) );//存入save中
				save_history_log( save); // 调用函数存入服务器日志
		}
		else //如果用户未注册
		{
				sprintf( all_buf ,"%s:%s:%d",send->name ,send->passwd, id );//将用户名，密码，ｉｄ，类型写入
				n = write(fd ,all_buf ,strlen(all_buf)+1 );//n为成功写入的字数
				m = write(fd ,"\n",1);//成功写入'\n'
				if( m != 0 && n != 0){
					ret = id;
					write (newfd ,&ret ,sizeof(int ));//将内容传给客户端
				}
				else{
						ret = REGIST_FALIED;//错误
						write(newfd ,&ret ,sizeof(int));//将内容传给客户端
						time(&timep);
						sprintf( save ,"时间： %s \n  注册失败 \n失败原因：秘密，就不告诉你\n",ctime(&timep) );//存入save中
						save_history_log( save); // 调用函数存入服务器日志
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
		char save[BUF_SIZE] ={0};
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
				j = 0;//将buf中内容储存至t[]中
				t[j++] = strtok(buf ,":");
				while( t[j++] = strtok(NULL ,":"));
				if( strcmp(t[0] ,send->name) == 0)
				{
						if( strcmp(t[1] ,send->passwd) == 0)//密码正确
						{
								bzero(&ret ,sizeof(ret ));
								pthread_mutex_unlock(&mutex); //减锁
								if( init_clents(t))  // ret = １:在线，ret = 0不在线
								{
										ret = NORMAL_USER_LOAD_FAILED_ONLINE ;//已经在线
										time(&timep);
										sprintf( save ,"时间： %s \n  登陆失败 \n失败原因：用户:%s已在线",ctime(&timep) ,send->name );//存入save中
										save_history_log( save); // 调用函数存入服务器日志
								}
								else
								{
										ret = NORMAL_USER_LOAD_SUCCESS; //登陆成功
										time(&timep);
										sprintf( save ,"时间： %s \n用户名 :%s  \t 登陆成功 \n",ctime(&timep) ,send->name  );//存入save中
										save_history_log( save); // 调用函数存入服务器日志
								}break;

						}
						else //密码错误
						{
								bzero(&ret ,sizeof(ret ));
								ret = NORMAL_USER_LOAD_PASSWD_ERROR;//密码错误
								time(&timep);
							    sprintf( save ,"时间： %s \n用户名 :%s \t  登陆失败 \n失败原因：密码错误\n",ctime(&timep) ,send->name );//存入save中
								save_history_log( save); // 调用函数存入服务器日志
								break;
						}
				}else {//没有此帐号
						bzero(&ret ,sizeof(ret));
						ret = NORMAL_USER_LOAD_FAILED;
						time(&timep);
					    sprintf( save ,"时间： %s \n登陆失败 \n失败原因：用户名不存在\n",ctime(&timep) );//存入save中
						save_history_log( save); // 调用函数存入服务器日志
				}
				memset(buf ,0 ,sizeof(buf));
		}
		write(newfd ,&ret ,sizeof(ret)); //将内容传给客户端1:登陆成功 0:登陆失败 2:在线 3:密码错误
		pthread_mutex_unlock (&mutex) ;	 //解锁
		close( fd);
}


void client_exit(send_info *send , int exit_client_socket_fd)//客户端的退出处理
{
		int i = 0;
		char save[BUF_SIZE] = {0};
		for(i = 0; i < QUEUELEN; i++)
		{
				if(clients[i].sock_fd == exit_client_socket_fd)
						break;
		}
		//把该用户相关信息置为空
		fprintf(stderr ,"用户名 :%s \t id:%d \t 退出 \n",clients[i].client_name ,clients[i].client_id);
		
		time(&timep);
		sprintf( save ,"时间： %s \n用户名 :%s \t id:%d \t 退出 \n",ctime(&timep) ,clients[i].client_name ,clients[i].client_id );//存入save中
		save_history_log( save);
		memset(&clients[i] ,0 ,sizeof(clients[i]));//初始化：结构体数组
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
       				if(send->type == CHAT_HISTORY)
       				{
        				   chat_history( newfd);	
       				}
					if(send->type == FIND_FRIEND)//好友信息查看
					{
					  	   find_friend(newfd);
					}
					
					if(send->type == ADD_FRIEND)//添加好友
					{
							add_friend(send ,newfd);
					}
					if(send->type == ACCEPT_FRIEND)//接受好友
					{
							accept_friend(send ,newfd);
					}
					if(send->type == DISACCEPT_FRIEND)//拒绝好友
					{
							disaccept_friend(send ,newfd);
					}
				   
}	

void add_friend( send_info *send ,int newfd)//验证要加的好友是否存在或者在线，如果存在并且在线的话，则向被加好友方发送加好友请求，
{

		pthread_mutex_lock(&mutex);
		int fd;
		int i;
		char save[1024];//存储好友文件中读取的信息
		char name[32];//存储Ｂ用户名
		char msg[BUF_SIZE];//存储请求信息
		int  sockefd;//存储Ｂ的套接字
		char wait[BUF_SIZE];//存储等待语句
		char result[BUF_SIZE];//存储加好友结果
		int  ret ;// strstr 的返回值
		
		sockefd = get_sock_fd( send->id);//通过函数获得Ｂ 的套接字
		for (i = 0; i < QUEUELEN; i++)
		{
				if(clients[i].sock_fd == newfd )
				{
						strcpy( name,clients[i].client_name);//获得A用户的用户名				
				}
		}
		
		
	//	printf(" a= %s\n" ,name);
	//	printf("send->id = %s \n" ,send->id);
		
		char s[255];
		sprintf(s, "%s.odt", name);
		if((fd = open ( s, O_CREAT |O_RDONLY ,0644)) == -1 )  {//打开文件，没有则创建
				my_err("open ",__LINE__);
		}
		read(fd ,save ,sizeof(save)+1);	//将文件中的好友信息读取到save中		
		
		

		for (i = 0; i< QUEUELEN ;i++)
		{
				 if(ret = (strstr(save ,send->id)) != NULL )	//比较buf中的用户名和好友文件中的用户名
				{
				
								sprintf(result ,"对方已经是你的好友\n");	
								write(newfd , result ,strlen(result)+1 );//发送给 A 的客户端
								DEBUG("添加好友重复\n");
								break;			
				}
				else if( strcmp(name ,send->id) == 0  )//当客户端的名字和客户端发来的用户名相同
				{	
						sprintf(result ,"不能添加自己为好友\n");	
						write(newfd , result ,strlen(result)+1 );//发送给 A 的客户端
						DEBUG("不能添加自己为好友\n");
						break;
						
				}
			   
				else if( (clients[i].sock_fd == sockefd) && (clients[i].is_online == 1) )//当数组中的名字和客户端发来的用户名相同，且用户在线
				{
						
						printf("%s请求加 %s 为好友\n" ,name ,send->id);//在服务器提示
						sprintf( msg ,"%s请求加你为好友,是否同意\n",name);//将请求信息存入数组
						write(sockefd , msg ,strlen(msg)+1 );//发送给 B 的客户端
						sprintf(wait ,"请等待，正在等待对方回应...\n");
						printf("请等待，正在等待%s 回应...\n" ,send->id);//服务器端的提示
						write(newfd , wait ,strlen(msg)+1 );//发送给 A 的客户端
						break;
						
				}
				
								  
		
				else if((clients[i].client_name != send->id) && (clients[i].is_online == 0) &&  (i == QUEUELEN-1))
				{	
					    sprintf(result ,"用户不存在或者不在线\n");
						write(newfd , result ,strlen(result)+1 );//发送给 A 的客户端
						DEBUG("用户不存在或者不在线\n"); 
						break;
				}  		
		}	
		close( fd);//关闭文件
		pthread_mutex_unlock(&mutex);
	
}



void accept_friend(send_info *send ,int newfd )//添加好友成功
{
				
				int fd;
				char name[32];//存储Ｂ用户名
				int  sockefd;//存储Ｂ的套接字
				char result[BUF_SIZE];//存储加好友结果
				int i;
		
			//	printf("send->id = %s\n" ,send->id);
				sockefd = get_sock_fd( send->id);//通过函数获得Ｂ 的套接字
				for (i = 0; i < QUEUELEN; i++)
				{
						if(clients[i].sock_fd == newfd )
						{
								strcpy( name,clients[i].client_name);//获得A用户的用户名				
						}
				}
			//	printf("name = %s",name);
				save_friend( send->id ,newfd); //将 B 的信息放入 A 的好友文件中
				save_friend( name ,sockefd );    //将 A 的信息放入 B 的好友文件中
				sprintf(result ,"\n加好友成功\n");
				write(newfd , result ,strlen(result)+1 );//发送给 A 的客户端
				write(sockefd , result ,strlen(result)+1 );//发送给 B 的客户端
				DEBUG("\n加好友成功\n");

}
void disaccept_friend( send_info *send ,int newfd )//添加好友失败
{		
				
				int fd;
				char name[32];//存储Ｂ用户名
				int  sockefd;//存储Ｂ的套接字
				char result[BUF_SIZE];//存储加好友结果
				
			//	printf("send->id = %s\n" ,send->id);
				sockefd = get_sock_fd( send->id);//通过函数获得Ｂ 的套接字
	      		sprintf(result ,"\n对方拒绝了你的请求\n");
				write(sockefd , result ,strlen(result)+1 );//发送给 A 的客户端
				DEBUG("\n加好友失败\n");
				
}



void find_friend( int newfd)//查看文件中所有好友数
{
	int fd;
	int i;
	char name[32];
	for (i = 0; i < QUEUELEN; i++)
	{
				if(clients[i].sock_fd == newfd )
				{
						strcpy( name,clients[i].client_name);//获得用户的用户名				
				}
	}
	char s[255];
	sprintf(s, "%s.odt", name);
	if((fd = open ( s, O_CREAT |O_RDONLY ,0644)) == -1 )//打开文件，没有则创建
				my_err("open ",__LINE__);
	
	printf("\n\n====================%s的好友数======================\n\n",name);
	my_read_friend(fd , newfd);//查看好友列表
	printf("\n\n==================================================\n\n");
	close(fd);	
}

int my_read_friend(int fd , int newfd)//读取文件中数据内容(查看好友专用)
{
	int len;
	int ret;
	int i;
	char read_buf[2048];//储存文件内容;
	char name[32];//保存请求查看聊天记录的用户名
	//获取文件长度并保持文件读写指针在文件的开头处
	if( lseek(fd ,0 ,SEEK_END ) == -1)
	{
		my_err("lseek" ,__LINE__);
	}
	if( (len =lseek(fd ,0 ,SEEK_CUR )) == -1)
	{
		my_err("len" ,__LINE__);
	}
	if( lseek(fd ,0 ,SEEK_SET ) == -1)
	{
		my_err("lseek" ,__LINE__);
	}
	

	if( ret = read(fd ,read_buf ,len ) <= 0)
	{
		printf("记录读取失败或者记录为空\n");
	
	}
	
	for (i=0; i<len ;i++ ) {//打印文件中数据
		printf("%c" ,read_buf[i]);
	}
	write(newfd ,read_buf ,len+1);//传送给客户端
	return ret;
	
}



void save_friend(char *dest ,int newfd)//建立或打开一个以用户名命名的文档保存好友信息
{
	int fd ,m , n;
	int i;
	char name[32];
	for (i = 0; i < QUEUELEN; i++)
	{
				if(clients[i].sock_fd == newfd )
				{
						strcpy( name,clients[i].client_name);//获得用户的用户名				
				}
	}
	char s[255];
	sprintf(s, "%s.odt", name);
	if((fd = open ( s, O_CREAT |O_RDWR |O_APPEND ,0644)) == -1 )//打开文件，没有则创建
				my_err("open ",__LINE__);
	
	n = write(fd ,dest ,strlen(dest)+1 );//n为成功写入的字数
	m = write(fd ,"\n",1);//成功写入'\n'
	close(fd);
	
}



int my_read(int fd , int newfd)//读取文件中数据内容(聊天记录专用)
{
	int len;
	int ret;
	int i;
	char read_buf[2048];//储存文件内容;
	char *name;//保存请求查看聊天记录的用户名
	/*获取文件长度并保持文件读写指针在文件的开头处*/
	if( lseek(fd ,0 ,SEEK_END ) == -1)
	{
		my_err("lseek" ,__LINE__);
	}
	if( (len =lseek(fd ,0 ,SEEK_CUR )) == -1)
	{
		my_err("len" ,__LINE__);
	}
	if( lseek(fd ,0 ,SEEK_SET ) == -1)
	{
		my_err("lseek" ,__LINE__);
	}
	for (i = 0; i < QUEUELEN; i++)
	{
				if(clients[i].sock_fd == newfd )
				{
						name = clients[i].client_name;//获得用户的用户名				
				}
	}
		
	printf("用户: %s 请求查看聊天记录:\n\n",name);
	printf("聊天记录的总字节数:%d \n", len);

	if( ret = read(fd ,read_buf ,len ) <= 0)
	{
		printf("记录读取失败或者记录为空\n");
	
	}
	for (i=0; i<len ;i++ ) {//打印文件中数据
		printf("%c" ,read_buf[i]);
	}
	
	write(newfd ,read_buf ,len+1);//传送给客户端
	return ret;
	
	
 	
}

void chat_history(int newfd) //聊天记录查询
{
	int fd;
	if((fd = open(C_HISTORY , O_CREAT |O_RDONLY ,0644)) == -1)
	{
		my_err("open",__LINE__);
	}
	printf("\n\n====================聊天记录======================\n\n");
	my_read(fd , newfd);//查看C_HISTORY中内容
	printf("\n\n==================================================\n\n");
	close(fd);
	
	
	
}      		
void save_history_chat( char *dest )//聊天记录保存
{
	int fd ,m , n;
	if((fd = open (C_HISTORY , O_CREAT |O_RDWR |O_APPEND ,0644)) == -1 )//打开文件，没有则创建
				my_err("open ",__LINE__);
	
	n = write(fd ,dest ,strlen(dest)+1 );//n为成功写入的字数
	m = write(fd ,"\n",1);//成功写入'\n'
	close(fd);
}  


void save_history_log( char *dest )//服务器日志保存
{
	int fd ,m , n;
	if((fd = open (SERVER_LOG , O_CREAT |O_RDWR |O_APPEND ,0644)) == -1 )//打开文件，没有则创建
				my_err("open ",__LINE__);
	
	n = write(fd ,dest ,strlen(dest)+1 );//n为成功写入的字数
	m = write(fd ,"\n",1);//成功写入'\n'
	close(fd);
}  

void format_buf_public(char *dest,char *content,int newfd)//格式化字符串 (加上时间，用户昵称和ID等)(群聊专用)
{  
		    int i=0;
    	  	time_t  timep;
     		for(;i<QUEUELEN;i++)
		    { 
	    	  	    if(clients[i].sock_fd == newfd)
					break;
     		}
      		time(&timep);
            sprintf(dest,"%s\t\t%s对大家说：%s\n",ctime(&timep),clients[i].client_name,content);//将聊天用户名称，时间，内容发送 存入dest 
		    
} 
void format_buf_private(char *dest,char *content,int newfd ,send_info *send)//格式化字符串 (加上时间，用户昵称和ID等)
{  
		
		    int i=0;
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
				save_history_chat( dest);
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
				    save_history_chat(str);
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
			printf("很抱歉，你已经开了一个服务器啦(。・`ω´・)\n");
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



