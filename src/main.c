#include "stm32f0xx_conf.h"
#include "3595-LCD-Driver.h"
#include "main.h"
#include "STM32-Debounce.h"
#include <math.h>

#define BACKGROUND 0xFF
#define FOREGROUND 0x00

typedef struct {
  uint8_t x;
  uint8_t y;
} point;

point head;
int8_t dirY = 1;
int8_t dirX = 1;


//Variables
static __IO uint32_t TimingDelay;
volatile uint8_t move_tick = 0;

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
      GPIOC->ODR ^= (1 << 8);
      move_tick = 1;
      break;
  }
  
  TimingDelay_Decrement();
  
  if (ten_ms_tick++ > 9) {
    ten_ms_tick = 0;
    debounce_interrupt_service();
  }
}

int main(void)
{
  SysTick_Config(SystemCoreClock/1000);
  
  SPI_Config();

  //Setup output for blinking blue LED  
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   // enable the clock to GPIOC
  GPIOC->MODER |= (1<<16);
  GPIOC->PUPDR |= (1<<8) | (1<<4) | (1<<2) | (1<<0); //Pull-up resistor
  
  LCD_init();
  head.x = 10;
  head.y = 10;
  Draw_Box(0,0,97,66,BACKGROUND);
  while(1) {
    if (move_tick) {
      Draw_Box(head.x,head.y,head.x+1,head.y+1,BACKGROUND); //Erase
      //Move
      if (head.x == 0) dirX = 1;
      if (head.x == PAGE_SIZE) dirX = -1;
      if (head.y == 0) dirY = 1;
      if (head.y == ROW_SIZE) dirY = -1;
      head.x += dirX;
      head.y += dirY;
      Draw_Box(head.x,head.y,head.x+1,head.y+1,FOREGROUND); //Redraw
      move_tick = 0;
    }
    if (get_key_press(1<<KEY0)) { dirX = -1; dirY = 0; }
    if (get_key_press(1<<KEY1)) { dirX = 0; dirY = -1; }
    if (get_key_press(1<<KEY2)) { dirX = 1; dirY = 0; }
    if (get_key_press(1<<KEY3)) { dirX = 0; dirY = 1; }
  }
}