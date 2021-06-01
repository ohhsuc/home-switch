#ifndef BooleanAccessory_h
#define BooleanAccessory_h

#include <Arduino.h>

namespace Victoria {
  namespace Components {
    class BooleanAccessory {
      public:
        BooleanAccessory(uint8_t outputPin);
        void setValue(bool value);
        bool getValue();
      private:
        uint8_t _outputPin;
    };
  }
}

#endif //BooleanAccessory_h
