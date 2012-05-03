
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
#include "colors.h"
#include "hex.h"
#include "input.h"
#include "main.h"


extern void buttons_main();
extern void SysTick_main();

typedef struct
{
  char         *name;
  VoidFunction *main;
}
Experiment;

Experiment static experiment[] =
{
  {name:"buttons", main:buttons_main},
  {name:"SysTick", main:SysTick_main},
};

int  experiment_cursor = 0;


// "should_run" indicates whether the currently running experiment should
// continue to run or to instead drop out of its main loop
bool volatile  should_run = false;

// Pull this in from BIOS.h
extern VoidFunction *counter3_overflow_handler;

void when_counter3_overflows()
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
  int  x = 0;
  int  y = SCREEN_HEIGHT - FONT_HEIGHT;
  __Display_Str( x, y, WHITE, 1, experiment[experiment_cursor].name);
}


u8 *dump_cursor = (u8*) 0x20003000;

void render_dump()
{
  // 400x240 screen with 8x14 glyphs gives a character matrix of 50x17
  // Show the address on the left (8 digits) then space then 8x 2 digits plus space then 8 characters
  char  line[] = "12345678 12 12 12 12-12 12 12 12 abcdefgh";
  //char  line[] = "1234567812345678-1234567812345678 abcdefghijklmnop";
  int  row;
  for ( row = 0;  row < 16;  row += 1)
  {
    u8 *ptr = dump_cursor + 8 * row;
    u32_to_hex( (u32) ptr, line);
    int  ofs;
    for ( ofs = 0;  ofs < 8;  ofs += 1)
    {
      u8  value = ptr[ ofs];
      u8_to_hex( value, &line[ 9 + 3*ofs]);
      line[ 9 + 3*8 + ofs] = 32 <= value && value < 127 ? value : '.';
    }
    __Display_Str( 0, SCREEN_HEIGHT - FONT_HEIGHT*1 - FONT_HEIGHT * row - FONT_HEIGHT, WHITE, 0, line);
  }
}


int main()
{
  InputEvent  ev;

  counter3_overflow_handler = when_counter3_overflows;
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
          experiment_cursor = (experiment_cursor - 1) % ITEMS_IN_ARRAY( experiment);
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
      __Clear_Screen( BLACK);
    }
  }
}

