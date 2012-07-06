#include "stm32f0xx_conf.h"
#include "main.h"

/* ATTENTION: *************************************
* main.h/main.c MUST implement a delay function
* that allows for a delay passed in milliseconds!
*
* Define a macro pointing to that funciton here
**************************************************/
#define LCD_DELAY_MS(x)           _delay_ms(x)

//LCD Definitions
#define LCD_RST_PORT              GPIOC->ODR
#define LCD_RST_CLK_ENR           RCC->AHBENR
#define LCD_RST_CLK_ENABLE_BIT    RCC_AHBENR_GPIOCEN
#define LCD_RST_MODER             GPIOC->MODER
#define LCD_RST_MODER_BIT         (1<<6)
#define LCD_RST                   (1<<3)

#define PAGE_SIZE	97
#define ROW_SIZE	66

#define red	0b00000111
#define yellow	0b00111111
#define green	0b00111100
#define cyan	0b11111000
#define blue	0b11000000
#define magenta	0b11000111
#define white	0b11111111
#define black	0b00000000

/* SPI Definitions */
#define SPIx                             SPI1
#define SPIx_CLK                         RCC_APB2Periph_SPI1
#define SPIx_GPIO_PORT                   GPIOA
#define SPIx_GPIO_CLK                    RCC_AHBPeriph_GPIOA
#define SPIx_AF                          GPIO_AF_0
#define SPIx_SCK_PIN                     GPIO_Pin_5
#define SPIx_SCK_SOURCE                  GPIO_PinSource5
#define SPIx_MISO_PIN                    GPIO_Pin_6
#define SPIx_MISO_SOURCE                 GPIO_PinSource6
#define SPIx_MOSI_PIN                    GPIO_Pin_7
#define SPIx_MOSI_SOURCE                 GPIO_PinSource7
#define SPIx_NSS_PIN                     GPIO_Pin_4
#define SPIx_NSS_SOURCE                  GPIO_PinSource4

/* Function Prototypes */
void LCD_init(void);
void LCD_Out(unsigned char Data, unsigned char isCmd);
void SPI_Config(void);
void StripedScreen(void);
void Hello_World(void);
void Draw_Box(uint8_t upperX, uint8_t upperY, uint8_t lowerX, uint8_t lowerY, uint8_t color);
void Fill_Screen(unsigned char color);
