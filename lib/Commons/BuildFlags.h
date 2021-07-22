#ifndef BuildFlags_h
#define BuildFlags_h

#ifndef UNIX_TIME
#define UNIX_TIME 0
#endif

#ifndef FIRMWARE_NAME
#define FIRMWARE_NAME "Victoria"
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

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

namespace Victoria {
  const unsigned long UnixTime = UNIX_TIME;
  const String FirmwareName = FIRMWARE_NAME;
  const String FirmwareVersion = FIRMWARE_VERSION;
  const String AccessoryInformationManufacturer = ACCESSORY_INFORMATION_MANUFACTURER;
  const String AccessoryInformationModel = ACCESSORY_INFORMATION_MODEL;
  const String AccessoryInformationSerialNumber = ACCESSORY_INFORMATION_SERIAL_NUMBER;
  const String HomekitServerPassword = HOMEKIT_SERVER_PASSWORD;
} // namespace Victoria

#endif // BuildFlags_h
