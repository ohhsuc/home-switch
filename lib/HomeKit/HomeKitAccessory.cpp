#include "HomeKitAccessory.h"

namespace Victoria::HomeKit {

  extern "C" homekit_characteristic_t accessoryName;
  extern "C" homekit_characteristic_t accessoryManufacturer;
  extern "C" homekit_characteristic_t accessoryModel;
  extern "C" homekit_characteristic_t accessorySerialNumber;
  extern "C" homekit_characteristic_t accessoryVersion;
  extern "C" homekit_server_config_t boolServerConfig;

  String _forKeepingHostName;

  HomeKitAccessory::HomeKitAccessory() {}

  void HomeKitAccessory::setup(const String& hostName) {
    _forKeepingHostName = hostName;
    // accessory information
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

  void HomeKitAccessory::loop() {
    arduino_homekit_loop();
  }

  void HomeKitAccessory::reset() {
    homekit_server_reset();
  }

} // namespace Victoria::HomeKit
