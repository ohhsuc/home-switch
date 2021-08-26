#include "DigitalInput.h"

namespace Victoria::Components {

  DigitalInput::DigitalInput(uint8_t pin, uint8_t trueValue) {
    _pin = pin;
    _trueValue = trueValue;
    pinMode(_pin, INPUT);
  }

  bool DigitalInput::getValue() {
    auto value = digitalRead(_pin);
    return value == _trueValue;
  }

} // namespace Victoria::Components
