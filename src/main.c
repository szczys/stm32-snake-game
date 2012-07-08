#include "stm32f0xx_conf.h"
#include "3595-LCD-Driver.h"
#include "main.h"
#include "STM32-Debounce.h"
#include "font5x8.h"
#include "98x67-snake.h"

#define BACKGROUND 0xFF
#define FOREGROUND 0x00

#define SNAKE_GIRTH   2
#define GAMEBOARD_X   (PAGE_SIZE+1)/SNAKE_GIRTH
#define GAMEBOARD_Y   (ROW_SIZE+1)/SNAKE_GIRTH

uint8_t font_cursor_x = 15;
uint8_t font_cursor_y = 40;
uint16_t tail = 0;
uint16_t head = 1;

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

void Double_Char(unsigned char letter, unsigned char fgcolor, unsigned char bgcolor)		//Function that writes one character to display
{
  //TODO: Prevent non-valid characters from crashing program
  
  //Setup display to write one char:
  LCD_Out(0x2A, 1);
  LCD_Out(font_cursor_x, 0);
  LCD_Out(font_cursor_x+10, 0);
  LCD_Out(0x2B, 1);
  LCD_Out(font_cursor_y, 0);
  LCD_Out(font_cursor_y+15, 0);
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
      if (temp[k] & 1<<j) { LCD_Out(fgcolor, 0); LCD_Out(fgcolor, 0); }
	    else { LCD_Out(bgcolor, 0); LCD_Out(bgcolor, 0); }
    }
    LCD_Out(bgcolor, 0);    
    for (unsigned char k=0; k<5; k++)
    {
      if (temp[k] & 1<<j) { LCD_Out(fgcolor, 0); LCD_Out(fgcolor, 0); }
	    else { LCD_Out(bgcolor, 0); LCD_Out(bgcolor, 0); }
    }
  }
}

