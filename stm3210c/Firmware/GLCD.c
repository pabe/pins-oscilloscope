/*----------------------------------------------------------------------------
 * Name:    GLCD.c
 * Purpose: MCBSTM32C low level Graphic LCD (320x240 pixels) functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#pragma diag_suppress=550

#include <stm32f10x.h>
#include "GLCD.h"
#include "font.h"

/* SPI_SR - bit definitions. */
#define RXNE    0x01
#define TXE     0x02
#define BSY     0x80


/*********************** Hardware specific configuration **********************/

/*------------------------- Speed dependant settings -------------------------*/

/* If processor works on high frequency delay has to be increased, it can be 
   increased by factor 2^N by this constant                                   */
#define DELAY_2N    18

/*---------------------- Graphic LCD size definitions ------------------------*/

#define WIDTH       320                 /* Screen Width (in pixels)           */
#define HEIGHT      240                 /* Screen Hight (in pixels)           */
#define BPP         16                  /* Bits per pixel                     */
#define BYPP        ((BPP+7)/8)         /* Bytes per pixel                    */

/*--------------- Graphic LCD interface hardware definitions -----------------*/

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */
 
/*---------------------------- Global variables ------------------------------*/

/******************************************************************************/
static volatile unsigned short TextColor = Black, BackColor = White;


/************************ Local auxiliary functions ***************************/

/*******************************************************************************
* Delay in while loop cycles                                                   *
*   Parameter:    cnt:    number of while cycles to delay                      *
*   Return:                                                                    *
*******************************************************************************/

static void delay (int cnt) {

  cnt <<= DELAY_2N;
  while (cnt--);
}


/*******************************************************************************
* Send 1 byte over serial communication                                        *
*   Parameter:    byte:   byte to be sent                                      *
*   Return:                                                                    *
*******************************************************************************/

static __inline unsigned char spi_send (unsigned char byte) {

  SPI3->DR = byte;
  /* Wait if RNE cleared, Rx FIFO is empty. */
  while (!(SPI3->SR & RXNE));
  return (SPI3->DR);
}


/*******************************************************************************
* Write command to LCD controller                                              *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_cmd (unsigned char c) {

  GPIOB->BRR  = 0x0004;
  spi_send(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
  spi_send(0);
  spi_send(c);
  GPIOB->BSRR = 0x0004;
}


/*******************************************************************************
* Write data to LCD controller                                                 *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat (unsigned short c) {

  GPIOB->BRR  = 0x0004;
  spi_send(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
  spi_send((c >>   8));                       /* Write D8..D15                */
  spi_send((c & 0xFF));                       /* Write D0..D7                 */
  GPIOB->BSRR = 0x0004;
}


/*******************************************************************************
* Read data from LCD controller                                                *
*   Parameter:                                                                 *
*   Return:               read data                                            *
*******************************************************************************/

static __inline unsigned short rd_dat (void) {
  unsigned short val = 0;

  GPIOB->BRR  = 0x0004;
  spi_send(SPI_START | SPI_RD | SPI_DATA);    /* Read: RS = 1, RW = 1         */
  spi_send(0);                                /* Dummy read                   */
  val   = spi_send(0);                        /* Read D8..D15                 */
  val <<= 8;
  val  |= spi_send(0);                        /* Read D0..D7                  */
  GPIOB->BSRR = 0x0004;
  return (val);
}

/*******************************************************************************
* Write to LCD register                                                        *
*   Parameter:    reg:    register to be read                                  *
*                 val:    value to write to register                           *
*******************************************************************************/

static __inline void wr_reg (unsigned char reg, unsigned short val) {

  wr_cmd(reg);
  wr_dat(val);
}


/*******************************************************************************
* Read from LCD register                                                       *
*   Parameter:    reg:    register to be read                                  *
*   Return:               value read from register                             *
*******************************************************************************/

static unsigned short rd_reg (unsigned char reg) {

  wr_cmd(reg);
  return (rd_dat());
}


