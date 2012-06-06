
/*

A proof-of-concept for loading apps in to RAM over the USART and executing them
in RAM to avoid flash wear.

 + RAM begins at 0x20000000
 + SYS reserves 12K, up to 0x20003000, which is where most apps base .data and
   .bss
 + This app cheekily uses RAM based at 0x20002000.  The gcc SYS I built only
   uses 1396 bytes of its 12K anyway
 + Apps loaded by this app should base their .text ( beginning with the vector
   table) at 0x20003000 so that this app knows where to find the entry point
   ( typically Reset_Handler).  RAM for apps will therefore have to be based
   higher in RAM than the 0x20003000 when the same app is installed to flash
 + The MCU has 48K of RAM.  12K is taken by SYS and the top 4K is suggested for
   stack.  This leaves 32K for .text, .data and .bss, which should still be
   enough to try some ideas out

*/

#include <stdbool.h>
#include "BIOS.h"
#include "core.h"
#include "hex.h"
#include "hex_file.h"


HexFileParser  hex_file_parser;


// Note: CPU_CLK / BAUD_RATE must fit within 16-bits
#define  CPU_CLK  72000000
#define  BAUD_RATE  115200


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
  char byte[ 256];
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


bool static problem = false;

void static read()
{
  if ( problem)
    return;

  while ( received_data_is_waiting() )
  {
    char  c = next_received_char();
    send_char( c);
    if ( parse_hex_file( &hex_file_parser, c) )
    {
      __Display_Str( 0, 0, 0x001f, 1, "PROBLEM");
      problem = true;
      break;
    }
  }
  if ( hex_file_parser.entry_point != 0)
  {
    char  text[8+1];
    u32_to_hex( hex_file_parser.entry_point, text);
    text[8]=0;
    __Display_Str( 0, FONT_HEIGHT, 0xf800, 0, text);
    VoidFunction *entry_point = (VoidFunction*)hex_file_parser.entry_point;
    entry_point();
  }
}


bool volatile new_millisecond = false;


void static when_counter3_overflows()
{
  new_millisecond = true;
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


void sink( u32 address, u8 *data, u8 data_on_line)
{
  if ( 0x20003000 <= address  &&  address <= 0x20003000 + 0x8000)
  {
    u8 *dest = (u8*)address;
    while ( data_on_line != 0)
    {
      *dest++ = *data++;
      data_on_line -= 1;
    }
  }
}


int main()
{
  __Set( BEEP_VOLUME, 0);
  __Clear_Screen( 0x0000);

  init_serial();
  prime_hex_file_parser( &hex_file_parser, sink);

  while( true)
  {
    __WFE();
    if ( new_millisecond)
    {
      read();
      new_millisecond = false;
    }
  }
}

