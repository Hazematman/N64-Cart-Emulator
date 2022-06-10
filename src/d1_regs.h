#ifndef D1_REGS_H
#define D1_REGS_H
#include <stdint.h>

// System registers
#define write_csr(csr, val) \
({ \
    uint64_t v = (uint64_t)val; \
    asm volatile ( \
        "csrw " #csr ", %0" \
        : \
        : "rK"(v) \
        : "memory" \
    ); \
})

#define read_csr(csr) \
({ \
    uint64_t v; \
    asm volatile ( \
        "csrr %0, " #csr  \
        : "=r"(v) \
        : \
        : \
    ); \
    v; \
})

#define csr_set_bit(csr, bitmask) \
({ \
    uint64_t mask = (uint64_t)(bitmask); \
    asm volatile ( \
        "csrs " #csr ", %0" \
        : \
        : "rK"(mask) \
        : "memory" \
    ); \
})

#define csr_clear_bit(csr, bitmask) \
({ \
    uint64_t mask = (uint64_t)(bitmask); \
    asm volatile ( \
        "csrc " #csr ", %0" \
        : \
        : "rK"(mask) \
        : "memory" \
    ); \
})

// System macros
#define write_reg(r, v) write_reg_handler((volatile uint32_t*)(r), (v))
void write_reg_handler(volatile uint32_t *reg, uint32_t value);
#define read_reg(r) read_reg_handler((volatile uint32_t*)(r))
uint32_t read_reg_handler(volatile uint32_t *reg);

// Registers 
#define RISCV_CFG_BASE 0x06010000
#define RISCV_STA_ADDR0 (RISCV_CFG_BASE + 0x0004)
#define RISCV_STA_ADDR1 (RISCV_CFG_BASE + 0x0008)

#define RISCV_PLIC_BASE 0x10000000
#define PLIC_CTRL_REG (RISCV_PLIC_BASE + 0x1ffffc)
#define PLIC_PRIO_REG(n) (RISCV_PLIC_BASE + 0x0000 + (n)*0x0004)
#define PLIC_IP_REG(n) (RISCV_PLIC_BASE + 0x1000 + ((n)/32)*0x0004)
#define PLIC_MIE(n) (RISCV_PLIC_BASE + 0x2000 + ((n)/32)*0x0004)
#define PLIC_SIE(n) (RISCV_PLIC_BASE + 0x2080 + ((n)/32)*0x0004)
#define PLIC_STH_REG (RISCV_PLIC_BASE + 0x201000)
#define PLIC_SCLAIM_REG (RISCV_PLIC_BASE + 0x201004)

#define TIMER_BASE 0x02050000
#define TIMER_SOFTWARE_RESET (TIMER_BASE + 0x00A8)
    #define RESET_SYSTEM 0x16AA0001

#define UART0_BASE 0x02500000
#define UART0_DATA_REG (UART0_BASE + 0x0000)
#define UART0_USR (UART0_BASE + 0x007c)

#define GPIO_BASE 0x02000000
#define GPIO_PB_CFG_0 (GPIO_BASE + 0x0030)
#define GPIO_PB_DAT (GPIO_BASE + 0x0040)
#define GPIO_PB_PULL0 (GPIO_BASE + 0x0054);
#define GPIO_PB_EINT_CFG0 (GPIO_BASE + 0x0220)
#define GPIO_PB_EINT_CTL (GPIO_BASE + 0x0230)
#define GPIO_PB_EINT_STATUS (GPIO_BASE + 0x0234)
#define GPIO_PB_EINT_DEB (GPIO_BASE + 0x0238)

// Interrupts
#define GPIOB_NS 85

#endif
