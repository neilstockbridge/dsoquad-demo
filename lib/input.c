
#include "BIOS.h"
#include "main.h"
#include "input.h"


// This queue is a circular buffer.  The buffer is empty when oldest == cursor.
// The buffer is full when advanced( cursor) == oldest.
typedef struct
{
  InputEvent  event[ 8];
  u8          oldest; // refers to the oldest unconsumed event on the queue
  u8          cursor; // refers to the slot that will be used by the next event
}
EventQueue;


EventQueue volatile  event_queue = { oldest:0, cursor:0};


u8 inline  advanced( u8 index)
{
  return (index + 1) & (ITEMS_IN_ARRAY(event_queue.event) - 1);
}


void inline queue_event( Input input, u8 state)
{
  u8  advanced_cursor = advanced( event_queue.cursor);
  // If the cursor would refer to the same slot as "oldest" once advanced then
  // the queue is full and the event is discarded
  if ( advanced_cursor != event_queue.oldest)
  {
    InputEvent volatile *ev = &event_queue.event[ event_queue.cursor];
    ev->input = input;
    ev->state = state;
    event_queue.cursor = advanced_cursor;
  }
}


// FIXME: The potential exists for the interrupt to interact badly with the
// main thread here
void check_event( InputEvent *event)
{
  __disable_irq();

  if ( event_queue.cursor != event_queue.oldest)
  {
    InputEvent volatile  *ev = &event_queue.event[ event_queue.oldest];
    event->input = ev->input;
    event->state = ev->state;
    event_queue.oldest = advanced( event_queue.oldest);
  }
  else {
    event->input = NO_INPUT;
  }
  __enable_irq();
}


// The index in this array corresponds to the Input enum
uc16  mask_for_input[] =
{
  KEY1_STATUS,
  KEY2_STATUS,
  KEY3_STATUS,
  KEY4_STATUS,
  K_INDEX_D_STATUS,
  K_INDEX_I_STATUS,
  K_INDEX_S_STATUS,
  K_ITEM_D_STATUS,
  K_ITEM_I_STATUS,
  K_ITEM_S_STATUS,
};


void check_inputs()
{
  u32  input_state = __Get( KEY_STATUS);
  u32 static  state_last_time = 0xff48;
  u32  changes = state_last_time ^ input_state;
  if ( changes)
  {
    Input  ip;
    for ( ip = 0;  ip < ITEMS_IN_ARRAY( mask_for_input);  ip += 1)
    {
      if ( changes & mask_for_input[ ip])
      {
        queue_event( ip, ( input_state & mask_for_input[ip] ) != 0);
      }
    }
  }
  state_last_time = input_state;
}

