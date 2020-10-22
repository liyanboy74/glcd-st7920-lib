/*
 * ST7920_SERIAL.c
 *
 *  Created on: 07-Jun-2019
 *      Author: poe
 *
 *	Edit By	:		Liyanboy74
 *	Date	:	21-July-2020
 */
 
#include "main.h"
#include "ST7920_SERIAL.h"
#include "font.h"

#define SCLK_PIN GLCD_SCK_Pin
#define SCLK_PORT GLCD_SCK_GPIO_Port
#define CS_PIN GLCD_CS_Pin
#define CS_PORT GLCD_CS_GPIO_Port
#define SID_PIN GLCD_SID_Pin
#define SID_PORT GLCD_SID_GPIO_Port
#define RST_PIN GLCD_RST_Pin
#define RST_PORT GLCD_RST_GPIO_Port

uint8_t startRow, startCol, endRow, endCol; // coordinates of the dirty rectangle
uint8_t numRows = 64;
uint8_t numCols = 128;
uint8_t Graphic_Check = 0;

//GLCD Buf
uint8_t GLCD_Buf[1024];

void delay_us(uint32_t Time_us)
{
	//uint16_t i;
	while(Time_us--);//for(i=0;i<0xf;i++);
}

// A replacement for SPI_TRANSMIT

void SendByteSPI(uint8_t byte)
{
	for(int i=0;i<8;i++)
	{
		if((byte<<i)&0x80)
			{
				HAL_GPIO_WritePin(SID_PORT, SID_PIN, GPIO_PIN_SET);  // SID=1  OR MOSI
			}

		else HAL_GPIO_WritePin(SID_PORT, SID_PIN, GPIO_PIN_RESET);  // SID=0

		HAL_GPIO_WritePin(SCLK_PORT, SCLK_PIN, GPIO_PIN_RESET);  // SCLK =0  OR SCK

		HAL_GPIO_WritePin(SCLK_PORT, SCLK_PIN, GPIO_PIN_SET);  // SCLK=1

	}
}

void ST7920_SendCmd (uint8_t cmd)
{

	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);  // PUll the CS high

	SendByteSPI(0xf8+(0<<1));  // send the SYNC + RS(0)
	SendByteSPI(cmd&0xf0);  // send the higher nibble first
	SendByteSPI((cmd<<4)&0xf0);  // send the lower nibble
	delay_us(50);

	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);  // PUll the CS LOW

}

void ST7920_SendData (uint8_t data)
{

	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);  // PUll the CS high

	SendByteSPI(0xf8+(1<<1));  // send the SYNC + RS(1)
	SendByteSPI(data&0xf0);  // send the higher nibble first
	SendByteSPI((data<<4)&0xf0);  // send the lower nibble
	delay_us(50);
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);  // PUll the CS LOW
}

void ST7920_SendString(int row, int col, char* string)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0x90;
            break;
        case 2:
            col |= 0x88;
            break;
        case 3:
            col |= 0x98;
            break;
        default:
            col |= 0x80;
            break;
    }

    ST7920_SendCmd(col);

    while (*string)
    	{
    		ST7920_SendData(*string++);
    	}
}
// switch to graphic mode or normal mode::: enable = 1 -> graphic mode enable = 0 -> normal mode

void ST7920_GraphicMode (int enable)   // 1-enable, 0-disable
{
	if (enable == 1)
	{
		ST7920_SendCmd(0x30);  // 8 bit mode
		HAL_Delay (1);
		ST7920_SendCmd(0x34);  // switch to Extended instructions
		HAL_Delay (1);
		ST7920_SendCmd(0x36);  // enable graphics
		HAL_Delay (1);
		Graphic_Check = 1;  // update the variable
	}

	else if (enable == 0)
	{
		ST7920_SendCmd(0x30);  // 8 bit mode
		HAL_Delay (1);
		Graphic_Check = 0;  // update the variable
	}
}

