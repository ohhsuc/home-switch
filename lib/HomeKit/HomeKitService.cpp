#include "HomeKitService.h"

namespace Victoria::HomeKit {

  std::map<homekit_characteristic_t*, HomeKitService*> _services;

  HomeKitService::HomeKitService(String id, uint8_t outputPin, homekit_characteristic_t* mainCharacteristic) {
    serviceId = id;
    _outputPin = outputPin;
    _mainCharacteristic = mainCharacteristic;
    _services[_mainCharacteristic] = this;
  }

  HomeKitService::~HomeKitService() {
    if (_mainCharacteristic) {
      if (_services.count(_mainCharacteristic) > 0) {
        _services.erase(_mainCharacteristic);
      }
      delete _mainCharacteristic;
      _mainCharacteristic = NULL;
    }
    console.log("Service disposed " + serviceId);
  }

  ServiceState HomeKitService::getState() {
    console.log("HomeKitService::getState");
  }

  void HomeKitService::setState(const ServiceState& state) {
    if (onStateChange) {
      onStateChange(state);
    }
  }

  void HomeKitService::_notify() {
    if (_mainCharacteristic) {
      homekit_characteristic_notify(_mainCharacteristic, _mainCharacteristic->value);
    }
  }

  HomeKitService* HomeKitService::findServiceById(const String& serviceId) {
    for (auto const& pair : _services) {
      if (pair.second->serviceId == serviceId) {
        return pair.second;
      }
    }
    return NULL;
  }

  void HomeKitService::heartbeat() {
    for (auto const& pair : _services) {
      pair.second->_notify();
    }
  }

  HomeKitService* HomeKitService::_findService(homekit_characteristic_t* mainCharacteristic) {
    if (_services.count(mainCharacteristic) > 0) {
      return _services[mainCharacteristic];
    }
    return NULL;
  }

} // namespace Victoria::HomeKit
