/*================================================================
 * 
 *   文件名称：server.c
 *   创 建 者：Ch
 *   创建日期：2020.8.10
 *
 ================================================================*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <net/if.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sqlite3.h>   

#define SERV_PORT 6666
#define BACKLOG   10
#define ENTER        100        //登录
#define ENTERWIN     1001       //登录成功
#define QUIT         101 		//退出
#define REGISTER     102 		//注册
#define REGISTERWIN  1021       //注册成功
#define USERADD      103        //注册填充信息
#define USERADDWIN   1031       //填充信息成功     
#define ERR          -1        
#define DATABASE "users.db"
#define LOOKMES      104        //查询个人信息

#define ALTERMES     105        //修改个人信息
#define ALTERMESWIN  1051       //
#define ALTERCODE    106        //修改密码
#define ALTERCODEWIN 1061       //
#define ROOT         107        //超级用户功能
#define ROOTWIN      1071       //超级功能成功
#define ROOTDL       108        //超级用户删除用户信息
#define ROOTDLWIN    1081       //超级用户删除用户信息成功
#define ROOTALT      109        //超级用户修改功能
#define ROOTALTWIN   1091       //超级用户修改功能成功

int look_account = 0;  //查看帐号是否存在标志
int look_code = 0;     //查看密码是否正确标志
int root_flag = 1;     //超级用户标志位

//客户端用户信息结构题 
struct  cli_users{  
	int opt;              //操作数
	char name[20];        //姓名
	int  age;             //年龄
	int  account;         //账号  
	int  code;            //密码
	int  jobnum;          //工号
	int  salary;  	      //工资
	char department[20];  //部门
	int phone;   //电话
}user;

/************数据库变量定义*******************/    
sqlite3* db = NULL;                                           
char *errmsg = NULL;                                          
char * alter_mes_stu;
char * alter_code_stu;
/*********************************************/
/****************数据库函数定义***************/
int user_add(int *fd,struct cli_users *user, sqlite3 *db);//用户添加函数
int user_altermes(int *fd,struct cli_users *user,sqlite3 *db); //修改个人信息
int root_delete(int *fd, struct cli_users *user,sqlite3 *db);//超级用户删除用户信息
int root_altermes(int *fd,struct cli_users *user,sqlite3 *db);//超级用户修改个人信息
int user_lookmes(int *fd,struct cli_users *user,sqlite3 *db);//查询个人信息
void cli_info(struct sockaddr_in cin);
static int callback(void*,int ,char ** ,char ** );
/**********************************************/

