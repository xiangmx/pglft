#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "ts.h"
#include <sys/mman.h>
#include <math.h>
#include <dirent.h>
#include <string.h>


/*int get_touchscreen_index(void)
{
	int ts_fd = open("/dev/input/event0",O_RDONLY);
	if(ts_fd == -1)
	{
		perror("open ts_fd error");
	}
	int x1=0,y1=0;//x1,y1的起始坐标，表示x1,y1只记录一次
	struct input_event ev;//定义用来保存输入事件的信息的结构体
	while(1)
	{
		int r = read(ts_fd,&ev,sizeof(ev));
		
		int x2,y2;
		if(r != sizeof(ev))//如果读到的不是这个结构体的信息，就继续读
		{
			continue;
		}
		if(ev.type == EV_ABS && ev.code == ABS_X)
			//读到的是描述x轴的那个结构体
		{
			if(x1==0)
			{
				printf("**********\n");
				x1=ev.value;
			}
			x1=ev.value;
			x2=ev.value;
		}
		if(ev.type == EV_ABS && ev.code == ABS_Y)
			//读到的是描述y轴的那个结构体
		{
			if(y1==0)
			{
				y1=ev.value;
			}
			y2=ev.value;
		}
		if(ev.type == EV_KEY && ev.value == 0)//把触摸屏当作
						//按键，并且按键弹起(手指离开触摸屏)
		{
			printf("x1==%d,y1==%d,x2==%d,y2==%d\n",x1,y1,x2,y2);
			if(abs(x2-x1)>abs(y2-y1))//水平
			{
				if(x2<x1)//向右
				{
					return 3;
				}
				else
				{
					return 4;
				}
			}
			else//竖直
			{
				if(y2>y1)
				{
					return 2;
				}
				else
				{
					return 1;
				}
			}
		}
	}
}*/




//触摸屏分辨率1024*600
//返回值：
/*
	1：上
	2：下
	3：左
	4：右

*/
int get_touchscreen_index(int *x,int *y)
{
	int ts_fd = open("/dev/input/event0",O_RDONLY);
	if(ts_fd == -1)
	{
		perror("open ts_fd error");
	}
	struct input_event ev;//定义用来保存输入事件的信息的结构体
	int x1=-1,y1=-1;//x1,y1的起始坐标，表示x1,y1只记录一次
	int x2,y2;
	while(1)
	{
		int r = read(ts_fd,&ev,sizeof(ev));
		
		
		if(r != sizeof(ev))//如果读到的不是这个结构体的信息，就继续读
		{
			continue;
		}
		if(ev.type == EV_ABS && ev.code == ABS_X)
			//读到的是描述x轴的那个结构体
		{
			if(x1==-1)
			{
				x1=ev.value;
			}
			x2=ev.value;
			*x = ev.value;
		}
		if(ev.type == EV_ABS && ev.code == ABS_Y)
			//读到的是描述y轴的那个结构体
		{
			if(y1 == -1)
			{
				y1=ev.value;
			}
			y2=ev.value;
			*y = ev.value;
		}
		if(ev.type == EV_KEY && ev.value == 0)//把触摸屏当作
						//按键，并且按键弹起(手指离开触摸屏)
		{
			if(abs(x2-x1)>abs(y2-y1))//水平
			{
				if(x2<x1)//向右
				{
					return 3;
				}
				else
				{
					return 4;
				}
			}
			else//竖直
			{
				if(y2>y1)
				{
					return 2;
				}
				else
				{
					return 1;
				}
			}
			return -1;//说明是点击
		}
	}
	close(ts_fd);
	
}

Node *create_node(char *pathname)
{
    Node *p=malloc(sizeof(*p));
	p->next=NULL;
	p->prev=NULL;
	strncpy(p->s, pathname,9);
	return p;
}



void printf_list(Node*p)
{
    Node *over=p;
	while(over)
	{
	    printf("%s\n",over->s);
		over=over->next;
	}
}


Node* picture_cell(char*s)
{	
	 DIR*p=opendir(s);//打开目录
	 char op[100]="pl";
	 Node*z=create_node(op);
	 Node*x=z;
	if(p==NULL)
		perror("open error:");
	struct dirent *q=NULL;
	struct stat buf;//定义一个保存文件属性的结构体
	char filename[512];//用来保存目录下的文件名
	while(q=readdir(p))
	{
		sprintf(filename,"%s%s%s",s,"/",q->d_name);
		//printf("%s\n",filename);
		z->next=create_node(filename);
		z->next->prev=z;
		z=z->next;
	}
	x=x->next->next->next;
	z->next=x;
	x->prev=z;
	return x;
}







