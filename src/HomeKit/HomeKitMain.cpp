#include "HomeKitMain.h"

namespace Victoria::HomeKit {

  extern "C" homekit_characteristic_t accessoryName;
  extern "C" homekit_characteristic_t accessoryManufacturer;
  extern "C" homekit_characteristic_t accessoryModel;
  extern "C" homekit_characteristic_t accessorySerialNumber;
  extern "C" homekit_characteristic_t accessoryVersion;
  extern "C" homekit_server_config_t boolServerConfig;

  String _forKeepingHostName;
  std::map<String, HomeKitService*> _id_service_map;

  HomeKitService* HomeKitMain::createService(const String& serviceId, const ServiceSetting& serviceSetting) {
    if (serviceSetting.type == BooleanServiceType) {
      auto booleanService = new BooleanHomeKitService(serviceId, serviceSetting);
      _id_service_map[booleanService->serviceId] = booleanService;
      return booleanService;
    } else if (serviceSetting.type == IntegerServiceType) {
      // TODO:
    }
    return NULL;
  }

  HomeKitService* HomeKitMain::findServiceById(const String& serviceId) {
    if (_id_service_map.count(serviceId) > 0) {
      return _id_service_map[serviceId];
    }
    return NULL;
  }

  void HomeKitMain::removeService(const String& serviceId) {
    if (_id_service_map.count(serviceId) > 0) {
      auto service = _id_service_map[serviceId];
      _id_service_map.erase(serviceId);
      delete service;
    }
  }

  void HomeKitMain::heartbeat() {
    for (const auto& pair : _id_service_map) {
      pair.second->notifyState();
    }
  }

  void HomeKitMain::setup(const String& hostName) {
    for (const auto& pair : _id_service_map) {
      pair.second->setup();
    }

    // accessory information
    _forKeepingHostName = hostName;
    accessoryName.value.string_value         = const_cast<char*>(_forKeepingHostName.c_str());
    accessoryManufacturer.value.string_value = const_cast<char*>(AccessoryInformationManufacturer.c_str());
    accessoryModel.value.string_value        = const_cast<char*>(AccessoryInformationModel.c_str());
    accessorySerialNumber.value.string_value = const_cast<char*>(AccessoryInformationSerialNumber.c_str());
    accessoryVersion.value.string_value      = const_cast<char*>(FirmwareVersion.c_str());

    // server configuration
    auto password = const_cast<char*>(HomekitServerPassword.c_str());
    boolServerConfig.password = password;
    arduino_homekit_setup(&boolServerConfig);
  }

  void HomeKitMain::loop() {
    for (const auto& pair : _id_service_map) {
      pair.second->loop();
    }
    arduino_homekit_loop();
  }

  void HomeKitMain::reset() {
    homekit_server_reset();
  }

} // namespace Victoria::HomeKit
