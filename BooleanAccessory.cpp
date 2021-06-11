#include "BooleanAccessory.h"

namespace Victoria {
  namespace Components {

    extern "C" homekit_server_config_t boolServerConfig;
    extern "C" homekit_characteristic_t boolCharacteristic;

    BooleanAccessory::BooleanAccessory(String id, uint8_t outputPin) : BaseAccessory(id, outputPin, &boolServerConfig, &boolCharacteristic) {
      _mainCharacteristic->setter_ex = _setter_ex;
      _init();
    }

    AccessoryState BooleanAccessory::getState() {
      return {
        .boolValue = _mainCharacteristic->value.bool_value,
      };
    }

    void BooleanAccessory::setState(const AccessoryState& state) {
      _innerSetState(state, true);
    }

    void BooleanAccessory::_innerSetState(const AccessoryState& state, bool notify) {
      if (_mainCharacteristic) {
        _mainCharacteristic->value.bool_value = state.boolValue;
        if (notify) {
          _notify();
        }
      }
      if (state.boolValue) {
        digitalWrite(_outputPin, LOW);
      } else {
        digitalWrite(_outputPin, HIGH);
      }
      BaseAccessory::setState(state);
    }

    void BooleanAccessory::_setter_ex(homekit_characteristic_t *ch, const homekit_value_t value) {
      auto accessory = static_cast<BooleanAccessory*>(_findAccessory(ch));
      if (accessory) {
        accessory->_innerSetState({
          .boolValue = value.bool_value,
        }, false);
      }
    }

  }
}
