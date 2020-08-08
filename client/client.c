#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERV_IP   "192.168.208.128"
#define SERV_PORT 6666

#define ENTER        100        //登录
#define ENTERWIN     1001       //登录成功
#define QUIT         101 		//退出
#define REGISTER     102 		//注册
#define REGISTERWIN  1021       //注册成功
#define USERADD      103        //注册填充信息
#define USERADDWIN   1031       //填充信息成功     
#define ERR          -1        //
//客户端用户信息结构题 
struct  cli_users{	
		int  opt;            //操作数
		char name[20];       //姓名
		int  age; 	         //年龄
		int  account;        //账号
		int  code;           //密码
		int  jobnum;         //工号
		int  salary;         //工资
		char department;     //部门
};

int main(int argc, const char *argv[])
{
	//创建流式套接字
	int fd = -1;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd < 0){
		perror("socket err");
		return -1;
	}

	//创建地址信息对象，填充服务器地址信息
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_port   = htons(SERV_PORT),
		.sin_addr.s_addr = inet_addr(SERV_IP)
	};

	//与服务器建立链接
	if(connect(fd,(struct sockaddr*)&sin,sizeof(sin))){
		perror("connect");
		return -2;
	}
	printf("链接成功\n");


int flag = -1;
int ret = -1;
int user_account[3] = {0};
struct cli_users user;

	while(1){
	//登录 注册 退出 功能选择
	printf("*******************************************\n请输入您的操作码：1>登录   2>退出  3>注册\n*******************************************\n");
		scanf("%d",&flag);
		switch (flag){
		case 1://登录帐号
			user.opt = ENTER;
			break;

		case 2://退出帐号
			user_account[0] = QUIT;
			do{
				ret = send(fd,&user,sizeof(user),0);
			}while(ret < 0 && EINTR == errno);
			return 0;
			break;

		case 3://注册帐号
			user.opt =  REGISTER;
			printf("请输入您注册的帐号:");
			scanf("%d",&user.account);
			printf("请输入您的帐号密码:");
			scanf("%d",&user.code);
			do{
				ret = send(fd,&user,sizeof(user),0);
			}while(ret < 0 && EINTR == errno);
			if (ret == 0){
				printf("服务器关闭\n");
				close(fd);
				return 0;
			}

			do{
				ret = recv(fd,&user,sizeof(user),0);
			}while(ret < 0 && EINTR == errno);
			if (ret == 0){
				printf("服务器关闭\n");
				close(fd);
				return 0;
			}

			if(user.opt == REGISTERWIN){
				printf("注册成功,您的工号为:%d\n请填充基本信息\n",user.jobnum);
			}else{
				printf("注册失败,帐号已存在\n");
					break;
			}

			//帐号注册成功填充姓名年龄
			printf("请输入您的姓名:");
			scanf("%s",&user.name);
			printf("请输入您的年龄:");
			scanf("%d",&user.age);
			user.opt = USERADD;
				do{
				ret = send(fd,&user,sizeof(user),0);
			}while(ret < 0 && EINTR == errno);
			if (ret == 0){
				printf("服务器关闭\n");
				close(fd);
				return 0;
			}

		do{
				ret = recv(fd,&user,sizeof(user),0);
			}while(ret < 0 && EINTR == errno);
			if (ret == 0){
				printf("服务器关闭\n");
				close(fd);
				return 0;
			}

			if(user.opt == USERADDWIN){
				printf("填充成功，您的基本信息:\n");
				printf("您的姓名为：%s\n",user.name);
				printf("您的年龄为：%d\n",user.age);
				printf("您的工号为：%d\n",user.jobnum);
			}
			break;
		default:
		printf("输入错误，请输入正确的操作码: 1 or 2 or 3\n");
		}

	}

	close(fd);
	return 0;
}
