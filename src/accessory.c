#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
  printf("accessory identify\n");
}

homekit_characteristic_t switchState = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t switchName = HOMEKIT_CHARACTERISTIC_(NAME, "Switch");

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, VICTOR_ACCESSORY_INFORMATION_NAME),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER),
      HOMEKIT_CHARACTERISTIC(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL,
    }),
    HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &switchState,
      &switchName,
      NULL,
    }),
    NULL,
  }),
  NULL,
};

homekit_server_config_t switchConfig = {
  .accessories = accessories,
  .password = VICTOR_HOMEKIT_SERVER_PASSWORD,
};
