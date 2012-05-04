
// This module polls the input devices and provides a SDL-esque event queue.
//
// Invoke check_inputs() every 20ms
// Invoke check_event( ev) periodically.  ev.input will be NO_INPUT or the
//   input that changed.  "state" will be INPUT_PRESSED or INPUT_RELEASED
//

#ifndef __INPUT_H
#define __INPUT_H


typedef enum {
  BUTTON1,
  BUTTON2,
  BUTTON3,
  BUTTON4,
  LROCKER_LEFT,
  LROCKER_RIGHT,
  LROCKER_DOWN,
  RROCKER_LEFT,
  RROCKER_RIGHT,
  RROCKER_DOWN,
  NO_INPUT,
}
Input;


#define  INPUT_PRESSED  0
#define  INPUT_RELEASED  1


typedef struct
{
  Input  input;
  u8     state; // INPUT_PRESSED or INPUT_RELEASED
}
InputEvent;


extern
void check_inputs()
;

// Fills in the specified event or sets the event->input to NO_INPUT
extern
void check_event( InputEvent *event)
;

#endif

