
#include "hex.h"


char inline hex_digit( u8 digit)
{
  //assert( digit < 16);
  return (digit <= 9) ? ('0' + digit) : ('a' + (digit - 10));
}


void u8_to_hex( u8 value, char *s)
{
  s[ 0] = hex_digit( value >> 4);
  s[ 1] = hex_digit( value & 0xf);
}


void u16_to_hex( u16 value, char *s)
{
  u8  p;
  s8  os;
  for ( os = 8*sizeof(u16) - 4, p = 0;  0 <= os;  os -= 4, ++ p )
  {
    s[ p] = hex_digit( value >> os & 0xf);
  }
}


void u32_to_hex( u32 value, char *s)
{
  u8  p;
  s8  os;
  for ( os = 8*sizeof(u32) - 4, p = 0;  0 <= os;  os -= 4, ++ p )
  {
    s[ p] = hex_digit( value >> os & 0xf);
  }
}

