/********************* (C) COPYRIGHT 2010 e-Design Co.,Ltd. ********************
 File Name : Interrupt.c  
 Version   : DS203_APP Ver 2.3x                                  Author : bure
*******************************************************************************/
#include "Interrupt.h"
#include "BIOS.h"


void NMIException(void)
{}

/* Some simple debug routines */
static char linebuf[64];
static int pos = 0;
static int y = 220;

static void _putc(char b)
{
    if (b == '\n' || pos == 63)
    {
        linebuf[pos] = 0;
        __Display_Str(10, y, 0xFFFF, 0, linebuf);
        pos = 0;
        y -= 14;
    }
    else
    {
        linebuf[pos++] = b;
    }
}

static char hexnibble(int n)
{
    if (n < 10)
        return '0' + n;
    else
        return 'A' - 10 + n;
}

static void putstring(const char *p)
{
    while (*p)
    {
        _putc(*p++);
    }
}

static void puthex(const char *p, int count)
{
    while (count--)
    {
        _putc(hexnibble(*(p + count) >> 4));
        _putc(hexnibble(*(p + count) & 0x0F));
    }
}

static uint16_t color = 0b0000000000011111;
static void **sp;

void _HardFaultException();
void HardFaultException(void) __attribute__((noreturn, naked));
void HardFaultException()
{
    // Rescue stack pointer and program counter
    asm("mrs %0, msp" : "=r"(sp) : :);
    _HardFaultException();
}

void _HardFaultException()
{
    __Point_SCR(0, 0);
    __LCD_Fill(&color,240*200);
    __LCD_Fill(&color,240*200);

    __Set(BEEP_VOLUME, 0);

    putstring("                    HARDFAULT                ");

    putstring("\nSP: ");
    puthex((char*) &sp, sizeof(void*));
    putstring("  PC: ");
    puthex((char*) (sp + 6), sizeof(void*));
    putstring("  LR: ");
    puthex((char*) (sp + 5), sizeof(void*));
    putstring("\nREGS: r0 ");
    puthex((char*) (sp + 0), sizeof(void*));
    putstring(", r1 ");
    puthex((char*) (sp + 1), sizeof(void*));
    putstring("\n      r2 ");
    puthex((char*) (sp + 2), sizeof(void*));
    putstring(", r3 ");
    puthex((char*) (sp + 3), sizeof(void*));
    putstring(", r12 ");
    puthex((char*) (sp + 4), sizeof(void*));

    putstring("\nSCB HFSR: ");
    puthex((char*) &(SCB->HFSR), sizeof(SCB->HFSR));

    putstring("\nSCB CFSR: ");
    puthex((char*) &(SCB->CFSR), sizeof(SCB->CFSR));

    putstring("\nSTACK DUMP:\n");
    for (int i = 0; i < 32; i++)
    {
        puthex((char*) (sp + i), sizeof(void*));
        if (i % 4 == 3)
            putstring("\n");
        else
            putstring(" ");
    }

    putstring("\n");

    while (1) {}
}

void MemManageException(void)
{
  while (1) {}
}

void BusFaultException(void)
{
  while (1) {}
}
void UsageFaultException(void)
{
  while (1) {}
}

void USB_HP_CAN_TX_IRQHandler(void)
{
  __CTR_HP();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
  __USB_Istr();
}

