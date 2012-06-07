
# USART-loader

USART-loader loads apps directly in to RAM via the USART.  It won't work unless you have access to the [serial port] on your DSO Quad.

USART-loader installs to slot 4 by default so that it starts when you power on the Quad with button 4 held down.  It will then wait for data to be sent to it via the USART.  It expects [Intel HEX] format, which is that used by most if not all Quad apps.  If the hex file includes an entry point ( most do) then the app will be started automatically.

Your app must have been linked so that all sections (`.text`, `.data` and `.bss`) reside between `0x20003000` and `0x2000b000` ( a 32K block).  Any data from the hex file that lies outside this region is silently discarded.  The stack ( 4K) grows down from the end of RAM at `0x0x2000c000` ( 48K) and the bottom 12K is reserved by SYS.


  [serial port]: /neilstockbridge/dsoquad-doc/wiki/The-serial-port
  [Intel HEX]: http://en.wikipedia.org/wiki/Intel_HEX

