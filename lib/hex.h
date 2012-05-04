
#ifndef __HEX_H
#define __HEX_H

#include "stm32f10x.h" // for u8, u32, etc.


extern
char hex_digit( u8 digit)
;

extern
void u8_to_hex( u8 value, char *s)
;

extern
void u16_to_hex( u16 value, char *s)
;

extern
void u32_to_hex( u32 value, char *s)
;


#endif

