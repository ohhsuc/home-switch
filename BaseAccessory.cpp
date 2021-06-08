#include "BaseAccessory.h"

namespace Victoria {
  namespace Components {

    extern "C" homekit_characteristic_t versionCharacteristic;
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
      console.log("Accessory disposed " + accessoryId);
    }

    AccessoryState BaseAccessory::getState() {
      console.log("BaseAccessory::getState");
    }

    void BaseAccessory::setState(const AccessoryState& state) {
      console.log("BaseAccessory::setState");
    }

    void BaseAccessory::notify() {
      if (_mainCharacteristic) {
        homekit_characteristic_notify(_mainCharacteristic, _mainCharacteristic->value);
      }
    }

    void BaseAccessory::_init() {
      if (_serverConfig) {
        arduino_homekit_setup(_serverConfig);
      }
    }

    BaseAccessory* BaseAccessory::findAccessoryById(const String& accessoryId) {
      for (auto const& pair : _accessories) {
        if (pair.second->accessoryId == accessoryId) {
          return pair.second;
        }
      }
      return NULL;
    }

    void BaseAccessory::heartbeatAll() {
      for (auto const& pair : _accessories) {
        pair.second->notify();
      }
    }

    void BaseAccessory::loopAll() {
      arduino_homekit_loop();
    }

    void BaseAccessory::resetAll() {
      homekit_server_reset();
    }

    String BaseAccessory::getVersion() {
      return versionCharacteristic.value.string_value;
    }

    BaseAccessory* BaseAccessory::_findAccessory(homekit_characteristic_t* mainCharacteristic) {
      if (_accessories.count(mainCharacteristic) > 0) {
        return _accessories[mainCharacteristic];
      }
      return NULL;
    }

  }
}
