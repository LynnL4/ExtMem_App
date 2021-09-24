/*
 * LCD.h
 *
 *  Created on: Sep 23, 2021
 *      Author: lht
 */

#ifndef BSP_LCD_LCD_H_
#define BSP_LCD_LCD_H_

#include "main.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

extern uint8_t  LCD_BUFFER[320 * 240 * 2];

enum LCD_ROTATION
{
    ROTATION_UP,
    ROTATION_LEFT,
    ROTATION_RIGHT,
    ROTATION_DOWN
};

void LCD_Clear(uint16_t color);
void LCD_Draw_Point(uint16_t x, uint16_t y, uint16_t data, uint8_t rotation);
void LCD_Draw_Image(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t *data, uint8_t rotation);
void LCD_Draw_Char(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode, uint8_t rotation);
void LCD_Draw_String(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p, uint8_t rotation);
void LCD_Frame_Draw_Image(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t *data, uint8_t rotation);
void LCD_Frame_Draw_Char(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode, uint8_t rotation);
void LCD_Frame_Draw_String(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p, uint8_t rotation);
void LCD_Frame_Show();

#endif /* BSP_LCD_LCD_H_ */
