/*
 * LCD.c
 *
 *  Created on: Sep 23, 2021
 *      Author: lht
 */

#include "LCD.h"
#include "main.h"
#include "string.h"
#include "font.h"

EXTMEM uint8_t  LCD_BUFFER[320 * 240 * 2];
EXTMEM static uint8_t  LCD_FRAME_BUFFER[320 * 240 * 2];

void LCD_Draw_Point(uint16_t x, uint16_t y, uint16_t data, uint8_t rotation)
{
  switch (rotation)
  {
  case ROTATION_UP:
  {
	if(x > TFT_WIDTH) break;
	if(y > TFT_HEIGHT) break;
    *(volatile uint16_t *)((uint32_t)LCD_BUFFER + ((TFT_WIDTH * y) + x) * 2) = (uint16_t)(data);
    break;
  }
  case ROTATION_LEFT:
  {
	if(x > TFT_HEIGHT) break;
	if(y > TFT_WIDTH) break;
    *(volatile uint16_t *)((uint32_t)LCD_BUFFER + ((TFT_WIDTH * x) + TFT_WIDTH - y - 1) * 2) = (uint16_t)(data);
    break;
  }
  case ROTATION_RIGHT:
  {
  	if(x > TFT_HEIGHT) break;
  	if(y > TFT_WIDTH) break;
    *(volatile uint16_t *)((uint32_t)LCD_BUFFER + ((TFT_WIDTH * (TFT_HEIGHT - x - 1)) + y) * 2) = (uint16_t)(data);
    break;
  }
  case ROTATION_DOWN:
  {
  	if(x > TFT_WIDTH) break;
  	if(y > TFT_HEIGHT) break;
    *(volatile uint16_t *)((uint32_t)LCD_BUFFER + ((TFT_WIDTH * (TFT_HEIGHT - y - 1)) + TFT_WIDTH - x - 1) * 2) = (uint16_t)(data);
    break;
  }
  default:
    break;
  }
}

void LCD_Draw_Image(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t *data, uint8_t rotation)
{
	volatile uint16_t *temp = (uint16_t *)data;
	for(uint16_t y = start_y; y < height; y++)
		for(uint16_t x = start_x; x < width; x++)
			LCD_Draw_Point(x, y, *temp++, rotation);
}

void LCD_Clear(uint16_t color)
{
	for(uint16_t x = 0; x < TFT_WIDTH; x++){
		for(uint16_t y = 0; y < TFT_HEIGHT; y++){
			LCD_Draw_Point(x, y, color, ROTATION_UP);
		}
	}
}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24/32
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_Draw_Char(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode, uint8_t rotation)
{
		uint8_t temp,t1,t;
		uint16_t y0=y;
		uint8_t csize=(size/8+((size%8)?1:0))*(size/2);
	 	num=num-' ';
		for(t=0;t<csize;t++)
		{
			if(size==12)temp=asc2_1206[num][t];
			else if(size==16)temp=asc2_1608[num][t];
			else if(size==24)temp=asc2_2412[num][t];
			else if(size==32)temp=asc2_3216[num][t];
			else return;
			for(t1=0;t1<8;t1++)
			{
				if(temp&0x80)		LCD_Draw_Point(x,y,0xffff, rotation);
				else if(mode==0) 	LCD_Draw_Point(x,y,0x0000, rotation);
				temp<<=1;
				y++;
				if(rotation == ROTATION_UP || rotation ==  ROTATION_DOWN){
					if(y>=TFT_HEIGHT)return;
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=TFT_WIDTH)return;
						break;
					}
				}
				if(rotation == ROTATION_LEFT || rotation ==  ROTATION_RIGHT){
					if(y>=TFT_WIDTH)return;
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=TFT_HEIGHT)return;
						break;
					}
				}
			}
		}
}

//显示字符串
//x,y:起点坐标
//width,height:区域大小
//size:字体大小
//*p:字符串起始地址
void LCD_Draw_String(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p, uint8_t rotation)
{
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))
    {
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;
        LCD_Draw_Char(x,y,*p,size,0, rotation);
        x+=size/2;
        p++;
    }
}

