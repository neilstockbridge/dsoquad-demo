
// This experiment exercises the __Get BIOS API call.

#include "BIOS.h"
#include "main.h"
#include "input.h"
#include "display.h"
#include "hex.h"


#define  COLUMNS  ( SCREEN_WIDTH / FONT_HEIGHT)
#define  LABEL_X  ( FONT_WIDTH * (COLUMNS - (13+1+8)) / 2 )  // 13 is "Serial Number".length
#define  VALUE_X  ( LABEL_X + FONT_WIDTH*(13+1) )


void static update( u32 value, u8 row)
{
  char  hex[9];
  hex[8] = '\0';
  u32_to_hex( value, hex);
  draw_text( VALUE_X, FONT_HEIGHT*row, RGB(0xff, 0x77, 0x00), 0, hex);
}


void static render()
{
  update(__Get(USB_POWER), 6); // 1:power provided by USB
  update(__Get(V_BATTERY), 5); // mV
  update(__Get(CHARGE), 4);
}


bool volatile should_render = false;


void static when_counter3_overflows()
{
  u16 static to_next_second; // Doesn't seem to take an initial value
  if ( 0 < to_next_second  &&  to_next_second <= 1000)
    to_next_second -= 1;
  else {
    should_render = true;
    to_next_second = 1000;
  }
}


typedef struct
{
  char const *label;
  u32         value;
}
Object;


void Get_main()
{
  InputEvent  ev;
  int         i;

  Object objects[] =
  {
    {"Serial-Number", __GetDev_SN() },
    {"HDWVER",        __Get(HDWVER) },
    {"DFUVER",        __Get(DFUVER) },
    {"SYSVER",        __Get(SYSVER) },
    {"FPGAVER",       __Get(FPGAVER) },
    {"FPGA_OK",       __Get(FPGA_OK) },
    {"USB_POWER",     0 },
    {"V_BATTERY",     0 },
    {"CHARGE",        0 },
  };
  for ( i = 0;  i < ITEMS_IN_ARRAY(objects);  i += 1)
  {
    Object *oj = &objects[ i];
    u8  row = 12 - i;
    draw_text( LABEL_X, FONT_HEIGHT*row, WHITE, 0, oj->label);
    update( oj->value, row);
  }

  counter3_overflow_hook = when_counter3_overflows;

  while( should_run)
  {
    check_event( &ev);
    if ( BUTTON4 == ev.input)
      should_run = false;

    // Only render once each second
    if ( should_render)
    {
      __Set( BETTERY_DT, 1);
      render();
      should_render = false;
    }
    else
      __WFE();
  }
}