int main(int argc, char *argv[])
{
	int fd = -1;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){
		perror("socket");
		exit(1);
	}

	//02设置地址绑定快速重用
	int reuse = 1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

	//03填充服务器要绑定的地址信息结构体，为bind铺路
	struct sockaddr_in sin;
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port   = htons(SERV_PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	//04把地址信息绑定到套接字上
	if(bind(fd,(struct sockaddr*)&sin,sizeof(sin))<0){
		perror("bind");
		exit(1);
	}

	//05把主动套接字变为被动套接字
	if(listen(fd,BACKLOG)<0){
		perror("listen");
		exit(1);
	}

	//06定义一些变量
	struct sockaddr_in cin;//存放客户端信息的结构体
	socklen_t addrlen = sizeof(cin);
	int newfd =-1;

	//创建集合
	fd_set readfds,temp;//创建集合
	FD_ZERO(&readfds);//清空集合
	FD_ZERO(&temp);
	FD_SET(fd,&readfds);//设置用于监听的文件描述符进入到集合

	int maxfd = fd;
	int val = -1;
	int i = 0;
	int setfd=-1;
	int input_t = -1;
	int recv_t  = -1;
	int send_t  = -1;
	int flags = -1;
	puts("服务器已经启动了啦");

	if(sqlite3_open(DATABASE,&db)!=SQLITE_OK){                    
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db));           
		return -1;                                                
	}                                                             
	else {                                                        
		printf("数据库打开成功\n");                               
	}                                                             

	//创建一个表格                                                
	char * create_table =                                         
		"create table if not exists stu(opt int,name char,age int,account int,code int, jobnum int,salary int,department char,phone int)";
	//如果表格不存在就创建                                    
	if(sqlite3_exec(db,create_table,NULL,NULL,&errmsg)!=SQLITE_OK){
		fprintf(stderr,"操作出错：%s",errmsg);                    
		sqlite3_close(db);                                        
		return -2;                                                
	}                                                             
	else{                                                         
		puts("创建或打开成功");                                   
	}                             

	//如果没有创建777管理员帐号，就创建777管理员帐号
	user.account = 777;
	sqlite3_exec(db,"select * from stu;",callback,&user.account,NULL);
	if(look_account == 1){
		look_account = 0;
	}else{
		char * write_root_stu = sqlite3_mprintf("insert into stu values('%d','%s','%d','%d','%d','%d',\
								'%d','%s','%d')",0,"root",0,777,777,666666,6666,"root",0);
		sqlite3_exec(db,write_root_stu,NULL,NULL,NULL);
	}                                                                                                                                                           

	while(1){
		temp = readfds;
		val = select(maxfd+1,&temp,NULL,NULL,NULL);
		if(val<0){
			perror("select");
			exit(1);
		}

		//轮训判断那个文件描述符产生了事件
		for(i=0;i<maxfd+1;i++){
			if(FD_ISSET(i,&temp)){
				if(fd == i){//是不是fd前台产生事件
					newfd = accept(fd,(struct sockaddr*)&cin,&addrlen);
					if(newfd <0){
						perror("accept:");
						break;
					}
					cli_info(cin);
					//把新连接的newfd加入到集合中
					FD_SET(newfd,&readfds);
					//更新maxfd，判断那个是最大的赋值给maxfd
					maxfd = (maxfd>newfd)?maxfd:newfd;
				}
				else{//剩下的文件描述符产生的事件就是客户端发来的消息
					bzero(&user,sizeof(user));
					//收消息
					do{
						recv_t = recv(i,&user,sizeof(user),0);
					}while(recv_t<0 && EINTR==errno);
					if(recv_t<0){
						perror("recv");
						FD_CLR(i,&readfds);
						close(i);
					}
					else if(0 == recv_t){
						printf("客户端fd=%d已经关闭!\n",i);
						FD_CLR(i,&readfds);
						close(i);
					}
					else{
						printf("收到fd=%d操作码%d帐号%d密码%d\n",i,user.opt,user.account,user.code);
						flags = user.opt;

						switch(flags){
						case ENTER:
							user_enter(&i,&user,db);//登录
							break;

						case QUIT:
							break;

						case REGISTER:
							user_register(&i,&user, db);//注册
							break;

						case USERADD:
							user_add(&i,&user,db);//添加用户
							break;
						
						case ALTERCODE:
							user_altercode(&i,&user,db);
							break;

						case ALTERMES:							
							user_add(&i,&user,db); //修改个人信息
							break;
						
						case ROOT:                      //超级用户查看个人信息

						case LOOKMES:
							user_lookmes(&i,&user,db);//查询个人信息
							break; 
                        
                        case ROOTALT:                    //超级用户修改用户信息
                            root_altermes(&i,&user,db);//超级用户修改个人信息
                            break;

						case ROOTDL:                     //超级用户删除用户信息					    
                            root_delete(&i,&user,db);//超级用户删除用户信息
							break;
							 
						default : 
							break;

						}

					}
				}
			}
		}
	}
	sqlite3_close(db);
	close(fd);
	return 0;
}

