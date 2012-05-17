/* Define memory regions. */
MEMORY
{
  rom (rx) : ORIGIN = 0x0801e000, LENGTH = 8K
  ram (rwx) : ORIGIN = 0x20003000, LENGTH = 32K /* Total size is 36K, leave 4K for stack */
}

INCLUDE main.lds
