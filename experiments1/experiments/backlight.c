
// This experiment tries out the backlight and beeper controls

#include "BIOS.h"
#include "main.h"
#include "input.h"
#include "display.h"
#include "hex.h"


u8 volatile value;
u8 volatile target_value;
u8 volatile when_to_move = -1; // number of ticks before moving value towards target_value


int inline abs( int value)
{
  return (0 < value) ? value : -value;
}


void draw_line( u8 value, u8 scale,int y, u32 fg_color, u32 bg_color)
{
  u16 pixels[ 100];
  int  i;
  for ( i = 0;  i < scale;  i += 1)
  {
    pixels[ i] = (i < value) ? fg_color : bg_color;
  }
  __LCD_Set_Block( 0, 49, y, y+1);
  __LCD_DMA_Ready();
  __LCD_Copy( pixels, scale);
}


void static when_counter3_overflows()
{
  if ( 0 < when_to_move)
  {
    when_to_move -= 1;
  }
  else {
    if ( 0 == when_to_move)
    {
      if ( value < target_value)
        value += 1;
      else if ( target_value < value)
        value -= 1;
    }
    // When when_to_move is 0, value moves towards target_value at a rate of
    // 1000 moves per second, which would go from 0 to 100 in 0.1s
    s8  alpha = 100 - abs( target_value - value);
    when_to_move = (alpha < 50) ? (49 - alpha) : (alpha - 50);
    // ap:0,wtm:49  ap:49,wtm:0  ap:50,wtm:0  ap:99,wtm:49
    draw_line( when_to_move, 50, FONT_HEIGHT, RGB(0xee, 0xff, 0x77), RGB(0x66, 0x66, 0x66));
    draw_line( value, 100, FONT_HEIGHT*2, RGB(0xee, 0xff, 0x77), RGB(0x66, 0x66, 0x66));
  }
}


void backlight_main()
{
  InputEvent  ev;

  counter3_overflow_hook = when_counter3_overflows;

  draw_text( FONT_WIDTH*(50-20)/2, FONT_HEIGHT*(17-1)/2, WHITE, 1, "Backlight-experiment");

  value = 0;
  target_value = 100;
  while ( value != target_value)
  {
    __WFE(); // Go to low-power mode until counter3 overflows
    __Set( BEEP_VOLUME, value);
  }
  target_value = 0;
  while ( value != target_value)
  {
    __WFE();
    __Set( BEEP_VOLUME, value);
  }

  while( should_run)
  {
    check_event( &ev);
    if ( BUTTON4 == ev.input)
      should_run = false;

    while ( value != target_value)
    {
      __WFE();
      __Set( BACKLIGHT, value);
    }
    target_value = (100 == value) ? 0 : 100;
  }
}

