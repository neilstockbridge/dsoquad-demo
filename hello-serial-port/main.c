
/*

A simple demo of sending and receiving data using USART1 ( the serial port).

*/

#include <stdbool.h>
#include "BIOS.h"
#include "core.h"
#include "hex.h"


// Note: CPU_CLK / BAUD_RATE must fit within 16-bits
#define  CPU_CLK  72000000
#define  BAUD_RATE  1200


// Values for the 2-bit pairs for MODE found in the CRL/CRH register:
#define  GPIO_MODE_INPUT            0x0
#define  GPIO_MODE_OUTPUT_MAX_2MHZ  0x2

#define  GPIO_CNF_AF_PUSH_PULL      0x2
#define  GPIO_CNF_PULL_UP_OR_DOWN   0x2 // Set the bit corresponding to the port in the DR to 1 to choose UP rather than DOWN

#define  PIN9_MODE_POS   4 // bit position 4 is bit0 of the MODE9 setting
#define  PIN9_CNF_POS    6
#define  PIN10_MODE_POS  8
#define  PIN10_CNF_POS  10


typedef struct
{
  char byte[ 32];
  u8 oldest; // Refers to the oldest unconsumed byte
  u8 cursor; // Refers to the slot that will be filled next
}
Received;

Received static volatile received = { oldest:0, cursor:0 };


void static init_serial()
{
  // Enable USART1:
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  // Set the baud rate:
  USART1->BRR = CPU_CLK / BAUD_RATE;
  // Enable the USART for both transmission and reception:
  USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

  // I guess these might mean enable pull up for RX and set TX as hard-driven output
  // AFOUT_10 probably mean b10, i.e. function 2, which is AF output, push-pull
  //gpio_usart1_tx_mode(GPIO_AFOUT_10);
  //gpio_usart1_rx_mode(GPIO_HIGHZ_INPUT);

  // TX is pin 9 on port A (PA 9)
  // RX is pin 10 on port A (PA 10)
  // Mask out any current values for the MODE and CNF values for pins 9 and 10
  GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9 | GPIO_CRH_MODE10 | GPIO_CRH_CNF10)) |
              (GPIO_MODE_OUTPUT_MAX_2MHZ << PIN9_MODE_POS) |
              (GPIO_CNF_AF_PUSH_PULL << PIN9_CNF_POS) |
              (GPIO_MODE_INPUT << PIN10_MODE_POS) |
              (GPIO_CNF_PULL_UP_OR_DOWN << PIN10_CNF_POS)
              ;
  // Enable the pull-up for PA 10 (RX):
  GPIOA->BSRR = GPIO_BSRR_BS10;

  // Enable the RX interrupt in the NVIC:
  NVIC_EnableIRQ( USART1_IRQn);
}


// Provides the index in received.byte that should be used after the specified
// index.
//
u8 static advanced( u8 index)
{
  // NOTE: ITEMS_IN_ARRAY(..) must be an integer power of two
  return (index + 1) & (ITEMS_IN_ARRAY(received.byte) - 1);
}


// Invoked when a character has been received.
//
void USART1_IRQHandler()
{
  u8  advanced_cursor = advanced( received.cursor);
  // If the cursor would refer to the same slot as "oldest" once advanced then
  // the queue is full and the event is discarded
  if ( advanced_cursor != received.oldest)
  {
    // Reading DR acknowledges the interrupt
    received.byte[ received.cursor] = USART1->DR;
    received.cursor = advanced_cursor;
  }
}


// Indicates whether data has been received and is now waiting in the RX buffer.
//
bool static received_data_is_waiting()
{
  return received.cursor != received.oldest;
}


// Provides the next character that has been received.
//
char static next_received_char()
{
  char  ch = received.byte[ received.oldest];
  __disable_irq();
  received.oldest = advanced( received.oldest);
  __enable_irq();
  return ch;
}


void static send_char( char ch)
{
  // Wait until DR is ready for the next frame:
  while ( 0 == (USART1->SR & USART_SR_TXE) );
  // Send the next frame:
  USART1->DR = ch;
}


void static send_text( char const *text)
{
  char  ch;

  while ( (ch = *text) != '\0')
  {
    send_char( ch);
    text += 1;
  }
}


void static emit()
{
  send_text("Received:");
  char  text[2+1];
  text[2] = '\0';
  while ( received_data_is_waiting() )
  {
    send_text(" 0x");
    u8_to_hex( next_received_char(), text);
    send_text( text);
  }
  send_text("\r\n");
}


u16 when_to_emit = 1000; // milliseconds from now
bool volatile should_emit = false;


void static when_counter3_overflows()
{
  if ( 0 < when_to_emit)
    when_to_emit -= 1;
  else {
    should_emit = true;
    when_to_emit = 1000;
  }
}


void TIM3_IRQHandler(void)
{
  when_counter3_overflows();
  __Set( KEY_IF_RST, 0);  // Clear TIM3 interrupt flag
}

void USB_HP_CAN_TX_IRQHandler(void)
{
  __CTR_HP();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
  __USB_Istr();
}


int main()
{
  __Set( BEEP_VOLUME, 0);
  __Clear_Screen( 0xffff);
  when_to_emit = 1000;

  init_serial();

  while( true)
  {
    __WFE();
    if ( should_emit)
    {
      emit();
      should_emit = false;
    }
  }
}

