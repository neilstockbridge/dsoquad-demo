
/*

This is a single 32kB app that has many demonstrations of the features of the
Quad.  This app allows me to make changes to many separate experiments and test
them all with a single erase cycle of the first page of the APP4 slot, which I
worry about exceeding the 10k cycles of.

The right-hand rocker switch selects the experiment and pushing down on the
rocker switch will run the experiment.  Experiments should fall off the end of
their main methods in response to button 4 ( EJECT, triangle) being pressed.
Any changes to the CPU such as interrupts enabled for the experiment should be
undone before returning from the experimental main method.

*/

#include "BIOS.h"
#include "stdbool.h"
#include "core.h"
#include "interrupts.h"
#include "hex.h"
#include "input.h"
#include "display.h"
#include "dump.h"
#include "main.h"


typedef struct
{
  char         *name;
  VoidFunction *main;
}
Experiment;

extern void WFE_main();
extern void SysTick_main();
extern void buttons_main();
extern void backlight_main();
extern void glyphs_main();
extern void font_perf_main();

Experiment static experiment[] =
{
  {"WFE",       WFE_main},
  {"SysTick",   SysTick_main},
  {"buttons",   buttons_main},
  {"backlight", backlight_main},
  {"glyphs",    glyphs_main},
  {"font_perf", font_perf_main},
};

int  experiment_cursor = 0;


// "should_run" indicates whether the currently running experiment should
// continue to run or to instead drop out of its main loop
bool volatile  should_run = false;


// This function is for experiments to run code when counter3 overflows:
VoidFunction *counter3_overflow_hook = NULL;

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
  // Let the experiment hang code off the counter3 overflow interrupt too:
  if ( counter3_overflow_hook != NULL)
    counter3_overflow_hook();
}


// A super-primitive debugging where messages are shown first at the bottom of
// the screen, then further up and finally wrapping around to the bottom and
// overwriting the older messages
void db( char *message)
{
  int static  y = 0;
  int  x = 0;
  __Display_Str( x, y, WHITE, 0, message);
  y += FONT_HEIGHT;
  if ( 240 - FONT_HEIGHT*2 < y)
    y = 0;
}


void render()
{
  char static *blank_line = "                                                  ";
  int  y = SCREEN_HEIGHT - FONT_HEIGHT;
  __Display_Str( 0, y, WHITE, 1, experiment[experiment_cursor].name);
  int  len = strlen( experiment[experiment_cursor].name);
  __Display_Str( FONT_WIDTH*len, y, WHITE, 0, blank_line + len);
}


int main()
{
  InputEvent  ev;

  attach_handler( TIMER3_INTERRUPT, when_counter3_overflows);
  __Set( BEEP_VOLUME, 0);
  __Clear_Screen( BLACK);

  while( true)
  {
    check_event( &ev);
    switch( ev.input)
    {
      case LROCKER_LEFT:
        if ( INPUT_PRESSED == ev.state)
          dump_cursor -= 8;
        break;
      case LROCKER_RIGHT:
        if ( INPUT_PRESSED == ev.state)
          dump_cursor += 8;
        break;
      case RROCKER_LEFT:
        if ( INPUT_RELEASED == ev.state)
          experiment_cursor = (experiment_cursor - 1 + ITEMS_IN_ARRAY( experiment)) % ITEMS_IN_ARRAY( experiment);
        break;
      case RROCKER_RIGHT:
        if ( INPUT_RELEASED == ev.state)
          experiment_cursor = (experiment_cursor + 1) % ITEMS_IN_ARRAY( experiment);
        break;
      case RROCKER_DOWN:
        if ( INPUT_RELEASED == ev.state)
          should_run = true;
        break;
      default:
        break;
    }

    //render_dump();
    render();

    if ( should_run)
    {
      experiment[experiment_cursor].main();
      counter3_overflow_hook = NULL;
      __Clear_Screen( BLACK);
    }
  }
}

