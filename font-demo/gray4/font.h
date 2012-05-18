
/*

A font where each glyph pixel is one of 4 shades encoded in 2 bits, to provide
a degree of anti-aliasing.  The four shades are:

  + black
  + 33%
  + 67%
  + white

*/

#ifndef __FONT_H
#define __FONT_H


#include "BIOS.h"


// This format is packed quite tightly:
//  + Blank rows and columns are not stored, which is why both the height and
//    the width are encoded on a per-glyph basis
//  + Glyph pixels are stored in a stream of ( pairs of) bits.  Only glyphs
//    themselves are byte-aligned
//
// Every glyph gets a blank column at the left AND right ( so that a single
// reversed char has a border), although the advance is only width+1, so that
// the next glyph's left border will overwrite the right border of this one.
//
// FIXME: Only the last glyph should render it right-hand-side border
//
typedef struct
{
  u16  data;     // address of the glyph data relative to the beginning of the glyph data
  u8   width:6;  // number of stored pixels across the glyph
  u8   height:6; // number of stored pixels up the glyph
  u8   lift:4;   // numbers of pixels from the bottom of the glyph cell to the first row that contains part of the glyph.  This avoids the need to store blank rows while maintaining vertical alignment with other glyphs
}
Glyph;

// glyph_data:  A big stream of pairs of bits, queued first up the glyph and
//              then across.  New glyphs start on a byte boundary so that
//              Glyph.data need only be capable of indexing bytes rather than
//              pairs of bits within bytes.
//              FIXME: It would be straightforward for Glyph.data to be capable
//              of sub-byte addressing and would save ~ 48 bytes typically, at
//              the acceptable cost of capping glyph data at 16K ( or 8K for a
//              b&w typeface)
// absent_code: The code point to substitute when this typeface doesn't support
//              the requested code point.  Typically the code point maps to a
//              glyph that is a rectangle
typedef struct
{
  u8           height;           // The number of pixels up each glyph cell
  u8           first_character;  // ASCII encoding of first glyph
  u16          glyphs;           // The number of glyphs in the "glyph" array
  Glyph const *glyph;            // The details required to render each glyph
  u8    const *glyph_data;
  u8           absent_code;
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
// This *could* return the width of the rendered text but it wouldn't be used
// in most cases but would slow it down in all cases
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

