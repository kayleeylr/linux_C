
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


int my_err( const char *err_string ,int line)
{
		fprintf(stderr ,"line :%d",line);
		perror(err_string);
		exit(1);
}
//菜单提示的数据结构
typedef struct {
		char 	cmd[40];		//命令
		char	explain[100];	//命令描述
		int 	admin;			//权限
}usage;

int sock_fd;	//套接字描述符
int global_admin_flag = NORMAL_USER;	//用户权限	默认为０；	




pthread_mutex_t 	mutex = PTHREAD_MUTEX_INITIALIZER;//初始化锁
void inits()//建立套接字，连接，初始化客户端
{
		struct 	sockaddr_in		serv_addr;
		if((sock_fd = socket(AF_INET ,SOCK_STREAM ,0 )) == -1)	//创建套接字
		{
				my_err("socket",__LINE__);
		}
		//初始化服务器端地址结构
		bzero(&serv_addr, sizeof(serv_addr)); //内存初始化
		serv_addr.sin_family = AF_INET;						//地址类型为tcp/ip
		serv_addr.sin_port = htons(PORT);					//设置端口号
		inet_pton(AF_INET ,IP ,&serv_addr.sin_addr);		//将ＩＰ地址转化为二进制
		if(( connect (sock_fd ,(struct sockaddr* )&serv_addr ,sizeof(struct sockaddr))) == -1)	{	//向服务器发送连接请求
				my_err("connect ",__LINE__);
		}
		reg_log();	//登陆还是注册
		pthread_t tid ; //设置线程
		if((pthread_create(&tid ,NULL ,(void* )pthread_load,&sock_fd )) == -1){	//创建线程
				my_err("pthread_creaate", __LINE__);
		}
		write_data();//发送数据
}



void show_menu()//显示菜单
{
		int i = 0;
		printf("\033[40;35m\n\n\n\t===========================(●'◡'●)ﾉ♥================================\n\n\n");
		printf("\t\t 功能\t\t\t  功能描述\n");
		printf("\t\t>信息\t\t\t与所有用户聊天\n");
		printf("\t\t:用户名>信息\t\t选择用户聊天\n");
		printf("\t\t--online\t\t显示在线用户\n");
		printf("\t\t--help\t\t\t显示帮助信息\n");
		printf("\t\texit\t\t\t退出\n");
		printf("\n\n\t=========================== (=￣ω￣=)================================\n");

}
	
void reg_log()//登陆或者注册
{
		char ch ;
		printf("\n\n\t\t---------------------------------------------\n");
		printf("\n\t\t   1  注册");
		printf("\n\t\t   2  登陆\n");
		while(1)
		{		
				printf("请输入1或2:");
				ch = getchar();
				if(ch=='2')
				{
						int get_ret=0;
  						while(1)
						{ 
								login();//注册用户
								read(sock_fd ,&get_ret ,sizeof(get_ret));//从服务器读取所要注册账户的信息
								if(get_ret == NORMAL_USER_LOAD_SUCCESS)//如果账户不存在
								{
									printf("用户您已成功登录。欢迎您！\n\n");
									global_admin_flag = NORMAL_USER;//普通用户
			    						show_menu();//显示菜单
									break;
								}
						
								else if(get_ret == NORMAL_USER_LOAD_FAILED_ONLINE)//用户已经在线
        						        {
									printf("登录失败，该用户在线!╮(╯▽╰)╭\n");
                							exit(EXIT_SUCCESS);
									break;
        						        }
          						        else if(get_ret == NORMAL_USER_LOAD_PASSWD_ERROR) //密码错误
          							        printf("密码错误，请重新登录！\n");
                						else//用户名错误
									printf("用户名错误，请重新登录！\n");
						}
						break;
				}
				else if(ch=='1')
				{
						 int get_ret=0;
						 while(1)
						 {
								regiser_client();//登陆用户
								read(sock_fd,&get_ret,sizeof(int));//从服务器端验证用户帐号的相关信息
								if(get_ret == REGIST_EXITED)//用户已經存在
          								 printf("该用户已存在，请重新输入！இwஇ\n ");
 								else if(get_ret == REGIST_FALIED)//注册失败 
									 printf("注册失败，请重新输入!இwஇ\n ");
								else 
								{
									 printf("恭喜您，注册成功,您的帐号ID为：%d \n ",get_ret);
									 printf("请重新进入客户端，进行登陆(●'◡'●)ﾉ♥\n");
									 exit(EXIT_SUCCESS);	
								   	 break;
				 				 }
	 			 		  }
						  break;
				}
				else
				{
					printf("输入错误，请重新选择.\n");
				}
		
				//清空输入
				for(;(ch = getchar()) != '\n' && ch != EOF ;)
						continue;
		}
}



