
/*

A font with black and white ( no anti-aliasing) glyphs.

*/

#ifndef __FONT_H
#define __FONT_H


#include "BIOS.h"


// This format is packed quite tightly:
//  + Blank rows and columns are not stored, which is why the width and height
//    are needed for each glyph
//  + Glyph pixels are stored in a stream of bits.  Only glyphs themselves are
//    byte-aligned
//
// Every glyph gets a blank column at the left AND right ( so that a single
// reversed char has a border), although the advance is only width+1, so that
// the next glyph's left border will overwrite the right border of this one.
//
typedef struct
{
  u16  data;     // address of the glyph data relative to the beginning of the glyph data
  u8   width:6;  // number of stored pixels across the glyph
  u8   height:6; // number of stored pixels up the glyph
  u8   lift:4;   // numbers of pixels from the bottom of the glyph cell to the first row that contains part of the glyph.  This avoids the need to store blank rows while maintaining vertical alignment with other glyphs
}
Glyph;

typedef struct
{
  u8           height;  // number of pixels up each glyph cell
  u8           first_character;  // ASCII encoding of first glyph
  u16          glyphs;  // number of glyphs
  Glyph const *glyph;
  u8    const *glyph_data;
  u8           absent_code; // the code for the glyph to show when this font doesn't support the requested code point
}
Font;


extern
void use_font( Font const *font)
;

extern
void text_color( u16 foreground_color, u16 background_color)
;

// Draws around each glyph, so a 3x5 glyph with lift=1 in a font with height:7 will get:
//
// -----
// --x--
// -x-x-
// -xxx-
// -x-x-
// -x-x-
// -----
//
// i.e. a column of background color on the left and right, "lift" rows of
// background color at the bottom and rows of background color at the top to
// pad to font.height.
//
extern
void render_text( char const *text, u16 left, u16 bottom)
;


// Provides the number of pixels across required to render "text", including
// leading and trailing background columns
extern
u16 width_of_text( char const *text)
;


#endif

