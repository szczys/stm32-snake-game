#include "stm32f0xx_conf.h"
#include "3595-LCD-Driver.h"
#include "main.h"
#include "STM32-Debounce.h"
#include "font5x8.h"

#define BACKGROUND 0xFF
#define FOREGROUND 0x00

uint8_t font_cursor_x = 15;
uint8_t font_cursor_y = 40;

typedef struct {
  uint8_t x;
  uint8_t y;
} point;

point corners[400];
int8_t dirY = 0;
int8_t dirX = 1;


//Variables
static __IO uint32_t TimingDelay;
volatile uint8_t move_tick = 0;

void Write_Char(unsigned char letter, unsigned char fgcolor, unsigned char bgcolor)		//Function that writes one character to display
{
  //TODO: Prevent non-valid characters from crashing program
  
  //Setup display to write one char:
  LCD_Out(0x2A, 1);
  LCD_Out(font_cursor_x, 0);
  LCD_Out(font_cursor_x+5, 0);
  LCD_Out(0x2B, 1);
  LCD_Out(font_cursor_y, 0);
  LCD_Out(font_cursor_y+7, 0);
  LCD_Out(0x2C, 1);
  
  //letters come from font5x8[] in progmem (font5x8.h)
  letter -= 32;						//Adjust char value to match our font array indicies
  
  unsigned char temp[5];
  for (unsigned char i=0; i<5; i++)				//Read one column of char at a time
  {
    temp[i] = font5x8[(5 * letter) + i];	//Get column from progmem
  }
  
  for (unsigned char j=0; j<8; j++)						//Cycle through each bit in column
  {
    LCD_Out(bgcolor, 0);
    for (unsigned char k=0; k<5; k++)
    {
	    if (temp[k] & 1<<j) LCD_Out(fgcolor, 0);
	    else LCD_Out(bgcolor, 0);
    }
  }
}

void Write_String(char* myString, unsigned char fgcolor, unsigned char bgcolor)
{
  while (*myString)
  {
    Write_Char(*myString,fgcolor,bgcolor);
    font_cursor_x += 6; //advance cursor
    ++myString;
  }
}


void _delay_ms(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}


void SysTick_Handler(void) {
  static uint16_t tick = 0;
  static uint16_t ten_ms_tick = 0;

  switch (tick++) {
    case 100:
      tick = 0;
      //GPIOC->ODR ^= (1 << 8);
      move_tick = 1;
      break;
  }
  
  TimingDelay_Decrement();
  
  if (ten_ms_tick++ > 9) {
    ten_ms_tick = 0;
    debounce_interrupt_service();
  }
}

void change_direction(uint8_t new_dir)
{
  switch (new_dir) {
    case 1:
      dirX = -1;
      dirY = 0;
      break;
    case 2:
      dirX = 0;
      dirY = -1;
      break;
    case 3:
      dirX = 1;
      dirY = 0;
      break;
    case 4:
      dirX = 0;
      dirY = 1;
      break;
    }
}

int main(void)
{ 
  unsigned char change_dir = 0;

  SPI_Config();

  //Setup output for blinking blue LED  
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   // enable the clock to GPIOC
  GPIOC->MODER |= (1<<16);
  GPIOC->PUPDR |= (1<<8) | (1<<4) | (1<<2) | (1<<0); //Pull-up resistor
  
  SysTick_Config(SystemCoreClock/1000);
  
  LCD_init();
  uint16_t tail = 0;
  uint16_t head = 1;
  
  corners[head].x = 10;
  corners[head].y = 10;
  
  Draw_Box(0,0,97,66,BACKGROUND);
  Write_String("GAME OVER",FOREGROUND,BACKGROUND);
  clear_keys(REPEAT_MASK); //Clear false reading due to active high buttons
  while(1) {
    if (move_tick) {
      //TODO: Draw_Box(corners[head].x,corners[head].y,corners[head].x+1,corners[head].y+1,BACKGROUND); //Erase
      
      //Change direction
      if (change_dir) {
        change_direction(change_dir);
        change_dir = 0;
      }
      /*
      //Move
      if (corners[head].x == 0) dirX = 1;
      if (corners[head].x == PAGE_SIZE) dirX = -1;
      if (corners[head].y == 0) dirY = 1;
      if (corners[head].y == ROW_SIZE) dirY = -1;
      */
      corners[head].x += dirX;
      corners[head].y += dirY;
      Draw_Box(corners[head].x,corners[head].y,corners[head].x+1,corners[head].y+1,FOREGROUND); //Redraw
      move_tick = 0;
    }
    if (get_key_press(1<<KEY0)) 
      { if (dirX != 1) change_dir = 1; } // Left { dirX = -1; dirY = 0; }
    if (get_key_press(1<<KEY1)) 
      { if (dirY != 1) change_dir = 2; } // Up { dirX = 0; dirY = -1; }
    if (get_key_press(1<<KEY2)) 
      { if (dirX != -1) change_dir = 3; } // Right { dirX = 1; dirY = 0; }
    if (get_key_press(1<<KEY3))
      { if (dirY != -1) change_dir = 4; } // Down { dirX = 0; dirY = 1; }
  }
}
