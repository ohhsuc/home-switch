#ifndef HomeKitBuildFlags_h
#define HomeKitBuildFlags_h

#ifndef VICTOR_ACCESSORY_INFORMATION_MANUFACTURER
#define VICTOR_ACCESSORY_INFORMATION_MANUFACTURER "VictorSmart Co.,Ltd."
#endif

#ifndef VICTOR_ACCESSORY_INFORMATION_MODEL
#define VICTOR_ACCESSORY_INFORMATION_MODEL "ESP8266/ESP32"
#endif

#ifndef VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER
#define VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER "0123456"
#endif

#ifndef VICTOR_HOMEKIT_SERVER_PASSWORD
#define VICTOR_HOMEKIT_SERVER_PASSWORD "111-11-111"
#endif

#include <Arduino.h>

namespace Victor {
  const String AccessoryInformationManufacturer = VICTOR_ACCESSORY_INFORMATION_MANUFACTURER;
  const String AccessoryInformationModel = VICTOR_ACCESSORY_INFORMATION_MODEL;
  const String AccessoryInformationSerialNumber = VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER;
  const String HomekitServerPassword = VICTOR_HOMEKIT_SERVER_PASSWORD;
} // namespace Victor

#endif // HomeKitBuildFlags_h
