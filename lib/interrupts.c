
#include "interrupts.h"
#include "BIOS.h" // for __Set


VoidFunction *handler_for[ NUMBER_OF_INTERRUPT_TYPES];


void inline handle( Interrupt interrupt)
{
  if ( handler_for[interrupt])
    handler_for[interrupt]();
}


void SysTickHandler()
{
  handle( SYSTICK_INTERRUPT);
}


void TIM3_IRQHandler()
{
  handle( TIMER3_INTERRUPT);
  __Set( KEY_IF_RST, 0); // Clear TIM3 interrupt flag
}


void attach_handler( Interrupt interrupt, VoidFunction *handler)
{
  handler_for[ interrupt] = handler;
}


void detach_handler( Interrupt interrupt, VoidFunction *handler)
{
  if ( handler == handler_for[interrupt])
    handler_for[ interrupt] = NULL;
}

