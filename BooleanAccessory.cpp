#include "BooleanAccessory.h"

namespace Victoria {
  namespace Components {

    BooleanAccessory::BooleanAccessory(uint8_t outputPin) {
      _outputPin = outputPin;
    }

    void BooleanAccessory::setValue(bool value) {
      if (value) {
        digitalWrite(_outputPin, LOW);
        Serial.println("write -> LOW");
      } else {
        digitalWrite(_outputPin, HIGH);
        Serial.println("write -> HIGH");
      }
    }

    bool BooleanAccessory::getValue() {
      return true;
    }

  }
}
