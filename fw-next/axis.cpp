#include "axis.h"
#include "config.h"
#include "pins.h"

AnalogAxis analog_x(adc_gpio_pins[0]);
AnalogAxis analog_y(adc_gpio_pins[1]);
QeAxis tt_x(tt_pins[0]);
QeAxis tt_y(tt_pins[2]);

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

QeAxis::QeAxis(uint8_t a_pin)
{
    encoder.begin(a_pin, true);
}

void QeAxis::poll()
{
    encoder.update();
    const auto max = 256 * config.tt_ratio;
    position = encoder.step % max;
    if (!config.reverse_tt) position = max - position;
}

uint8_t QeAxis::get() const
{
    return position / config.tt_ratio;
}
