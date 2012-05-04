
// This experiment shows which glyphs the BIOS supports by drawing them all on
// the screen.

#include "BIOS.h"
#include "display.h"
#include "main.h"
#include "input.h"


void static render()
{
  // The screen is 400x240 and the font 8x14, which results in a 50x17
  // character matrix, more than enough for 32x8 glyphs
  char  line[ 32+1];
  line[ 32] = '\0';
  int  row, column;
  for ( row = 0;  row < 8;  row += 1)
  {
    for ( column = 0;  column < 32;  column += 1)
    {
      line[ column] = 32*row+ column;
    }
    int  x = FONT_WIDTH * (50 - 32) / 2;
    int  y = FONT_HEIGHT * row + (17 - 8) / 2;
    int  mode = 0; // non-inverted
    __Display_Str( x, (SCREEN_HEIGHT - y - FONT_HEIGHT), WHITE, mode, line);
  }
}


void glyphs_main()
{
  InputEvent  ev;
  render();
  while( should_run)
  {
    check_event( &ev);
    if ( BUTTON4 == ev.input)
      should_run = false;
  }
}

