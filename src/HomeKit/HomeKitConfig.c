#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void onAccessoryIdentify(homekit_value_t value) {
  printf("accessory identify\n");
}

homekit_characteristic_t accessoryName         = HOMEKIT_CHARACTERISTIC_(NAME, "Victor-AB12CD-21210");
homekit_characteristic_t accessoryManufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, "VictorSmart Co.,Ltd.");
homekit_characteristic_t accessoryModel        = HOMEKIT_CHARACTERISTIC_(MODEL, "ESP8266/ESP32");
homekit_characteristic_t accessorySerialNumber = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, "0123456");
homekit_characteristic_t accessoryVersion      = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, "21.2.10");

homekit_service_t informationService = HOMEKIT_SERVICE_(
  ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &accessoryName,
    &accessoryManufacturer,
    &accessoryModel,
    &accessorySerialNumber,
    &accessoryVersion,
    HOMEKIT_CHARACTERISTIC(IDENTIFY, onAccessoryIdentify),
    NULL,
  },
);

homekit_characteristic_t boolCharacteristic = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t boolNameCharacteristic = HOMEKIT_CHARACTERISTIC_(NAME, "VictorBool");

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
  .password = "111-11-111",
};