int user_add(int *fd,struct cli_users *user, sqlite3 *db)//添加或修改个人信息
{
	alter_mes_stu = sqlite3_mprintf("update stu set age='%d' where account = '%d'",user->age,user->account);
	sqlite3_exec(db,alter_mes_stu,NULL,NULL,&errmsg);

	alter_mes_stu = sqlite3_mprintf("update stu set phone='%d' where account = '%d'",user->phone,user->account);
	sqlite3_exec(db,alter_mes_stu,NULL,NULL,&errmsg);

	alter_mes_stu = sqlite3_mprintf("update stu set name='%s' where account = '%d'",user->name,user->account);
	if( sqlite3_exec(db,alter_mes_stu,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("创建失败\n");
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
	}else{
		sqlite3_exec(db,"select * from stu;",callback,&user->account,&errmsg);							
		printf("创建成功\n");
		user->opt = ALTERMESWIN;
		send(*fd,user,sizeof(user),0);
	}
	look_account = 0;
	look_code = 0;
	return 0;
}

int user_enter(int *fd,struct cli_users *user, sqlite3 *db)
{
	look_account = 0;
	look_code = 0;
	if(sqlite3_exec(db,"select * from stu;",callback,&user->account,NULL)!=SQLITE_OK){
		fprintf(stderr,"查询 %s\n",sqlite3_errmsg(db));           
		sqlite3_close(db);                                        
		return -3;                                                
	}                                                             
	else{
		if(look_account == 1 && look_code == 1){
			look_account = 0;
			look_code = 0;
			printf("帐号密码存在\n");
			user->opt = ENTERWIN;

			printf("%d\n",user->opt);
			send(*fd,user,sizeof(*user),0);
		}else{
			look_account = 0;
			look_code = 0;
			printf("帐号密码错误或帐号未注册\n");
			user->opt = ERR;
			printf("%d\n",user->opt);
			send(*fd,user,sizeof(*user),0);		
		}

	}//查询成功
	return 0;
}

int user_register(int *fd,struct cli_users *user, sqlite3 *db)
{
	char *errmsg = NULL;
	int ret;
	look_account = 0;
	look_code = 0;
	printf("准备注册\n");
	if(sqlite3_exec(db,"select * from stu;",callback,&user->account,&errmsg)!=SQLITE_OK){
		fprintf(stderr,"查询 %s\n",sqlite3_errmsg(db));           
		sqlite3_close(db);                                        
		return -3;                                                
	}                                                             
	else{                                                          
		if(look_account == 1){
			look_account = 0;
			printf("帐号已存在\n");
			user->opt == ERR;

			do{
				ret = send(*fd,user,sizeof(*user),0);
			}while(ret < 0 && EINTR == errno);
			if (ret == 0){
				printf("客户端关闭\n");
				return -1;
			}
			return 0;			
		}else {
			printf("正在注册\n");

			//写入数据库
			char * write_stu = sqlite3_mprintf("insert into stu values('%d','%s','%d','%d','%d','%d','%d','%s','%d')",REGISTERWIN,"weizhi",0,user->account,user->code,user->account*10,4000,"aaa",0);
			if( sqlite3_exec(db,write_stu,
						NULL,NULL,&errmsg) != SQLITE_OK){                     
				fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
				user->opt = ERR;

				do{
					ret = send(*fd,user,sizeof(*user),0);
				}while(ret < 0 && EINTR == errno);
				if (ret == 0){
					printf("客户端关闭\n");
					return -1;
				}	                                        
				return -3;                                                
			}else{                                                         
				puts("注册成功"); 
				user->opt = REGISTERWIN;
				user->jobnum = user->account*10;	
				do{
					ret = send(*fd,user,sizeof(*user),0);
				}while(ret < 0 && EINTR == errno);
				if (ret == 0){
					printf("客户端关闭\n");
					return -1;
				}
			}                
		}

		return 0;
	}              
}

int user_altercode(int *fd,struct cli_users *user,sqlite3 *db) //修改密码
{
	alter_code_stu = sqlite3_mprintf("update stu set code='%d' where account = '%d'",user->code,user->account);
	if( sqlite3_exec(db,alter_code_stu,NULL,NULL,&errmsg) != SQLITE_OK){                     
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
	}
	user->opt = ALTERCODEWIN;
	printf("\n%s\n",user->name);
	send(*fd,user,sizeof(*user),0);
}

int user_lookmes(int *fd,struct cli_users *user,sqlite3 *db)//查询个人信息
{
	sqlite3_exec(db,"select * from stu;",callback,&user->account,NULL);
	printf("%s,%d\n",user->name,user->phone);
		if(look_account != 1){
			look_account = 0;
			printf("帐号不存在\n");
			user->opt = ERR;
		}
	send(*fd,user,sizeof(*user),0);
}

int root_delete(int *fd, struct cli_users *user,sqlite3 *db)//超级用户删除用户信息
{
   if(look_account != 1){
			look_account = 0;
			printf("帐号不存在\n");
			user->opt == ERR;
			send(*fd,user,sizeof(*user),0);
			return 0;
		}
    alter_code_stu = sqlite3_mprintf("delete from stu where account = '%d'",user->account);
	if( sqlite3_exec(db,alter_code_stu,NULL,NULL,&errmsg) != SQLITE_OK){                     
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
	}
    user->opt = ROOTDLWIN;
	send(*fd,&user,sizeof(user),0);
}

int root_altermes(int *fd,struct cli_users *user,sqlite3 *db)//超级用户修改个人信息
{   
	sqlite3_exec(db,"select * from stu;",callback,&user->account,NULL);
		if(look_account != 1){
			look_account = 0;
			printf("帐号不存在\n");
			user->opt == ERR;
			send(*fd,user,sizeof(*user),0);
			return 0;
		}
    alter_code_stu = sqlite3_mprintf("update stu set salary = '%d' where account = '%d'",user->salary,user->account);
	if( sqlite3_exec(db,alter_code_stu,NULL,NULL,&errmsg) != SQLITE_OK){                     
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
	}
    alter_code_stu = sqlite3_mprintf("update stu set department = '%s' where account = '%d'",user->department,user->account);
	if( sqlite3_exec(db,alter_code_stu,NULL,NULL,&errmsg) != SQLITE_OK){                     
		fprintf(stderr,"open %s\n",sqlite3_errmsg(db)); 
	}
	user->opt = ROOTALTWIN;
	send(*fd,&user,sizeof(user),0);
}

static int callback(void*para,int f_num,char ** f_value,char ** f_name)
{                                                                 
	int i =-1,j=-1,k = 0;                             	
	for(i=0;i<f_num;i++){                                         
		if(*(int*)para == atoi(f_value[3])){   
			k++;	
			printf("%-15s",f_value[i]);            
		}
	}  

	if(*(int*)para == atoi(f_value[3])){
		if(user.code == atoi(f_value[4])){
		look_code = 1;
		}
			for (i = 0; f_value[1][i];i++){
				user.name[i] = f_value[1][i];
			}
			user.name[i] = f_value[1][i];
			user.age  = atoi(f_value[2]);
			user.code = atoi(f_value[4]);
			user.jobnum =atoi(f_value[5]);
			user.salary = atoi(f_value[6]);
			user.phone = atoi(f_value[8]);
			for (i = 0; f_value[7][i];i++){
				user.department[i] = f_value[7][i];
			}
			user.department[i] = f_value[7][i];
			putchar(10);    
	}
	if (k == f_num) {
		look_account = 1;
	}
	return 0;                                                     
}                       

void cli_info(struct sockaddr_in cin)
{
	//显示客户端信息
	char ipv4_addr[16];
	const char* ntop_t;
	ntop_t = inet_ntop(AF_INET,(void*)&cin.sin_addr.s_addr,ipv4_addr,16);
	if(NULL == ntop_t){
		perror("inet_ntop");
		exit(1);
	}
	printf("***客户端(%s:%d)已经连接！\n",ipv4_addr,ntohs(cin.sin_port));
}







