#ifndef HomeKitBuildFlags_h
#define HomeKitBuildFlags_h

#ifndef ACCESSORY_INFORMATION_MANUFACTURER
#define ACCESSORY_INFORMATION_MANUFACTURER "VictorSmart Co.,Ltd."
#endif

#ifndef ACCESSORY_INFORMATION_MODEL
#define ACCESSORY_INFORMATION_MODEL "ESP8266/ESP32"
#endif

#ifndef ACCESSORY_INFORMATION_SERIAL_NUMBER
#define ACCESSORY_INFORMATION_SERIAL_NUMBER "0123456"
#endif

#ifndef HOMEKIT_SERVER_PASSWORD
#define HOMEKIT_SERVER_PASSWORD "111-11-111"
#endif

#include <Arduino.h>

namespace Victor {
  const String AccessoryInformationManufacturer = ACCESSORY_INFORMATION_MANUFACTURER;
  const String AccessoryInformationModel = ACCESSORY_INFORMATION_MODEL;
  const String AccessoryInformationSerialNumber = ACCESSORY_INFORMATION_SERIAL_NUMBER;
  const String HomekitServerPassword = HOMEKIT_SERVER_PASSWORD;
} // namespace Victor

#endif // HomeKitBuildFlags_h
