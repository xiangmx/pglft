#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

//服务器的“目录”
#define FTP_ROOT_DIR  "/home/china/ftpboot"

//命令号"整数"

enum CMD_NO
{
	FTP_CMD_LS=1024,
	FTP_CMD_GET,
	FTP_CMD_PUT,
	FTP_CMD_BYE
};
	
#endif

	
//协议的具体格式

/*
1.客户端发送的请求格式

	包头  pkg_len  cmd_no 	arg_1 arg_2	... 包尾
	
		参数??
			put,get的时候都需要说明文件的名字,长度...
	
	0xc0 包头:数据包的第一个字节,规定每一个数据包都已0xc0开头  1bytes
		在实际的应用中,
			1.为了保证包头的唯一性,一般会设置多个字节
			2.使用一种方法,保证数据中没有0xc0
	pkg_len  //4byte 表示我这个数据包的总长度(不包括包头包尾),以小端模式存放(先存放低字节)
	cmd_no 	//4byte 表示我这个数据包的请求,以小端模式存放(先存放低字节)
	arg_1 
			//arg_1_len  //4byte 表示第一个参数的长度,以小端模式存放(先存放低字节)
			//arg_1_data //占用arg_1_len个字节,表示参数的内容
	arg_2	
			//arg_2_len  //4byte 表示第二个参数的长度,以小端模式存放(先存放低字节)
			//arg_2_data //占用arg_2_len个字节,表示参数的内容
	...

	0xc0 包尾:数据包的最后一个字节,规定每一个数据包都已0xc0结尾 1bytes
	
	例子:
		发送ls请求
		0xc0 pkg_len cmd_no 0xc0
		
		char cmd[1024] = {0};//表示数据包
		int pkg_len = 8;
		enum CMD_NO cmd_no = FTP_CMD_LS;
		int i = 0;
		cmd[i++] = x0c0;//包头
		cmd[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
		cmd[i++] = (pkg_len >> 8) & 0xff;
		cmd[i++] = (pkg_len >> 16) & 0xff;
		cmd[i++] = (pkg_len >> 24) & 0xff;
		cmd[i++] = cmd_no & 0xff;//第一个字节(低字节) cmd_no
		cmd[i++] = (cmd_no >> 8) & 0xff;
		cmd[i++] = (cmd_no >> 16) & 0xff;
		cmd[i++] = (cmd_no >> 24) & 0xff;
		cmd[i++] = 0xc0;//包尾
		
		write(sockfd,cmd,i);
		
		=========================================================
		cmd:get filename
		0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) 0xc0
		
		char cmd[1024] = {0};//表示数据包
		int arg_1_len = strlen(filename); //参数长度
		int pkg_len = 4+4+4+arg_1_len;
		enum CMD_NO cmd_no = FTP_CMD_GET;
		int i = 0;
		
		//0xc0 
		cmd[i++] = x0c0;//包头
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
		cmd[i++] = 0xc0;	
		//0xc0
		
		write(sockfd,cmd,i);
		===============================================
		cmd:put filename
		
		0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) arg_1_data2(filesize) 0xc0
		发送文件的所有的内容
		
		cmd:byebye
			
	服务器一定需要按照这个方式去读取数据流
		
2.服务器回复的格式
	
	0xc0  pkg_len cmd_no resp_len result resp_conent 0xc0

	0xc0 包头:数据包的第一个字节,规定每一个数据包都已0xc0开头  1bytes
		在实际的应用中,
			1.为了保证包头的唯一性,一般会设置多个字节
			2.使用一种方法,保证数据中没有0xc0
	pkg_len  //4byte 表示我这个数据包的总长度(不包括包头包尾),以小端模式存放(先存放低字节)
	cmd_no 	//4byte 表示我这个数据包的请求,以小端模式存放(先存放低字节) 
	resp_len //4byte 表示我这个数据包的回复的内容长度,以小端模式存放(先存放低字节) 
	result  //1bytes 表示执行成功或者失败,0表示成功,1表示失败
	resp_conent //根据命令号有不同的回复
		成功:
			ls:所有的文件的名字列表,每一个文件名之间使用空格隔开
			get:回复一个文件大小,发完这个包后,就把文件所有的内容发送过去
			put:收到请求包后,就能够知道你要发送的文件的名字(filename)和大小(filesize)
				直接接受filesize个字节存入到名字叫做filename的文件
			bye:你都和我说BYEBYE了,直接关闭套接字,退出子进程
		失败:
			错误码(数字),程序员自己定义
	0xc0 包尾:数据包的最后一个字节,规定每一个数据包都已0xc0结尾 1bytes

	例子:
	ls的回复
	
	0xc0  pkg_len(4) cmd_no(4) resp_len(4) result(1) resp_conent(x) 0xc0
	
	unsigned char filelist[4096] = {0}; //保存了服务器目录中的文件列表
	unsigned char resp[8192] = {0};//回复数据包
	//构造数据包前,要提前准备好文件名(文件IO读目录,把目录中的文件名放到filelist中)
	int result = 0;//假设获取文件名成功
	int x = (result==0)?strlen(filelist):4; //所有文件列表的长度
	int pkg_len = 4+4+4+1+x; //整个数据包的长度
	int cmd_no = FTP_CMD_LS;
	int resp_len = 1+x;
	
	int i = 0;
	resp[i++] = 0xc0;//包头
	//pkg_len 
	resp[i++] = pkg_len & 0xff;//第一个字节(低字节) pkg_len
	resp[i++] = (pkg_len >> 8) & 0xff;
	resp[i++] = (pkg_len >> 16) & 0xff;
	resp[i++] = (pkg_len >> 24) & 0xff;
	//cmd_no(4) 
	resp[i++] = cmd_no & 0xff;//第一个字节(低字节) pkg_len
	resp[i++] = (cmd_no >> 8) & 0xff;
	resp[i++] = (cmd_no >> 16) & 0xff;
	resp[i++] = (cmd_no >> 24) & 0xff;
	//resp_len(4) 
	resp[i++] = resp_len & 0xff;//第一个字节(低字节) pkg_len
	resp[i++] = (resp_len >> 8) & 0xff;
	resp[i++] = (resp_len >> 16) & 0xff;
	resp[i++] = (resp_len >> 24) & 0xff;
	//result(1) 
	resp[i++] = result;
	//resp_conent(x) 
	if(result == 1) //失败发送错误码
	{
		resp[i++] = errno & 0xff;//第一个字节(低字节) pkg_len
		resp[i++] = (errno >> 8) & 0xff;
		resp[i++] = (errno >> 16) & 0xff;
		resp[i++] = (errno >> 24) & 0xff;
	}else //成功发送文件名列表
	{
		//strncpy(cmd+i,filelist,x)
		//cmd[i+x] = 0xc0;
		
		int j;
		for(j=0;j<x;j++)
		{
			cmd[i++] = filelist[j];
		}
	}
	
	//0xc0
	cmd[i++] = 0xc0;
	
	write(confd,resp,i);
	
	=============================================================================
	回复get:
	(1).0xc0  pkg_len(4) cmd_no(4) resp_len(4) result(1) resp_conent(filesize) 0xc0	
	(2).文件的所有内容
*/	


