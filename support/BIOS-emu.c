
/*

Interesting threading.

 + Can use separate thread for counter3 since is practically invoked in
   separate thread on device ( IRQ)
 + the main thread cannot be relied upon to periodically invoke any BIOS
   methods
 + video requests should all be on the same thread

*/

#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <SDL.h>
#include "BIOS.h"
#include "core.h"
#include "display.h"


SDL_Surface static *screen;
SDL_Surface static *glyph_sheet;



void SDL_die( char *message)
{
  fprintf( stderr, "%s: %s\n", message, SDL_GetError() );
  exit( 1);
}


pthread_t  counter3_thread;

bool volatile counter3_thread_should_run;

extern void TIM3_IRQHandler();

void *counter3_thread_main( void *arg)
{
  printf("Starting\n");
  int  time_to_flip = 10;
  while( counter3_thread_should_run)
  {
    usleep( 1000);
    TIM3_IRQHandler();
    time_to_flip -= 1;
    if ( 0 == time_to_flip)
    {
      SDL_Flip( screen);
      time_to_flip = 10;
    }
  }
  printf("Stopping\n");
  return NULL;
}


void static shutdown()
{
  if ( pthread_self() != counter3_thread)
  {
    counter3_thread_should_run = false;
    printf("Waiting\n");
    usleep( 2000);
    if ( pthread_join( counter3_thread, NULL) )
    {
      fprintf( stderr, "Could not join thread\n");
      pthread_cancel( counter3_thread);
    }
  }
  printf("Quitting\n");
  SDL_Quit();
}


void static initialize()
{
  if ( SDL_Init( SDL_INIT_VIDEO) < 0 ) SDL_die("Unable to init SDL");
  atexit( shutdown);

  screen = SDL_SetVideoMode( 400, 240, 16, SDL_SWSURFACE);
  if ( NULL == screen) SDL_die("Unable to set video mode");

  SDL_ShowCursor( SDL_FALSE);

  counter3_thread_should_run = true;
  if ( pthread_create( &counter3_thread, NULL, counter3_thread_main, NULL) )
    SDL_die("Unable to create thread");

  glyph_sheet = SDL_LoadBMP("BIOS-glyphs.bmp");
}


void static ensure_initialized()
{
  bool static initialized = false;

  if ( ! initialized)
  {
    initialize();

    initialized = true;
  }
}


void __enable_irq()
{
}


void __disable_irq()
{
}


void __CTR_HP()
{
}


void __USB_Istr()
{
}


u32 static keys_held();

void __WFE()
{
  // This is to check for SDL_QUIT in programs that while( true) { __WFE(); }
  // instead of polling the buttons via __Get()
  keys_held();
  usleep( 1);
}


// The "screen" surface must be locked BEFORE invocation of this method.
//
void set_pixel( Sint16 x, Sint16 y, Uint16 color)
{
  Uint16  *pixel_row = (Uint16 *) ( screen->pixels + screen->pitch * (screen->h - 1 -  y) );
  Uint16  cl = SDL_MapRGB( screen->format, RED_IN(color), GRN_IN(color), BLU_IN(color));
  pixel_row[ x] = cl;
}


void __Clear_Screen( u16 color)
{
  ensure_initialized();
  SDL_FillRect( screen, NULL, color);
}


void __Display_Str( u16 left, u16 bottom, u16 color, u8 mode, char const *text)
{
  char  code;

  ensure_initialized();

  // FIXME: Support "color" and "mode"
  SDL_Rect  glyph = { w:FONT_WIDTH, h:FONT_HEIGHT };
  SDL_Rect  target = { x:left,  y:(SCREEN_HEIGHT - bottom - FONT_HEIGHT)};
  while( (code = *text) != '\0')
  {
    if ( code < ' '  ||  127 < code)
      code = 127;
    glyph.x = FONT_WIDTH * ( (code - ' ') % 32);
    glyph.y = FONT_HEIGHT * ( (code - ' ') / 32);
    SDL_BlitSurface( glyph_sheet, &glyph, screen, &target);
    text += 1;
    target.x += FONT_WIDTH;
  }

  usleep( 1);
}


void __LCD_DMA_Ready()
{
}


u16  block_left;
u16  block_right;
u16  block_bottom;
u16  block_top;
u16  cursor_x;
u16  cursor_y;

void __LCD_Set_Block( u16 left, u16 right, u16 bottom, u16 top)
{
  block_left = left;
  block_right = right;
  block_bottom = bottom;
  block_top = top;
  cursor_x = block_left;
  cursor_y = block_bottom;
}


void static advance_cursor()
{
  if ( cursor_y < block_top)
    cursor_y += 1;
  else {
    cursor_y = block_bottom;
    if ( cursor_x < block_right)
      cursor_x += 1;
    else
      cursor_x = block_left;
  }
}


void __LCD_SetPixl( u16 color)
{
  SDL_LockSurface( screen);
  set_pixel( cursor_x, cursor_y, color);
  SDL_UnlockSurface( screen);
  advance_cursor();
}


void __LCD_Copy( uc16 *pixel, u16 pixels)
{
  SDL_LockSurface( screen);
  while ( 0 < pixels)
  {
    set_pixel( cursor_x, cursor_y, *pixel);
    advance_cursor();
    pixel += 1;
    pixels -= 1;
  }
  SDL_UnlockSurface( screen);
}


typedef struct
{
  SDLKey  sdl_key;
  u16     emu_key;
}
KeyMapping;

KeyMapping static key_map[] =
{
  {'1',                KEY1_STATUS },
  {'2',                KEY2_STATUS },
  {'3',                KEY3_STATUS },
  {'4',                KEY4_STATUS },
  { SDLK_LEFTBRACKET,  K_INDEX_D_STATUS },
  { SDLK_RIGHTBRACKET, K_INDEX_I_STATUS },
  { SDLK_SPACE,        K_INDEX_S_STATUS },
  { SDLK_LEFT,         K_ITEM_D_STATUS },
  { SDLK_RIGHT,        K_ITEM_I_STATUS },
  { SDLK_DOWN,         K_ITEM_S_STATUS },
};

#define  NO_KEY  0

u16 static emu_key( SDLKey sdl_key)
{
  int i;
  for ( i = 0;  i < ITEMS_IN_ARRAY(key_map);  i += 1)
  {
    KeyMapping  *mp = &key_map[ i];
    if ( mp->sdl_key == sdl_key)
      return mp->emu_key;
  }
  return NO_KEY;
}


u32 static keys_held()
{
  SDL_Event  event;

  u32 static keys_held = 0xffffffff;
  while ( SDL_PollEvent(&event) )
  {
    switch (event.type)
    {
      case SDL_KEYDOWN:
        // When a key is held down, the bit is 0
        keys_held &= ~ emu_key( event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        keys_held |= emu_key( event.key.keysym.sym);
        break;
      case SDL_QUIT:
        exit( 0);
        break;
      default:
        break;
    }
  }
  return keys_held;
}


u32 __Get( u8 object)
{
  switch( object)
  {
    case KEY_STATUS:
      return keys_held();
      break;
    default:
      fprintf( stderr, "Unhandled __Get: %hu", object);
      return 0;
      break;
  }
}


void __Set( u8 object, u32 value)
{
  ensure_initialized();
  usleep( 1000); // Just so the app doesn't redline the desktop
}

