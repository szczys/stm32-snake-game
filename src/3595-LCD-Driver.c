#include "stm32f0xx_conf.h"
#include "3595-LCD-Driver.h"
#include "main.h"

//Variables
unsigned char cursor_x = 0;	// Tracks cursor position (top-left corner)
unsigned char cursor_y = 0;	// Tracks cursor position (top-left corner)

void LCD_init(void)
{
  //Turn on clock for Reset pin
  LCD_RST_CLK_ENR |= LCD_RST_CLK_ENABLE_BIT;
  //Make Reset pin an output
  LCD_RST_MODER |= LCD_RST_MODER_BIT;
  LCD_RST_PORT |= LCD_RST;
  LCD_DELAY_MS(1);

  //Hardware Reset
  LCD_RST_PORT &= ~LCD_RST;
  LCD_DELAY_MS(5);
  LCD_RST_PORT |= LCD_RST;
  LCD_DELAY_MS(5);

  //Software Reset
  LCD_Out(0x01, 1);
  LCD_DELAY_MS(10);

/*
  //Refresh set
  LCD_Out(0xB9, 1);
  LCD_Out(0x00, 0);
*/


  //Display Control
  LCD_Out(0xB6, 0);
  LCD_Out(128, 0);
  LCD_Out(128, 0);
  LCD_Out(129, 0);
  LCD_Out(84, 0);
  LCD_Out(69, 0);
  LCD_Out(82, 0);
  LCD_Out(67, 0);


/*
  //Temperature gradient set
  LCD_Out(0xB7, 1);
  for(char i=0; i<14; i++)  LCD_Out(0, 0);
*/

  //Booster Voltage On
  LCD_Out(0x03, 1);
  LCD_DELAY_MS(50);  //NOTE: At least 40ms must pass between voltage on and display on.
		  //Other operations may be carried out as long as the display is off
		  //for this length of time.

/*
  //Test Mode
  LCD_Out(0x04, 1);
*/

/*
  // Power Control
  LCD_Out(0xBE, 1);
  LCD_Out(4, 0);
*/

  //Sleep Out
  LCD_Out(0x11, 1);

  //Display mode Normal
  LCD_Out(0x13, 1);

  //Display On
  LCD_Out(0x29, 1);

  //Set Color Lookup Table
  LCD_Out(0x2D, 1);		//Red and Green (3 bits each)
  char x, y;
  for(y = 0; y < 2; y++) {
	  for(x = 0; x <= 14; x+=2) {
		  LCD_Out(x, 0);
	  }
  }
  //Set Color Lookup Table	//Blue (2 bits)
  LCD_Out(0, 0);
  LCD_Out(4, 0);
  LCD_Out(9, 0);
  LCD_Out(14, 0);

  //Set Pixel format to 8-bit color codes
  LCD_Out(0x3A, 1);
  LCD_Out(0b00000010, 0);

//***************************************
//Initialization sequence from datasheet:

//Power to chip
//RES pin=low
//RES pin=high -- 5ms pause
//Software Reset
//5ms Pause
//INIESC
  //<Display Setup 1>
    //REFSET
    //Display Control
    //Gray Scale position set
    //Gamma Curve Set
    //Common Driver Output Select
  //<Power Supply Setup>
    //Power Control
    //Sleep Out
    //Voltage Control
    //Write Contrast
    //Temperature Gradient
    //Boost Voltage On
  //<Display Setup 2>
    //Inversion On
    //Partial Area
    //Vertical Scroll Definition
    //Vertical Scroll Start Address
  //<Display Setup 3>
    //Interface Pixel Format
    //Colour Set
    //Memory access control
    //Page Address Set
    //Column Address Set
    //Memory Write
  //Display On

//****************************************
}

/*--------------------------------------------------------------------------
  FUNC: 6/23/12 - Writes 9-bits on SPI bus
  PARAMS: 8-bits of data, 1 to signify that the data is an LCD command
  RETURNS: None
--------------------------------------------------------------------------*/
void LCD_Out(unsigned char Data, unsigned char isCmd) 
{
  //Wait until transmit buffer is ready
  //TODO: Add timeout just in case
  while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) ;;
  //Commands start with 0
  if (isCmd) SPI_I2S_SendData16(SPIx, (uint16_t)Data);
  //Data starts with 1
  else SPI_I2S_SendData16(SPIx, ((uint16_t)Data | (1<<8)));
}

