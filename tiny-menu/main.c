
/*

A tiny menu application that allows the user to select a flash page to boot
from.  This allows developers to place applications anywhere within the
application flash space, which can help to level wear on the flash when small
applications are deployed.

*/

#include <stdbool.h>
#include "BIOS.h"
#include "core.h"
#include "input.h"
#include "display.h"
#include "hex.h"


// "slots" are 4K in size and begin at APP1_BASE for 128KB ( 32 slots).  4
// bytes offset from the beginning of the slot is a 32-bit pointer to the
// Reset_Handler function
#define  FIRST_PAGE  APP1_BASE
#define  PAGE_SIZE   0x1000
#define  PAGES       32

#define  ORANGE  RGB( 0xee, 0x99, 0x11)


#define  BACKGROUND_COLOR  BLACK
#define  LABEL_COLOR       WHITE
#define  VALUE_COLOR       ORANGE
#define  MUTED_COLOR       RGB( 0x55, 0x44, 0x11)

#define  MIDDLE( available, consumed)  ( ((available) - (consumed)) / 2 )


// When this is -1, the UI allows to choose slot, otherwise, this is 0-3 and indicates APP slot 1-4 ( page 8*focus_slot)
#define  UNDEFINED  (-1)
int chosen_slot = UNDEFINED;
int chosen_page = UNDEFINED;


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


void TIM3_IRQHandler()
{
  when_counter3_overflows();
  __Set( KEY_IF_RST, 0); // Clear TIM3 interrupt flag
}


void USB_HP_CAN_TX_IRQHandler()
{
  __CTR_HP();
}


void USB_LP_CAN_RX0_IRQHandler()
{
  __USB_Istr();
}


void static render_slot_buttons()
{
  int  i;

  __Clear_Screen( BACKGROUND_COLOR);

  for ( i = 0;  i < 4;  i += 1)
  {
    char static label[] = "Slot X";
    label[ 5] = '0'+ 1 + i;
    __Display_Str( 67*i, (SCREEN_HEIGHT - FONT_HEIGHT), LABEL_COLOR, 1, label);
  }
}


void static render_slots()
{
  int            i;
  char           slot_number_label[2+1];
  char static    slot_address_label[] = "0x12345678";
  u32            slot_address;
  VoidFunction **reset_vector;
  int            left;
  int            bottom;
  // If a slot has been chosen then show the other slots in muted colors
  for ( i = 0;  i < PAGES;  i += 1)
  {
    u16  color = (UNDEFINED == chosen_slot || i / 8 == chosen_slot) ? VALUE_COLOR : MUTED_COLOR;
    left = ( i < 16) ? 0 : (SCREEN_WIDTH / 2);
    bottom = SCREEN_HEIGHT - FONT_HEIGHT * (2 + i % 16);
    slot_number_label[ 0] = (i < 10) ? ' ' : ('0'+ i / 10);
    slot_number_label[ 1] = '0'+ (i % 10);
    slot_number_label[ 2] = '\0';
    __Display_Str( left, bottom, color, 0, slot_number_label);

    // The slot address is not as interesting as the Reset vector and the slot
    // number, so show it in a muted color
    slot_address = FIRST_PAGE + PAGE_SIZE * i;
    u32_to_hex( slot_address, &slot_address_label[2]);
    __Display_Str( (left + FONT_WIDTH * 3), bottom, MUTED_COLOR, 0, slot_address_label);

    reset_vector = (VoidFunction**)(slot_address + 0x4);
    u32_to_hex( (u32)*reset_vector, &slot_address_label[2]);
    __Display_Str( (left + FONT_WIDTH * 14), bottom, color, 0, slot_address_label);
  }
}


void static render_page_buttons()
{
  int         i;
  int         page;
  char static label[] = "Pg XX";

  __Clear_Screen( BACKGROUND_COLOR);

  for ( i = 0;  i < 8;  i += 1)
  {
    page = 8 * chosen_slot + i;
    label[ 3] = (page < 10) ? ' ' : ('0'+ page / 10);
    label[ 4] = '0'+ (page % 10);
    __Display_Str( (SCREEN_WIDTH / 8) * i, (SCREEN_HEIGHT - FONT_HEIGHT), LABEL_COLOR, 1, label);
  }
}


int main()
{
  InputEvent  ev;
  Input static const buttons[] = { BUTTON1, BUTTON2, BUTTON3, BUTTON4, LROCKER_LEFT, LROCKER_RIGHT, RROCKER_LEFT, RROCKER_RIGHT};

  __Set( BEEP_VOLUME, 0);

  //chosen_slot = UNDEFINED;
  //chosen_page = UNDEFINED;

  render_slot_buttons();
  render_slots();

  while( true)
  {
    check_event( &ev);

    if ( INPUT_PRESSED == ev.state)
    {
      // The four buttons and both directions of both rockers make up eight
      // buttons numbered 0..7.  Work out which ( if any) were pressed:
      Input  button_pressed = UNDEFINED;
      int  i;
      for ( i = 0;  i < ITEMS_IN_ARRAY(buttons);  i += 1)
      {
        if ( buttons[i] == ev.input)
        {
          button_pressed = i;
          break;
        }
      }

      // If the slot has not yet been chosen..
      if ( UNDEFINED == chosen_slot)
      {
        if ( button_pressed != UNDEFINED  &&  button_pressed < 4)
        {
          chosen_slot = button_pressed;
          render_page_buttons();
          render_slots();
        }
      }
      // Otherwise the slot *has* been chosen and all that it left is to choose
      // the page within the slot
      else {
        if ( button_pressed != UNDEFINED)
        {
          chosen_page = 8 * chosen_slot + button_pressed;
          VoidFunction **reset_vector = (VoidFunction**)( FIRST_PAGE + (PAGE_SIZE * chosen_page) + 0x4);
          (*reset_vector)();
        }
      }
    }

    // Go to sleep until it's time to check the buttons again
    __WFE();
  }
}

