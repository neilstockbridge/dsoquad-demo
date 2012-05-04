
#include "BIOS.h"
#include "interrupts.h"
#include "hex.h"
#include "input.h"
#include "display.h"
#include "main.h"


u32 volatile systick_reloads = 0;

void static when_systick_is_zero()
{
  systick_reloads += 1;
}


void static render()
{
  int  x = 0;
  int  y = SCREEN_HEIGHT - FONT_HEIGHT;
  char  msg[] = "VAL:0x12345678";
  u32_to_hex( SysTick->VAL, &msg[6]);
  __Display_Str( x, y, WHITE, 0, msg);
  y -= FONT_HEIGHT;
  char  m[] = "reloads:0x12345678";
  u32_to_hex( systick_reloads, &m[10]);
  __Display_Str( x, y, WHITE, 0, m);
}


void SysTick_main()
{
  InputEvent  ev;

  attach_handler( SYSTICK_INTERRUPT, when_systick_is_zero);

  SysTick_Config( SYSTICK_MAXCOUNT);

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
  // Disable SysTick and its interrupt:
  SysTick->CTRL &= ~ ((1 << SYSTICK_ENABLE) | (1 << SYSTICK_TICKINT));
}

