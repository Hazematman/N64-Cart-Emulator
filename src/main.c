#include <stdbool.h>
#include <stdint.h>
#include "d1_regs.h"
#include "gpio.h"
#include "printf.h"

#define ALE_L_PORT GPIO_PORT_B 
#define ALE_L_PIN 1
#define ALE_L_INT GPIOB_NS

// TODO update these defines once pins are decided
#define ALE_H_PORT GPIO_PORT_B 
#define ALE_H_PIN 1
#define ALE_H_INT GPIOB_NS

#define READ_PORT GPIO_PORT_B 
#define READ_PIN 1
#define READ_INT GPIOB_NS

#define MAX_ROM_SIZE (64*1024*1024)

uint8_t rom_data[MAX_ROM_SIZE];
uint32_t address;
bool in_read = false;

uint16_t read_ad_bus()
{
    // TODO implement this function once we pick ports
    return 0;
}

void write_ad_bus(uint16_t value)
{
    // TODO implement this function once we pick ports
    (void)value;
}

void set_ad_bus_output()
{
}

void set_ad_bus_input()
{
}

__attribute__((interrupt("supervisor")))
void interrupt_handler(void)
{
    uint64_t cause = read_csr(sip);
    uint32_t claim = read_reg(PLIC_SCLAIM_REG);
    if(cause & SIP_SW_INT_PENDING)
    {
        printf("Software interrupt!\r\n");
        csr_clear_bit(sip, SIP_SW_INT_PENDING);
    }
    else if(cause & SIP_EXT_INT_PENDING)
    {
        bool ale_l_status = get_gpio_interrupt_status(ALE_L_PORT, ALE_L_PIN);
        bool ale_h_status = get_gpio_interrupt_status(ALE_H_PORT, ALE_H_PIN);
        bool read_status = get_gpio_interrupt_status(READ_PORT, READ_PIN);

        printf("GPIO Interrupt 0x%x!\r\ncause 0x%llx\r\n", read_reg(GPIO_PB_EINT_STATUS), cause);
        if(ale_l_status && claim == ALE_L_INT)
        {
            printf("ALE_L ACTIVE\r\n");
            address |= read_ad_bus();
            clear_gpio_interrupt(ALE_L_PORT, ALE_L_PIN);
        }

        if(ale_h_status && claim == ALE_H_INT)
        {
            printf("ALE_H ACTIVE\r\n");
            address |= (read_ad_bus() << 16);
            clear_gpio_interrupt(ALE_H_PORT, ALE_H_PIN);
        }

        if(read_status && claim == READ_INT)
        {
            printf("READ ACTIVE\r\n");
            if(in_read)
            {
                // Set AD bus to be an input so N64 and SBC don't try to drive the pins at the same time
                // and set GPIO int mode to neg edge to trigger on next read
                set_ad_bus_input();
                set_gpio_interrupt(READ_PORT, READ_PIN, GPIO_INT_MODE_NEG_EDGE);

                in_read = false;
            }
            else
            {
                uint16_t *data = (uint16_t*)&rom_data[address];
                set_ad_bus_output();
                write_ad_bus(data[0]);
                address += 2;

                // Set GPIO int mode to trigger on rising edge so we can stop the SBC from driving
                // the bus
                set_gpio_interrupt(READ_PORT, READ_PIN, GPIO_INT_MODE_POS_EDGE);

                in_read = true;
            }
            clear_gpio_interrupt(READ_PORT, READ_PIN);
        }
    }
    else
    {
        printf("Unknown cause 0x%llx, 0x%x\r\n", cause, claim);
    }

    write_reg(PLIC_SCLAIM_REG, claim);
}

void enable_interrupts()
{
    // Enable interrupts for supervisor
    csr_set_bit(sstatus, (1<<1));

    // Enable external interrupts for supervisor
    csr_set_bit(sie, (1<<9));
    
    // Setup interrupt handler
    set_interrupt_handler(interrupt_handler);
}

void init_interrupts()
{
    // Enable ALE_L interrupt at platform level
    plic_enable_interrupt(ALE_L_INT, 1);
    // Setup ALE_L interrupt mode to negative edge
    set_gpio_interrupt(ALE_L_PORT, ALE_L_PIN, GPIO_INT_MODE_NEG_EDGE);
    // Enable ALE_L interrupt
    enable_gpio_interrupt(ALE_L_PORT, ALE_L_PIN, true);
    // Set ALE_H pint mode to interrupt
    set_gpio_pin_mode(ALE_H_PORT, ALE_H_PIN, GPIO_MODE_INT);

    // Enable ALE_H interrupt at platform level
    plic_enable_interrupt(ALE_H_INT, 1);
    // Setup ALE_H interrupt mode to negative edge
    set_gpio_interrupt(ALE_H_PORT, ALE_H_PIN, GPIO_INT_MODE_NEG_EDGE);
    // Enable ALE_H interrupt
    enable_gpio_interrupt(ALE_H_PORT, ALE_H_PIN, true);
    // Set ALE_H pint mode to interrupt
    set_gpio_pin_mode(ALE_H_PORT, ALE_H_PIN, GPIO_MODE_INT);

    // Enable READ interrupt at platform level
    plic_enable_interrupt(READ_INT, 1);
    // Setup READ interrupt mode to negative edge
    set_gpio_interrupt(READ_PORT, READ_PIN, GPIO_INT_MODE_NEG_EDGE);
    // Enable READ interrupt
    enable_gpio_interrupt(READ_PORT, READ_PIN, true);
    // Set READ pint mode to interrupt
    set_gpio_pin_mode(READ_PORT, READ_PIN, GPIO_MODE_INT);
}

int main(void)
{
    enable_interrupts();
    init_interrupts();

    printf("Hello world!\r\n");

    bool output = true;

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
