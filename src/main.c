#include <stdbool.h>
#include <stdint.h>
#include "d1_regs.h"
#include "gpio.h"
#include "printf.h"

// Nezeha board configuration
// PB3  ALE_L (Header 38)
// PB4  ALE_H (Header 40)
// PB5 /READ (Header 12)

const char *hello_world = "Hello World!\r\n";

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
