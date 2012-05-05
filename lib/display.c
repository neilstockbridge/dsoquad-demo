
#include "BIOS.h"
#include "display.h"


void draw_text( int left, int bottom, u16 color, int mode, char const *text)
{
  // There are two glyph buffers so that one can be rendered while the other is
  // making its way to the LCD:
  u16  buffer[2][ FONT_WIDTH* FONT_HEIGHT];
  int  bf = 0; // indicates the glyph buffer being filled
  char  ch;
  int  x, y;
  u16  bg_color = mode ? color : BLACK;
  u16  fg_color = mode ? BLACK : color;
  while( (ch = *text) != '\0')
  {
    u16  *pixel = buffer[ bf];
    for ( x = 0;  x < FONT_WIDTH;  x += 1)
    {
      u16  strip = __Get_TAB_8x14( ch, x);
      // putting "strip >>= 1" in the loop actually make it slower
      for ( y = 16 - FONT_HEIGHT;  y < 16;  y += 1, pixel += 1)
      {
        *pixel = (strip >> y) & 0x1 ? fg_color : bg_color;
      }
    }
    __LCD_Set_Block( left, left+FONT_WIDTH-1, bottom, bottom+FONT_HEIGHT-1);
    __LCD_DMA_Ready();
    __LCD_Copy( buffer[bf], sizeof(buffer[0]) / sizeof(u16) );
    text += 1;
    left += FONT_WIDTH;
    bf ^= 1;
  }
  // Shouldn't really be required but some buggy routines initiate DMA transfer
  // without waiting for DMA to be free first:
  __LCD_DMA_Ready();
}

