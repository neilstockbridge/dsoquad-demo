
/*
Screen layout:

 + Top row is ( from L to R): labels for four buttons then left editor and right editor under the rockers
 + Next row is configurable display slots ( to show batt remain, Vpp, etc.), OR
 + maybe instead of general-purpose, need to think of scenarios.  when want to set up wave gen, prob want to edit form, freq and duty all at once
 + could use DOWN to cycle between a few editor attached to (say) left rocker.  since DOWN on either rocker could change editors attached to BOTH rockers, or DOWN on LROCKER could mean "finished setting up"

 + Scenarios:
   + set up channel.  first choose channel ( often only want to conf one)
     + coupling AC/DC/off
     + V per div
     + time per div
     + xpos
     + ypos
   + choose trigger ( source, condition)
     + cap buf size and cap mode
   + measure wave ( v1,v2,t1,t2)
     + pan view of buffer ( maybe only in HOLD mode)
   + setup wave generator
   + setup backlight and volume ( prob not often)

 + maybe long hold rocker down could mean "finished setting up, return to panning view pls"
 + maybe black buttons on white bg will make the soft button labels look more attached to the physical buttons
 + press Setup.  shows opts for buts 1..3.  but4 changes to More.  press but4 to get more opts for buts1..3.  when run out of opts but4 says "Back", which goes back to normal mode.  Option "channel" presents new menu where buts1..4 select channel A..D to load in to rocker editor
 + when press rocker down to cycle editors, briefly shows what the editor is, such as "coupling"

Editors:

 + backlight: 0 - 100 in steps of 10
 + volume: as backlight
 + V1, V2
 + T1, T2
 + pan view of capture buffer
 + trigg event: rising, falling, </> Vt, </> TL, </> TH  &Delta;T = T2 - T1
 + trigg source: channel A-D
 + size of cap buffer: 360 .5k 1k 2k 4k
 + cap mode: AUTO, NORM, SINGL, NONE, SCAN
 + gen wave: square, tri, saw, sine
 + gen freq: 10, 20 50 100 200 500 1k 2k 5k 10k 20k
 + gen dut: 50-90 in step 10

For each channel:

 + off/AC/DC
 + V per div
 + time per div
 + xpos
 + ypos

Displays:

 + V1
 + V2
 + T1
 + T2
 + Delta T
 + Delta V
 + battery time remaining + cute icon.  always
 + Vpp peak2peak?
 + Vdc ( average V)
 + RMS
 + Min/Max V
 + FRQ ( frequency)
 + CIR ( period?)
 + DUT ( duty cycle)
 + TH ( time high?)
 + TL ( time low)

Help text explaining labels


There is the concept of "widgets".  Each widget is set up with its location on
screen and knows how to render itself.  Instead of a monolithic render()
function, the display can be rendered by instructing each widget in a list to
render itself.

An *indicator* is a widget that shows some information.

An *editor* is a widget that is attached to either the left or right rocker
switches and that changes a value in response to input from the switch.

*/

#include <stdbool.h>
#include "BIOS.h"
#include "input.h"
#include "UI.h"


void static when_counter3_overflows()
{
  // The buttons are read only every 20 milliseconds to overcome switch bounce
  u8 static  when_to_read_inputs = 20; // milliseconds from now

  if ( 0 < when_to_read_inputs)
  {
    when_to_read_inputs -= 1;
  }
  else {
    check_inputs();
    when_to_read_inputs = 20;
  }
}


void TIM3_IRQHandler(void)
{
  when_counter3_overflows();
  __Set( KEY_IF_RST, 0);  // Clear TIM3 interrupt flag
}


s8  brightness = 80; // of the screen
s8  volume = 30;     // of the beeper
s8  duty_cycle = 50; // for the wave form generator


void brightness_changed( Indicator *idct)
{
  PercentContext *ct = idct->context;
  __Set( BACKLIGHT, *ct->value);
}

PercentContext  brightness_context = {
  value:  &brightness,
  min:    10,
  max:    100,
  update: brightness_changed,
};


void volume_changed( Indicator *idct)
{
  PercentContext *ct = idct->context;
  __Set( BEEP_VOLUME, *ct->value);
}

PercentContext  volume_context = {
  value:  &volume,
  min:    0,
  max:    100,
  update: volume_changed,
};


int main()
{
  __Set( BEEP_VOLUME, 0);

  init_UI();

  while( true)
  {
    check_for_input();
    render();
  }
}

