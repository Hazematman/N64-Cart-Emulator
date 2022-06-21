#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
#include <stdbool.h>

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

typedef uint64_t gpio_pin_mode_block_t; 

void set_gpio_interrupt(gpio_port_t port, int pin, gpio_interrupt_mode_t mode);
void enable_gpio_interrupt(gpio_port_t port, int pin, bool enable);
void set_gpio_pin_mode(gpio_port_t port, int pin, gpio_pin_mode_t mode);
void set_gpio_pin_output(gpio_port_t port, int pin, bool enable);
void set_gpio_port_mode(gpio_port_t port, int start_pin, int end_pin, gpio_pin_mode_t mode);
gpio_pin_mode_block_t get_gpio_port_mode_block(gpio_port_t port, int start_pin, int end_pin, gpio_pin_mode_t mode);
void set_gpio_port_mode_block(gpio_port_t port, gpio_pin_mode_block_t mode_block);
uint32_t get_gpio_port_output(gpio_port_t port);
void set_gpio_port_output(gpio_port_t port, uint32_t value);
bool get_gpio_interrupt_status(gpio_port_t port, int pin);
void clear_gpio_interrupt(gpio_port_t port, int pin);

#endif
