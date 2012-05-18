
/*

This is the User Interface code, which mostly consist of:

 + rendering
 + processing input

An *indicator* is a rectangular area of pixels on the screen devoted to
displaying something such as peak-to-peak voltage or what button 1 does if you
were to press it now.

Instead of one massive render() function, render() asks each indicator that is
currently being displayed ( as determined by the "indicator" array) to render
itself.

An *editor* is an indicator that is attached to one of the four buttons or
either the left or the right rocker switch.  The editor that is attached to an
input device receives input from that device.  The input changes the state of
the system and the indicator renders the new state.

*/

#include "UI.h"
#include "core.h"
#include "font.h"
#include "display.h"
#include "nice_font.h"
#include "main.h"


// Used so that shared code can be used for buttons that do nothing except
// assign functions to the four buttons.
typedef struct
{
  Editor const *button1;
  Editor const *button2;
  Editor const *button3;
  Editor const *button4;
}
MenuContext;


// Used so that shared code can be used for buttons that do nothing except load
// a set of editors to the rocker switches.
typedef const struct
{
  Editor const **editor;   // ptr 2 array of (ptrs 2) editors to attach
  u8             editors;  // arity of "editor"
}
EditorsContext;


// ---------------------------------------------- this really belongs elsewhere

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


// ----------------------------------------------------------------------- data

