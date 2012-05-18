
#ifndef __DISPLAY_H
#define __DISPLAY_H


#include "BIOS.h"


#define  BLACK  0x0000
#define  WHITE  0xffff

#define  _CPN( amount, len, ofs)  ( (amount & 0xff) >> (8 - len) << ofs )
#define  RGB( red, grn, blu) ( _CPN( (blu), 5, 11) \
                             | _CPN( (grn), 6, 5) \
                             | _CPN( (red), 5, 0) \
                             )
#define  RED_IN( color)  ( (color >>  0 & 0x1f) << 3 )
#define  GRN_IN( color)  ( (color >>  5 & 0x3f) << 2 )
#define  BLU_IN( color)  ( (color >> 11 & 0x1f) << 3 )


#define  SCREEN_WIDTH   400
#define  SCREEN_HEIGHT  240


extern
void draw_text( int left, int bottom, u16 color, int mode, char const *text)
;


#endif

