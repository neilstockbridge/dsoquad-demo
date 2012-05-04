
#include "BIOS.h"
#include "hex.h"
#include "display.h"


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

