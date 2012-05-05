
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

