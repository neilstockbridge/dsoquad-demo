
#include <stdbool.h>
#include "BIOS.h"
#include "display.h"
#include "font.h"


extern Font const nice_font;


void TIM3_IRQHandler()
{
  __Set( KEY_IF_RST, 0); // Clear TIM3 interrupt flag
}


void static render()
{
  char  line[32+1];
  int   row;
  int   i;

  text_color( BLACK, WHITE);
  render_text("Hello, proportional font World!", 0, (SCREEN_HEIGHT - nice_font.height) );
  text_color( WHITE, BLACK);
  line[32] = '\0';
  for ( row = 0;  row < 4;  row += 1)
  {
    for ( i = 0;  i < 32;  i += 1)
    {
      line[ i] = 32 * row + i;
    }
    if ('\0' == line[ 0])
      line[ 0] = 0x7f;
    render_text( line, 0, (125 - nice_font.height * row) );
  }
}


int main()
{
  __Set( BEEP_VOLUME, 0);
  __Clear_Screen( BLACK);

  use_font( &nice_font);

  render();

  while( true)
  {
    __WFE();
  }
}

