#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void onAccessoryIdentify(homekit_value_t _value) {
  printf("accessory identify\n");
}

// format: bool; HAP section 9.70; write the .setter function to get the switch-event sent from iOS Home APP.
homekit_characteristic_t homekitBoolCha = HOMEKIT_CHARACTERISTIC_(ON, false);

// format: string; HAP section 9.62; max length 64
homekit_characteristic_t homekitNameCha = HOMEKIT_CHARACTERISTIC_(NAME, "VictoriaSwitch");

homekit_service_t accessoryInformation = HOMEKIT_SERVICE_(
  ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    HOMEKIT_CHARACTERISTIC(NAME, "VictoriaSwitch"),
    HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Victoria 9# Inc."),
    HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
    HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
    HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
    HOMEKIT_CHARACTERISTIC(IDENTIFY, onAccessoryIdentify),
    NULL,
  },
);

homekit_service_t accessorySwitch = HOMEKIT_SERVICE_(
  SWITCH,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &homekitBoolCha,
    &homekitNameCha,
    NULL,
  },
);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_switch,
    .services = (homekit_service_t*[]) {
      &accessoryInformation,
      &accessorySwitch,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t homekitConfig = {
  .accessories = accessories,
  .password = "111-11-111",
};
