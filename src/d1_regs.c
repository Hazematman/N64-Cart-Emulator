#include "d1_regs.h"

void write_reg_handler(volatile uint32_t *reg, const uint32_t value)
{
    reg[0] = value;
}

uint32_t read_reg_handler(volatile uint32_t *reg)
{
    return reg[0];
}


