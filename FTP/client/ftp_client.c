#include"ftp_client.h"

extern int terminate;
/*
sighandler:信号处理函数
sig:具体的信号值
返回值:
	无
*/
void sighandler(int sig)
{
	switch (sig)
	{
		case SIGINT:
			terminate = 1;
			break;
		//case SIGCHLD:
       		//wait(NULL);
			//break;
	}
}

/*
cmd_ls:处理用户的ls请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
返回值:
	无
*/
void cmd_ls(int sockfd)
{
	//发送ls请求到服务器
	char cmd[1024] = {0};//表示数据包
	int pkg_len = 8;
	enum CMD_NO cmd_no = FTP_CMD_LS;
	int i = 0;
	cmd[i++] = 0xc0;//包头
	cmd[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
	cmd[i++] = (pkg_len >> 8) & 0xff;
	cmd[i++] = (pkg_len >> 16) & 0xff;
	cmd[i++] = (pkg_len >> 24) & 0xff;
	cmd[i++] = cmd_no & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (cmd_no >> 8) & 0xff;
	cmd[i++] = (cmd_no >> 16) & 0xff;
	cmd[i++] = (cmd_no >> 24) & 0xff;
	cmd[i++] = 0xc0;//包尾
	
	int r = write(sockfd,cmd,i);
	if(r != i)
	{
		printf("send ls cmd failed!\n");
		return;
	}
	//接受服务器的回复,并且解析
	memset(cmd,0,1024);
	//读取服务器发送给我的命令(按照协议读取数据),找到第一个0xc0(用户数据中不能出现0xc0)
	unsigned char ch;
	do
	{
		r = read(sockfd,&ch,1);
		if(r <= 0)
		{
			exit(0);
		}
	}while(ch != 0xc0);

	//防止上一次遗留的数据包的包尾
	while(ch == 0xc0)
	{
		r = read(sockfd,&ch,1);
		if(r <= 0)
		{
			exit(0);
		}
	}
	//ch肯定不是0xc0,是数据包中的第一个有效字节
	i = 0;
	while(ch != 0xc0) //一次性把数据包读取到cmd中
	{
		cmd[i++] = ch;
		read(sockfd,&ch,1);
	}

	//做一个简单的校验(数据包长度校验)
	pkg_len = (cmd[0]&0xff) | ((cmd[1] & 0xff) << 8) |
					((cmd[2] & 0xff) << 16) | ((cmd[3] & 0xff) << 24);
	if(i != pkg_len) //读取到的实际长度和理论长短不一样,直接丢弃
	{
		printf("pkg_len = %d,i = %d\n",pkg_len,i);
		return;
	}
	
	//解析命令号,根据命令号去解析剩下的参数
	cmd_no = (cmd[4]&0xff) | ((cmd[5] & 0xff) << 8) |
					((cmd[6] & 0xff) << 16) | ((cmd[7] & 0xff) << 24);
	if(cmd_no != FTP_CMD_LS)
	{
		printf("FTP_CMD_LS::cmd_no = %d\n",cmd_no);
	}

	if(cmd[12] != 0) //如果获取失败,解析服务器的错误码
	{
		printf("FTP_CMD_LS failed!\n");
		int errnum = (cmd[13]&0xff) | ((cmd[14] & 0xff) << 8) |
					((cmd[15] & 0xff) << 16) | ((cmd[16] & 0xff) << 24);
		printf("errno = %d\n",errnum);
	}else 
	{
		printf("%s\n",cmd+13);
	}
}


/*
cmd_get:处理客户端的get请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
@filename:客户端相要获取的文件名

返回值:
	无
*/

void cmd_get(int sockfd,char *filename)
{
	//按照私有协议给服务器发送请求
	//0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) 0xc0
		
	char cmd[1024] = {0};//表示发送的数据包
	int arg_1_len = strlen(filename); //参数长度
	int pkg_len = 4+4+4+arg_1_len;
	enum CMD_NO cmd_no = FTP_CMD_GET;
	int i = 0;
	//0xc0 
	cmd[i++] = 0xc0;//包头
	//pkg_len 
	cmd[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
	cmd[i++] = (pkg_len >> 8) & 0xff;
	cmd[i++] = (pkg_len >> 16) & 0xff;
	cmd[i++] = (pkg_len >> 24) & 0xff;
	//cmd_no 
	cmd[i++] = cmd_no & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (cmd_no >> 8) & 0xff;
	cmd[i++] = (cmd_no >> 16) & 0xff;
	cmd[i++] = (cmd_no >> 24) & 0xff;
	//arg_1_len 
	cmd[i++] = arg_1_len & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (arg_1_len >> 8) & 0xff;
	cmd[i++] = (arg_1_len >> 16) & 0xff;
	cmd[i++] = (arg_1_len >> 24) & 0xff;
	//arg_1_data(filename) 
	//strncpy(cmd+i,filename,arg_1_len)
	//cmd[i+arg_1_len] = 0xc0;
	int j;
	for(j=0;j<arg_1_len;j++)
	{
		cmd[i++] = filename[j];
	}
	//包尾
	cmd[i++] = 0xc0;	
	
	int ret = write(sockfd,cmd,i);
	if(ret != i)
	{
		printf("send get_cmd failed!\n");
		return;
	}
	//等待服务器的回复
	//(1).0xc0  pkg_len(4) cmd_no(4) resp_len(4) result(1) resp_conent(filesize) 0xc0
	memset(cmd,0,1024);
	//读取服务器发送给我的命令(按照协议读取数据),找到第一个0xc0(用户数据中不能出现0xc0)
	unsigned char ch;
	int r;
	do
	{
		r = read(sockfd,&ch,1);
		if(r <= 0)
		{
			exit(0);
		}
	}while(ch != 0xc0);

	//防止上一次遗留的数据包的包尾
	while(ch == 0xc0)
	{
		r = read(sockfd,&ch,1);
		if(r <= 0)
		{
			exit(0);
		}
	}
	//ch肯定不是0xc0,是数据包中的第一个有效字节
	i = 0;
	while(ch != 0xc0) //一次性把数据包读取到cmd中
	{
		cmd[i++] = ch;
		read(sockfd,&ch,1);
	}

	//做一个简单的校验(数据包长度校验)
	pkg_len = (cmd[0]&0xff) | ((cmd[1] & 0xff) << 8) |
					((cmd[2] & 0xff) << 16) | ((cmd[3] & 0xff) << 24);
	if(i != pkg_len) //读取到的实际长度和理论长短不一样,直接丢弃
	{
		printf("pkg_len = %d,i = %d\n",pkg_len,i);
		return;
	}
	
	//解析命令号,根据命令号去解析剩下的参数
	cmd_no = (cmd[4]&0xff) | ((cmd[5] & 0xff) << 8) |
			 ((cmd[6] & 0xff) << 16) | ((cmd[7] & 0xff) << 24);
	if(cmd_no != FTP_CMD_GET)
	{
		printf("FTP_CMD_LS::cmd_no = %d\n",cmd_no);
	}

	if(cmd[12] != 0) //如果获取失败,解析服务器的错误码
	{
		printf("FTP_CMD_GET failed!\n");
		int errnum = (cmd[13]&0xff) | ((cmd[14] & 0xff) << 8) |
					((cmd[15] & 0xff) << 16) | ((cmd[16] & 0xff) << 24);
		printf("errno = %d\n",errnum);
	}else  //获取成功,准备接受文件的所有内容
	{
		//(2).文件的所有内容
		int recv_size = (cmd[13]&0xff) | ((cmd[14] & 0xff) << 8) |
					((cmd[15] & 0xff) << 16) | ((cmd[16] & 0xff) << 24);
		FILE *fp = fopen(filename,"w+");
		if(fp == NULL)
		{
			perror("cmd_get:fopen file failed!\n");
			return;
		}
		int cur_read = 0;
		while(cur_read < recv_size)
		{
			unsigned char buf[1024] = {0};
			int r = read(sockfd,buf,1024);
			if(r>0)
			{
				cur_read += r;
				fwrite(buf,1,r,fp);
			}
		}
		fclose(fp);
	}
	return;
}


/*
cmd_bye:处理客户端的bye请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
返回值:
	无
*/
void cmd_bye(int sockfd)
{
	//发送bye请求到服务器
	char cmd[1024] = {0};//表示数据包
	int pkg_len = 8;
	enum CMD_NO cmd_no = FTP_CMD_BYE;
	int i = 0;
	cmd[i++] = 0xc0;//包头
	cmd[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
	cmd[i++] = (pkg_len >> 8) & 0xff;
	cmd[i++] = (pkg_len >> 16) & 0xff;
	cmd[i++] = (pkg_len >> 24) & 0xff;
	cmd[i++] = cmd_no & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (cmd_no >> 8) & 0xff;
	cmd[i++] = (cmd_no >> 16) & 0xff;
	cmd[i++] = (cmd_no >> 24) & 0xff;
	cmd[i++] = 0xc0;//包尾
	
	int r = write(sockfd,cmd,i);
	if(r != i)
	{
		printf("send byecmd failed!\n");
		return;
	}
}
/*
cmd_get:处理客户端的put请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
@filename:客户端相要推送的文件名
返回值:
	无
*/
void cmd_put(int sockfd,char *filename)
{
	//0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) arg_1_data2(filesize) 0xc0
	char cmd[1024] = {0};//表示发送的数据包
	//获取文件大小
	struct stat st;
	int r = lstat(filename,&st);
	if(r == -1)
	{
		perror("cmd_put lstat failed!");
		return;
	}
	int arg_1_len = strlen(filename); //参数长度
	int pkg_len = 4+4+4+arg_1_len+4;
	enum CMD_NO cmd_no = FTP_CMD_PUT;
	int file_size = st.st_size;

	int i = 0;
	//0xc0 
	cmd[i++] = 0xc0;//包头
	//pkg_len 
	cmd[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
	cmd[i++] = (pkg_len >> 8) & 0xff;
	cmd[i++] = (pkg_len >> 16) & 0xff;
	cmd[i++] = (pkg_len >> 24) & 0xff;
	//cmd_no 
	cmd[i++] = cmd_no & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (cmd_no >> 8) & 0xff;
	cmd[i++] = (cmd_no >> 16) & 0xff;
	cmd[i++] = (cmd_no >> 24) & 0xff;
	//arg_1_len 
	cmd[i++] = arg_1_len & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (arg_1_len >> 8) & 0xff;
	cmd[i++] = (arg_1_len >> 16) & 0xff;
	cmd[i++] = (arg_1_len >> 24) & 0xff;
	//文件名
	int j;
	for(j=0;j<arg_1_len;j++)
	{
		cmd[i++] = filename[j];
	}
	//文件大小
	cmd[i++] = file_size & 0xff;//第一个字节(低字节) cmd_no
	cmd[i++] = (file_size  >> 8) & 0xff;
	cmd[i++] = (file_size  >> 16) & 0xff;
	cmd[i++] = (file_size  >> 24) & 0xff;
	//包尾
	cmd[i++] = 0xc0;	
	int ret = write(sockfd,cmd,i);
	if(ret != i)
	{
		printf("cmd_put send failed!\n");
		return;
	}

	//发送文件的所有的内容
	FILE *fp = fopen(filename,"r");
	if(NULL == fp)
	{
		perror("cmd_put fopen error");
		return;
	}
	unsigned char buf[1024] = {0};
	while(1) //循环发送内容
	{
		memset(buf,0,1024);
		size_t r = fread(buf,1,1024,fp);
		if(r > 0)
		{
			write(sockfd,buf,r);
		}else if(r == 0) 
		{
			break;
		}else 
		{
			perror("cmd_put fread error");
			break;
		}
	}
	fclose(fp);
}




