#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void onAccessoryIdentify(homekit_value_t value) {
  printf("accessory identify\n");
}

homekit_characteristic_t accessoryName         = HOMEKIT_CHARACTERISTIC_(NAME, VICTOR_ACCESSORY_INFORMATION_NAME);
homekit_characteristic_t accessoryManufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER);
homekit_characteristic_t accessoryModel        = HOMEKIT_CHARACTERISTIC_(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL);
homekit_characteristic_t accessorySerialNumber = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER);
homekit_characteristic_t accessoryVersion      = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION);
homekit_characteristic_t accessoryIdentify     = HOMEKIT_CHARACTERISTIC_(IDENTIFY, onAccessoryIdentify);

homekit_service_t informationService = HOMEKIT_SERVICE_(
  ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &accessoryName,
    &accessoryManufacturer,
    &accessoryModel,
    &accessorySerialNumber,
    &accessoryVersion,
    &accessoryIdentify,
    NULL,
  },
);

homekit_characteristic_t switchState = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t switchName  = HOMEKIT_CHARACTERISTIC_(NAME, VICTOR_ACCESSORY_INFORMATION_NAME);

homekit_service_t stateService = HOMEKIT_SERVICE_(
  SWITCH,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &switchState,
    &switchName,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_switch,
    .services = (homekit_service_t*[]) {
      &informationService,
      &stateService,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t serverConfig = {
  .accessories = accessories,
  .password = VICTOR_HOMEKIT_SERVER_PASSWORD,
};
