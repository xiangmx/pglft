#include"ftp_server.h"

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
		case SIGCHLD:
       		wait(NULL);  //阻塞的函数(回收子进程资源)
			break;
	}
}


/*
make_tcp_server:用来创建并且初始化一个TCP服务器
@ip:你创建的服务器绑定的IP地址
@port:你创建的服务器绑定的端口号
@listen_num:最大监听数量
返回值:
    成功返回创建好的套接字描述符
    失败返回-1,报错
*/
int make_tcp_server(const char *ip,unsigned short port)
{   
    //创建一个TCP套接字
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sock)
    {
        perror("socket error");
        return -1;
    }

    //绑定指定的服务器地址(本地的网卡地址)
    //a.描述一个服务器地址
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET; //设置地址协议族
    saddr.sin_port = htons(port); //端口号
    inet_aton(ip,&(saddr.sin_addr));
    //b.绑定,把一个指定的网络地址绑定到sock
    int r = bind(sock,(struct sockaddr *)&saddr,sizeof(saddr));
    if(-1 == r)
    {
        perror("bind error");
        return -1;
    }
    //开启对套接字的监听
    listen(sock,MAX_LISTEN);
    //返回初始化好的套接字描述符
    printf("init TCP server success!\n");
    return sock;
}


/*
handle_connection:处理和客户端的通信
confd:和客户端通信的连接套接字
返回值:
	无
*/
void handle_connection(int confd,struct CLI_INFO *ap)
{
    unsigned char cmd[1024] = {0};
    int i = 0;
    while(1)
    {
        memset(cmd,0,1024);
        //读取客户端发送给我的命令(按照协议读取数据)
        //找到第一个0xc0(用户数据中不能出现0xc0)
        int r;
        unsigned char ch;
        do
        {
            r = read(confd,&ch,1);
            if(r <= 0)
            {
                exit(0);
            }
        }while(ch != 0xc0);

        //防止上一次遗留的数据包的包尾
        while(ch == 0xc0)
        {
            r = read(confd,&ch,1);
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
            read(confd,&ch,1);
        }

        //做一个简单的校验(数据包长度校验)
        int pkg_len = (cmd[0]&0xff) | ((cmd[1] & 0xff) << 8) |
                     ((cmd[2] & 0xff) << 16) | ((cmd[3] & 0xff) << 24);
        if(i != pkg_len) //读取到的实际长度和理论长短不一样,直接丢弃
        {
            printf("pkg_len = %d,i = %d\n",pkg_len,i);
            continue;
        }
        
        //解析命令号,根据命令号去解析剩下的参数
        int cmd_no = (cmd[4]&0xff) | ((cmd[5] & 0xff) << 8) |
                     ((cmd[6] & 0xff) << 16) | ((cmd[7] & 0xff) << 24);

        //根据命令号做相应的回复
        if(cmd_no == FTP_CMD_LS)
        {
            //回复客户端的LS请求
            resp_ls(confd);
        }else if(cmd_no == FTP_CMD_GET)
        {
            //读取客户端需要的文件名
            //0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) 0xc0
            char filename[256] = {0};
            int arg_1_len = (cmd[8]&0xff) | ((cmd[9] & 0xff) << 8) |
                     ((cmd[10] & 0xff) << 16) | ((cmd[11] & 0xff) << 24);
            int j = 0;
            for(j=0;j<arg_1_len;j++)
            {
                filename[j] = cmd[12+j];
            }
            //回复客户端的请求
            printf("arg_1_len:%d\n",arg_1_len);
            printf("get file:%s\n",filename);
            resp_get(confd,filename);

        }else if(cmd_no == FTP_CMD_PUT)
        {
            //读取客户端需要推送的文件名和文件大小
            //0xc0  pkg_len cmd_no arg_1_len arg_1_data(filename) arg_1_data2(filesize) 0xc0
            char filename[256] = {0};
            int arg_1_len = (cmd[8]&0xff) | ((cmd[9] & 0xff) << 8) |
                     ((cmd[10] & 0xff) << 16) | ((cmd[11] & 0xff) << 24);
            int j = 0;
            for(j=0;j<arg_1_len;j++)
            {
                filename[j] = cmd[12+j];
            }
            printf("put file:%s\n",filename);
            //获取文件大小
            int filesize = (cmd[12+j]&0xff) | ((cmd[13+j] & 0xff) << 8) |
                     ((cmd[14+j] & 0xff) << 16) | ((cmd[15+j] & 0xff) << 24);
            printf("put file_size:%d\n",filesize);
            //接收文件的所有的内容
            resp_put(confd,filename,filesize); //仅仅是接受filesize个字节的内容存储到filename表示的文件

        }else if(cmd_no == FTP_CMD_BYE)
        {
            //把客户端信息和资源都回收
            printf("disconnect client IP:%s,client Port:%d\n",ap->ip,ap->port);
            close(confd);
            exit(0);
        }else 
        {

        }

    }
}