void is_check( char *string )//判断输入的用户名是否存在非法字符
{
		while (1)
		{
				scanf("%s",string);
				int i = 0 ;
				int flag = -1;
				for( i= 0 ;string[i] != 0 ;i++)
				{
						if(ispunct(string[i]))//ispunct参数c是否为标点符号或特殊符号
						{
								flag = i;//
								break;
						}
				}
				if(flag != -1)
				{
						printf("对不起，您输入了非法字符,请重新输入!\n");
						bzero(string , sizeof(string));//对字符串清零
				}
				else 
						break;
		}
}

void regiser_client()//注册新用户
{
		pthread_mutex_lock(&mutex);
		send_info	send;
		
		//system("clear");
		printf("\033[40;34m\n\t\t\t****************************\n");
		printf("\t\t  用户名: ");
		is_check(send.name);
		printf("\n\t\t　密  码: ");
		is_check(send.passwd);
		send.type = CLIENT_REGISTER;//用户注册
		write(sock_fd ,&send ,sizeof(send));//向服务器发送数据
		pthread_mutex_unlock(&mutex);
}

void login() //登陆界面
{
		send_info send ;
		//system("clear");
		printf("\033[40;36m\n\n\n\t\t欢迎你，登陆界面\n");
		printf("\n\t*************************************************\n");
		printf("\n\n\t\t  用户名: ");
		is_check(send.name);
		printf("\n\t\t　密  码: ");
		is_check(send.passwd);
		send.type = CLIENT_LOAD;
		write(sock_fd ,&send ,sizeof(send));//向服务端写数据
}

void pthread_load(void *arg)//接受数据
{
		char buf[BUF_SIZE] = {0};
		int  length = 0;
		while (1)
		{
				length = read(sock_fd ,buf ,BUF_SIZE);//向服务器读取数据，返回读取字节数
				if(length <= 0)//读取数据失败
				{
						printf("服务器已关闭，请先打开服务器\n");
						exit(EXIT_SUCCESS);
						break;
				}		
				if(strcmp("exit",buf)==0)
				{
						printf("您下线了\n");
		    				exit(EXIT_SUCCESS);	
		    				break;
				}
		printf("%s\n",buf);
		memset(buf,0,sizeof(buf)); // 清除ｂｕｆ中数据
		}
		close(sock_fd);//关闭套接字
		pthread_exit(NULL);
		exit(EXIT_SUCCESS);
}

void parse_input_buf( char *p ,send_info *send)//解析命令
{
	
		switch(p[0] ) {
				
				case '>':
						send->type = PUBLIC_CHAT;//选择群聊
						strcpy( send->buf ,p+1);//把信息内容传入
						break;
				case':':
						//添加逻辑判断，是否符合规则
						if((strstr(p,">") == NULL) ||(p[1] == '>') )
						{		
							send->type = 0;
							strcpy( send->buf ,p);//将输入的内容存入结构体之中
							return;
						}
						strcpy(send->id ,strtok( p+1 ,">"));//用strtok把用户名和所要发送的信息分开
						send->type = PRIVATE_CHAT ;//私聊情况下
						strcpy( send->buf ,strtok(NULL ,">"));//用strtok把要发送的信息发送至结构体中
						break;
				case'-':
						if(strcmp( p,"--online") == 0)//如果用户输入的是查看在线人数
								send->type = CLIENT_ONLINE;
						else if (strcmp(p ,"--help" ) == 0)//如果用户需要查看帮助栏
								show_menu();
						break;
				default:
						send->type = 0;
						strcpy( send->buf ,p);//将输入的内容存入结构体之中
						break;
		}
}



void write_data()//发送数据
{
		char buf[BUF_SIZE] = {'\0'};//储存输入的命令
		send_info 	send;
		while(1)
		{
				scanf("%s",buf);
				parse_input_buf( buf ,&send);
				if(strcmp("--help",buf) == 0)//如果buf中存放的是查看帮助
						continue;
				write( sock_fd ,&send ,sizeof(send));
				if(strcmp("exit",buf) == 0)//如果buf中存放的是退出
				{
						close(sock_fd);//关闭套接字;
						exit(EXIT_SUCCESS);
				}
				memset(buf ,0 , sizeof(buf));//清空buf中的内容
		}
}


int main(int argc ,char *argv[])
{
	inits();
	close(sock_fd);
	return 0;
}





































