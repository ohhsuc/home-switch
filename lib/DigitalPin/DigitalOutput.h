#ifndef DigitalOutput_h
#define DigitalOutput_h

#include <Arduino.h>

namespace Victoria::Components {
  class DigitalOutput {
   public:
    DigitalOutput(uint8_t pin, uint8_t trueValue);
    void setValue(bool value);

   private:
    uint8_t _pin;
    uint8_t _trueValue;
  };

} // namespace Victoria::Components

#endif // DigitalOutput_h