Area const button1_area = { lower_left:{ 67*0, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button2_area = { lower_left:{ 67*1, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button3_area = { lower_left:{ 67*2, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const button4_area = { lower_left:{ 67*3, TOP_LINE}, extents:{ 67, NICE_FONT_HEIGHT} };
Area const left_rocker_area = { lower_left:{ 67*4, TOP_LINE}, extents:{ 66, NICE_FONT_HEIGHT} };
Area const right_rocker_area = { lower_left:{ 67*4+66, TOP_LINE}, extents:{ 66, NICE_FONT_HEIGHT} };
Area const second_line_area = { lower_left:{ 0, SECOND_LINE}, extents:{ SCREEN_WIDTH, NICE_FONT_HEIGHT} };

// Forward declarations of functions referenced by editors:
IndicatorRenderer static render_percentage, render_selector;
InputHandler static change_percentage, change_selection;

Editor const brightness_editor = {
  indicator: {
    name: "Screen brightness",
    area: &left_rocker_area,
    render: render_percentage,
    context: &screen_brightness_context,
  },
  input: change_percentage,
};

Editor const volume_editor = {
  indicator: {
    name: "Beep volume",
    area: &right_rocker_area,
    render: render_percentage,
    context: &beep_volume_context,
  },
  input: change_percentage,
};

Editor const coupling_editor = {
  indicator: {
    name: "Coupling",
    area: &right_rocker_area,
    render: render_selector,
    context: &coupling_context,
  },
  input: change_selection,
};

Editor const voltage_scaling_editor = {
  indicator: {
    name: "Voltage scaling",
    area: &left_rocker_area,
    render: render_selector,
    context: &voltage_scaling_context,
  },
  input: change_selection,
};

Editor const generator_waveform_editor = {
  indicator: {
    name: "Waveform",
    area: &left_rocker_area,
    render: render_selector,
    context: &generator_waveform_context,
  },
  input: change_selection,
};

Editor const generator_frequency_editor = {
  indicator: {
    name: "Frequency",
    area: &left_rocker_area,
    render: render_selector,
    context: &generator_frequency_context,
  },
  input: change_selection,
};

Editor const generator_duty_cycle_editor = {
  indicator: {
    name: "Duty cycle",
    area: &right_rocker_area,
    render: render_percentage,
    context: &generator_duty_cycle_context,
  },
  input: change_percentage,
};

Editor const trigger_source_editor = {
  indicator: {
    name: "Trigger source",
    area: &left_rocker_area,
    render: render_selector,
    context: &trigger_source_context,
  },
  input: change_selection,
};

Editor const trigger_condition_editor = {
  indicator: {
    name: "Trigger condition",
    area: &right_rocker_area,
    render: render_selector,
    context: &trigger_condition_context,
  },
  input: change_selection,
};


// Forward declarations of functions referenced by buttons:
InputHandler static open_menu, setup_channel, broken_button, setup_editors;
IndicatorRenderer static render_button;

#define  MENU( var_name, bt1, bt2, bt3, bt4)  MenuContext const var_name = { &bt1##_button, &bt2##_button, &bt3##_button, &bt4##_button }

#define  EDITORS_CONTEXT( var, ...)  Editor const *var##_editors[] = { __VA_ARGS__ }; \
  EditorsContext  var##_context = { editor:(Editor const **)&var##_editors, editors:ITEMS_IN_ARRAY(var##_editors) }

#define  BUTTON( var, nm, ar, ip, ct)  Editor const var = { indicator:{ name:nm, area:&ar, render:render_button, context:(void*)ct }, input:ip }

// Forward declarations of buttons:
Editor const setup_button;

BUTTON( blank_button, "", button3_area, broken_button, NULL);

BUTTON( hold_button, "Hold", button1_area, broken_button, NULL);
BUTTON( save_button, "Save", button2_area, broken_button, NULL);

BUTTON( setup_channel_A_button, "Ch A", button1_area, setup_channel, NULL);
BUTTON( setup_channel_B_button, "Ch B", button2_area, setup_channel, NULL);
BUTTON( setup_channel_C_button, "Ch C", button3_area, setup_channel, NULL);
BUTTON( setup_channel_D_button, "Ch D", button4_area, setup_channel, NULL);

EDITORS_CONTEXT( setup_generator, &generator_waveform_editor, &generator_frequency_editor, &generator_duty_cycle_editor);
BUTTON( setup_generator_button, "Generator", button1_area, setup_editors, &setup_generator_context);

EDITORS_CONTEXT( setup_levels, &brightness_editor, &volume_editor);
BUTTON( setup_levels_button, "Bright&Vol", button2_area, setup_editors, &setup_levels_context);

MENU( close_menu_context, hold, save, blank, setup);
BUTTON( close_button, "Close", button4_area, open_menu, &close_menu_context);

MENU( more_menu_context, setup_generator, setup_levels, blank, close);
BUTTON( more_button, "More", button4_area, open_menu, &more_menu_context);

MENU( channels_menu_context, setup_channel_A, setup_channel_B, setup_channel_C, setup_channel_D);
BUTTON( list_channels_button, "Channel", button1_area, open_menu, &channels_menu_context);
EDITORS_CONTEXT( setup_trigger, &trigger_condition_editor, &trigger_source_editor);
BUTTON( setup_trigger_button, "Trigger", button2_area, setup_editors, &setup_trigger_context);
BUTTON( measure_button, "Measure", button3_area, broken_button, NULL);

MENU( setup_menu_context, list_channels, setup_trigger, measure, more);
BUTTON( setup_button, "Setup", button4_area, open_menu, &setup_menu_context);


// The first four slots are hard-coded for buttons 1 through 4 and the next two
// for the two editors for the left and right rockers
Indicator static *indicator[8];
u8 static indicators = 0;

// These pointers remember which editors are attached to the actual buttons at
// the moment.
Editor static *button[4];


// -------------------------------------------------------------------- Editors

// Each rocker has a list of editors that can be cycled through by pressing
// down on the rocker.
//
typedef struct
{
  Editor *editor[ 4];
  u8      editors;
  u8      cursor;
}
Editors;


Editors  left_editors;
Editors  right_editors;


// Removes all editors from the given list.
//
void static no_editors( Editors *list)
{
  list->editors = 0;
  list->cursor = 0;
}

void static add_editor( Editors *list, Editor const *editor)
{
  if ( ITEMS_IN_ARRAY(list->editor) <= list->editors)
    return;
  list->editor[ list->editors] = (Editor*) editor;
  list->editors += 1;
}

Editor static *chosen_editor( Editors *list)
{
  return list->editor[ list->cursor];
}

void static next_editor( Editors *list)
{
  list->cursor = (list->cursor + 1) % list->editors;
}

void static editors_changed()
{
  indicator[ 4] = &chosen_editor(&left_editors)->indicator;
  indicator[ 5] = &chosen_editor(&right_editors)->indicator;
}


// ------------------------------------------------------------- public methods

void static open_top_menu();


void init_UI()
{
  __Clear_Screen( BLACK);

  use_font( &nice_font);
  text_color( RGB(0x81,0x34,0x54), RGB(0xFF,0xFF,0xD1) );
  //E1AEAA
  //9C5D68

  no_editors( &left_editors);
  add_editor( &left_editors, &brightness_editor);
  no_editors( &right_editors);
  add_editor( &right_editors, &trigger_source_editor);
  add_editor( &right_editors, &volume_editor);

  open_top_menu();
  editors_changed();
  indicators = 6;
}


void check_for_input()
{
  InputEvent  ev;

  check_event( &ev);

  if ( INPUT_PRESSED == ev.state)
  {
    Editor  *left_editor = chosen_editor( &left_editors);
    Editor  *right_editor = chosen_editor( &right_editors);

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
        next_editor( &left_editors);
        editors_changed();
        break;
      case RROCKER_LEFT:
        right_editor->input( &right_editor->indicator, ev.input);
        break;
      case RROCKER_RIGHT:
        right_editor->input( &right_editor->indicator, ev.input);
        break;
      case RROCKER_DOWN:
        next_editor( &right_editors);
        editors_changed();
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


// ------------------------------------------------------------ support methods

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

// Shared between render_button and render_choice.
//
void static render_label( Area const *area, char const *label)
{
  Point const ll = area->lower_left;
  render_text( label, ll.x, ll.y);
  Area  space;
  space.lower_left = area->lower_left;
  space.extents = area->extents;
  u16  consumed = width_of_text( label);
  space.lower_left.x += consumed;
  space.extents.x -= consumed;
  fill_rect( &space, BLACK);
}


// --------------------- button renderer and generic broken_button InputHandler

// This is a generic renderer that can render any button.
//
void static render_button( Indicator *idct)
{
  render_label( idct->area, idct->name);
}

// No-op code for blank buttons ( spaces in the menu where there's no function
// yet).
//
void static broken_button( Indicator *idct, Input ip)
{
}


// ------------------------------------------------------------ menu navigation

void static open_top_menu()
{
  open_menu( (Indicator*)&close_button.indicator, NO_INPUT);
}

void static open_menu( Indicator *idct, Input ip)
{
  MenuContext *ct = idct->context;
  task_buttons( ct->button1, ct->button2, ct->button3, ct->button4);
}


// Shared by buttons that do nothing other than set up the lists of editors
// attached to the left and right rockers.
//
void static setup_editors( Indicator *idct, Input input)
{
  EditorsContext *ct = idct->context;
  no_editors( &left_editors);
  no_editors( &right_editors);
  int  i;
  for ( i = 0;  i < ct->editors;  i += 1)
  {
    Editor const *edt = ct->editor[ i];
    Editors *list = (&left_rocker_area == edt->indicator.area) ? &left_editors : &right_editors;
    add_editor( list, edt);
  }
  editors_changed();
  open_top_menu();
}


void static setup_channel( Indicator *idct, Input input)
{
  // load editors for channel A..D depending on which button (input) was  pressed
  no_editors( &left_editors);
  no_editors( &right_editors);
  add_editor( &left_editors, &voltage_scaling_editor);
  add_editor( &right_editors, &coupling_editor);
  //add_editor( &right_editors, &offset_editor);
  editors_changed();
  open_top_menu();
}


// ------------------------ percentage editor ( screen brightness, beep volume)

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
  render_text( text, ll.x, ll.y);
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

  ct->update( idct->context);
}


// ---------------------------------- selector ( choosing one item from a list)

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

  ct->update( idct->context);
}

