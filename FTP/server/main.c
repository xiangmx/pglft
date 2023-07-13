#include<stdio.h>
#include"ftp_server.h"
#include <unistd.h>

volatile int terminate = 0;//结束标志，0为不退出,1为退出

//ip: argv[1] 
//port: argv[2]
int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		printf("please input the IP and Port!\n");
		exit(0);
	}
	//捕获一个信号(SIGCHLD),当子进程挂掉的时候去回收
	signal(SIGCHLD,sighandler);
	//捕获一个信号(SIGINT)Ctrl+C,把terminate置1,好处是:肯定可以完成当前连接
	//signal(SIGINT,sighandler);
	
	//创建一个TCP的服务器套接字
	int sockfd = make_tcp_server(argv[1],atoi(argv[2]));
	if(sockfd == -1)
	{
		printf("make_tcp_server failed\n");
		exit(0);
	}

	while(!terminate) //只要结束标志没有被设置
	{
		//接收新的客户端accept()
		struct sockaddr_in caddr; //保存客户端的地址(IP+port)
		memset(&caddr,0,sizeof(caddr));
		socklen_t len = sizeof(caddr);
		//客户端连接成功,返回一个连接套接字专门用来和客户端通信
		int confd = accept(sockfd,(struct sockaddr *)&caddr,&len);
		if(confd == -1)
		{
			perror("accept error");	
			continue;
		}
		
		//一个客户端连接成功.创建一个子进程去处理连接(IO复用,线程池,进程池)
		printf("client IP:%s,clientPort:%d\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
		//开辟空间,记录客户端的信息(IP+端口号)
		struct CLI_INFO *ap = (struct CLI_INFO *)malloc(sizeof(*ap));
		memset(ap,0,sizeof(struct CLI_INFO));
		strcpy(ap->ip,inet_ntoa(caddr.sin_addr));
		ap->port = ntohs(caddr.sin_port);
		//开一个子进程
		pid_t pid = fork(); 
		if(pid == 0) //子进程
		{
			//子进程处理新的连接(处理客户端的所有请求)
			close(sockfd);
			handle_connection(confd,ap);
			free(ap);
			exit(0); //触发SIGCHLD ----->回收资源的函数
		}else if(pid >0) //父进程,继续监听套接字,处理新的连接
		{
			free(ap);
			close(confd);//父进程只负责处理新的客户端连接
			continue;
		}
	}
	return 0;
}



