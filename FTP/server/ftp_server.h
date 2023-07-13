#ifndef __FTP_SERVER_H__
#define __FTP_SERVER_H__

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>


#include"protocol.h"

struct CLI_INFO
{
	char ip[16];
	unsigned short port;
};

#define MAX_LISTEN 128

/*
sighandler:信号处理函数
sig:具体的信号值
返回值:
	无
*/
void sighandler(int sig);

/*
make_tcp_server:创建一个TCP服务器套接字
@IP:你的TCP服务器的地址
@port:你的TCP服务器的端口号
返回值：
	tcp的连接套接字
*/
int make_tcp_server(const char *ip,unsigned short port);

/*
handle_connection:处理和客户端的通信
confd:和客户端通信的连接套接字
@ap:客户端的通信地址
返回值:
	无
*/
void handle_connection(int confd,struct CLI_INFO *ap);

/*
resp_ls:回复客户端的ls请求
confd:和客户端通信的连接套接字
返回值:
	无
*/
void resp_ls(int confd);

/*
resp_get:回复客户端的get请求
confd:和客户端通信的连接套接字
filename:客户端需要获取的文件名
返回值:
	无
*/

void resp_get(int confd,char *filename);

/*
resp_put:回复客户端的put请求(读取推送文件的数据)
confd:和客户端通信的连接套接字
filename:客户端需要推送的文件名
file_size:客户端需要推送的文件大小
返回值:
	无
*/
void resp_put(int confd,char *filename,int file_size);	


#endif