void Write_String(char* myString, unsigned char fgcolor, unsigned char bgcolor)
{
  while (*myString)
  {
    Write_Char(*myString,fgcolor,bgcolor);
    Double_Char(*myString,fgcolor,bgcolor);
    font_cursor_x += 12; //advance cursor
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
    case 200:
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

uint8_t absolute_difference(uint8_t a, uint8_t b)
{
  int16_t unknown = (int16_t)a - b;
  return (uint8_t)(unknown<0?0-unknown:unknown); 
}

uint8_t neighbors(point node1, point node2)
{
  if ((absolute_difference(node1.x,node2.x) == 1) || (absolute_difference(node1.y,node2.y) == 1)) return 1;
  return 0;
}

void game_over(void)
{
  Write_String("OVER",FOREGROUND,BACKGROUND);
  while(1) {;;}
}

void move_head(uint8_t new_dir)
{
  if (new_dir)
  {
    //Copy head to new position
    ++head; //increment head
    corners[head].x = corners[head-1].x;
    corners[head].y = corners[head-1].y;
    change_direction(new_dir);  //change direction
  }
  
  //Have we left the game board?
  if ((corners[head].x == 0) && (dirX == -1)) game_over();
  if ((corners[head].y == 0) && (dirY == -1)) game_over();
  if ((corners[head].x == GAMEBOARD_X-1) && (dirX == 1)) game_over();
  if ((corners[head].y == GAMEBOARD_Y-1) && (dirY == 1)) game_over();
  corners[head].x += dirX;
  corners[head].y += dirY;
}

void follow_tail(void)
{

  //is tail a neighbor of next?
  if (neighbors(corners[tail],corners[tail+1]))
  {
    ++tail;
  }
  
  //find which axis tail and next have in common
  else
  {
    if (corners[tail].x == corners[tail+1].x)
    {
      //These points have the same X, so make adjustment to the Y
      if ((corners[tail].y - corners[tail+1].y) < 0) corners[tail].y += 1;
      else corners[tail].y -= 1; 
    }
    else
    {
      //These points have the same Y, so make adjustment to the X
      if ((corners[tail].x - corners[tail+1].x) < 0) corners[tail].x += 1;
      else corners[tail].x -= 1; 
    }
  }
}

uint8_t collision(void)
{
  uint8_t lower, upper, testpoint;

  //Check to see if we hit part of the snake
  //traverse all nodes from tail forward
  for (uint8_t i=tail; i<head-3; i++) 
    //( check head-3 because you can't run into a segment any close than that to the head)
  {
    //check to see if head's x or y are shared with the current point
    if ((corners[head].x == corners[i].x) && (corners[i].x == corners[i+1].x))
    {
      //which point is the higher  number?
      if (corners[i].y < corners[i+1].y) {lower = corners[i].y; upper = corners[i+1].y;}
      else {lower = corners[i+1].y; upper = corners[i].y;}
      testpoint = corners[head].y;
    }
    else if ((corners[head].y == corners[i].y) && (corners[i].y == corners[i+1].y))
    {
      //which point is the higher  number?
      if (corners[i].x < corners[i+1].x) {lower = corners[i].x; upper = corners[i+1].x;}
      else {lower = corners[i+1].x; upper = corners[i].x;}
      testpoint = corners[head].x;
    }
    else continue;
    
    //Now check to see if head is a point between this node and the next
    if ((lower<=testpoint) && (testpoint<= upper)) return 1;
  }
  return 0;
}

void draw_graphic(void) {

  LCD_Out(0x2A, 1);
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1);
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1);
  
  for (uint16_t i=0; i<6566; i++)
  {
    LCD_Out(snake_graphic_cmap[snake_graphic[i]],0);
    //LCD_Out(white,0);
  }
}
int main(void)
{ 
  uint8_t change_dir = 0;

  SPI_Config();

  //Setup output for blinking blue LED  
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   // enable the clock to GPIOC
  GPIOC->MODER |= (1<<16);
  GPIOC->PUPDR |= (1<<8) | (1<<4) | (1<<2) | (1<<0); //Pull-up resistor
  
  SysTick_Config(SystemCoreClock/1000);
  
  LCD_init();
  
  corners[head].x = 15;
  corners[head].y = 3;
  corners[tail].x = 3;
  corners[tail].y = 3;
  
  draw_graphic();
  //StripedScreen();
  while(1) {;;}
  
  Draw_Box(0,0,PAGE_SIZE,ROW_SIZE,black);
  Draw_Box(0,0,PAGE_SIZE-((PAGE_SIZE+1)%SNAKE_GIRTH),ROW_SIZE-((ROW_SIZE+1)%SNAKE_GIRTH),BACKGROUND);
  Draw_Box(corners[tail].x*SNAKE_GIRTH,corners[tail].y*SNAKE_GIRTH,(corners[head].x*SNAKE_GIRTH)+SNAKE_GIRTH-1,(corners[head].y*SNAKE_GIRTH)+SNAKE_GIRTH-1,FOREGROUND); //FIXME thickness hack
  
  clear_keys(REPEAT_MASK); //Clear false reading due to active high buttons
  while(1) {
    if (move_tick) {
           
      /*
      //Move
      if (corners[head].x == 0) dirX = 1;
      if (corners[head].x == PAGE_SIZE) dirX = -1;
      if (corners[head].y == 0) dirY = 1;
      if (corners[head].y == ROW_SIZE) dirY = -1;
      */
      //corners[head].x += dirX;
      //corners[head].y += dirY;

      move_head(change_dir);
      Draw_Box(corners[head].x*SNAKE_GIRTH,corners[head].y*SNAKE_GIRTH,(corners[head].x*SNAKE_GIRTH)+SNAKE_GIRTH-1,(corners[head].y*SNAKE_GIRTH)+SNAKE_GIRTH-1,FOREGROUND); //Redraw
      if (collision()) game_over();
      Draw_Box(corners[tail].x*SNAKE_GIRTH,corners[tail].y*SNAKE_GIRTH,(corners[tail].x*SNAKE_GIRTH)+SNAKE_GIRTH-1,(corners[tail].y*SNAKE_GIRTH)+SNAKE_GIRTH-1,BACKGROUND); //Erase
      follow_tail();
      
      
      
      move_tick = 0;
    }
    if (get_key_press(1<<KEY0)) 
      { if (dirX == 0) change_dir = 1; } // Left { dirX = -1; dirY = 0; }
    if (get_key_press(1<<KEY1)) 
      { if (dirY == 0) change_dir = 2; } // Up { dirX = 0; dirY = -1; }
    if (get_key_press(1<<KEY2)) 
      { if (dirX == 0) change_dir = 3; } // Right { dirX = 1; dirY = 0; }
    if (get_key_press(1<<KEY3))
      { if (dirY == 0) change_dir = 4; } // Down { dirX = 0; dirY = 1; }
  }
}
