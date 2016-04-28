#ifndef __GPIOFAST_H__
#define __GPIOFAST_H__

#include "os_type.h"

#define GPIO_FAST_OUTPUT_SET(gpio_no, bit_value) \
    gpio_fast_output_set(gpio_no, bit_value)

static inline void gpio_fast_output_set(uint16_t gpio, int bit_value)
{
    if (bit_value)
        GPIO_REG_WRITE( GPIO_OUT_W1TS_ADDRESS, 1<<gpio);
    else
        GPIO_REG_WRITE( GPIO_OUT_W1TC_ADDRESS, 1<<gpio);
}


#endif
