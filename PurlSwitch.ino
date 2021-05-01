#include <arduino_homekit_server.h>
#include "PurlWebServer.h"
#include "TimesTrigger.h"

const String productName = "Purl Switch";
const String hostName = "Purl-Switch-001";
PurlWebServer webServer(80, productName, hostName);
TimesTrigger timesTrigger(10, 5 * 1000);

// access your HomeKit characteristics defined
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch;

// const int led = LED_BUILTIN;
const uint8_t GPIO0 = 0; // GPIO-0
const uint8_t GPIO2 = 2; // GPIO-2 (Led Builtin)
const uint8_t TXD = 1; // TXD (Transmitter)
const uint8_t RXD = 3; // RXD (Receiver)

uint8_t LedPin;
uint8_t RelayPin;
uint8_t InputPin;

void ledOn() {
  digitalWrite(LedPin, LOW);
  Serial.println("LED -> ON");
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LedPin, HIGH);
  Serial.println("LED -> OFF");
}

void setRelay(bool isOn) {
  if (isOn) {
    digitalWrite(RelayPin, LOW);
    Serial.println("SWITCH -> ON");
  } else {
    digitalWrite(RelayPin, HIGH);
    Serial.println("SWITCH -> OFF");
  }
  timesTrigger.count();
}

homekit_value_t cha_switch_getter() {
  return cha_switch.value;
}

void cha_switch_setter(const homekit_value_t value) {
  ledOn();
  cha_switch.value.bool_value = value.bool_value;
  setRelay(value.bool_value);
  ledOff();
}

void resetAccessory() {
  homekit_server_reset();
}

void setState(PurlWebServer::AccessoryState& state) {
  cha_switch.value.bool_value = state.isSwitchOn;
  setRelay(state.isSwitchOn);
}
void getState(PurlWebServer::AccessoryState& state) {
  bool value = cha_switch.value.bool_value;
  state.isSwitchOn = value;
}

void timesOut() {
  Serial.println("times out!");
}

void setup(void) {
  LedPin = GPIO2;
  RelayPin = RXD;
  InputPin = GPIO0;
  Serial.begin(115200);

  pinMode(LedPin, OUTPUT);
  pinMode(RelayPin, OUTPUT);
  pinMode(InputPin, INPUT_PULLUP);
  digitalWrite(RelayPin, HIGH);
  ledOn();

  webServer.onSetState = setState;
  webServer.onGetState = getState;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onResetAccessory = resetAccessory;
  webServer.setup();

  timesTrigger.onTimesOut = timesOut;

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
