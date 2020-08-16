1.本服务器实现了员工的姓名、年龄、账号、密码、工号、工资、部门等信息的注册管理。
2.登录服务器端启动后，可以接受客户端发来的请求（登录，退出，注册，查询，修改等）并执行相应操作后，
   将相应的信息发送回客户端。
3.Makefile实现了软件的编译和中间文件的删除（不包括数据库文件）
   使用方法：（1）make  编译服务器server.c代码生成名为server的可执行文件
                  make clean 删除server可执行文件


4.实现原理：  （1）服务启动后 先建立好网络和数据库，等待客户端的连接请求
              （2）服务器与客户端连接请求建立好后，接收客户端发来的信息，并根据信息结构体中的opt操作数
                   识别对应的操作 操作数功能对应如下
                   #define ENTER        100        //登录
                   
#define ENTERWIN     1001       //登录成功
                   
#define QUIT         101 	   //退出
                   
#define REGISTER     102 	   //注册

                   #define REGISTERWIN  1021       //注册成功

                   #define USERADD      103        //注册填充信息
                   
#define USERADDWIN   1031       //填充信息成功     
                   
#define ERR          -1        
                   
#define LOOKMES      104        //查询个人信息
                   

#define ALTERMES     105        //修改个人信息
                   
#define ALTERMESWIN  1051       //修改个人信息成功
                   
#define ALTERCODE    106        //修改密码
                   
#define ALTERCODEWIN 1061       //
修改密码成功
                   #define ROOT         107        //超级用户功能
                   
#define ROOTWIN      1071       //超级功能成功
                   
#define ROOTDL       108        //超级用户删除用户信息

                   #define ROOTDLWIN    1081       //超级用户删除用户信息成功
                   
#define ROOTALT      109        //超级用户修改功能
                   
#define ROOTALTWIN   1091       //超级用户修改功能成功


 

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


1.1：登录，服务器收到的数据结构体中 user.opt=ENTER时，说明客户端发出登录请求。
		   user.account=账号， user.code=密码
		 服务器接收到数据后与数据库比对，若有此账号，向客户端发送宏 user.opt=ENTERWIN   //登录成功
		否则发送-1(ENTER_ERR)；
		登录失败：提示账号或者密码错误,请重试；	
		登陆成功：允许客户端1、查询个人信息，2、修改个人信息，3、修改密码
	
1.2：退出，服务器收到的数据结构体中 user.opt=QUIT时，说明客户端发出退出请求。

1.3：注册->user.opt=REGISTER  服务器收到的数据结构体中 user.opt=PEGISTER时，说明客户端发出退出请求。
                服务器接收到数据后，查看账号是否已被注册，
		若未被注册，user.opt=REGISTERWIN  分配工号，发送给客户端；
			客户端让用户填充姓名年龄信息，发送给服务器，服务器把收到的客户端个人信息保存到数据库中。
		已被注册，服务器发送操作码 ERR；

1.4  查看个人信息（opt=LOOKMES）：服务器收到请求后 ，从数据库中调取相应信息发送回客户端。

1.5  修改个人信息（opt=ALTERMES）：服务器收到请求后 ，把从客户端收到的信息保存进数据库

1.6  修改密码    （opt=ALTERCODE）：服务器收到请求后 ，把从客户端收到的信息中的新密码保存进数据库


2.1    超级用户功能类似，只是权限更大，可以查询修改删除所有用户的信息 



3.1 创建流式套接字

3.2创建服务器结构体对象： struct sockaddr_in sin

3.3填充服务端地址信息：
		sin.sinfamily = AF_INET;
		sin.sin_port    6666
		sin.sin_addr.s_addr  192.168.208.128  

3.4服务器受到客户端的连接请求： connect

3.5连接成功：等待接受客户端的 1登录，2退出，3注册 的操作
			#define ENTER     100    登录
			#define QUIT      101    退出
			#define REGISTER  102    注册
		
				
               