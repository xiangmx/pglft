#include "lcd.h"
#include "ts.h"
#include <stdlib.h>
#include <stdio.h>
int main(int argc,char *argv[])
{
	char *s = "ph";
	int fd = lcd_init();//打开屏幕
	//lcd_clear(0x00ffffff);
	//lcd_draw_ellipse(400, 240, atoi(argv[1]),atoi(argv[2]),0x00ff0000);
	//lcd_draw_triangle(0,0,600,438,640,180,0x00ff0000);
	//lcd_draw_star(400,240,100,0x0000ff00);

	//lcd_draw_bmppicture("ph/1.bmp",0,0);
	/*while(1)
		{
			int x,y;
		int r =get_touchscreen_index(&x,&y);
		printf("%d-----%d------derection:---%d\n",x,y,r);
		
		
		}
	*/
	Node*h= picture_cell(s);
	while(1)
	{
	 	lcd_draw_bmppicture(h->s,0,0);
		int x,y;
		int r =get_touchscreen_index(&x,&y);
		if(r==3||r==1)
			h=h->prev;
		else
			h=h->next;
	}
	
	lcd_uninit(fd); //关闭屏幕
}























