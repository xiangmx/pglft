#ifndef __FTP_CILENT_H__
#define __FTP_CILENT_H__

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include"protocol.h"
#include <sys/stat.h>

/*
sighandler:信号处理函数
@sig:具体的信号值
返回值:
	无
*/
void sighandler(int sig);

/*
cmd_ls:处理客户端的ls请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
返回值:
	无
*/
void cmd_ls(int sockfd);


/*
cmd_get:处理客户端的get请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
@filename:客户端相要获取的文件名

返回值:
	无
*/

void cmd_get(int sockfd,char *filename);


/*
cmd_bye:处理客户端的bye请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
返回值:
	无
*/
void cmd_bye(int sockfd);

/*
cmd_get:处理客户端的put请求
@sockfd:客户端的套接字描述符,用来和服务器进行通信
@filename:客户端相要推送的文件名

返回值:
	无
*/
void cmd_put(int sockfd,char *filename);

#endif




