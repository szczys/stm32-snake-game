#include "STM32-Debounce.h"

volatile uint16_t key_press;
volatile uint16_t key_state;
volatile uint16_t key_rpt;

/*--------------------------------------------------------------------------
  FUNC: 8/1/11 - Used to read debounced button presses
  PARAMS: A keymask corresponding to the pin for the button you with to poll
  RETURNS: A keymask where any high bits represent a button press
--------------------------------------------------------------------------*/
unsigned char get_key_press( unsigned char key_mask )
{
  __disable_irq ();			// read and clear atomic !
  key_mask &= key_press;	// read key(s)
  key_press ^= key_mask;	// clear key(s)
  __enable_irq ();
  return key_mask;
}

/*--------------------------------------------------------------------------
  FUNC: 8/1/11 - Used to check for debounced buttons that are held down
  PARAMS: A keymask corresponding to the pin for the button you with to poll
  RETURNS: A keymask where any high bits is a button held long enough for
		its input to be repeated
--------------------------------------------------------------------------*/
unsigned char get_key_rpt( unsigned char key_mask ) 
{ 
  __disable_irq ();               // read and clear atomic ! 
  key_mask &= key_rpt;                           // read key(s) 
  key_rpt ^= key_mask;                           // clear key(s) 
  __enable_irq (); 
  return key_mask; 
} 

/*--------------------------------------------------------------------------
  FUNC: 8/1/11 - Used to read debounced button released after a short press
  PARAMS: A keymask corresponding to the pin for the button you with to poll
  RETURNS: A keymask where any high bits represent a quick press and release
--------------------------------------------------------------------------*/
unsigned char get_key_short( unsigned char key_mask ) 
{ 
  __disable_irq ();         // read key state and key press atomic ! 
  return get_key_press( ~key_state & key_mask ); 
} 

/*--------------------------------------------------------------------------
  FUNC: 8/1/11 - Used to read debounced button held for REPEAT_START amount
	of time.
  PARAMS: A keymask corresponding to the pin for the button you with to poll
  RETURNS: A keymask where any high bits represent a long button press
--------------------------------------------------------------------------*/
unsigned char get_key_long( unsigned char key_mask ) 
{ 
  return get_key_press( get_key_rpt( key_mask )); 
}

/*--------------------------------------------------------------------------
  FUNC: 6/24/12 - Debounce counter - needs to be called every 10ms
  PARAMS: None
  RETURNS: None
  NOTE: Sets global variables: key_press, key_state, key_rpt
--------------------------------------------------------------------------*/
void debounce_interrupt_service(void) {
    static unsigned char ct0, ct1, rpt;
    uint16_t i;

    i = key_state ^ KEY_PIN;    // key changed ?
    ct0 = ~( ct0 & i );          // reset or count ct0
    ct1 = ct0 ^ (ct1 & i);       // reset or count ct1
    i &= ct0 & ct1;              // count until roll over ?
    key_state ^= i;              // then toggle debounced state
    key_press |= key_state & i;  // 0->1: key press detect

    if( (key_state & REPEAT_MASK) == 0 )   // check repeat function 
      rpt = REPEAT_START;      // start delay 
    if( --rpt == 0 ){ 
      rpt = REPEAT_NEXT;         // repeat delay 
      key_rpt |= key_state & REPEAT_MASK; 
    } 
}
