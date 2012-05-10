
/*

Turns the Quad in to a "torch" by filling the screen with white pixels.  The
first two buttons choose between 10% and 100% brightness.

Perceived brightness doesn't vary linearly with the duty cycle applied to the
backlight LED

*/

#include "BIOS.h"
#include "stdbool.h"
#include "interrupts.h"
#include "core.h"
#include "input.h"
#include "display.h"


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


int main()
{
  InputEvent  ev;

  __Set( BEEP_VOLUME, 0);
  __Clear_Screen( WHITE);

  attach_handler( TIMER3_INTERRUPT, when_counter3_overflows);

  while( true)
  {
    check_event( &ev);
    if ( INPUT_PRESSED == ev.state)
    {
      switch( ev.input)
      {
        case BUTTON1:
          __Set( BACKLIGHT, 10);
          break;
        case BUTTON2:
          __Set( BACKLIGHT, 100);
          break;
        default:
          break;
      }
    }
  }
}

