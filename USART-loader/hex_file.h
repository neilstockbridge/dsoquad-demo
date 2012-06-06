
#ifndef __HEX_FILE_H
#define __HEX_FILE_H


#include <stdbool.h>
#include "stm32f10x.h"


#define  MAX_DATA_ON_LINE  16


typedef enum
{
  EXPECTING_START_CODE,
  EXPECTING_DATA_LENGTH,
  EXPECTING_ADDRESS,
  EXPECTING_RECORD_TYPE,
  EXPECTING_DATA,
  EXPECTING_CHECKSUM,
}
Expecting;


typedef void (HexFileSink)( u32 address, u8 *data, u8 data_on_line);


typedef struct
{
  Expecting    expecting;
  u8           digits_needed; // number of remaining hex digits expected in order to complete the next field
  u16          value; // values of hex digits are constructed here until the value ( 8 or 16 bit) is complete
  u8           checksum;
  u8           record_type;
  u32          address;
  u8           data_on_line; // number of remaining bytes expected in order to complete the line of data
  u8           data[ MAX_DATA_ON_LINE];
  u8           data_cursor;
  u32          entry_point;
  HexFileSink *sink;
}
HexFileParser;


extern
void prime_hex_file_parser( HexFileParser *ps, HexFileSink *sink)
;

extern
bool parse_hex_file( HexFileParser *ps, char c)
;

// Writes a concise representation of the state of the parser in to the
// specified buffer.  This can be used to explain a parser error.
extern
void hex_file_parser_to_s( HexFileParser *ps, char *buf, int buf_size)
;


#endif

