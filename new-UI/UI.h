
#ifndef __UI_H
#define __UI_H


#include "BIOS.h"
#include "input.h"


#define  INVERSE  1

#define  NICE_FONT_HEIGHT  10
#define  TOP_LINE  (SCREEN_HEIGHT - NICE_FONT_HEIGHT)
#define  SECOND_LINE  (SCREEN_HEIGHT - NICE_FONT_HEIGHT * 2)


// Represents a point on screen
typedef struct
{
  u16  x;
  u16  y;
}
Point, Dimensions;


// Represents an area on screen
typedef struct
{
  Point       lower_left;
  Dimensions  extents;
}
Area;


// A User Interface element, an area on screen such as a button.
//  name: shown on screen
//  area: the area of the screen that this indicator uses.  It's a pointer
//        because the same indicator might sometimes show in one area and
//        sometimes in another
//  render: the function that renders this indicator
//  context: a pointer to an Indicator-specific structure.  This makes it
//           possible to share rendering code, so that code to show the
//           percentage of screen brightness can be also be used to show beep
//           volume
typedef struct Indicator
{
  char const *name;
  Area const *area;
  void (*render)( struct Indicator *idct);
  void *context;
}
Indicator;


typedef void IndicatorRenderer( Indicator *idct);

typedef void InputHandler( Indicator *idct, Input ip);


typedef enum
{
  PUSHED_LEFT,
  PUSHED_RIGHT,
  NOT_PUSHED,
}
Pushed;


typedef struct
{
  Indicator  indicator;
  InputHandler  *input;
}
Editor;


typedef struct
{
  s8 *value;
  u8 min;
  u8 max;
  void (*update)( Indicator *idct);
}
PercentContext;


extern
void init_UI()
;


extern
void check_for_input()
;


extern
void render()
;


#endif

