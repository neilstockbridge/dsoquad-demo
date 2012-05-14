
#include "font.h"


#ifndef NULL
#define  NULL  0
#endif


// The Glyph structure supports glyphs up to 63x63 but this would require 16K of stack for draw_char!
#define  MAX_FONT_WIDTH   8
#define  MAX_FONT_HEIGHT  16


Font const *font = NULL;


void use_font( Font const *f)
{
  font = f;
}


Glyph const static *draw_char( u16 left, u16 bottom, u16 bg_color, u16 fg_color, char code, u16 *pixel)
{
  int          x;
  int          y;
  Glyph const *glyph;
  u8    const *glyph_data;
  int          position = 0; // position within the data byte of the next glyph pixel

  if ( code < font->first_character  ||  font->glyphs <= code - font->first_character)
    code = font->absent_code;

  // Look up the details of the glyph for "code" such as how wide it is and
  // where the glyph data is
  glyph = &font->glyph[ code - font->first_character];
  glyph_data = &font->glyph_data[ glyph->data];

  for ( x = 0;  x < glyph->width + 1;  x += 1)
  {
    for ( y = 0;  y < font->height;  y += 1, pixel += 1)
    {
      if ( y < glyph->lift  ||  (glyph->lift + glyph->height) <= y  ||  glyph->width == x)
      {
        *pixel = bg_color;
      }
      else {
        *pixel = ( (*glyph_data >> position) & 0x1) ? fg_color : bg_color;
        position += 1;
        if ( 8 == position)
        {
          glyph_data += 1;
          position = 0;
        }
      }
    }
  }
  return glyph;
}


void render_text( u16 left, u16 bottom, u16 color, u8 mode, char const *text)
{
  // There are two glyph buffers so that one can be rendered while the other is
  // making its way to the LCD:
  u16  buffer[2][ (1 + MAX_FONT_WIDTH + 1) * MAX_FONT_HEIGHT];
  int  bf = 0; // indicates the glyph buffer being filled
  char  ch;
  u16  bg_color = mode ? color : 0x0000;
  u16  fg_color = mode ? 0x0000 : color;
  Glyph const *glyph;
  // Fill the first column of font->height pixels in each buffer with bg_color
  // to provide space around the glyphs in inverted mode and space between the
  // glyphs in either mode
  int  y;
  for ( y = 0;  y < font->height;  y += 1)
  {
    buffer[ 0][ y] = bg_color;
    buffer[ 1][ y] = bg_color;
  }
  while( (ch = *text) != '\0')
  {
    glyph = draw_char( left, bottom, bg_color, fg_color, ch, (buffer[bf] + font->height) );
    __LCD_DMA_Ready();
    __LCD_Set_Block( left, left+glyph->width+2, bottom, bottom+font->height-1);
    __LCD_Copy( buffer[bf], font->height * (1 + glyph->width + 1) );
    text += 1;
    left += glyph->width + 1;
    bf ^= 1;
  }
  // Shouldn't really be required but some buggy routines initiate DMA transfer
  // without waiting for DMA to be free first:
  __LCD_DMA_Ready();
}

