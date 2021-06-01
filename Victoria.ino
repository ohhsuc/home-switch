#include <map>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "ConfigStore.h"
#include "WebServer.h"
#include "Timer.h"
#include "TimesTrigger.h"
#include "ButtonEvents.h"
#include "OnOffEvents.h"
#include "Mesher.h"
#include "BooleanAccessory.h"

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
BooleanAccessory* booleanAccessory;

// access your HomeKit characteristics defined
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch;

void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("led -> ON");
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("led -> OFF");
}

void homekitNotify() {
  homekit_characteristic_notify(&cha_switch, cha_switch.value);
}

void setAccessory(bool value) {
  ledOn();
  cha_switch.value.bool_value = value;
  homekitNotify();
  if (booleanAccessory) {
    booleanAccessory->setValue(value);
  }
  timesTrigger.count();
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
void saveSetting(const String& id, const AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings[id] = setting;
  configStore->save(model);
}
void deleteSetting(const String& id, const AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings.erase(id);
  configStore->save(model);
}

void getState(const String& id, const AccessorySetting& setting, AccessoryState& state) {
  state.boolValue = cha_switch.value.bool_value;
}
void setState(const String& id, const AccessorySetting& setting, AccessoryState& state) {
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
  pinMode(LED_BUILTIN, OUTPUT);
  ledOn();

  configStore = new ConfigStore();
  auto settings = loadSettings();
  if (settings.size() > 0) {
    auto setting = settings.begin()->second;
    // outputs
    if (setting.outputIO > 0) {
      auto outputPin = setting.outputIO;
      pinMode(outputPin, OUTPUT);
      if (setting.outputLevel > 0) {
        digitalWrite(outputPin, setting.outputLevel);
      }
      booleanAccessory = NULL;
      if (setting.type == BooleanAccessoryType) {
        booleanAccessory = new BooleanAccessory(setting.outputIO);
      } else if(setting.type == IntegerAccessoryType) {
        //TODO:
      }
    }
    // inputs
    if (setting.inputIO > 0) {
      auto inputPin = setting.inputIO;
      pinMode(inputPin, INPUT_PULLUP);
      if (setting.inputLevel > 0) {
        digitalWrite(inputPin, setting.inputLevel);
      }
      inputEvents = new ButtonEvents(inputPin);
      inputEvents->onClick = buttonClick;
      onOffEvents = new OnOffEvents(inputPin);
      onOffEvents->onToggle = inputToggle;
    }
  }

  webServer.onLoadSettings = loadSettings;
  webServer.onSaveSetting = saveSetting;
  webServer.onDeleteSetting = deleteSetting;
  webServer.onGetState = getState;
  webServer.onSetState = setState;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onResetAccessory = resetAccessory;
  webServer.setup();

  cha_switch.getter = cha_switch_getter;
  cha_switch.setter = cha_switch_setter;
  arduino_homekit_setup(&config);

  timesTrigger.onTimesOut = timesOut;
  timer.setInterval(10 * 1000, homekitNotify); // heatbeat

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  Serial.println("Setup complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  webServer.loop();
  arduino_homekit_loop();
  if (inputEvents) {
    inputEvents->loop();
  }
  if (onOffEvents) {
    onOffEvents->loop();
  }
}
