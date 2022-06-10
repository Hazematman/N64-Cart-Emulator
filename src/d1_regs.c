#include "d1_regs.h"

void write_reg_handler(volatile uint32_t *reg, const uint32_t value)
{
    reg[0] = value;
}

uint32_t read_reg_handler(volatile uint32_t *reg)
{
    return reg[0];
}

void plic_enable_interrupt(uint64_t offset, uint32_t priority)
{
    int bit = offset % 32;
    write_reg(PLIC_SIE(offset), 1<<bit);

    write_reg(PLIC_PRIO_REG(offset), priority);
}

void set_interrupt_handler(void *handler)
{
    // Setup interrupt handler
    uint64_t stvec_val = ((uint64_t)handler & ~(0b11));
    write_csr(stvec, stvec_val);
}
