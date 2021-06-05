#include "BaseAccessory.h"

namespace Victoria {
  namespace Components {

    std::map<homekit_characteristic_t*, BaseAccessory*> _accessories;

    BaseAccessory::BaseAccessory(String id, uint8_t outputPin, homekit_server_config_t* serverConfig, homekit_characteristic_t* mainCharacteristic) {
      accessoryId = id;
      _outputPin = outputPin;
      _serverConfig = serverConfig;
      _mainCharacteristic = mainCharacteristic;
      _accessories[_mainCharacteristic] = this;
    }

    BaseAccessory::~BaseAccessory() {
      if (_serverConfig) {
        delete _serverConfig;
        _serverConfig = NULL;
      }
      if (_mainCharacteristic) {
        if (_accessories.count(_mainCharacteristic) > 0) {
          _accessories.erase(_mainCharacteristic);
        }
        delete _mainCharacteristic;
        _mainCharacteristic = NULL;
      }
    }

    void BaseAccessory::loop() {
      arduino_homekit_loop();
    }

    void BaseAccessory::reset() {
      homekit_server_reset();
    }

    void BaseAccessory::heartbeat() {
      _notify();
    }

    void BaseAccessory::_init() {
      if (_serverConfig) {
        arduino_homekit_setup(_serverConfig);
      }
    }

    void BaseAccessory::_notify() {
      if (_mainCharacteristic) {
        homekit_characteristic_notify(_mainCharacteristic, _mainCharacteristic->value);
      }
    }

    BaseAccessory* BaseAccessory::_findAccessory(homekit_characteristic_t* mainCharacteristic) {
      if (_accessories.count(mainCharacteristic) > 0) {
        return _accessories[mainCharacteristic];
      }
      return NULL;
    }

  }
}
