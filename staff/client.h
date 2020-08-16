#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>                                                
#include <sqlite3.h>                                              
#include <string.h>    


#define SERV_IP   "192.168.43.185"
#define SERV_PORT 6666
#define DATABASE "users.db"

#define ENTER        100        //登录
#define ENTERWIN     1001       //登录成功
#define QUIT         101 		//退出
#define REGISTER     102 		//注册
#define REGISTERWIN  1021       //注册成功
#define USERADD      103        //注册填充信息
#define USERADDWIN   1031       //填充信息成功     
#define LOOKMES      104        //查询个人信息
#define ALTERMES     105        //修改个人信息
#define ALTERMESWIN  1051       //修改个人信息成功
#define ALTERCODE    106        //修改密码
#define ALTERCODEWIN 1061       //修改密码成功
#define ROOT         107        //管理员修改工资，部门
#define ROOTWIN      1071       //修改成功
#define ROOTDEL      108        //删除用户信息
#define ROOTDELWIN   1081       //删除用户信息成功
#define ROOTALT      109        //管理员修改用户信息
#define ROOTALTWIN   1091       //管理员修改用户信息成功
#define ERR          -1         //操作失败 





//客户端用户信息结构题 
struct  cli_users{	
	int  opt;            //操作数
	char name[20];       //姓名
	int  age; 	         //年龄
	int  account;        //账号
	int  code;           //密码
	int  jobnum;         //工号
	int  salary;         //工资
	char department[20]; //部门
	int  phone;          //电话            

};

int account_register(int *, struct cli_users * );
int callback(void*para,int f_num,char ** f_value,char ** f_name);
void account_quit(int *fd,struct cli_users *user);
int alter_mes(int *fd,struct cli_users *user);
int operation(int *fd,struct cli_users *user);
int operation_root(int *fd,struct cli_users *user);
int operation_user(int *fd,struct cli_users *user);


#endif