void ST7920_DrawBitmap(const unsigned char* graphic)
{
	
	uint8_t x, y;
	
	uint16_t Index=0;
	uint8_t Temp,Db;
	
	for(y=0;y<64;y++)
	{
		for(x=0;x<8;x++)
		{
			if(y<32)//Up
			{
				ST7920_SendCmd(0x80 | y);										//Y(0-31)
				ST7920_SendCmd(0x80 | x);										//X(0-8)
			}
			else
			{
				ST7920_SendCmd(0x80 | y-32);//Y(0-31)
				ST7920_SendCmd(0x88 | x);//X(0-8)
			}
			
			Index=((y/8)*128)+(x*16);
			Db=y%8;			
			
			Temp=	(((graphic[Index+0]>>Db)&0x01)<<7)|
						(((graphic[Index+1]>>Db)&0x01)<<6)|
						(((graphic[Index+2]>>Db)&0x01)<<5)|
						(((graphic[Index+3]>>Db)&0x01)<<4)|
						(((graphic[Index+4]>>Db)&0x01)<<3)|
						(((graphic[Index+5]>>Db)&0x01)<<2)|
						(((graphic[Index+6]>>Db)&0x01)<<1)|
						(((graphic[Index+7]>>Db)&0x01)<<0);
			ST7920_SendData(Temp);
			
			Temp=	(((graphic[Index+8]>>Db)&0x01)<<7)|
						(((graphic[Index+9]>>Db)&0x01)<<6)|
						(((graphic[Index+10]>>Db)&0x01)<<5)|
						(((graphic[Index+11]>>Db)&0x01)<<4)|
						(((graphic[Index+12]>>Db)&0x01)<<3)|
						(((graphic[Index+13]>>Db)&0x01)<<2)|
						(((graphic[Index+14]>>Db)&0x01)<<1)|
						(((graphic[Index+15]>>Db)&0x01)<<0);
			
			ST7920_SendData(Temp);
		}
	}		
}

//Clear GLCD Buf
void GLCD_Buf_Clear(void)
{
	uint16_t i;
	for(i=0;i<1024;i++)GLCD_Buf[i]=0;
}

// Update the display with the selected graphics
void ST7920_Update(void)
{
	ST7920_DrawBitmap(GLCD_Buf);
}

void ST7920_Clear()
{
	if (Graphic_Check == 1)  // if the graphic mode is set
	{
		uint8_t x, y;
		for(y = 0; y < 64; y++)
		{
			if(y < 32)
			{
				ST7920_SendCmd(0x80 | y);
				ST7920_SendCmd(0x80);
			}
			else
			{
				ST7920_SendCmd(0x80 | (y-32));
				ST7920_SendCmd(0x88);
			}
			for(x = 0; x < 8; x++)
			{
				ST7920_SendData(0);
				ST7920_SendData(0);
			}
		}
		GLCD_Buf_Clear();
	}

	else
	{
		ST7920_SendCmd(0x01);   // clear the display using command
		HAL_Delay(2); // delay >1.6 ms
	}
}

void ST7920_Init (void)
{
	HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_RESET);  // RESET=0
	HAL_Delay(10);   // wait for 10ms
	HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);  // RESET=1

	HAL_Delay(50);   //wait for >40 ms


	ST7920_SendCmd(0x30);  // 8bit mode
	delay_us(110);  //  >100us delay

	ST7920_SendCmd(0x30);  // 8bit mode
	delay_us(40);  // >37us delay

	ST7920_SendCmd(0x08);  // D=0, C=0, B=0
	delay_us(110);  // >100us delay

	ST7920_SendCmd(0x01);  // clear screen
	HAL_Delay(12);  // >10 ms delay


	ST7920_SendCmd(0x06);  // cursor increment right no shift
	HAL_Delay(1);  // 1ms delay

	ST7920_SendCmd(0x0C);  // D=1, C=0, B=0
    HAL_Delay(1);  // 1ms delay

	ST7920_SendCmd(0x02);  // return to home
	HAL_Delay(1);  // 1ms delay
}

// Set Pixel
void SetPixel(uint8_t x, uint8_t y)
{
  if (y < numRows && x < numCols)
  {
		GLCD_Buf[(x)+((y/8)*128)]|=0x01<<y%8;

    // Change the dirty rectangle to account for a pixel being dirty (we assume it was changed)
    if (startRow > y) { startRow = y; }
    if (endRow <= y)  { endRow = y + 1; }
    if (startCol > x) { startCol = x; }
    if (endCol <= x)  { endCol = x + 1; }
  }
}

// Clear Pixel
void ClearPixel(uint8_t x, uint8_t y)
{
  if (y < numRows && x < numCols)
  {
		GLCD_Buf[(x)+((y/8)*128)]&=(~(0x01<<y%8));

    // Change the dirty rectangle to account for a pixel being dirty (we assume it was changed)
    if (startRow > y) { startRow = y; }
    if (endRow <= y)  { endRow = y + 1; }
    if (startCol > x) { startCol = x; }
    if (endCol <= x)  { endCol = x + 1; }
  }
}

// Toggle Pixel
void TogglePixel(uint8_t x, uint8_t y)
{
  if (y < numRows && x < numCols)
  {
		if((GLCD_Buf[(x)+((y/8)*128)]>>y%8)&0x01)
			ClearPixel(x,y);
		else
			SetPixel(x,y);
  }
}
/* draw a line
 * start point (X0, Y0)
 * end point (X1, Y1)
 */
void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
  int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  for (;;)
  {
    SetPixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = err + err;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

/* Draw rectangle
 * start point (x,y)
 * w -> width
 * h -> height
 */
void DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	/* Check input parameters */
	if (
		x >= numCols ||
		y >= numRows
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= numCols) {
		w = numCols - x;
	}
	if ((y + h) >= numRows) {
		h = numRows - y;
	}

	/* Draw 4 lines */
	DrawLine(x, y, x + w, y);         /* Top line */
	DrawLine(x, y + h, x + w, y + h); /* Bottom line */
	DrawLine(x, y, x, y + h);         /* Left line */
	DrawLine(x + w, y, x + w, y + h); /* Right line */
}

