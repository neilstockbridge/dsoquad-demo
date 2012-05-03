
// This experiment tries out the WFE instruction.  I hope that the timer3
// interrupt will wake the processor and it should be possible to return from
// the experiment to the main menu.

#include "BIOS.h"
#include "main.h"
#include "input.h"


void WFE_main()
{
  InputEvent  ev;
  while( should_run)
  {
    check_event( &ev);
    switch( ev.input)
    {
      case BUTTON4:
        should_run = false;
        break;
      default:
        break;
    }
    __WFE();
  }
}