/*
resp_ls:回复客户端的ls请求
confd:和客户端通信的连接套接字
返回值:
	无
*/
void resp_ls(int confd)
{
	//0xc0  pkg_len(4) cmd_no(4) resp_len(4) result(1) resp_conent(x) 0xc0
    unsigned char filelist[4096] = {0}; //保存了服务器目录中的文件列表
    int result = 0;//假设获取文件名成功
    //获取文件名列表(打开目录/获取文件名列表)
    DIR *dir = opendir(FTP_ROOT_DIR);
    if(dir == NULL)
    {
        perror("opendir error");
        result = 1; //失败
    }
    struct dirent *dirp = NULL;
    int r = 0;
    while(dirp = readdir(dir))
    {
        r+=sprintf(filelist+r,"%s  ",dirp->d_name);
    }

    //准备回复数据包
	unsigned char resp[8192] = {0};
	//构造数据包前,要提前准备好文件名(文件IO读目录,把目录中的文件名放到filelist中)
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
			resp[i++] = filelist[j];
		}
	}
	//0xc0
	resp[i++] = 0xc0;
	//发送给客户端
	write(confd,resp,i);
}



/*
resp_get:回复客户端的get请求
confd:和客户端通信的连接套接字
filename:客户端需要获取的文件名
返回值:
	无
*/

void resp_get(int confd,char *filename)
{
    //发送数据包给客户端,告诉客户端需要获取的文件的大小
    //(1).0xc0  pkg_len(4) cmd_no(4) resp_len(4) result(1) resp_conent(filesize) 0xc0	
    //准备回复数据包
	unsigned char resp[20] = {0};  //回复数据包
    char result = 0;//假设获取成功
    //找到共享目录中对应的文件,获取文件大小
    unsigned char pathname[256] = {0};
    snprintf(pathname,256,"%s/%s",FTP_ROOT_DIR,filename);
    struct stat st;
    int r = lstat(pathname,&st);
    if(r == -1)
    {
        result = 1;//告诉客户端,过去失败
        perror("resp_get lstat failed");
    }
    int file_size = st.st_size;
	int pkg_len = 4+4+4+1+4; //整个数据包的长度
	int cmd_no = FTP_CMD_GET;
	int resp_len = 1+4;
	
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
	}else //成功发送文件大小
	{
		resp[i++] = file_size & 0xff;//第一个字节(低字节) pkg_len
	    resp[i++] = (file_size >> 8) & 0xff;
	    resp[i++] = (file_size >> 16) & 0xff;
	    resp[i++] = (file_size >> 24) & 0xff;
	}
	//0xc0
	resp[i++] = 0xc0;
	//发送给客户端
	write(confd,resp,i);

	//(2).文件的所有内容
    if(result == 0)
    {
        FILE *fp = fopen(pathname,"r");
        if(NULL == fp)
        {
            perror("resp_get:fopen error");
            return;
        }
        unsigned char buf[1024] = {0};
        while(1)
        {
            memset(buf,0,1024);
            size_t r = fread(buf,1,1024,fp);
            if(r > 0)
            {
                write(confd,buf,r);
            }else if(r == 0)
            {
                break;
            }else 
            {
                perror("resp_get:fread error");
                break;
            }
        }
        fclose(fp);
    }
}


/*
resp_put:回复客户端的put请求(读取推送文件的数据)
confd:和客户端通信的连接套接字
filename:客户端需要推送的文件名
file_size:客户端需要推送的文件大小
返回值:
	无
*/
void resp_put(int confd,char *filename,int file_size)
{
	unsigned char pathname[256] = {0};
    snprintf(pathname,256,"%s/%s",FTP_ROOT_DIR,filename);
    
    FILE *fp = fopen(pathname,"w+");
    if(NULL == fp)
    {
        perror("resp_put fopen failed!");
        return;
    }
    //包裹思想(recv/send------MSG_WAITALL)
    int cur_read = 0;
    while(cur_read < file_size)
    {
        unsigned char buf[1024] = {0};
        int r = read(confd,buf,1024);
        if(r > 0)
        {
            cur_read += r;
            fwrite(buf,1,r,fp);
        }
    }
    fclose(fp); 
}  







