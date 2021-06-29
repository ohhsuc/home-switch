#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#ifndef ACCESSORY_INFORMATION_NAME
#define ACCESSORY_INFORMATION_NAME "Victoria"
#endif

#ifndef ACCESSORY_INFORMATION_MANUFACTURER
#define ACCESSORY_INFORMATION_MANUFACTURER "Victoria 9# Inc."
#endif

#ifndef ACCESSORY_INFORMATION_MODEL
#define ACCESSORY_INFORMATION_MODEL "ESP8266/ESP32"
#endif

#ifndef ACCESSORY_INFORMATION_SERIAL_NUMBER
#define ACCESSORY_INFORMATION_SERIAL_NUMBER "0123456"
#endif

#ifndef ACCESSORY_INFORMATION_REVISION
#define ACCESSORY_INFORMATION_REVISION "0.0.1"
#endif

#ifndef HOMEKIT_SERVER_PASSWORD
#define HOMEKIT_SERVER_PASSWORD "111-11-111"
#endif

#define CHARACTERISTIC_NAME_BOOL "VictoriaBool"

void onAccessoryIdentify(homekit_value_t value) {
  printf("accessory identify\n");
}

homekit_service_t informationService = HOMEKIT_SERVICE_(
  ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    HOMEKIT_CHARACTERISTIC(NAME, ACCESSORY_INFORMATION_NAME),
    HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_INFORMATION_MANUFACTURER),
    HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, ACCESSORY_INFORMATION_SERIAL_NUMBER),
    HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_INFORMATION_MODEL),
    HOMEKIT_CHARACTERISTIC(IDENTIFY, onAccessoryIdentify),
    HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ACCESSORY_INFORMATION_REVISION),
    NULL,
  },
);

homekit_characteristic_t boolCharacteristic = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t boolNameCharacteristic = HOMEKIT_CHARACTERISTIC_(NAME, CHARACTERISTIC_NAME_BOOL);

homekit_service_t boolService = HOMEKIT_SERVICE_(
  SWITCH,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &boolCharacteristic,
    &boolNameCharacteristic,
    NULL,
  },
);

homekit_accessory_t* boolAccessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_switch,
    .services = (homekit_service_t*[]) {
      &informationService,
      &boolService,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t boolServerConfig = {
  .accessories = boolAccessories,
  .password = HOMEKIT_SERVER_PASSWORD,
};