/* Draw filled rectangle
 * Start point (x,y)
 * w -> width
 * h -> height
 */
void DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint8_t i;

	/* Check input parameters */
	if (
		x >= numCols ||
		y >= numRows
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= numCols) {
		w = numCols - x;
	}
	if ((y + h) >= numRows) {
		h = numRows - y;
	}

	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		DrawLine(x, y + i, x + w, y + i);
	}
}

/* draw circle
 * centre (x0,y0)
 * radius = radius
 */
void DrawCircle(uint8_t x0, uint8_t y0, uint8_t radius)
{
  int f = 1 - (int)radius;
  int ddF_x = 1;

  int ddF_y = -2 * (int)radius;
  int x = 0;

  SetPixel(x0, y0 + radius);
  SetPixel(x0, y0 - radius);
  SetPixel(x0 + radius, y0);
  SetPixel(x0 - radius, y0);

  int y = radius;
  while(x < y)
  {
    if(f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    SetPixel(x0 + x, y0 + y);
    SetPixel(x0 - x, y0 + y);
    SetPixel(x0 + x, y0 - y);
    SetPixel(x0 - x, y0 - y);
    SetPixel(x0 + y, y0 + x);
    SetPixel(x0 - y, y0 + x);
    SetPixel(x0 + y, y0 - x);
    SetPixel(x0 - y, y0 - x);
  }
}

// Draw Filled Circle
void DrawFilledCircle(int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SetPixel(x0, y0 + r);
    SetPixel(x0, y0 - r);
    SetPixel(x0 + r, y0);
    SetPixel(x0 - r, y0);
    DrawLine(x0 - r, y0, x0 + r, y0);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        DrawLine(x0 - x, y0 + y, x0 + x, y0 + y);
        DrawLine(x0 + x, y0 - y, x0 - x, y0 - y);

        DrawLine(x0 + y, y0 + x, x0 - y, y0 + x);
        DrawLine(x0 + y, y0 - x, x0 - y, y0 - x);
    }
}

// Clear Filled Circle
void ClearFilledCircle(int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    ClearPixel(x0, y0 + r);
    ClearPixel(x0, y0 - r);
    ClearPixel(x0 + r, y0);
    ClearPixel(x0 - r, y0);
    ClearLine(x0 - r, y0, x0 + r, y0);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ClearLine(x0 - x, y0 + y, x0 + x, y0 + y);
        ClearLine(x0 + x, y0 - y, x0 - x, y0 - y);

        ClearLine(x0 + y, y0 + x, x0 - y, y0 + x);
        ClearLine(x0 + y, y0 - x, x0 - y, y0 - x);
    }
}

// Draw Traingle with coordimates (x1, y1), (x2, y2), (x3, y3)
void DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3)
{
	/* Draw lines */
	DrawLine(x1, y1, x2, y2);
	DrawLine(x2, y2, x3, y3);
	DrawLine(x3, y3, x1, y1);
}

// Draw Filled Traingle with coordimates (x1, y1), (x2, y2), (x3, y3)
void DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

#define ABS(x)   ((x) > 0 ? (x) : -(x))

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		DrawLine(x, y, x3, y3);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

//Clear Line
void ClearLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
  int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  for (;;)
  {
    ClearPixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = err + err;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

//Toggle Line
void ToggleLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
  int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  for (;;)
  {
    TogglePixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = err + err;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

//Print Fonted String x=0-15 y=0-7
void GLCD_Font_Print(uint8_t x,uint8_t y,char * String)
{
	int i;
	while(*String)
	{
		for(i=0;i<8;i++)
			GLCD_Buf[i+(x*8)+(y*128)]=Font[(*String)*8+i];
		String++;
		x++;
		if(x>15)
		{
			x=0;
			y++;
		}
	}
}

//Print ICON(8*8) x=0-15 y=0-7
void GLCD_ICON_Print(uint8_t x,uint8_t y,const uint8_t * ICON)
{
	int i;
	for(i=0;i<8;i++)
		GLCD_Buf[i+(x*8)+(y*128)]=ICON[i];
}

/* Toggle rectangle
 * Start point (x,y)
 * w -> width
 * h -> height
 */
void ToggleRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint8_t i;

	/* Check input parameters */
	if (
		x >= numCols ||
		y >= numRows
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= numCols) {
		w = numCols - x;
	}
	if ((y + h) >= numRows) {
		h = numRows - y;
	}

	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		ToggleLine(x, y + i, x + w, y + i);
	}
}

//Debug
void GLCD_Test(void)
{

}
