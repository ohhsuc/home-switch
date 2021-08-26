#ifndef BuiltinLed_h
#define BuiltinLed_h

#include <Arduino.h>
#include "DigitalOutput.h"
#include "AppStorage.h"

namespace Victoria::Components {
  class BuiltinLed {
   public:
    BuiltinLed();
    ~BuiltinLed();
    void turnOn();
    void turnOff();
    void flash();

   private:
    DigitalOutput* _outputPin;
  };

} // namespace Victoria::Components

#endif // BuiltinLed_h
