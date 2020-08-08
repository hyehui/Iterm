/*================================================================
*   Copyright (C) 2020 hqyj Ltd. All rights reserved.
*   
*   文件名称：01_server.c
*   创 建 者：Chens
*   创建日期：2020年05月28日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define SERV_PORT 6666
#define BACKLOG   10
#define ENTER     100
#define QUIT      101
#define REGISTER  102  

//客户端用户信息结构题 
struct  cli_users{  
	int opt;             //操作数
	char name[20];        //姓名
	int  age;             //年龄
	int  account;         //账号  
	int  code;            //密码
	int  jobnum;          //工号
	int  salary;  	      //工资
	char department;      //部门
};

void cli_info(struct sockaddr_in cin);

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
	struct cli_users user;
	int setfd=-1;
	int input_t = -1;
	int recv_t  = -1;
	int send_t  = -1;

	puts("服务器已经启动了啦");
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
						printf("收到fd=%d操作吗%d 帐号%d 密码%d\n",i,user.opt,user.account,user.code);

					}
				}
			}
		}
	}
	close(fd);
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

