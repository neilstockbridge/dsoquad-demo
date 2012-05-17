
#ifndef __UI_H
#define __UI_H


#include "BIOS.h"
#include "input.h"


#define  INVERSE  1

#define  NICE_FONT_HEIGHT  10
#define  TOP_LINE  (SCREEN_HEIGHT - NICE_FONT_HEIGHT)
#define  SECOND_LINE  (SCREEN_HEIGHT - NICE_FONT_HEIGHT * 2)


// Represents a pixel-granularity position on the screen.
//
typedef struct
{
  u16  x;
  u16  y;
}
Point, Dimensions;


// Represents a rectangular area of pixels on the screen.
//
typedef struct
{
  Point       lower_left;
  Dimensions  extents;
}
Area;


// A User Interface element that occupies an area on screen such as a button.
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


// These types facilitate the forward declaration of methods of these types so
// that indicators and editors may be declared at the top of UI.c and the
// functions declared further on:
//
typedef void IndicatorRenderer( Indicator *idct);

typedef void InputHandler( Indicator *idct, Input ip);


// Generic LEFT or RIGHT input, so that code can be written that is independent
// of which rocker it is attached to.
//
typedef enum
{
  PUSHED_LEFT,
  PUSHED_RIGHT,
  NOT_PUSHED,
}
Pushed;


// Extends an Indicator to include code to handle input events.
//
typedef struct
{
  Indicator     indicator;
  InputHandler *input;
}
Editor;


// An Indicator context so that percentage controls such as screen brightness
// and waveform generator duty cycle may be edited.
//
typedef struct PercentContext
{
  s8 *value;
  u8 min;
  u8 max;
  void (*update)( struct PercentContext *ct);
}
PercentContext;


// An Indicator context so that "choose one item from a list" controls such as
// channel coupling may be edited.
//
typedef struct
{
  char *label;
  u8    value;
}
SelectOption;

typedef struct SelectContext
{
  SelectOption const *option;
  int                 options;
  int                 selected;
  void (*update)( struct SelectContext *ct);
}
SelectContext;


// -------------------------------------------------------------- public methods

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

