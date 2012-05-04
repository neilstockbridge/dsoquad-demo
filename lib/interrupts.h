
/*

Interrupt handlers may be declaraed in only one place yet more than one
experiment might want to handle an interrupt.  This module is home to interrupt
handlers that delegate control to the currently attached interrupt handler.

*/

#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H


#include "core.h"


// This should not be confused with InterruptType_Type or InterruptType
typedef enum
{
  SYSTICK_INTERRUPT,
  TIMER3_INTERRUPT,
  NUMBER_OF_INTERRUPT_TYPES,
}
Interrupt;


extern
void attach_handler( Interrupt interrupt, VoidFunction *handler)
;


extern
void detach_handler( Interrupt interrupt, VoidFunction *handler)
;


#endif

