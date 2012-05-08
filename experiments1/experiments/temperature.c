
// This experiment exercises the internal temperature sensor.
//
// Notes on the temperature sensor and the ADC:
//  + Vref+ is tied to 2.8V
//  + Vref- is tied to GND
//  + I assume since the reference manual does not say that:
//      vSENSE = (vREFplus - vREFminus) * ADC->DR / 2**12
//  + The internal temperature sensor:
//    + is wired to ADC channel 16
//    + is only connected to ADC1
//    + requires a 17.1us conversion time
//    + is enabled by setting TSVREFE to 1
//  + Calibration of the ADC improves accuracy.  Set CR2.CAL=1 while ADON=0.
//    Finished when CAL=0
//  + CR2.ADON=1 when ADON=0 powers up the ADC.  wait a bit before beginning
//    conversions
//  + CR2.ADON=1 when ADON=1 already begins a conversion
//  + the ADC result is in DR
//  + CR2.ALIGN = 0(right), 1(left) right aligned means LSB of DR is LSB of ADC
//    result
//  + ADCSMPRx set the sample time for each individual channel
//      0: 1.5 cycles
//      1: 7.5 cycles
//      2: 13.5 cycles
//      3: 28.5 cycles
//      4: 41.5 cycles
//      5: 55.5 cycles
//      6: 71.5 cycles
//      7: 239.5 cycles
//  + The ADC clock should not exceed 14 MHz and is formed by dividing SYSCLK
//    ( 72 MHz) first by the AHB prescaler ( HPRE bits) then the ADC prescaler
//    ( ADCPRE bits) in the RCC_CFGR register.  See RCC_CFGR_HPRE_DIVx and
//    RCC_CFGR_ADCPRE_DIVx constants in stm32f10x.h
//  + The default value of RCC->CFGR ( whether set by hardware or BIOS I don't
//    know) is 001d840a, which means HPRE=0 and ADCPRE=2 ( div 6)
//      => ADC clock = 72 / 6 = 12 MHz
//  + temperature_in_C = (V25 - vSENSE) / Avg_slope + 25
//      where V25 = what Vsense reads at 25 degC ( typically 1.43V)
//      Avg_slope is typically 4.3mV/degC
//

#include "BIOS.h"
#include "main.h"
#include "input.h"
#include "display.h"
#include "hex.h"


#define  GRAY  RGB( 128, 128, 128)


bool volatile should_update = false;


void static when_counter3_overflows()
{
  u16 static to_next_second; // Doesn't seem to take an initial value
  if ( 0 < to_next_second  &&  to_next_second <= 1000)
    to_next_second -= 1;
  else {
    should_update = true;
    to_next_second = 1000;
  }
}


void static update()
{
  // Begin a conversion:
  ADC1->CR2 |= ADC_CR2_SWSTART;
  // Wait for the conversion to finish:
  while ( ! (ADC1->SR & ADC_SR_EOC) );

  // V25 = 1.43
  // Avg_slope = 4.3
  // vREFplus = 2.8
  // vREFminus = 0
  // vSENSE = (vREFplus - vREFminus) * ADC1->DR / 2**12
  // vSENSE = 2.8 * ADC1->DR / 4096
  // temperature_in_C = (V25 - vSENSE) / Avg_slope + 25
  // temperature_in_C = (1.43 - 2.8 * ADC1->DR / 4096) / 0.0043 + 25
  //                  = 1.43 / 0.0043 - (2.8 / 0.0043) * ADC1->DR / 4096 + 25
  //                  = 332 - 651 * ADC1->DR / 4096 + 25
  //                  = 357 - 651 * ADC1->DR / 4096
  // To maximize precision:
  //  highest value for ADC1->DR is 4096
  //  highest value of "651" that won't overflow s32: 524288 = 2**31 / 4096
  //  actual value to multiply 651 by in order to use shift rather than divide: 512.  651*512 = 333312 < 524288
  // Therefore:
  //  + use 183070 ( (1.43 / 0.0043 + 25) * 512) in place of 357
  //  + and 333395 ( 2.8 / 0.0043 * 512) in place of 651
  //  + and undo the * 512 with a >> 9
  // NOTE: The & 0xfff below because the MS 16 bits of DR contain the reading
  // from ADC2
  s8  temperature_in_C = ( 183070 - (333395 * (ADC1->DR & 0xfff) >> 12) ) >> 9;
  // Constrain temperature_in_C simply because the i_to_a below can't handle
  // negative numbers or more than 2 digits
  if ( temperature_in_C < 0)
    temperature_in_C = 0;
  if ( 99 < temperature_in_C)
    temperature_in_C = 99;
  char  m[] = "12";
  m[ 0] = '0'+ temperature_in_C / 10;
  m[ 1] = '0'+ temperature_in_C % 10;
  draw_text( 0, 0, WHITE, 0, m);
}


void temperature_main()
{
  InputEvent  ev;

  counter3_overflow_hook = when_counter3_overflows;

  // Request that the ADC calibrate and then wait for it:
  ADC1->CR2 |= ADC_CR2_CAL;
  while( ADC1->CR2 & ADC_CR2_CAL);

  // Switch the ADC on:
  ADC1->CR2 |= ADC_CR2_ADON;

  // Switch on the temperature sensor and set the ADC DR as right-aligned:
  ADC1->CR2 |= ADC_CR2_TSVREFE;
  ADC1->CR2 &= ~ADC_CR2_ALIGN;
  // Should wait a little while for the ADC and the sensor to settle after
  // powering on but since we make a new reading every second, simply discard
  // the first reading

  // "select channel 16" means make a regular sequence of length 1
  // There is only 1 channel in the sequence ( channel 16):
  ADC1->SQR1 = (1 & 0xf) << 20; // 20:offset of "L"
  ADC1->SQR3 = (16 & 0x1f) << 0; // 0:offset of SQ1

  // Samples for 239.5 cycles (@ 14 MHz that's 17.1us)
  ADC1->SMPR1 = (7 & 0x7) << 18; // 18:offset of SMP16;

  // Draw a degrees symbol
  __LCD_Set_Block( FONT_WIDTH*2, FONT_WIDTH*2+2, FONT_HEIGHT-3, FONT_HEIGHT-3+2);
  u16 static const degrees_symbol[] =
  {
    // Remember, these begin in the bottom left and go up first then across
    GRAY, WHITE, GRAY,
    WHITE, BLACK, WHITE,
    GRAY, WHITE, GRAY,
  };
  __LCD_Copy( degrees_symbol, ITEMS_IN_ARRAY(degrees_symbol) );
  draw_text( FONT_WIDTH*2+3, 0, WHITE, 0, "C");

  while( should_run)
  {
    check_event( &ev);
    if ( BUTTON4 == ev.input)
      should_run = false;

    // Only render once each second
    if ( should_update)
    {
      update();
      should_update = false;
    }
    else
      __WFE();
  }
}

