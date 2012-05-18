
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

 + xpos is scrolling thru buff I think
 + time per div
 + seems Vthresh & Tthresh can be set regardless of the measuring marks

For each channel:

 + off/AC/DC
 + V per div
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

*/

#include <stdbool.h>
#include "BIOS.h"
#include "core.h"
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

void USB_HP_CAN_TX_IRQHandler(void)
{
  __CTR_HP();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
  __USB_Istr();
}


s8  screen_brightness    = 80;
s8  beep_volume          = 30;
u8  generator_waveform   = 0;
u8  generator_frequency  = 0;
s8  generator_duty_cycle = 50;

u8  trigger_source = 0;
u8  trigger_condition = 0;


u8 static selected_value( SelectContext *ct)
{
  return ct->option[ct->selected].value;
}

#define  PERCENT_CONTEXT( var, mn, mx)  PercentContext const var##_context = { value:&var, min:mn, max:mx, update:var##_changed }

void screen_brightness_changed( PercentContext *ct)
{
  __Set( BACKLIGHT, *ct->value);
}

PERCENT_CONTEXT( screen_brightness, 10, 100);


void beep_volume_changed( PercentContext *ct)
{
  __Set( BEEP_VOLUME, *ct->value);
}

PERCENT_CONTEXT( beep_volume, 0, 100);


void generator_duty_cycle_changed()
{
  // TODO
}

PERCENT_CONTEXT( generator_duty_cycle, 50, 90);


typedef void (SelectionChangeHandler)( SelectContext *ct);

#define  SELECT_CONTEXT( var)  SelectContext  var##_context = { option:var##_options, options:ITEMS_IN_ARRAY(var##_options), selected:0, update:var##_changed }

// The context needs to refer to the "changed" method and the methods need to
// refer to the contexts, so one or the other has to be forward declared:
//SelectionChangeHandler  trigger_source_changed, trigger_condition_changed;

SelectOption const coupling_options[] =
{
  { label:"AC", value: 0 },
  { label:"DC", value: 1 },
  { label:"off", value: 2 },
};

void coupling_changed( SelectContext *ct)
{
}

SELECT_CONTEXT( coupling);


SelectOption const voltage_scaling_options[] =
{
  { label:"50mV", value: ADC_50mV },
  { label:"0.1V", value: ADC_100mV },
  { label:"0.2V", value: ADC_200mV },
  { label:"0.5V", value: ADC_500mV },
  { label:"1V",   value: ADC_1V },
  { label:"2V",   value: ADC_2V },
  { label:"5V",   value: ADC_5V },
  { label:"10V",  value: ADC_10V },
};

void voltage_scaling_changed( SelectContext *ct)
{
}

SELECT_CONTEXT( voltage_scaling);


SelectOption const time_per_div_options[] =
{
  { label:"0.1us", value: 0 },
  { label:"0.2us", value: 0 },
  { label:"0.5us", value: 0 },
  { label:"1us", value: 0 },
  { label:"2us", value: 0 },
  { label:"5us", value: 0 },
  { label:"10us", value: 0 },
  { label:"20us", value: 0 },
  { label:"50us", value: 0 },
  { label:"100us", value: 0 },
  { label:"200us", value: 0 },
  { label:"500us", value: 0 },
  { label:"1ms", value: 0 },
  { label:"2ms", value: 0 },
  { label:"5ms", value: 0 },
  { label:"10ms", value: 0 },
  { label:"20ms", value: 0 },
  { label:"50ms", value: 0 },
  { label:"100ms", value: 0 },
  { label:"200ms", value: 0 },
  { label:"500ms", value: 0 },
  { label:"1s", value: 0 },
};


SelectOption const trigger_source_options[] =
{
  { label:"Ch A", value: 0x00 },
  { label:"Ch B", value: 0x08 },
  { label:"Ch C", value: 0x10 },
  { label:"Ch D", value: 0x18 },
  { label:"Unco", value: 0x20 },
};
void static setup_trigger();
void trigger_source_changed( SelectContext *ct)
{
  trigger_source = selected_value( ct);
  setup_trigger();
}
SELECT_CONTEXT( trigger_source);

SelectOption const trigger_condition_options[] =
{
  { label:"falling", value: 0 },
  { label:"rising", value: 1 },
  { label:"low", value: 2 },
  { label:"high", value: 3 },
  { label:"TL < dT", value: 4 }, // dT = T2 - T1
  { label:"TL > dT", value: 5 },
  { label:"TH < dT", value: 6 },
  { label:"TH > dT", value: 7 },
};
void trigger_condition_changed( SelectContext *ct)
{
  trigger_condition = selected_value( ct);
  setup_trigger();
}
SELECT_CONTEXT( trigger_condition);


SelectOption const buffer_size_options[] =
{
  { label:"360b", value: 0 },
  { label:"512b", value: 0 },
  { label:"1K", value: 0 },
  { label:"2K", value: 0 },
  { label:"4K", value: 0 },
};

SelectOption const capture_mode_options[] =
{
  { label:"AUTO", value: 0 },
  { label:"NORM", value: 0 },
  { label:"SINGL", value: 0 },
  { label:"NONE", value: 0 },
  { label:"SCAN", value: 0 },
};

SelectOption const generator_waveform_options[] =
{
  { label:"square", value: 0 },
  { label:"triangle", value: 0 },
  { label:"sawtooth", value: 0 },
  { label:"sine", value: 0 },
};
void static generator_waveform_changed( SelectContext *ct)
{
  // FIXME: Use generic code to copy the selected value to the variable and store the address of the variable in the context
  generator_waveform = selected_value( ct);
}
SELECT_CONTEXT( generator_waveform);

SelectOption const generator_frequency_options[] =
{
  { label:"10Hz", value: 0 },
  { label:"20Hz", value: 0 },
  { label:"50Hz", value: 0 },
  { label:"100Hz", value: 0 },
  { label:"200Hz", value: 0 },
  { label:"500Hz", value: 0 },
  { label:"1kHz", value: 0 },
  { label:"2kHz", value: 0 },
  { label:"5kHz", value: 0 },
  { label:"10kHz", value: 0 },
  { label:"20kHz", value: 0 },
};
void static generator_frequency_changed( SelectContext *ct)
{
  generator_frequency = selected_value( ct);
}
SELECT_CONTEXT( generator_frequency);


// Invoked when either of the two trigger parameters are changed
void static setup_trigger()
{
  u8  trigger_details = trigger_source | trigger_condition;
  printf("tg:%02x\n", trigger_details);
  __Set( TRIGG_MODE, trigger_details);
}


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

