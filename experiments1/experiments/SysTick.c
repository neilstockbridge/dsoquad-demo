
#include "BIOS.h"
#include "hex.h"
#include "colors.h"
#include "main.h"
#include "input.h"


void static render()
{
  int  x = 0;
  int  y = SCREEN_HEIGHT - FONT_HEIGHT;
  char  msg[] = "SysTick:0x12345678";
  u32_to_hex( SysTick->VAL, &msg[10]);
  __Display_Str( x, y, WHITE, 0, msg);
}


void SysTick_main()
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
    render();
  }
}

