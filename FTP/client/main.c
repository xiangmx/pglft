#include<stdio.h>
#include"ftp_client.h"
#include <unistd.h>

volatile int terminate = 0;//结束标志，0为不退出,1为退出
//server ip: argv[1] 
//server port: argv[2]
int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		printf("please input the IP and Port!\n");
		exit(0);
	}
	//捕获一个信号(SIGINT)Ctrl+C,把terminate置1,好处是肯定可以完成当前连接
	signal(SIGINT,sighandler);

	//1.创建一个套接字
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("create socket failed");
		exit(-1);
	}
	
	//连接目标服务器
	//a.描述服务的地址
	struct sockaddr_in saddr; //保存服务器的地址(IP+port)
	memset(&saddr,0,sizeof(struct sockaddr_in)); //清空结构体
	saddr.sin_family = AF_INET;
	inet_aton(argv[1], &saddr.sin_addr);
	saddr.sin_port = htons(atoi(argv[2]));
	//b.连接
	int ret = connect(sockfd, (struct sockaddr *)&saddr,sizeof(saddr));
	if(ret == -1)
	{
		perror("connect error");
		exit(-1);
	}
	printf("connect success\n");

	fprintf(stderr,"ftp>");
	char buf[256] = {0};
	while(!terminate)
	{
		memset(buf,0,256);
		//读取终端输入的命令(获取用户的请求)
		fgets(buf,256,stdin); //read(STDIN_FILENO,buf,256);
		
		//解析用户输入的命令,并且把命令按照私有协议的格式发送给服务器
		if((strncmp(buf,"ls",2) == 0) ||(strncmp(buf,"LS",2) == 0))
		{
			//处理ls请求
			cmd_ls(sockfd);
		}else if(strncmp(buf,"get",3) == 0)
		{
			//从buf中解析出文件名(filename)
			char filename[128] = {0};//保存文件名
			//把get后面的空格过滤掉
			int i = 3;
			while(buf[i] == ' ')
			{
				i++;
			}
			strcpy(filename,buf+i);
			filename[strlen(filename)-1] = 0;
			//printf("filename:%s\n",filename);
			
			//向服务器发起请求文件
			cmd_get(sockfd,filename);
		}else if(strncmp(buf,"put",3) == 0)
		{
			//从buf中解析出文件名(filename)
			char filename[128] = {0};//保存文件名
			//把put后面的空格过滤掉
			int i = 3;
			while(buf[i] == ' ')
			{
				i++;
			}
			strcpy(filename,buf+i);
			filename[strlen(filename)-1] = 0;
			//printf("filename:%s\n",filename);
			cmd_put(sockfd,filename);
			
		}else if(strncmp(buf,"bye",3) == 0)
		{
			cmd_bye(sockfd); //发送一个数据包给服务器,告诉他我断开了
			printf("Goodbye\n");
			break;
		}
		
		//等待服务器的回复
		fprintf(stderr,"ftp>");
	}
	//关闭套接字
	close(sockfd);
	return 0;
}

