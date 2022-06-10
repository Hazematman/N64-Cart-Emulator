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
