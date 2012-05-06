
#include "stdbool.h"
#include "image.h"


void draw_image_16bit( Image16bit const *image, int left, int bottom)
{
  __LCD_Set_Block( left, (left + image->width - 1), bottom, (bottom + image->height - 1) );
  __LCD_DMA_Ready();
  __LCD_Copy( image->pixels, (image->width * image->height) );
}


void draw_image_clut16( ImageCLUT16 const *image, int left, int bottom)
{
  u16  pixels[ 2048];
  u16 *pixel = pixels;
  int  bytes_per_column = ( (image->height - 1) >> 1) + 1;
  u8 const *data = image->data;
  int  x, y;
  for ( x = 0;  x < image->width;  x += 1)
  {
    u8  datum = 0x00;
    for ( y = 0;  y < image->height;  y += 1, pixel += 1)
    {
      bool  even_row = 0 == (y & 1);
      if ( even_row) datum = *data++;
      int  i = even_row ? (datum & 0xf) : (datum >> 4);
      *pixel = image->colors[ i];
    }
  }
  __LCD_Set_Block( left, (left + image->width - 1), bottom, (bottom + image->height - 1) );
  __LCD_DMA_Ready();
  __LCD_Copy( pixels, (image->width * image->height) );
}

