#include "uart.h"
#include "d1_regs.h"

void _putchar(char c)
{
    while((read_reg(UART0_USR) & 0b10) == 0)
    {
        asm("nop");
    }

    write_reg(UART0_DATA_REG, c);
}
