#include "HomeKitMain.h"

namespace Victor::HomeKit {

  extern "C" homekit_characteristic_t accessoryName;
  extern "C" homekit_characteristic_t accessoryManufacturer;
  extern "C" homekit_characteristic_t accessoryModel;
  extern "C" homekit_characteristic_t accessorySerialNumber;
  extern "C" homekit_characteristic_t accessoryVersion;
  extern "C" homekit_server_config_t boolServerConfig;

  HomeKitService* HomeKitMain::createService(const String& serviceId, const ServiceSetting& serviceSetting) {
    if (serviceSetting.type == BooleanServiceType) {
      const auto booleanService = new BooleanHomeKitService(serviceId, serviceSetting);
      _idServiceMap[booleanService->serviceId] = booleanService;
      return booleanService;
    } else if (serviceSetting.type == IntegerServiceType) {
      // TODO:
    }
    return NULL;
  }

  HomeKitService* HomeKitMain::findServiceById(const String& serviceId) {
    if (_idServiceMap.count(serviceId) > 0) {
      return _idServiceMap[serviceId];
    }
    return NULL;
  }

  void HomeKitMain::removeService(const String& serviceId) {
    if (_idServiceMap.count(serviceId) > 0) {
      const auto service = _idServiceMap[serviceId];
      _idServiceMap.erase(serviceId);
      delete service;
    }
  }

  void HomeKitMain::clear() {
    _idServiceMap.clear();
  }

  void HomeKitMain::setup(String hostName) {
    for (const auto& pair : _idServiceMap) {
      pair.second->setup();
    }

    // accessory information
    _hostName = hostName;
    accessoryName.value.string_value         = const_cast<char*>(_hostName.c_str());
    accessoryManufacturer.value.string_value = const_cast<char*>(AccessoryInformationManufacturer.c_str());
    accessoryModel.value.string_value        = const_cast<char*>(AccessoryInformationModel.c_str());
    accessorySerialNumber.value.string_value = const_cast<char*>(AccessoryInformationSerialNumber.c_str());
    accessoryVersion.value.string_value      = const_cast<char*>(FirmwareVersion.c_str());

    // server configuration
    const auto password = const_cast<char*>(HomekitServerPassword.c_str());
    boolServerConfig.password = password;
    arduino_homekit_setup(&boolServerConfig);
  }

  void HomeKitMain::loop() {
    arduino_homekit_loop();
    for (const auto& pair : _idServiceMap) {
      pair.second->loop();
    }
  }

  void HomeKitMain::reset() {
    homekit_server_reset();
  }

  int HomeKitMain::countClients() {
    return arduino_homekit_connected_clients_count();
  }

} // namespace Victor::HomeKit
