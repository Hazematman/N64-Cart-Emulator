#include <stdbool.h>
#include <stdint.h>
#include "printf.h"

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

#define GPIOB_NS 85

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

extern uint64_t _start;

const char *hello_world = "Hello World!\r\n";

volatile uint32_t vector_table[0x400];

typedef enum
{
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
    GPIO_PORT_G,
} gpio_port_t;

typedef enum
{
    GPIO_INT_MODE_POS_EDGE,
    GPIO_INT_MODE_NEG_EDGE,
    GPIO_INT_MODE_HIGH,
    GPIO_INT_MODE_LOW,
    GPIO_INT_MODE_DOUBLE_EDGE,
} gpio_interrupt_mode_t;

typedef enum
{
    GPIO_MODE_INPUT = 0b0000,
    GPIO_MODE_OUTPUT = 0b0001,
    GPIO_MODE_A,
    GPIO_MODE_B,
    GPIO_MODE_C,
    GPIO_MODE_D,
    GPIO_MODE_E,
    GPIO_MODE_F,
    GPIO_MODE_G,
    GPIO_MODE_H,
    GPIO_MODE_INT = 0b1110,
    GPIO_MODE_DISABLE = 0b1111,
} gpio_pin_mode_t;

static inline void dev_barrier(void) {
    asm volatile (
        "fence iorw, iorw"
        :
        :
        : "memory"
    ); 
}

// Nezeha board configuration
// PB3  ALE_L (Header 38)
// PB4  ALE_H (Header 40)
// PB5 /READ (Header 12)

#define write_reg(r, v) write_reg_handler((volatile uint32_t*)(r), (v))
void write_reg_handler(volatile uint32_t *reg, const uint32_t value)
{
    reg[0] = value;
}

#define read_reg(r) read_reg_handler((volatile uint32_t*)(r))
uint32_t read_reg_handler(volatile uint32_t *reg)
{
    return reg[0];
}

__attribute__((interrupt("supervisor")))
void gpiob_interrupt(void)
{
    uint64_t cause = read_csr(sip);
    uint32_t claim = read_reg(PLIC_SCLAIM_REG);
    if(cause & (1<<1))
    {
        printf("Software interrupt!\r\n");
        csr_clear_bit(sip, 1<<1);
    }
    else if(cause & (1<<9) && claim == GPIOB_NS)
    {
        printf("GPIO Interrupt 0x%x!\r\ncause 0x%llx\r\n", read_reg(GPIO_PB_EINT_STATUS), cause);

        // Clear interrupt register
        // TODO figure out why this register needs to be written to twice to clear the GPIO interrupt
        write_reg(GPIO_PB_EINT_STATUS, read_reg(GPIO_PB_EINT_STATUS));
        write_reg(GPIO_PB_EINT_STATUS, read_reg(GPIO_PB_EINT_STATUS));
    }
    else
    {
        printf("Unknown cause 0x%llx, 0x%x\r\n", cause, claim);
    }

    write_reg(PLIC_SCLAIM_REG, claim);
}

void _putchar(char c)
{
    while((read_reg(UART0_USR) & 0b10) == 0)
    {
        asm("nop");
    }

    write_reg(UART0_DATA_REG, c);
}

void set_gpio_interrupt(gpio_port_t port, int pin, gpio_interrupt_mode_t mode)
{
    uint32_t pin_offset = pin / 8;
    uint32_t pin_index = pin % 8;
    uintptr_t gpio_int_reg =  GPIO_PB_EINT_CFG0 + (port*0x20) + (pin_offset*sizeof(uint32_t));
    uint32_t gpio_int_value = read_reg(gpio_int_reg);
    gpio_int_value &= ~(0b1111 << (pin_index*4));
    gpio_int_value |= (mode << (pin_index*4));
    write_reg(gpio_int_reg, gpio_int_value); 
}

void enable_gpio_interrupt(gpio_port_t port, int pin, bool enable)
{
    uintptr_t gpio_int_reg = GPIO_PB_EINT_CTL + (port*0x20);
    if(enable)
    {
        write_reg(gpio_int_reg, read_reg(gpio_int_reg) | (1 << pin));
    }
    else
    {
        write_reg(gpio_int_reg, read_reg(gpio_int_reg) & ~(1 << pin));
    }
}

void set_gpio_pin_mode(gpio_port_t port, int pin, gpio_pin_mode_t mode)
{
    uint32_t pin_offset = pin / 8;
    uint32_t pin_index = pin % 8;
    uintptr_t gpio_mode_reg =  GPIO_PB_CFG_0 + (port*0x30) + (pin_offset*sizeof(uint32_t));
    uint32_t gpio_mode_value = read_reg(gpio_mode_reg);

    gpio_mode_value &= ~(0b1111 << (pin_index*4));
    gpio_mode_value |= (mode << (pin_index*4));
    write_reg(gpio_mode_reg, gpio_mode_value);
}

void set_gpio_pin_output(gpio_port_t port, int pin, bool enable)
{
    uintptr_t gpio_dat_reg = GPIO_PB_DAT + (port*0x30);

    if(enable)
    {
        write_reg(gpio_dat_reg, read_reg(gpio_dat_reg) | (1 << pin));
    }
    else
    {
        write_reg(gpio_dat_reg, read_reg(gpio_dat_reg) & ~(1 << pin));
    }
}

void enable_interrupt(uint64_t offset)
{
    // Enable interrupts for supervisor
    csr_set_bit(sstatus, (1<<1));

    // Enable external interrupts for supervisor
    csr_set_bit(sie, (1<<9));
    
    // Enable GPIOB interrupt for supervisor
    int bit = offset % 32;
    write_reg(PLIC_SIE(offset), 1<<bit);

    // Set priority
    write_reg(PLIC_PRIO_REG(offset), 1);

    // Setup interrupt controller
    write_csr(sscratch, 0);
    uint64_t stvec_val = ((uint64_t)gpiob_interrupt & ~(0b11));
    write_csr(stvec, stvec_val);
}

void init_interrupts()
{
    // Setup PB1 interrupt mode to positive edge
    set_gpio_interrupt(GPIO_PORT_B, 1, GPIO_INT_MODE_POS_EDGE);

    // Enable PB1 interrupt
    enable_gpio_interrupt(GPIO_PORT_B, 1, true);

    // Set GPIO PB1 to interrupt
    set_gpio_pin_mode(GPIO_PORT_B, 1, GPIO_MODE_INT);
}

int main(void)
{
    enable_interrupt(GPIOB_NS);
    init_interrupts();

    printf("Hello world!\r\n");
    uint64_t status = read_csr(sstatus);
    printf("status is 0x%0x\r\n", status);
    uint64_t sie = read_csr(sie);
    printf("sie is 0x%0x\r\n", sie);

    uint32_t output = 1;

    // Set GPIO PB0 to output
    set_gpio_pin_mode(GPIO_PORT_B, 0, GPIO_MODE_OUTPUT);
    set_gpio_pin_output(GPIO_PORT_B, 0, output);


    // Stay in an infinite loop so stuff doesn't crash
    while(true)
    {
#ifdef DEBUG_PRINT
        // Check if there is an interrupt pending for GPIOB
        printf("plic ip is 0x%0x\r\n", read_reg(PLIC_IP_REG(GPIOB_NS)));
        printf("eint status is 0x%0x\r\n", read_reg(GPIO_PB_EINT_STATUS));
#endif
        output = !output;
        for(int i=0; i < 1000*1000*10; i++) asm("nop");
        set_gpio_pin_output(GPIO_PORT_B, 0, output);
    }
}
