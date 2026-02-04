#include "axis.h"
#include "config.h"
#include "pins.h"

const int8_t tt_transitions[4][4] = {
    { 0, 1, -1, 0 },
    { -1, 0, 0, 1 },
    { 1, 0, 0, -1 },
    { 0, -1, 1, 0 }
};
AnalogAxis analog_x(adc_gpio_pins[0]);
AnalogAxis analog_y(adc_gpio_pins[1]);
QeAxis tt_x(tt_pins[0], tt_pins[1]);
QeAxis tt_y(tt_pins[2], tt_pins[3]);

AnalogAxis::AnalogAxis(const uint8_t pin) : pin(pin)
{
}

void AnalogAxis::poll()
{
}

uint8_t AnalogAxis::get() const
{
    return position;
}

QeAxis::QeAxis(uint8_t a_pin, uint8_t b_pin) : a_pin(a_pin), b_pin(b_pin)
{
}

void QeAxis::poll()
{
    // tt logic
    // example where tt_x wired to F0/F1:
    // curr is binary number ab
    // where a is the signal of F0
    // and b is the signal of F1
    // therefore when F0 == 1 and F1 == 0, then curr == 0b10
    const uint8_t a = gpio_get(a_pin);
    const uint8_t b = gpio_get(b_pin);
    const uint8_t curr = (b << 1) | a;

    auto dir = tt_transitions[prev][curr];
    prev = curr;

    if (config.reverse_tt) dir *= -1;
    position += dir;
    const auto max = 256 * config.tt_ratio;
    if (position < 0) position = max - position;
    position %= max;
}

uint8_t QeAxis::get() const
{
    return position / config.tt_ratio;
}
