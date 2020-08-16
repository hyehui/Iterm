#include "client.h"
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

	int flag = -1;          //功能选择标志位
	int ret = -1;
	int user_account[3] = {0};
	struct cli_users user;  //创建与客户端信息传递对象

	while(1){
		//登录 注册 退出 功能选择
		printf("*********************************************\n");
		printf("**请输入您的操作码：1>登录   2>退出  3>注册**\n");
		printf("*********************************************\n");
		scanf("%d",&flag);
		switch (flag){
		case 1://登录帐号
			ret = account_enter(&fd,&user);
			if(ret != 0){
				close(fd);
				return 0;
			}
			break;

		case 2://退出帐号
			account_quit(&fd,&user);
			return 0;
			break;

		case 3://注册帐号
			ret = account_register(&fd,&user);
			if(ret != 0){
				close(fd);
				return 0;
			}
			break;

		default:
			printf("输入错误，请输入正确的操作码: 1 or 2 or 3\n");
		}
	}
	close(fd);
	return 0;
}

int account_register(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：account_register
	 *   函数作用：帐号注册
	 *   返回值  ：成功返回0，服务器关闭返回-1
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/

	int ret = 0;
	char flag;

	int i = 0;
	while (1){
		user->opt =  REGISTER;
		printf("请输入您注册的帐号:");
		scanf("%d",&user->account);
		printf("请输入您的帐号密码:");
		scanf("%d",&user->code);


		//帐号密码信息发送服务器
		do{ 
			ret = send(*fd,user,sizeof(*user),0);
		}while(ret < 0 && EINTR == errno);
		if (ret == 0){
			printf("服务器关闭\n");
			return -1;
		}

		//接收服务器反馈信息
		do{
			ret = recv(*fd,user,sizeof(*user),0);
		}while(ret < 0 && EINTR == errno);

		if (ret == 0){
			printf("服务器关闭\n");
			return -1;
		}
		if (ret < 0){
			printf("account recv err\n");
			return -1;
		}

		//查看是否注册成功
		if(user->opt == REGISTERWIN){
			printf("注册成功,您的工号为:%d\n请填充基本信息\n",user->jobnum);
			break;
		}else{
			printf("注册失败,帐号已存在,重新注册输入r, 输入q退出\n");
			getchar();
			scanf("%c",&flag);
			if(flag == 'q'){
				return 0;
			}else if(flag == 'r'){
				continue;
			}else {
				printf("输入错误\n");
				return 0;
			}
		}
	}
	//帐号注册成功填充个人信息 
	ret = alter_mes(fd,user);
	if(ret < 0){
		return -1;
	}

	return 0;
}

int alter_mes(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：alter_mes
	 *   函数作用：添加或更改个人信息
	 *   返回值  ：成功返回0，服务器关闭返回-1
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/
	char *name = (char*)malloc(sizeof(name)); 
	int i = 0;
	int ret = 0;

	//输入个人信息，并填充信息传递结构体参数
	printf("请输入您的姓名:");
	scanf("%s",name);
	for(i = 0; name[i]; i++){
		user->name[i] = name[i];
	}
	user->name[i] = name[i];
	//printf("%s\n",user->name);//tioashi 

	printf("请输入您的年龄:");
	scanf("%d",&user->age);

	printf("请输入你的电话:");
	scanf("%d",&user->phone);

	//个人信息发送服务器
	user->opt = USERADD;
	do{
		ret = send(*fd,user,sizeof(*user),0);
	}while(ret < 0 && EINTR == errno);
	if (ret == 0){
		printf("服务器关闭\n");
		return -1;
	}

	//接收服务器反馈信息
	do{
		ret = recv(*fd,user,sizeof(*user),0);
	}while(ret < 0 && EINTR == errno);
	if (ret == 0){
		printf("服务器关闭\n");
		return -1;
	}
	if (ret < 0){
		printf("account recv err\n");
		return -1;
	}

	//填充成功，打印个人信息
	if(user->opt == ALTERMESWIN){

		putchar(10);
		printf("填充成功，您的基本信息:\n");
		printf("您的姓名为：%s\n",user->name);
		printf("您的年龄为：%d\n",user->age);
		printf("您的电话为：%d\n",user->phone);
		printf("您的工号为：%d\n",user->jobnum);
		putchar(10);
	}
	return 0;
}

int account_enter(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：account_enter
	 *   函数作用：登录函数
	 *   返回值  ：成功返回0，服务器关闭返回-1
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/

	int ret = 0;
	char flag;
	while(1){
		memset(user,0,sizeof(*user));
		user->opt = ENTER;
		printf("请输入您的数字帐号:");
		scanf("%d",&user->account);
		printf("请输入您的数字密码:");
		scanf("%d",&user->code);

		//帐号密码信息发送服务器
		do{
			ret = send(*fd,user,sizeof(*user),0);
		}while(ret < 0 && EINTR == errno);
		if (ret == 0){
			printf("服务器关闭\n");
			close(*fd);
			return -1;
		}
		//接收服务器反馈信息
		do{
			ret = recv(*fd,user,sizeof(*user),0);
		}while(ret < 0 && EINTR == errno);
		if (ret == 0){
			printf("服务器关闭\n");
			close(*fd);
			return -1;
		}

		//查看帐号密码是否正确
		if(user->opt != ENTERWIN){
			printf("帐号或密码错误,重试输入r, 输入q退出\n");
			getchar();
			scanf("%c",&flag);
			if(flag == 'q'){
				return 0;
			}else if(flag == 'r'){
				continue;
			}else {
				printf("输入错误\n");
				return 0;
			}
		}
		printf("登录成功\n");

		//登录成功，进行操作
		ret = operation(fd,user);
		if(ret == 0){
			return 0;
		}
	}
	return 0;
}

