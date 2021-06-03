#include "BooleanAccessory.h"

#define CHARACTERISTIC_NAME "VictoriaSwitch"
#define CHARACTERISTIC_MANUFACTURER "Victoria 9# Inc."
#define CHARACTERISTIC_MODEL "ESP8266/ESP32"
#define CHARACTERISTIC_SERIAL_NUMBER "0123456"
#define CHARACTERISTIC_REVISION "0.3"
#define CHARACTERISTIC_PASSWORD "111-11-111"

namespace Victoria {
  namespace Components {

    BooleanAccessory::BooleanAccessory(uint8_t outputPin) {
      _outputPin = outputPin;
    }

    BooleanAccessory::~BooleanAccessory() {
    }

    void BooleanAccessory::setup() {
    }

    void BooleanAccessory::loop() {
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