void LCD_Frame_Draw_Point(uint16_t x, uint16_t y, uint16_t data, uint8_t rotation)
{
	switch (rotation)
	  {
	  case ROTATION_UP:
	  {
		if(x > TFT_WIDTH) break;
		if(y > TFT_HEIGHT) break;
	    *(volatile uint16_t *)((uint32_t)LCD_FRAME_BUFFER + ((TFT_WIDTH * y) + x) * 2) = (uint16_t)(data);
	    break;
	  }
	  case ROTATION_LEFT:
	  {
		if(x > TFT_HEIGHT) break;
		if(y > TFT_WIDTH) break;
	    *(volatile uint16_t *)((uint32_t)LCD_FRAME_BUFFER + ((TFT_WIDTH * x) + TFT_WIDTH - y - 1) * 2) = (uint16_t)(data);
	    break;
	  }
	  case ROTATION_RIGHT:
	  {
	  	if(x > TFT_HEIGHT) break;
	  	if(y > TFT_WIDTH) break;
	    *(volatile uint16_t *)((uint32_t)LCD_FRAME_BUFFER + ((TFT_WIDTH * (TFT_HEIGHT - x - 1)) + y) * 2) = (uint16_t)(data);
	    break;
	  }
	  case ROTATION_DOWN:
	  {
	  	if(x > TFT_WIDTH) break;
	  	if(y > TFT_HEIGHT) break;
	    *(volatile uint16_t *)((uint32_t)LCD_FRAME_BUFFER + ((TFT_WIDTH * (TFT_HEIGHT - y - 1)) + TFT_WIDTH - x - 1) * 2) = (uint16_t)(data);
	    break;
	  }
	  default:
	    break;
	  }
}

void LCD_Frame_Draw_Image(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t *data, uint8_t rotation)
{
	volatile uint16_t *temp = (uint16_t *)data;
	for(uint16_t y = start_y; y < height; y++)
		for(uint16_t x = start_x; x < width; x++)
			LCD_Frame_Draw_Point(x, y, *temp++, rotation);
}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24/32
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_Frame_Draw_Char(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode, uint8_t rotation)
{
		uint8_t temp,t1,t;
		uint16_t y0=y;
		uint8_t csize=(size/8+((size%8)?1:0))*(size/2);
	 	num=num-' ';
		for(t=0;t<csize;t++)
		{
			if(size==12)temp=asc2_1206[num][t];
			else if(size==16)temp=asc2_1608[num][t];
			else if(size==24)temp=asc2_2412[num][t];
			else if(size==32)temp=asc2_3216[num][t];
			else return;
			for(t1=0;t1<8;t1++)
			{
				if(temp&0x80)		LCD_Frame_Draw_Point(x,y,0xffff, rotation);
				else if(mode==0) 	LCD_Frame_Draw_Point(x,y,0x0000, rotation);
				temp<<=1;
				y++;
				if(rotation == ROTATION_UP || rotation ==  ROTATION_DOWN){
					if(y>=TFT_HEIGHT)return;
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=TFT_WIDTH)return;
						break;
					}
				}
				if(rotation == ROTATION_LEFT || rotation ==  ROTATION_RIGHT){
					if(y>=TFT_WIDTH)return;
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=TFT_HEIGHT)return;
						break;
					}
				}
			}
		}
}

//显示字符串
//x,y:起点坐标
//width,height:区域大小
//size:字体大小
//*p:字符串起始地址
void LCD_Frame_Draw_String(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p, uint8_t rotation)
{
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))
    {
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;
        LCD_Frame_Draw_Char(x,y,*p,size,1, rotation);
        x+=size/2;
        p++;
    }
}

void LCD_Frame_Show()
{
	memcpy((void *)LCD_BUFFER, (void *)LCD_FRAME_BUFFER, TFT_WIDTH * TFT_HEIGHT * 2);
}