int operation_root(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：operation_root
	 *   函数作用：管理员帐号操作模式
	 *   返回值  ：0 退出
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/
	int flags = -1;
	int i = 0;
	char *department = (char*)malloc(sizeof(department));
	printf("*****************************************************************************\n");
	printf("**请输入您的操作码：1>查看用户信息   2>修改用户信息  3>删除用户信息  4>退出**\n");
	printf("*****************************************************************************\n");
	getchar();
	scanf("%d",&flags);


	switch(flags){
	case 1://查询用户信息
		user->opt = ROOT;

		printf("请输入要查询用户的帐号:");
		scanf("%d",&user->account);

		send(*fd,user,sizeof(*user),0);
		recv(*fd,user,sizeof(*user),0);
		
		if(user->opt == ERR){
			printf("查询信息失败，帐号不存在\n");
			break;
		}

		printf("该用户的姓名：%s\n",user->name);
		printf("该用户的年龄：%d\n",user->age);
		printf("该用户的电话：%d\n",user->phone);
		printf("该用户的帐号：%d\n",user->account);
		printf("该用户的密码：%d\n",user->code);
		printf("该用户的工号：%d\n",user->jobnum);
		printf("该用户的薪水：%d\n",user->salary);
		printf("该用户的部门：%s\n",user->department);
		putchar(10);
		break;

	case 2://修改用户信息
		user->opt = ROOTALT;
		
		printf("请输入要修改用户的帐号:");
		scanf("%d",&user->account);
		 
		printf("请输入该用户的薪资:");
		scanf("%d",&user->salary);

		printf("请输入该用户的部门:");
		scanf("%s",department);
		for(i = 0; department[i]; i++){
			user->department[i] = department[i];
		}
		user->department[i] = department[i];
		send(*fd,user,sizeof(*user),0);
		recv(*fd,user,sizeof(*user),0);
		if(user->opt == ROOTALTWIN){
			printf("修改信息成功\n");
		}else{
			printf("修改信息失败，帐号不存在\n");	
		}
		break;

	case 3://删除用户信息
		user->opt = ROOTDEL;
		printf("请输入要删除用户的帐号:");
		scanf("%d",&user->account);
		send(*fd,user,sizeof(*user),0);
		recv(*fd,user,sizeof(*user),0);
		printf("%d\n",user->opt);
		if(user->opt == ROOTDELWIN){
			printf("删除用户信息成功\n");
		}else{
		printf("删除用户失败，用户不存在\n");
		}
		break;

	case 4://退出
		return -1;

	default:
		printf("输入错误，请重新输入\n");
		break;
	}
	//printf("管理员模式正在更新中,请关注后续版本 。。。。。\n");
	return 0;
}

int operation_user(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：operation_user
	 *   函数作用: 用户帐号操作模式
	 *   返回值  ：-1 退出  0正常结束
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/
	int flags = -1;
	int code = -1;
	printf("*************************************************************************\n");
	printf("**请输入您的操作码：1>查询个人信息   2>修改个人信息  3>修改密码  4>退出**\n");
	printf("*************************************************************************\n");
	getchar();
	scanf("%d",&flags);
	switch(flags){
	case 1://查询个人信息
		user->opt = LOOKMES;

		send(*fd,user,sizeof(*user),0);
		recv(*fd,user,sizeof(*user),0);

		printf("您的姓名：%s\n",user->name);
		printf("您的年龄：%d\n",user->age);
		printf("您的电话：%d\n",user->phone);
		printf("您的工号：%d\n",user->jobnum);
		printf("您的薪水：%d\n",user->salary);
		printf("您的部门：%s\n",user->department);
		putchar(10);
		break;

	case 2://修改个人信息
		alter_mes(fd,user);		
		break;

	case 3://修改密码
		user->opt = ALTERCODE;

		printf("请输入原密码\n");
		getchar();
		scanf("%d",&code);

		if(code != user->code){
			printf("密码错误\n");
			break;
		}

		printf("请输入新密码\n");
		getchar();
		scanf("%d",&code);

		user->code = code;
		send(*fd,user,sizeof(*user),0);

		recv(*fd,user,sizeof(*user),0);
		if(user->opt == ALTERCODEWIN){
			printf("修改密码成功，请重新登录\n");
		}
		return -1;

	case 4://退出
		return -1;

	default:
		printf("输入错误，请重新输入\n");
		break;
	}
	return 0;
}

int operation(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：operation
	 *   函数作用：登录成功，操作函数
	 *   返回值  ：0退出
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/
	int ret = 0;
	int save_account = 0;
	while(1){
		if(user->account == 777){
			save_account = user->account;
			ret = operation_root(fd,user);
			if(ret == -1){
				break;
			}
			user->account= save_account;
		}else{
			ret = operation_user(fd,user);
			if(ret == -1){
				break;
			}	
		}
	}
	return 0;
}

void account_quit(int *fd,struct cli_users *user)
{
	/*================================================================
	 *   
	 *   函数名称：account_quit
	 *   函数作用：告诉服务器退出函数
	 *   返回值  ：无
	 *   创 建 者：HuYehui
	 *
	 ================================================================*/

	int ret = 0;
	user->opt = QUIT;
	do{
		ret = send(*fd,user,sizeof(*user),0);
	}while(ret < 0 && EINTR == errno);
}
