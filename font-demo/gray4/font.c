
#include "core.h"
#include "display.h"
#include "font.h"


// The Glyph structure supports glyphs up to 63x63 but this would require 32K of stack for draw_char!
#define  MAX_FONT_WIDTH   8
#define  MAX_FONT_HEIGHT  16


Font const *font             = NULL;
u16         shade[] = { BLACK, RGB(0x55,0x55,0x55), RGB(0xaa,0xaa,0xaa), WHITE };


void use_font( Font const *f)
{
  font = f;
}


void text_color( u16 foreground_color, u16 background_color)
{
  shade[ 0] = background_color;
  shade[ 1] = RGB(
    ( RED_IN(background_color)*2 + RED_IN(foreground_color) ) / 3,
    ( GRN_IN(background_color)*2 + GRN_IN(foreground_color) ) / 3,
    ( BLU_IN(background_color)*2 + BLU_IN(foreground_color) ) / 3
  );
  shade[ 2] = RGB(
    ( RED_IN(background_color) + RED_IN( foreground_color)*2 ) / 3,
    ( GRN_IN(background_color) + GRN_IN( foreground_color)*2 ) / 3,
    ( BLU_IN(background_color) + BLU_IN( foreground_color)*2 ) / 3
  );
  shade[ 3] = foreground_color;
}


u16 width_of_text( char const *text)
{
  char         code;
  Glyph const *glyph;
  u16          width = (*text != '\0') ? 0 : -1; // empty str should be 0 width

  while( (code = *text) != '\0')
  {
    if ( code < font->first_character  ||  font->glyphs <= code - font->first_character)
      code = font->absent_code;

    glyph = &font->glyph[ code - font->first_character];
    width += 1 + glyph->width;

    text += 1;
  }

  return width + 1;
}


Glyph const static *render_code( char code, u16 *pixel)
{
  int          x;
  int          y;
  Glyph const *glyph;
  u8    const *glyph_data;
  int          position; // position within the data byte of the next glyph pixel

  if ( code < font->first_character  ||  font->glyphs <= code - font->first_character)
    code = font->absent_code;

  // Look up the details of the glyph for "code" such as how wide it is and
  // where the glyph data is
  glyph = &font->glyph[ code - font->first_character];
  glyph_data = &font->glyph_data[ glyph->data >> 2];
  position = (glyph->data & 0x3) << 1;

  for ( x = 0;  x < glyph->width + 1;  x += 1)
  {
    for ( y = 0;  y < font->height;  y += 1, pixel += 1)
    {
      if ( y < glyph->lift  ||  (glyph->lift + glyph->height) <= y  ||  glyph->width == x)
      {
        *pixel = shade[ 0];
      }
      else {
        *pixel = shade[ (*glyph_data >> position) & 0x3];
        position += 2;
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


void render_text( char const *text, u16 left, u16 bottom)
{
  // There are two glyph buffers so that one can be filled by the CPU while the
  // other is making its way to the LCD via DMA:
  u16          buffer[2][ (1 + MAX_FONT_WIDTH + 1) * MAX_FONT_HEIGHT];
  int          bf = 0; // indicates the glyph buffer being filled
  char         code;
  Glyph const *glyph;
  // Fill the first column of font->height pixels in each buffer with bg_color
  // to provide space around the glyphs in inverted mode and space between the
  // glyphs in either mode
  int  y;
  for ( y = 0;  y < font->height;  y += 1)
  {
    buffer[ 0][ y] = buffer[ 1][ y] = shade[ 0];
  }
  while( (code = *text) != '\0')
  {
    glyph = render_code( code, (buffer[bf] + font->height) );
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

