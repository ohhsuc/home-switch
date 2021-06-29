#include "HomeKitService.h"

namespace Victoria::Components {

  std::map<homekit_characteristic_t*, HomeKitService*> _services;

  HomeKitService::HomeKitService(String id, uint8_t outputPin, homekit_server_config_t* serverConfig, homekit_characteristic_t* mainCharacteristic) {
    serviceId = id;
    _outputPin = outputPin;
    _serverConfig = serverConfig;
    _mainCharacteristic = mainCharacteristic;
    _services[_mainCharacteristic] = this;
  }

  HomeKitService::~HomeKitService() {
    if (_serverConfig) {
      delete _serverConfig;
      _serverConfig = NULL;
    }
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

  void HomeKitService::_init() {
    if (_serverConfig) {
      arduino_homekit_setup(_serverConfig);
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

  void HomeKitService::heartbeatAll() {
    for (auto const& pair : _services) {
      pair.second->_notify();
    }
  }

  void HomeKitService::loopAll() {
    arduino_homekit_loop();
  }

  void HomeKitService::resetAll() {
    homekit_server_reset();
  }

  HomeKitService* HomeKitService::_findService(homekit_characteristic_t* mainCharacteristic) {
    if (_services.count(mainCharacteristic) > 0) {
      return _services[mainCharacteristic];
    }
    return NULL;
  }

} // namespace Victoria::Components
