/* Define memory regions. */
MEMORY
{
  rom (rx) : ORIGIN = 0x0802a000, LENGTH = 4K
  ram (rwx) : ORIGIN = 0x20003000, LENGTH = 32K /* Total size is 36K, leave 4K for stack */
}

INCLUDE main.lds
