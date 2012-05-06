
#ifndef __IMAGE_H
#define __IMAGE_H


#include "BIOS.h"


// Embedded image formats:
//
// 16-bit true color:
// u16: width
// u16: height
// u16 * with * height
//
// With 256 unique colors:
// u16: width
// u8: height -- u8 rather than u16 to help 16-bit alignment
// u8: number_of_colors in the lookup table
// u16 * number_of_colors: definition of each color
// u8 * width * height: pixels ( each pixel is an index in to the CLUT)
//
// With 16 unique colors:
// u16: width
// u16: height
// u16 * 16: definition of each color
// u8 * width * height / 2: pixels ( each nybble is an index in to the CLUT)
//
// With a pre-defined 16-color palette:
// u16: width
// u16: height
// u8 * width * height / 2: pixels ( each nybble is an index in to the pre-defined EGA CLUT)
//  The EGA CLUT defines: BLK, RED, GRN, BLU, CYN, MGNTA, YLW, LGRAY and lighter versions of those eight
//
// For an 8x14 image with 34 unique colors:
//  format  | bytes | lossy    | struct
// ---------|-------|----------|-----------------
//  16-bit  |   224 |    no    | 2*8*14
//  CLUT256 |   184 |    no    | 2+1+1+2*34+8*14
//  CLUT16  |    92 | slightly | 2+2+2*16+8*14/2
//  FCLUT16 |    60 |    yes   | 2+2+8*14/2
//
// All formats should sequence pixels up the screen and then across.  The
// 16-bit format is therefore immediately ready for __LCD_Copy
//
// In the nybble-pixel formats, pixels in evenly-numbered rows are packed in to
// the 4 LSB and oddly-numbered rows in the 4 MSB.  Columns are u8 padded so
// that this rule holds for every column.
//


typedef struct
{
  u16        width;
  u16        height;
  u16 const *pixels;
}
Image16bit;


typedef struct
{
  u16       width;
  u16       height;
  u16       colors[ 16];
  u8 const *data;
}
ImageCLUT16;


extern
void draw_image_16bit( Image16bit const *image, int left, int bottom)
;


// Building the image for blitting requires 4k of stack.  This is the limit on image size
extern
void draw_image_clut16( ImageCLUT16 const *image, int left, int bottom)
;


#endif

