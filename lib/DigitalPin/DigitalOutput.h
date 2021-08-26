#ifndef DigitalOutput_h
#define DigitalOutput_h

#include <Arduino.h>

namespace Victoria::Components {
  class DigitalOutput {
   public:
    DigitalOutput(uint8_t pin, uint8_t trueValue);
    void setValue(bool value);

   private:
    int _pin;
    bool _trueValue;
  };

} // namespace Victoria::Components

#endif // DigitalOutput_h