/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize the Graphic LCD controller                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_init (void) { 
  static unsigned short driverCode;

  /* Enable clock for GPIOB,C AFIO and SPI3. */
  RCC->APB2ENR |= 0x00000019;
  RCC->APB1ENR |= 0x00008000;

  /* Set SPI3 remap (use PC10..PC12). */
  AFIO->MAPR   |= 0x10000000;

  /* NCS is PB2, GPIO output set to high. */
  GPIOB->CRL &= 0xFFFFF0FF;
  GPIOB->CRL |= 0x00000300;
  GPIOB->BSRR = 0x00000004;

  /* SPI3_SCK, SPI3_MISO, SPI3_MOSI are SPI pins. */
  GPIOC->CRH &= 0xFFF000FF;
  GPIOC->CRH |= 0x000B8B00;

  /* Enable SPI in Master Mode, CPOL=1, CPHA=1. */
  /* Max. 18 MBit used for Data Transfer @ 72MHz. */
  SPI3->CR1  = 0x0347;
  SPI3->CR2  = 0x0000;

  delay(5);                             /* Delay 50 ms                        */
  driverCode = rd_reg(0x00);

  /* Start Initial Sequence --------------------------------------------------*/
  wr_reg(0xE5, 0x8000);                 /* Set the internal vcore voltage     */
  wr_reg(0x00, 0x0001);                 /* Start internal OSC                 */
  wr_reg(0x01, 0x0100);                 /* Set SS and SM bit                  */
  wr_reg(0x02, 0x0700);                 /* Set 1 line inversion               */
  wr_reg(0x03, 0x1030);                 /* Set GRAM write direction and BGR=1 */
  wr_reg(0x04, 0x0000);                 /* Resize register                    */
  wr_reg(0x08, 0x0202);                 /* 2 lines each, back and front porch */
  wr_reg(0x09, 0x0000);                 /* Set non-disp area refresh cyc ISC  */
  wr_reg(0x0A, 0x0000);                 /* FMARK function                     */
  wr_reg(0x0C, 0x0000);                 /* RGB interface setting              */
  wr_reg(0x0D, 0x0000);                 /* Frame marker Position              */
  wr_reg(0x0F, 0x0000);                 /* RGB interface polarity             */

  /* Power On sequence -------------------------------------------------------*/
  wr_reg(0x10, 0x0000);                 /* Reset Power Control 1              */
  wr_reg(0x11, 0x0000);                 /* Reset Power Control 2              */
  wr_reg(0x12, 0x0000);                 /* Reset Power Control 3              */
  wr_reg(0x13, 0x0000);                 /* Reset Power Control 4              */
  delay(20);                            /* Discharge cap power voltage (200ms)*/
  wr_reg(0x10, 0x17B0);                 /* SAP, BT[3:0], AP, DSTB, SLP, STB   */
  wr_reg(0x11, 0x0137);                 /* DC1[2:0], DC0[2:0], VC[2:0]        */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x12, 0x0139);                 /* VREG1OUT voltage                   */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x13, 0x1D00);                 /* VDV[4:0] for VCOM amplitude        */
  wr_reg(0x29, 0x0013);                 /* VCM[4:0] for VCOMH                 */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x20, 0x0000);                 /* GRAM horizontal Address            */
  wr_reg(0x21, 0x0000);                 /* GRAM Vertical Address              */

  /* Adjust the Gamma Curve --------------------------------------------------*/
  wr_reg(0x30, 0x0006);
  wr_reg(0x31, 0x0101);
  wr_reg(0x32, 0x0003);
  wr_reg(0x35, 0x0106);
  wr_reg(0x36, 0x0B02);
  wr_reg(0x37, 0x0302);
  wr_reg(0x38, 0x0707);
  wr_reg(0x39, 0x0007);
  wr_reg(0x3C, 0x0600);
  wr_reg(0x3D, 0x020B);
  
  /* Set GRAM area -----------------------------------------------------------*/
  wr_reg(0x50, 0x0000);                 /* Horizontal GRAM Start Address      */
  wr_reg(0x51, (HEIGHT-1));             /* Horizontal GRAM End   Address      */
  wr_reg(0x52, 0x0000);                 /* Vertical   GRAM Start Address      */
  wr_reg(0x53, (WIDTH-1));              /* Vertical   GRAM End   Address      */
  wr_reg(0x60, 0x2700);                 /* Gate Scan Line                     */
  wr_reg(0x61, 0x0001);                 /* NDL,VLE, REV                       */
  wr_reg(0x6A, 0x0000);                 /* Set scrolling line                 */

  /* Partial Display Control -------------------------------------------------*/
  wr_reg(0x80, 0x0000);
  wr_reg(0x81, 0x0000);
  wr_reg(0x82, 0x0000);
  wr_reg(0x83, 0x0000);
  wr_reg(0x84, 0x0000);
  wr_reg(0x85, 0x0000);

  /* Panel Control -----------------------------------------------------------*/
  wr_reg(0x90, 0x0010);
  wr_reg(0x92, 0x0000);
  wr_reg(0x93, 0x0003);
  wr_reg(0x95, 0x0110);
  wr_reg(0x97, 0x0000);
  wr_reg(0x98, 0x0000);

  /* Set GRAM write direction and BGR = 1
     I/D=10 (Horizontal : increment, Vertical : increment)
     AM=1 (address is updated in vertical writing direction)                  */
  wr_reg(0x03, 0x1038);

  wr_reg(0x07, 0x0173);                 /* 262K color and display ON          */
}


void GLCD_putPixel(unsigned int x, unsigned int y) {
  wr_reg(0x20, x);
  wr_reg(0x21, y);
  wr_cmd(0x22);
  wr_dat(TextColor);
}

