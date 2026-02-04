#pragma once

#include <cstdint>

class Axis
{
public:
    virtual ~Axis() = default;
    virtual void poll() = 0;
    [[nodiscard]] virtual uint8_t get() const = 0;
};

class AnalogAxis : public Axis
{
public:
    explicit AnalogAxis(uint8_t pin);

    void poll() override;
    [[nodiscard]] uint8_t get() const override;

private:
    uint8_t pin;
    uint8_t position = 0;
};

class QeAxis : public Axis
{
public:
    QeAxis(uint8_t a_pin, uint8_t b_pin);

    void poll() override;
    [[nodiscard]] uint8_t get() const override;

private:
    uint8_t a_pin;
    uint8_t b_pin;
    uint8_t prev = 0;
    uint16_t position = 0;
};

const extern int8_t tt_transitions[4][4];
extern AnalogAxis analog_x;
extern AnalogAxis analog_y;
extern QeAxis tt_x;
extern QeAxis tt_y;

