
// This experiment measures the performance of the BIOS __Display_Str function
// against some custom replacements for it.

#include "BIOS.h"
#include "hex.h"
#include "input.h"
#include "display.h"
#include "main.h"


// @ 1000 overflows/sec, this should last ~49 days
u32 volatile now = 0;


void static when_counter3_overflows()
{
  now += 1;
}


void draw_text( int left, int bottom, u16 color, int mode, char const *text)
{
  // There are two glyph buffers so that one can be rendered while the other is
  // making its way to the LCD:
  u16  buffer[2][ FONT_WIDTH* FONT_HEIGHT];
  int  bf = 0; // indicates the glyph buffer being filled
  char  ch;
  int  x, y;
  u16  bg_color = mode ? color : BLACK;
  u16  fg_color = mode ? BLACK : color;
  while( (ch = *text) != '\0')
  {
    u16  *pixel = buffer[ bf];
    for ( x = 0;  x < FONT_WIDTH;  x += 1)
    {
      u16  strip = __Get_TAB_8x14( ch, x);
      // putting "strip >>= 1" in the loop actually make it slower
      for ( y = 16 - FONT_HEIGHT;  y < 16;  y += 1, pixel += 1)
      {
        *pixel = (strip >> y) & 0x1 ? fg_color : bg_color;
      }
    }
    __LCD_Set_Block( left, left+FONT_WIDTH-1, bottom, bottom+FONT_HEIGHT-1);
    __LCD_DMA_Ready();
    __LCD_Copy( buffer[bf], sizeof(buffer[0]) / sizeof(u16) );
    text += 1;
    left += FONT_WIDTH;
    bf ^= 1;
  }
}


void static test_performance()
{
  // Test performance on a typical length string:
  char  text[] = "Setup";

  u32  invocations = 0;
  char  report[] = "BIOS:12345678";
  // Wait for the beginning of a millisecond:
  u32  when_began = now + 1;
  while ( now < when_began);
  // Render the text as many times as possible in 100ms:
  while( now < when_began + 100)
  {
    __Display_Str( 0, 0, WHITE, 0, text);
    invocations += 1;
  }
  // Render the invocation count:
  u32_to_hex( invocations, &report[5]);
  __Display_Str( 0, FONT_HEIGHT, WHITE, 0, report);
  // It manages 185 loops in 100ms

  invocations = 0;
  // Wait for the beginning of a millisecond:
  when_began = now + 1;
  while ( now < when_began);
  // Render the text as many times as possible in 100ms:
  while( now < when_began + 100)
  {
    draw_text( FONT_WIDTH, 0, WHITE, 0, text);
    invocations += 1;
  }
  // Render the invocation count:
  char  new_report[] = "draw_text:12345678";
  u32_to_hex( invocations, &new_report[10]);
  __Display_Str( 0, FONT_HEIGHT*2, WHITE, 0, new_report);
  // 408 loops in 100ms using a single glyph buffer
  // 583 loops in 100ms using double glyph buffers
}


void font_perf_main()
{
  InputEvent  ev;

  counter3_overflow_hook = when_counter3_overflows;

  test_performance();

  while( should_run)
  {
    check_event( &ev);
    if ( BUTTON4 == ev.input)
      should_run = false;
  }
}