/**
  * @brief  Displays a line.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Length: line length.
  * @param  Direction: line direction.
  *   This parameter can be one of the following values: Vertical or Horizontal.
  * @retval None
  */
void GLCD_drawLine(uint8_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction) {
  u16 i;

  wr_reg(0x21, Ypos);

  if (Direction == Horizontal) {
    wr_reg(0x20, Xpos);
    wr_cmd(0x22);
    for(i = 0; i < Length; i++)
      wr_dat(TextColor);
  } else {
    for(i = 0; i < Length; i++) {
      wr_reg(0x20, Xpos);
      wr_cmd(0x22);
      wr_dat(TextColor);
	  ++Xpos;
    }
  }
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void GLCD_drawRect(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  GLCD_drawLine(Xpos, Ypos, Width, Horizontal);
  GLCD_drawLine((Xpos + Height), Ypos, Width, Horizontal);
  
  GLCD_drawLine(Xpos, Ypos, Height, Vertical);
  GLCD_drawLine(Xpos, Ypos + Width, Height + 1, Vertical);
}

/**
  * @brief  Displays a filled rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void GLCD_fillRect(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  u16 i;
  for (i = 0; i <= Height; ++i)
    GLCD_drawLine(Xpos + i, Ypos, Width + 1, Horizontal);
}

/*******************************************************************************
* Set foreground color                                                         *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_setTextColor(unsigned short color) {
  TextColor = color;
}


/*******************************************************************************
* Set background color                                                         *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_setBackColor(unsigned short color) {
  BackColor = color;
}


/*******************************************************************************
* Clear display                                                                *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_clear (unsigned short color) {
  unsigned int   i;

  wr_reg(0x20, 0);
  wr_reg(0x21, 0);
  wr_cmd(0x22);
  for(i = 0; i < (WIDTH*HEIGHT); i++)
    wr_dat(color);
}


/*******************************************************************************
* Draw character on given position (line, coloum                               *
*   Parameter:     x :        horizontal position                              *
*                  y :        vertical position                                *
*                  c*:        pointer to color value                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_drawChar(unsigned int x, unsigned int y, unsigned short *c) {
  unsigned int index = 0;
  int  i = 0;
  unsigned int Xaddress = 0;
   
  Xaddress = x;
  
  wr_reg(0x20, Xaddress);
  wr_reg(0x21, y);
  
  for(index = 0; index < 24; index++)
  {
    wr_cmd(0x22);              /* Prepare to write GRAM */
    for(i = 15; i >= 0; i--)
//    for(i = 0; i < 16; i++)
    {
      if((c[index] & (1 << i)) == 0x00) {
        wr_dat(BackColor);
      } else {
        wr_dat(TextColor);
      }
    }
    Xaddress++;
    wr_reg(0x20, Xaddress);
    wr_reg(0x21, y);
  }
}


/*******************************************************************************
* Disply character on given line                                               *
*   Parameter:     c :        ascii character                                  *
*                  ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_displayChar(unsigned int ln, unsigned int col, unsigned char  c) {
  c -= 32;
  GLCD_drawChar(ln, col, &ASCII_Table[c * 24]);
}


/*******************************************************************************
* Disply string on given line                                                  *
*   Parameter:     s*:        pointer to string                                *
*                  ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_displayStringLn(unsigned int ln, unsigned char *s) {
  unsigned int i = 0;
  unsigned int refcolumn = (WIDTH/*-1*/)-16;

  while ((*s != 0) & (i < 20))                   /* write the string character by character on lCD */
  {
    GLCD_displayChar(ln, refcolumn, *s);         /* Display one character on LCD */
    refcolumn -= 16;                             /* Decrement the column position by 16 */
    s++;                                         /* Point on the next character */
    i++;                                         /* Increment the character counter */
  }
}


/*******************************************************************************
* Clear given line                                                             *
*   Parameter:     ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_clearLn(unsigned int ln) {
  GLCD_displayStringLn(ln, "                    ");
}


/*******************************************************************************
* Display graphical bitmap image at position x horizontally and y vertically   *
* (This function is optimized for 16 bits per pixel format, it has to be       *
*  adapted for any other bits per pixel format)                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        width of bitmap                                  *
*                   h:        height of bitmap                                 *
*                   bitmap:   address at which the bitmap data resides         *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_bitmap (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap) {
  unsigned int   i;
  unsigned int   len = w*h;
  unsigned short *bitmap_ptr = (unsigned short *)bitmap;

  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+h-1);                  /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

  wr_reg(0x20, y);
  wr_reg(0x21, x);

  wr_cmd(0x22);
  for (i = 0; i < len; i++) {
    wr_dat(*bitmap_ptr++);
  }
}


/******************************************************************************/
