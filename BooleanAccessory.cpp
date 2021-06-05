#include "BooleanAccessory.h"

namespace Victoria {
  namespace Components {

    extern "C" homekit_server_config_t boolServerConfig;
    extern "C" homekit_characteristic_t boolCharacteristic;

    BooleanAccessory::BooleanAccessory(String id, uint8_t outputPin) : BaseAccessory(id, outputPin, &boolServerConfig, &boolCharacteristic) {
      _mainCharacteristic->setter_ex = _setter_ex;
      _init();
    }

    void BooleanAccessory::setValue(bool value) {
      if (_mainCharacteristic) {
        _mainCharacteristic->value.bool_value = value;
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
      return _mainCharacteristic->value.bool_value;
    }

    void BooleanAccessory::_setter_ex(homekit_characteristic_t *ch, const homekit_value_t value) {
      BooleanAccessory* accessory = static_cast<BooleanAccessory*>(_findAccessory(ch));
      if (accessory) {
        accessory->setValue(value.bool_value);
      }
    }

  }
}
