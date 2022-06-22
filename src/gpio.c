#include "gpio.h"
#include "d1_regs.h"

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

bool get_gpio_pin_output(gpio_port_t port, int pin)
{
    uintptr_t gpio_dat_reg = GPIO_PB_DAT + (port*0x30);
    return ((read_reg(gpio_dat_reg) >> pin) & 0b1);
}

void set_gpio_port_mode(gpio_port_t port, int start_pin, int end_pin, gpio_pin_mode_t mode)
{
    for(int i=start_pin; i <= end_pin; i++)
    {
        set_gpio_pin_mode(port, i, mode);
    } 
}

uint32_t get_gpio_port_output(gpio_port_t port)
{
    uintptr_t gpio_dat_reg = GPIO_PB_DAT + (port*0x30);
    return read_reg(gpio_dat_reg);
}

void set_gpio_port_output(gpio_port_t port, uint32_t value)
{
    uintptr_t gpio_dat_reg = GPIO_PB_DAT + (port*0x30);
    write_reg(gpio_dat_reg, value);
}


gpio_pin_mode_block_t get_gpio_port_mode_block(gpio_port_t port, int start_pin, int end_pin, gpio_pin_mode_t mode)
{
    (void)port;
    uint32_t values[2] = {0, 0};
    for(int i=start_pin; i <= end_pin; i++)
    {
        uint32_t pin_offset = i / 8;
        uint32_t pin_index = i % 8;
        values[pin_offset] &= ~(0b1111 << (pin_index*4));
        values[pin_offset] |= (mode << (pin_index*4));
    } 

    return (uint64_t)values[1]<<32 | values[0];
}

void set_gpio_port_mode_block(gpio_port_t port, gpio_pin_mode_block_t mode_block)
{
    uintptr_t gpio_mode_reg =  GPIO_PB_CFG_0 + (port*0x30);
    uint32_t values[2] = {(mode_block>>32)&0xFFFFFFFF, mode_block&0xFFFFFFFF};

    write_reg(gpio_mode_reg, values[0]);
    write_reg(gpio_mode_reg+4, values[1]);
}

bool get_gpio_interrupt_status(gpio_port_t port, int pin)
{
    uintptr_t gpio_int_status_reg = GPIO_PB_EINT_STATUS + (port*0x20);
    uint32_t reg_value = read_reg(gpio_int_status_reg);

    return (reg_value >> pin) & 1;
}

void clear_gpio_interrupt(gpio_port_t port, int pin)
{
    uintptr_t gpio_int_status_reg = GPIO_PB_EINT_STATUS + (port*0x20);
    write_reg(gpio_int_status_reg, (1<<pin));
}

void set_gpio_debounce(gpio_port_t port, uint32_t prescale, gpio_debounce_source_t source)
{
    uintptr_t gpio_debounce_reg = GPIO_PB_EINT_DEB + (port*0x20);

    write_reg(gpio_debounce_reg, ((prescale&0b111)<<4) | (source & 0b1));
}


void set_gpio_pull(gpio_port_t port, int pin, gpio_pull_t pull)
{
    uintptr_t gpio_pull_reg = GPIO_PB_PULL0 + (port*0x30);
    uint32_t reg_value = read_reg(gpio_pull_reg);

    reg_value &= ~(0b11<<(pin*2));
    reg_value |= ((pull&0b11) << (pin*2));
    write_reg(gpio_pull_reg, reg_value); 
} 
