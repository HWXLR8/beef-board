#pragma once

class Axis {
public:
  virtual void poll() = 0;
  virtual uint8_t get() const = 0;
};

class AnalogAxis : public Axis {
public:
  AnalogAxis(uint8_t pin);

  void poll() override;
  uint8_t get() const override;

private:
  uint8_t pin;
  uint8_t position{};
};

class QeAxis : public Axis {
public:
  QeAxis(volatile uint8_t* PIN, uint8_t a_pin, uint8_t b_pin);

  void poll() override;
  uint8_t get() const override;

private:
  volatile uint8_t* PIN;
  uint8_t a_pin;
  uint8_t b_pin;
  uint8_t prev{};
  uint16_t position{};
};

extern int8_t tt_transitions[4][4];
extern AnalogAxis analog_x;
extern AnalogAxis analog_y;
extern QeAxis tt_x;
extern QeAxis tt_y;
