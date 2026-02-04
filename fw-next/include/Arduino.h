#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp/board_api.h"
#include "hardware/gpio.h"

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

inline uint32_t micros()
{
    return time_us_32();
}

inline uint32_t millis()
{
    return board_millis();
}

inline void delayMicroseconds(uint32_t us)
{
    sleep_us(us);
}

inline void pinMode(uint32_t pin, uint32_t mode)
{
    gpio_init(pin);
    if (mode == OUTPUT)
    {
        gpio_set_dir(pin, GPIO_OUT);
    }
    else
    {
        if (mode == INPUT_PULLUP)
        {
            gpio_pull_up(pin);
        }
    }
}

inline void digitalWrite(uint32_t pin, uint32_t val)
{
    gpio_put(pin, val);
}

inline int digitalRead(uint32_t pin)
{
    return gpio_get(pin);
}

inline void yield()
{
}

#ifdef __cplusplus
}
#endif
