#include <map>
#include <arduino_homekit_server.h>
#include "WebServer.h"
#include "Timer.h"
#include "TimesTrigger.h"
#include "ButtonEvents.h"
#include "OnOffEvents.h"
#include "ConfigStore.h"

using namespace Victoria;
using namespace Victoria::Events;
using namespace Victoria::Components;

const String productName = "Victoria";
const String hostName = "Victoria-91002";

auto timer = Timer();
auto timesTrigger = TimesTrigger(10, 5 * 1000);
auto webServer = WebServer(80, productName, hostName);
ConfigStore* configStore;
ButtonEvents* inputEvents;
OnOffEvents* onOffEvents;

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
  Serial.println("led -> ON");
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LedPin, HIGH);
  Serial.println("led -> OFF");
}

void setRelay(bool isOn) {
  if (isOn) {
    digitalWrite(RelayPin, LOW);
    Serial.println("relay -> LOW");
  } else {
    digitalWrite(RelayPin, HIGH);
    Serial.println("relay -> HIGH");
  }
  timesTrigger.count();
}

void homekitNotify() {
  homekit_characteristic_notify(&cha_switch, cha_switch.value);
}

void setAccessory(bool value) {
  ledOn();
  cha_switch.value.bool_value = value;
  homekitNotify();
  setRelay(value);
  ledOff();
}

homekit_value_t cha_switch_getter() {
  return cha_switch.value;
  // return (HOMEKIT_BOOL(cha_switch.value.bool_value));
}

void cha_switch_setter(const homekit_value_t value) {
  setAccessory(value.bool_value);
}

void resetAccessory() {
  homekit_server_reset();
}

std::map<String, AccessorySetting> loadSettings() {
  auto model = configStore->load();
  return model.settings;
}
void saveSetting(String id, AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings[id] = setting;
  configStore->save(model);
}
void deleteSetting(String id, AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings.erase(id);
  configStore->save(model);
}

void getState(String id, AccessorySetting& setting, AccessoryState& state) {
  state.boolValue = cha_switch.value.bool_value;
}
void setState(String id, AccessorySetting& setting, AccessoryState& state) {
  setAccessory(state.boolValue);
}

void timesOut() {
  Serial.println("times out!");
}

void buttonClick(int times) {
  if (times == 1) {
    bool value = !cha_switch.value.bool_value;
    setAccessory(value);
  }
}

void inputToggle(bool isOn) {
  Serial.print("toggle ");
  Serial.println(isOn ? "ON" : "OFF");
}

void setup(void) {
  Serial.begin(115200);
  configStore = new ConfigStore();
  auto settings = loadSettings();
  auto setting = settings.begin()->second;

  LedPin = GPIO2;
  pinMode(LedPin, OUTPUT);
  ledOn();

  InputPin = setting.inputIO;
  RelayPin = setting.outputIO;
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, HIGH);

  webServer.onLoadSettings = loadSettings;
  webServer.onSaveSetting = saveSetting;
  webServer.onDeleteSetting = deleteSetting;
  webServer.onGetState = getState;
  webServer.onSetState = setState;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onResetAccessory = resetAccessory;
  webServer.setup();

  timesTrigger.onTimesOut = timesOut;

  inputEvents = new ButtonEvents(InputPin);
  inputEvents->onClick = buttonClick;

  onOffEvents = new OnOffEvents(InputPin);
  onOffEvents->onToggle = inputToggle;

  cha_switch.getter = cha_switch_getter;
  cha_switch.setter = cha_switch_setter;
  arduino_homekit_setup(&config);
  timer.setInterval(10 * 1000, homekitNotify); // heatbeat

  Serial.println("Setup complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  webServer.loop();
  inputEvents->loop();
  onOffEvents->loop();
  arduino_homekit_loop();
}
