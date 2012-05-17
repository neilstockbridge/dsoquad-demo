
#include "UI.h"
#include "core.h"
#include "font.h"
#include "display.h"
#include "nice_font.h"
#include "main.h"


typedef struct
{
  char *label;
  u8    value;
}
SelectOption;

typedef struct
{
  SelectOption const *option;
  int                 options;
  int                 selected;
}
SelectContext;


void fill_rect( Area *area, u16 color)
{
  __LCD_DMA_Ready();
  u16  left = area->lower_left.x;
  u16  bottom = area->lower_left.y;
  u16  width = area->extents.x;
  u16  height = area->extents.y;
  __LCD_Set_Block( left, (left + width), bottom, (bottom + height));
  int  i;
  for ( i = 0;  i < (width * height);  i += 1)
  {
    __LCD_SetPixl( color);
  }
}


Area const button1_area = { lower_left:{ 67*0, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button2_area = { lower_left:{ 67*1, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button3_area = { lower_left:{ 67*2, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button4_area = { lower_left:{ 67*3, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const left_rocker_area = { lower_left:{ 67*4, TOP_LINE}, extents:{ 66, NICE_FONT_HEIGHT} };
Area const right_rocker_area = { lower_left:{ 67*4+66, TOP_LINE}, extents:{ 66, NICE_FONT_HEIGHT} };
Area const second_line_area = { lower_left:{ 0, SECOND_LINE}, extents:{ SCREEN_WIDTH, NICE_FONT_HEIGHT} };

// Forward declarations of functions referenced by buttons:
InputHandler static open_setup_menu, setup_channel, more_setup, quit_setup, list_channels, broken_button, change_percentage, change_selection;
IndicatorRenderer static render_button, render_percentage, render_selector;
IndicatorRenderer  brightness_changed, volume_changed;

#define  BUTTON( var, nm, ar, ip)  Editor const var = { indicator:{ name:nm, area:&ar, render:render_button }, input:ip }

BUTTON( blank_button, "", button3_area, broken_button);

BUTTON( hold_button, "Hold", button1_area, broken_button);
BUTTON( save_button, "Save", button2_area, broken_button);
BUTTON( setup_button, "Setup", button4_area, open_setup_menu);

BUTTON( list_channels_button, "Channel", button1_area, list_channels);
BUTTON( setup_trigger_button, "Trigg", button2_area, more_setup);
BUTTON( measure_button, "Measure", button3_area, broken_button);
BUTTON( more_button, "More", button4_area, more_setup);

BUTTON( setup_channel_A_button, "Ch A", button1_area, setup_channel);
BUTTON( setup_channel_B_button, "Ch B", button2_area, setup_channel);
BUTTON( setup_channel_C_button, "Ch C", button3_area, setup_channel);
BUTTON( setup_channel_D_button, "Ch D", button4_area, setup_channel);

BUTTON( setup_generator_button, "Generator", button1_area, broken_button);
BUTTON( setup_backlight_button, "Bright&Vol", button2_area, broken_button);
BUTTON( back_button, "Back", button4_area, quit_setup);


Editor  brightness_editor = {
  indicator: {
    name: "Screen brightness",
    area: &left_rocker_area,
    render: render_percentage,
    context: &brightness_context,
  },
  input: change_percentage,
};

Editor  volume_editor = {
  indicator: {
    name: "Beep volume",
    area: &right_rocker_area,
    render: render_percentage,
    context: &volume_context,
  },
  input: change_percentage,
};

SelectOption const trigger_source_options[] =
{
  { label:"falling", value: 0 },
  { label:"rising", value: 1 },
  { label:"low", value: 2 },
  { label:"high", value: 3 },
};

#define  SELECT_CONTEXT( var_name, option_list)  SelectContext  var_name = { option:option_list, options:ITEMS_IN_ARRAY(option_list), selected:0 }

SELECT_CONTEXT( trigger_source_context, trigger_source_options);

Editor  trigger_source_editor = {
  indicator: {
    name: "Trigger source",
    area: &right_rocker_area,
    render: render_selector,
    context: &trigger_source_context,
  },
  input: change_selection,
};

// The first four slots are hard-coded for buttons 1 through 4
Indicator *indicator[8];
u8 static indicators = 0;

// These pointers remember which editors are attached to the actual buttons at
// the moment.
Editor  *button[4];


// Each rocker has a list of editors that can be cycled through by pressing
// down on the rocker.
//
typedef struct
{
  Editor *editor[ 4];
  u8      editors;
  u8      cursor;
}
EditorList;


EditorList  left_editor_list;
EditorList  right_editor_list;


void static begin_editor_list( EditorList *list)
{
  list->editors = 0;
  list->cursor = 0;
}

void static add_editor_to_list( EditorList *list, Editor *editor)
{
  if ( ITEMS_IN_ARRAY(list->editor) <= list->editors)
    return;
  list->editor[ list->editors] = editor;
  list->editors += 1;
}

Editor static *chosen_editor( EditorList *list)
{
  return list->editor[ list->cursor];
}

Editor static *next_editor( EditorList *list)
{
  list->cursor = (list->cursor + 1) % list->editors;
  return chosen_editor( list);
}


void init_UI()
{
  __Clear_Screen( BLACK);

  use_font( &nice_font);

  quit_setup( NULL, NO_INPUT);
  indicator[ 4] = &brightness_editor.indicator;
  indicator[ 5] = &volume_editor.indicator;
  indicators = 6;

  begin_editor_list( &left_editor_list);
  add_editor_to_list( &left_editor_list, &brightness_editor);
  begin_editor_list( &right_editor_list);
  add_editor_to_list( &right_editor_list, &trigger_source_editor);
  add_editor_to_list( &right_editor_list, &volume_editor);
}


void check_for_input()
{
  InputEvent  ev;

  check_event( &ev);

  if ( INPUT_PRESSED == ev.state)
  {
    Editor  *left_editor = chosen_editor( &left_editor_list);
    Editor  *right_editor = chosen_editor( &right_editor_list);

    switch( ev.input)
    {
      case BUTTON1:
        button[0]->input( &button[0]->indicator, ev.input);
        break;
      case BUTTON2:
        button[1]->input( &button[1]->indicator, ev.input);
        break;
      case BUTTON3:
        button[2]->input( &button[2]->indicator, ev.input);
        break;
      case BUTTON4:
        button[3]->input( &button[3]->indicator, ev.input);
        break;
      case LROCKER_LEFT:
        left_editor->input( &left_editor->indicator, ev.input);
        break;
      case LROCKER_RIGHT:
        left_editor->input( &left_editor->indicator, ev.input);
        break;
      case LROCKER_DOWN:
        indicator[ 4] = &next_editor( &left_editor_list)->indicator;
        break;
      case RROCKER_LEFT:
        right_editor->input( &right_editor->indicator, ev.input);
        break;
      case RROCKER_RIGHT:
        right_editor->input( &right_editor->indicator, ev.input);
        break;
      case RROCKER_DOWN:
        indicator[ 5] = &next_editor( &right_editor_list)->indicator;
        break;
      case NO_INPUT:
        break;
    }
  }
}


void render()
{
  int  i;
  for ( i = indicators - 1;  0 <= i;  i -= 1)
  {
    Indicator *idct = indicator[ i];
    idct->render( idct);
  }
}

// Assigns the given editor to the specified button.
//
void static task_button( int bt, Editor const *editor)
{
  // some buttons are declared const so they need not be copied to RAM,
  // although some editors are mutable and thus need to be in RAM, so they
  // can't all be declared const, hence the cast:
  button[ bt] = (Editor*) editor;
  indicator[ bt] = (Indicator*) &editor->indicator;
}

void static task_buttons( Editor const *b1, Editor const *b2, Editor const *b3, Editor const *b4)
{
  task_button( 0, b1);
  task_button( 1, b2);
  task_button( 2, b3);
  task_button( 3, b4);
}

// Shared between render_button and render_choice.
//
void static render_label( Area const *area, char const *label)
{
  Point const ll = area->lower_left;
  render_text( ll.x, ll.y, WHITE, INVERSE, label);
  Area  space;
  space.lower_left = area->lower_left;
  space.extents = area->extents;
  u16  consumed = width_of_text( label);
  space.lower_left.x += consumed;
  space.extents.x -= consumed;
  fill_rect( &space, BLACK);
}

// This is a generic renderer that can render any button.
//
void static render_button( Indicator *idct)
{
  render_label( idct->area, idct->name);
}



void static render_percentage( Indicator *idct)
{
  Point const ll = idct->area->lower_left;
  char  text[4+1];
  PercentContext *ct = idct->context;
  u8  value = *ct->value;
  text[0]='0'+ value / 100;
  text[1]='0'+ value / 10 % 10;
  text[2]='0'+ (value % 10);
  text[3]='%';
  text[4]=0;
  render_text( ll.x, ll.y, WHITE, INVERSE, text);
}


// Given an input from one of the rocker switches, provides generic LEFT or
// RIGHT inputs so that code can be independent on the rocker they're attached
// to.
//
Pushed static direction_of( Input ip)
{
  switch ( ip)
  {
    case LROCKER_LEFT:
    case RROCKER_LEFT:
      return PUSHED_LEFT;
    case LROCKER_RIGHT:
    case RROCKER_RIGHT:
      return PUSHED_RIGHT;
    default:
      return NOT_PUSHED;
  }
}

void static change_percentage( Indicator *idct, Input ip)
{
  PercentContext *ct = idct->context;

  switch( direction_of(ip) )
  {
    case PUSHED_LEFT:
      *ct->value -= 10;
      break;
    case PUSHED_RIGHT:
      *ct->value += 10;
      break;
    default:
      break;
  }

  if ( *ct->value < ct->min) *ct->value = ct->min;
  if ( ct->max < *ct->value) *ct->value = ct->max;

  ct->update( idct);
}


void static setup_channel( Indicator *idct, Input input)
{
  // load editors for channel A..D depending on which button (input) was  pressed
  quit_setup( NULL, NO_INPUT);
}

void static list_channels( Indicator *idct, Input ip)
{
  task_buttons( &setup_channel_A_button, &setup_channel_B_button,
                &setup_channel_C_button, &setup_channel_D_button);
}

void static broken_button( Indicator *idct, Input ip)
{
}

void static more_setup( Indicator *idct, Input ip)
{
  task_buttons( &setup_generator_button, &setup_backlight_button, &blank_button, &back_button);
}

void static open_setup_menu( Indicator *idct, Input ip)
{
  task_buttons( &list_channels_button, &setup_trigger_button, &measure_button, &more_button);
}

void static quit_setup( Indicator *idct, Input ip)
{
  task_buttons( &hold_button, &save_button, &blank_button, &setup_button);
}

void static render_selector( Indicator *idct)
{
  SelectContext *ct = idct->context;

  render_label( idct->area, ct->option[ct->selected].label);
}

void static change_selection( Indicator *idct, Input ip)
{
  SelectContext *ct = idct->context;

  switch( direction_of(ip) )
  {
    case PUSHED_LEFT:
      ct->selected -= 1;
      if ( ct->selected < 0)
        ct->selected = ct->options - 1;
      break;
    case PUSHED_RIGHT:
      ct->selected += 1;
      if ( ct->options <= ct->selected)
        ct->selected = 0;
      break;
    default:
      break;
  }
}

