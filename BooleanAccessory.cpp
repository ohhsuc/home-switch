#include "BooleanAccessory.h"

namespace Victoria {
  namespace Components {

    extern "C" homekit_server_config_t boolServerConfig;
    extern "C" homekit_characteristic_t boolCharacteristic;
    std::map<homekit_characteristic_t*, BooleanAccessory*> _booleanAccessories;

    BooleanAccessory::BooleanAccessory(uint8_t outputPin) {
      _outputPin = outputPin;
    }

    BooleanAccessory::~BooleanAccessory() {
      if (_serverConfig) {
        delete _serverConfig;
        _serverConfig = NULL;
      }
      if (_boolCharacteristic) {
        if (_booleanAccessories.count(_boolCharacteristic) > 0) {
          _booleanAccessories.erase(_boolCharacteristic);
        }
        delete _boolCharacteristic;
        _boolCharacteristic = NULL;
      }
    }

    void BooleanAccessory::setup() {
      // ref
      _serverConfig = &boolServerConfig;
      _boolCharacteristic = &boolCharacteristic;
      _booleanAccessories[_boolCharacteristic] = this;
      // setup
      _boolCharacteristic->setter_ex = _setter_ex;
      arduino_homekit_setup(_serverConfig);
    }

    void BooleanAccessory::reset() {
      homekit_server_reset();
    }

    void BooleanAccessory::loop() {
      arduino_homekit_loop();
    }

    void BooleanAccessory::setValue(bool value) {
      if (_boolCharacteristic) {
        _boolCharacteristic->value.bool_value = value;
        _notify();
      }
      if (value) {
        digitalWrite(_outputPin, LOW);
      } else {
        digitalWrite(_outputPin, HIGH);
      }
      if (onChange) {
        onChange(value);
      }
    }

    bool BooleanAccessory::getValue() {
      return _boolCharacteristic->value.bool_value;
    }

    void BooleanAccessory::heartbeat() {
      _notify();
    }

    void BooleanAccessory::_notify() {
      if (_boolCharacteristic) {
        homekit_characteristic_notify(_boolCharacteristic, _boolCharacteristic->value);
      }
    }

    void BooleanAccessory::_setter_ex(homekit_characteristic_t *ch, const homekit_value_t value) {
      if (_booleanAccessories.count(ch) > 0) {
        auto accessory = _booleanAccessories[ch];
        accessory->setValue(value.bool_value);
      }
    }

  }
}
