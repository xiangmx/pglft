#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "lcd.h"

#include <math.h>

#define PI 3.1415

int *plcd=NULL;

/*屏幕初始化的函数*/
int lcd_init(void)
{
	int fd = open("/dev/fb0",O_RDWR);//获取帧缓冲的文件描述符
	if(fd == -1)
	{
		perror("open freambuffer error");
	}
	plcd =mmap(NULL, 800*480*4,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);	
	if(plcd ==NULL)
	{
		perror("mmap error:");
		return -1;
	}
	return fd;
}

/*关闭屏幕*/
void lcd_uninit(int fd)
{
	close(fd);//关闭帧缓冲文件
	munmap(plcd,800*480*4);//解映射
}

void lcd_clear(int color)
{
	int i,j;
	for(j=0;j<480;j++)
	{
		for(i=0;i<800;i++)
		{
			lcd_draw_point(i,j,color);
		}
	}
}

/*在屏幕上的(x,y处)，写一个颜色为color的像素点*/
void lcd_draw_point(int x,int y,int color)
{
	if(x>=0 && x<800 && y>=0 && y<480)
	*(plcd+800*y+x)=color;//画点

	/*(x,y)表示上面有y行像素点，前面有x个像素点*/
}

/*在x,y处画一个半径为r，颜色为color的椭圆*/
void lcd_draw_circle(int x,int y,int r,int color)
{
	int i,j;
	for(j=0;j<480;j++)
	{
		for(i=0;i<800;i++)
		{
			if((i-x)*(i-x) + (j-y)*(j-y) <= r*r)//(i,j)这个点满足圆内的关系，就画点
			{
				lcd_draw_point(i,j,color);
			}
		}
	}
}
/*在x,y处画一个长轴为a，短轴为b，颜色为color的椭圆*/
void lcd_draw_ellipse(long long int x,long long int y,long long int a,long long int b,int color)
{
	int i,j;
	for(j=0;j<480;j++)
	{
		for(i=0;i<800;i++)
		{
			if((i-x)*(i-x)*b*b + (j-y)*(j-y)*a*a <= a*a*b*b)
			{
				lcd_draw_point(i,j,color);
			}
		}
	}
}

/*
	以(x1,y1)(x2,y2)(x3,y3)为顶点，画一个三角形
		三角形面积公式：S=0.5*|x1y2 + x2y3+x3y1-x2y1-x3y2-x1y3|
*/
void lcd_draw_triangle(int x1,int y1,int x2,int y2,int x3,int y3,int color)
{
	int i,j;
	for(j=0;j<480;j++)
	{
		for(i=0;i<800;i++)
		{
			int s1=abs(x1*y2 + x2*j+i*y1-x2*y1-i*y2-x1*j);
			int s2=abs(x1*j + i*y3+x3*y1-i*y1-x3*j-x1*y3);
			int s3=abs(i*y2 + x2*y3+x3*j-x2*j-x3*y2-i*y3);
			int sbig=abs(x1*y2 + x2*y3+x3*y1-x2*y1-x3*y2-x1*y3);
			if(s1+s2+s3 == sbig)
			{
				lcd_draw_point(i,j,color);
			}
		}
	}

}

void lcd_draw_star(int x0,int y0, int a,int color)
{
	lcd_draw_triangle(0+x0,a*cos(PI/10)+y0,-a*cos(PI/5)+x0,-a*cos(PI/10)-a*sin(PI/5)+y0,a*sin(PI/10)+a-a*cos(PI/5)+x0,-a*sin(PI/5)+y0,color);//acz
	lcd_draw_triangle(-a*sin(PI/10)-a+x0,0+y0,a*sin(PI/10)+x0,0+y0,a*cos(PI/5)+x0,-a*cos(PI/10)-a*sin(PI/5)+y0,color);//bud
	lcd_draw_triangle(-a*sin(PI/10)+x0,0+y0,-a*cos(PI/5)+x0,-a*cos(PI/10)-a*sin(PI/5)+y0,a*sin(PI/10)+a+x0,0+y0,color);//XCE
}



/*在(x,y)坐标处显示一张路径为pathname的图片*/
void lcd_draw_bmppicture(char *pathname,int x0,int y0)
{
	int bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		perror("open bmp error");
	}

	int width,height;//定义宽，高
	short depth;//色深

	//光标偏移到18字节处，读宽
	lseek(bmp_fd,18,SEEK_SET);
	read(bmp_fd,&width,4);
	printf("width == %d\n",width);

	//光标偏移到22字节处，读高
	read(bmp_fd,&height,4);
	printf("height === %d\n",height);

	//光标偏移到28字节处，读色深
	lseek(bmp_fd,28,SEEK_SET);
	read(bmp_fd,&depth,2);
	printf("depth==%d\n",depth);

	//计算像素数组的大小
	
	int laizi=0;//表示无效字节数
	if((width *depth /8) %4)
	{
		laizi = 4- (width *depth /8)%4;
	}
	//总字节数
	int total = (width *depth/8 +laizi)*height;

	unsigned char piexe[total];//定义像素数组

	lseek(bmp_fd,54,SEEK_SET);
	read(bmp_fd,piexe,total);//读像素数组的信息
	int i,j,num=0;
	unsigned char a=0,r,g,b;
	for(j=0;j<abs(height);j++)
	{
		for(i=0;i<abs(width);i++)//遍历一行像素点
		{
			b=piexe[num++];
			g=piexe[num++];
			r=piexe[num++];
			if(depth == 32)
			{
				a=piexe[num++];
			}
			int color = b |g<<8 |r<<16 |a<<24;

			int x,y;//x0,y0代表画点实际要画的位置
			x = width>0 ? x0 +i:abs(width)-1-i+x0;
			y = height<0? y0 +j:abs(height)-1-j+y0;
			lcd_draw_point(x,y,color);
		}
		num+=laizi;//跳过每行末尾的癞子
	}	
}


























