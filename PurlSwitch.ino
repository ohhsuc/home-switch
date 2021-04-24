#include <arduino_homekit_server.h>
#include "PurlWebServer.h"

const String productName = "Purl Switch";
const String hostName = "Purl-Switch-001";
PurlWebServer webServer(80, productName, hostName);

// access your HomeKit characteristics defined
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch;

const int led = LED_BUILTIN;
void ledOn() {
  digitalWrite(led, LOW);
  Serial.println("LED -> ON");
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(led, HIGH);
  Serial.println("LED -> OFF");
}
void switchOnOff(bool isOn) {
  cha_switch.value.bool_value = isOn;
  if (isOn) {
    gpio_output_set(0, BIT0, BIT0, 0);
    Serial.println("SWITCH -> ON");
  } else {
    gpio_output_set(BIT0, 0, BIT0, 0);
    Serial.println("SWITCH -> OFF");
  }
}

homekit_value_t cha_switch_getter() {
  return cha_switch.value;
}

void cha_switch_setter(const homekit_value_t value) {
  ledOn();
  switchOnOff(value.bool_value);
  ledOff();
}

void resetAccessory() {
  homekit_server_reset();
}

void setState(PurlWebServerState state) {
  switchOnOff(state.isSwitchOn);
}
void getState(PurlWebServerState state) {
  bool value = cha_switch.value.bool_value;
  state.isSwitchOn = value;
}

void setup(void) {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  ledOn();

  webServer.onResetAccessory = resetAccessory;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onSetState = setState;
  webServer.onGetState = getState;
  webServer.setup();

  cha_switch.getter = cha_switch_getter;
  cha_switch.setter = cha_switch_setter;
  arduino_homekit_setup(&config);

  Serial.println("Setup complete!");
  ledOff();
}

void loop(void) {
  webServer.loop();
  arduino_homekit_loop();
}
