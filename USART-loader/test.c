
#include <stdio.h>
#include "hex_file.h"


void sink( u32 address, u8 *data, u8 data_on_line)
{
  printf("%08x:", address);
  for ( int i = 0; i < data_on_line; i += 1)
    printf(" %02x", data[i]);
  printf("\n");
}


int main( int argc, char **argv)
{
  HexFileParser  ps;
  prime_hex_file_parser( &ps, sink);
  FILE  *f = fopen( argv[1], "r");
  int  byte;
  while ( (byte = fgetc(f)) != EOF)
  {
    if ( parse_hex_file( &ps, (char)byte) )
      printf("ERR\n");
  }
  printf("ep:%08x\n", ps.entry_point);
  fclose( f);
}

