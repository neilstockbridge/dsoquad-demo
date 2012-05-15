
/*

A tiny menu application that allows the user to select a flash page to boot
from.  This allows developers to place applications anywhere within the
application flash space, which can help to level wear on the flash when small
applications are deployed.

*/

#include "BIOS.h"
#include "stdbool.h"
#include "interrupts.h"
#include "core.h"
#include "input.h"
#include "display.h"
#include "hex.h"


// "slots" are 4K in size and begin at APP1_BASE for 128KB ( 32 slots).  4
// bytes offset from the beginning of the slot is a 32-bit pointer to the
// Reset_Handler function
#define  FIRST_SLOT  APP1_BASE
#define  SLOT_SIZE   0x1000
#define  SLOTS       32

#define  ORANGE  RGB( 0xee, 0xcc, 0x11)


#define  BACKGROUND_COLOR  BLACK
#define  LABEL_COLOR       WHITE
#define  VALUE_COLOR       ORANGE

#define  MIDDLE( available, consumed)  ( ((available) - (consumed)) / 2 )
// 7 == strlen("Address")
#define  LABEL_LEFT  MIDDLE( 400, FONT_WIDTH*(7+1+8) )
#define  VALUE_LEFT  ( LABEL_LEFT + FONT_WIDTH*(7+1) )
#define  BOTTOM      MIDDLE( 240, FONT_HEIGHT*3 )


int  slot_cursor = 0;


void static when_counter3_overflows()
{
  // The buttons are read only every 20 milliseconds to overcome switch bounce
  u8 static  when_to_read_inputs = 20; // milliseconds from now
  if ( 0 < when_to_read_inputs)
    when_to_read_inputs -= 1;
  else {
    when_to_read_inputs = 20;
    check_inputs();
  }
}


void static initial_render()
{
  __Clear_Screen( BACKGROUND_COLOR);
  // The first of these labels is at the bottom
  char static const *label[] = {"Reset", "Address", "Slot"};
  int  i;
  for ( i = 0;  i < ITEMS_IN_ARRAY(label);  i += 1)
  {
    __Display_Str( LABEL_LEFT, BOTTOM+FONT_HEIGHT*i, LABEL_COLOR, 0, label[i] );
  }
}


void static render()
{
  // The display consists of:
  //  + The slot index
  //  + The address of the slot
  //  + The address of the Reset_Handler for that slot ( helps to identify if a
  //    slot is in use)
  char  text[8+1];
  u32   slot_address;
  VoidFunction **reset_vector;

  slot_address = FIRST_SLOT + SLOT_SIZE * slot_cursor;
  reset_vector = (VoidFunction**)(slot_address + 0x4);

  text[ 0] = '0'+ slot_cursor / 10;
  text[ 1] = '0'+ slot_cursor % 10;
  text[ 2] = '\0';
  __Display_Str( VALUE_LEFT, BOTTOM+FONT_HEIGHT*2, VALUE_COLOR, 0, text);

  u32_to_hex( slot_address, text);
  text[ 8] = '\0';
  __Display_Str( VALUE_LEFT, BOTTOM+FONT_HEIGHT*1, VALUE_COLOR, 0, text);

  u32_to_hex( (u32)*reset_vector, text);
  text[ 8] = '\0';
  __Display_Str( VALUE_LEFT, BOTTOM+FONT_HEIGHT*0, VALUE_COLOR, 0, text);
}


int main()
{
  InputEvent  ev;

  __Set( BEEP_VOLUME, 0);

  attach_handler( TIMER3_INTERRUPT, when_counter3_overflows);

  initial_render();

  while( true)
  {
    check_event( &ev);
    if ( INPUT_PRESSED == ev.state)
    {
      switch( ev.input)
      {
        case LROCKER_LEFT:
          slot_cursor = (slot_cursor - 10) & (SLOTS - 1);
          break;
        case LROCKER_RIGHT:
          slot_cursor = (slot_cursor + 10) & (SLOTS - 1);
          break;
        case RROCKER_LEFT:
          slot_cursor = (slot_cursor - 1) & (SLOTS - 1);
          break;
        case RROCKER_RIGHT:
          slot_cursor = (slot_cursor + 1) & (SLOTS - 1);
          break;
        case RROCKER_DOWN:
          {
          VoidFunction **reset_vector = (VoidFunction**)( FIRST_SLOT + SLOT_SIZE*slot_cursor + 0x4);
          (*reset_vector)();
          }
          break;
        default:
          break;
      }
    }

    render();
  }
}

