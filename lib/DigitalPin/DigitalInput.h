#ifndef DigitalInput_h
#define DigitalInput_h

#include <Arduino.h>

namespace Victoria::Components {
  class DigitalInput {
   public:
    DigitalInput(uint8_t pin, uint8_t trueValue);
    bool getValue();

   private:
    int _pin;
    bool _trueValue;
  };

} // namespace Victoria::Components

#endif // DigitalInput_h
