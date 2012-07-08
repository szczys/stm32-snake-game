/* ATTENTION: *****************************************
* debounce_interrupt_service(); must be run every
* 10ms. It is recommended that this function be
* called from an interrupt handler
******************************************************/

/* Button Debounce Definitions */
//#define KEY_DDR		DDRB
//#define KEY_PORT	PORTB
#define KEY_PIN		GPIOC->IDR
#define KEY0		0	//Left
#define KEY1		1	//Up
#define KEY2		2	//Right
#define KEY3		4 //Down
#define KEY4    5 //Select

//Repeat
#define REPEAT_MASK   ((1<<KEY4) | (1<<KEY3) | (1<<KEY2) | (1<<KEY1) | (1<<KEY0))   // repeat: key1, key2 
#define REPEAT_START  50          // after 500ms 
#define REPEAT_NEXT   20          // every 200ms

void clear_keys(unsigned char key_mask);
unsigned char get_key_press( unsigned char key_mask );
unsigned char get_key_rpt( unsigned char key_mask );
unsigned char get_key_short( unsigned char key_mask ); 
unsigned char get_key_long( unsigned char key_mask );
void debounce_interrupt_service(void);
