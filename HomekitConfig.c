#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#define INFORMATION_NAME "Victoria"
#define INFORMATION_MANUFACTURER "Victoria 9# Inc."
#define INFORMATION_MODEL "ESP8266/ESP32"
#define INFORMATION_SERIAL_NUMBER "0123456"
#define INFORMATION_REVISION "21.2.31"

#define CHARACTERISTIC_NAME_BOOL "VictoriaBool"
#define SERVER_PASSWORD "111-11-111"

void onAccessoryIdentify(homekit_value_t value) {
  printf("accessory identify\n");
}

homekit_characteristic_t versionCharacteristic = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, INFORMATION_REVISION);

homekit_service_t informationService = HOMEKIT_SERVICE_(
  ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    HOMEKIT_CHARACTERISTIC(NAME, INFORMATION_NAME),
    HOMEKIT_CHARACTERISTIC(MANUFACTURER, INFORMATION_MANUFACTURER),
    HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, INFORMATION_SERIAL_NUMBER),
    HOMEKIT_CHARACTERISTIC(MODEL, INFORMATION_MODEL),
    HOMEKIT_CHARACTERISTIC(IDENTIFY, onAccessoryIdentify),
    &versionCharacteristic,
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

homekit_accessory_t *boolAccessories[] = {
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
  .password = SERVER_PASSWORD,
};
