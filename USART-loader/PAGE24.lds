/* Define memory regions. */
MEMORY
{
  rom (rx) : ORIGIN = 0x08024000, LENGTH = 4K
  ram (rwx) : ORIGIN = 0x20002000, LENGTH = 4K /* This is cheekily in SYS RAM space but the gcc SYS only uses 1396 bytes of the 12K */
}

INCLUDE main.lds
