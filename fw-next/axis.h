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
    QeAxis(uint8_t a_pin);

    void poll() override;
    [[nodiscard]] uint8_t get() const override;

private:
    volatile uint32_t position = 0;
};
