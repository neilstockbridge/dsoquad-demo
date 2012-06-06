
#include "hex_file.h"


#define  REC_TYPE_DATA         0x00
#define  REC_TYPE_EOF          0x01
#define  REC_TYPE_ADDR_HI      0x04
#define  REC_TYPE_ENTRY_POINT  0x05


bool static is_hex_digit( char c)
{
  if ('A'<= c && c <='F')
    c += 'a' - 'A';
  return ('0'<= c && c <='9') || ('a'<= c && c <='f');
}


char static value_of_hex_digit( char c)
{
  if ('A'<= c && c <='F')
    c += 'a' - 'A';
  if ('0'<= c && c <='9')
    return c - '0';
  if ('a'<= c && c <='f')
    return 10 + c - 'a';
  return -1;
}


void prime_hex_file_parser( HexFileParser *ps, HexFileSink *sink)
{
  ps->expecting = EXPECTING_START_CODE;
  ps->address = 0x00000000;
  ps->entry_point = 0x00000000;
  ps->sink = sink;
}


bool parse_hex_file( HexFileParser *ps, char c)
{
  // The ( abridged) structure of Intel HEX is:
  //  + 2 hex digits: number of bytes of payload on this line
  //  + 4 hex digits: address offset for data on this line ( 64K limit)
  //  + 2 hex digits: record type
  //  + 2*payload_size hex digits of payload
  //  + 2 hex digits of checksum
  // Relevant record types:
  //  + 00: data
  //  + 01: end of file
  //  + 04: sets the upper 16-bits of the addresses for data
  //  + 05: entry point address ( jump here to start app)
  // All values are big-endian

  // At any point, the parser either expects:
  //  + white space or a colon
  //  + either two or four hex digits
  bool  problem = false;

  if ( EXPECTING_START_CODE == ps->expecting)
  {
    switch( c) {
      case '\r':
      case '\n':
        break;
      case ':':
        ps->expecting = EXPECTING_DATA_LENGTH;
        ps->checksum = 0;
        ps->value = 0x0000;
        ps->digits_needed = 2;
        break;
      default:
        problem = true;
    }
  }
  else if ( is_hex_digit(c) )
  {
    ps->digits_needed -= 1;
    ps->value |= value_of_hex_digit( c) << (4 * ps->digits_needed);

    if ( 0 == ps->digits_needed)
    {
      // Address is the only 16-bit field yet the checksum is defined in terms
      // of the bytes on the line hence Address must be handled separately:
      if ( EXPECTING_ADDRESS == ps->expecting) {
        ps->checksum += (ps->value >> 8) + (ps->value & 0xff);
      }
      else if ( ps->expecting != EXPECTING_CHECKSUM) {
        ps->checksum += ps->value;
      }

      switch( ps->expecting)
      {
        case EXPECTING_START_CODE:
          break;
        case EXPECTING_DATA_LENGTH:
          ps->data_on_line = ps->value;
          ps->data_cursor = 0;
          ps->expecting = EXPECTING_ADDRESS;
          ps->digits_needed = 4;
          if ( MAX_DATA_ON_LINE < ps->data_on_line)
            problem = true;
          break;
        case EXPECTING_ADDRESS:
          ps->address = (ps->address & 0xffff0000) | ps->value;
          ps->expecting = EXPECTING_RECORD_TYPE;
          break;
        case EXPECTING_RECORD_TYPE:
          ps->record_type = ps->value;
          ps->expecting = (0 < ps->data_on_line) ? EXPECTING_DATA : EXPECTING_CHECKSUM;
          break;
        case EXPECTING_DATA:
          ps->data[ ps->data_cursor] = ps->value;
          ps->data_cursor += 1;
          if ( ps->data_on_line <= ps->data_cursor)
            ps->expecting = EXPECTING_CHECKSUM;
          break;
        case EXPECTING_CHECKSUM:
          ps->checksum = 0x100 - ps->checksum;
          if ( ps->checksum != ps->value)
            problem = true;
          switch ( ps->record_type)
          {
            case REC_TYPE_DATA:
              ps->sink( ps->address, ps->data, ps->data_on_line);
              break;
            case REC_TYPE_ADDR_HI:
              ps->address = (ps->address & 0xffff) | (ps->data[0] << 24) | (ps->data[1] << 16);
              break;
            case REC_TYPE_ENTRY_POINT:
              ps->entry_point = 0;
              for ( int i = 0; i < 4; i += 1)
                ps->entry_point |= ps->data[i] << (8 * (3-i));
              break;
          }
          ps->expecting = EXPECTING_START_CODE;
          break;
      }
      ps->value = 0x0000;
      if ( 0 == ps->digits_needed)
        ps->digits_needed = 2;
    }
  } else {
    problem = true;
  }
  return problem;
}

/*
void hex_file_parser_to_s( HexFileParser *ps, char *buf, int buf_size)
{
  char *last = buf + buf_size - 1;
  buf_char
}
*/
