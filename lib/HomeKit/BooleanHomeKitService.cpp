#include "BooleanHomeKitService.h"

namespace Victoria::HomeKit {

  extern "C" homekit_characteristic_t boolCharacteristic;

  BooleanHomeKitService::BooleanHomeKitService(String id, uint8_t outputPin)
  : HomeKitService(id, outputPin, &boolCharacteristic) {
    _mainCharacteristic->setter_ex = _setter_ex;
  }

  ServiceState BooleanHomeKitService::getState() {
    return {
      .boolValue = _mainCharacteristic->value.bool_value,
    };
  }

  void BooleanHomeKitService::setState(const ServiceState& state) {
    _innerSetState(state, true);
  }

  void BooleanHomeKitService::_innerSetState(const ServiceState& state, bool notify) {
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
    HomeKitService::setState(state);
  }

  void BooleanHomeKitService::_setter_ex(homekit_characteristic_t* ch, const homekit_value_t value) {
    auto service = static_cast<BooleanHomeKitService*>(_findService(ch));
    if (service) {
      service->_innerSetState({
        .boolValue = value.bool_value,
      }, false);
    }
  }

} // namespace Victoria::HomeKit