void SPI_Config(void)
{
  SPI_InitTypeDef SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the SPI peripheral */
  RCC_APB2PeriphClockCmd(SPIx_CLK, ENABLE);
  /* Enable SCK, MOSI, MISO and NSS GPIO clocks */
  RCC_AHBPeriphClockCmd(SPIx_GPIO_CLK, ENABLE);

  /* SPI pin mappings */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_Init(SPIx_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  //GPIO_InitStructure.GPIO_Pin = SPIx_MISO_PIN;
  //GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  /* SPI NSS pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_NSS_PIN;
  GPIO_Init(SPIx_GPIO_PORT, &GPIO_InitStructure);
  
  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(SPIx);
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_9b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  
  SPI_Init(SPIx, &SPI_InitStructure);
  SPI_SSOutputCmd(SPIx, ENABLE);
  SPI_Cmd(SPIx, ENABLE);

}

void StripedScreen(void)
{
  unsigned char color_palate[] = {
	//BBGGGRRR
	0b00000111,	//Red
	0b00111111,	//Yellow
	0b00111100,	//Green
	0b11111000,	//Cyan
	0b11000000,	//Blue
	0b11000111,	//Magenta
	0b11111111,	//White
	0b00000111	//This should be 0x00(black) but for screen wrapping it was changed to Red
  };

  LCD_Out(0x13, 1);
  for (unsigned char i=0; i<8; i++)
  {
    LCD_Out(0x2A, 1);
    LCD_Out(0, 0);
    LCD_Out(97, 0);
    LCD_Out(0x2B, 1);
    LCD_Out(i*9, 0);
    LCD_Out((i*9)+8, 0);
    LCD_Out(0x2C, 1);
    for (int j=0; j<882; j++)
    {
      LCD_Out(color_palate[i], 0);
    }
  }
}

void Hello_World(void)
{
  //Binary representation of "Hello World"
  unsigned char Hello_World[5][5] = {
    { 0b10101110, 0b10001000, 0b01001010, 0b10010011, 0b00100110 },
    { 0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101 },
    { 0b11101100, 0b10001000, 0b10101010, 0b10101011, 0b00100101 },
    { 0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101 },
    { 0b10101110, 0b11101110, 0b01000101, 0b00010010, 0b10110110 }
  };

    LCD_Out(0x2A, 1);
    LCD_Out(8, 0);
    LCD_Out(87, 0);
    LCD_Out(0x2B, 1);
    LCD_Out(23, 0);
    LCD_Out(32, 0);
    LCD_Out(0x2C, 1);
    for (unsigned char i=0; i<5; i++) //Scan Rows
    {
      char h=2;
      while(h)
      {
	for (unsigned char k=0; k<5; k++) //Scan Columns
	{
	  for (char j=0; j<8; j++)
	  {
	    if (Hello_World[i][k] & 1<<(7-j))	//Should there be a letter pixel here?
	    {
	      LCD_Out(0x00, 0);			//yes - draw it in black
	      LCD_Out(0x00, 0);			
	    }
	    else 
	    {
	      LCD_Out(0xFF, 0);			//no - draw background in white
	      LCD_Out(0xFF, 0);
	    }
	  }
	}
	--h;
      }
    }
}

/*--------------------------------------------------------------------------
  FUNC: 6/23/12 - Draws a colored box (infilled) on the screen
  PARAMS: x,y coordinates for upper left and lower right 
          corners of the screen, color (BBGGGRRR)
  RETURNS: None
--------------------------------------------------------------------------*/
void Draw_Box(uint8_t upperX, uint8_t upperY, uint8_t lowerX, uint8_t lowerY, uint8_t color)
{

  if (upperX >= PAGE_SIZE) upperX = PAGE_SIZE-1;
  if (upperY >= ROW_SIZE) upperY = ROW_SIZE-1;
  if (lowerX > PAGE_SIZE) lowerX = PAGE_SIZE;
  if (lowerY > ROW_SIZE) lowerY = ROW_SIZE;
  uint16_t pixel_calc = (lowerX-upperX+1)*(lowerY-upperY+1);

  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(upperX, 0);
  LCD_Out(lowerX, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(upperY, 0);
  LCD_Out(lowerY, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (int i=0; i<pixel_calc; i++) LCD_Out(color, 0);
}

void Fill_Screen(unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (int i=0; i<6566; i++) LCD_Out(color, 0);
}
