#include <map>
#include <Arduino.h>
#include "Commons.h"
#include "ConfigStore.h"
#include "WebServer.h"
#include "Timer.h"
#include "TimesTrigger.h"
#include "ButtonEvents.h"
#include "OnOffEvents.h"
#include "Mesher.h"
#include "BaseAccessory.h"
#include "BooleanAccessory.h"

using namespace Victoria;
using namespace Victoria::Events;
using namespace Victoria::Components;

const String productName = "Victoria";
const String hostName = "Victoria-91002";
const String firmwareVersion = BaseAccessory::getVersion();

Timer timer;
TimesTrigger timesTrigger(10, 5 * 1000);
WebServer webServer(80, productName, hostName, firmwareVersion);
ConfigStore* configStore;
ButtonEvents* inputEvents;
OnOffEvents* onOffEvents;

void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}

std::map<String, AccessorySetting> loadSettings() {
  auto model = configStore->load();
  return model.settings;
}
void saveSetting(const String& accessoryId, const AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings[accessoryId] = setting;
  configStore->save(model);
}
void deleteSetting(const String& accessoryId, const AccessorySetting& setting) {
  auto model = configStore->load();
  model.settings.erase(accessoryId);
  configStore->save(model);
  // auto accessory = BaseAccessory::findAccessoryById(accessoryId);
  // if (accessory) {
  //   delete accessory;
  //   accessory = NULL;
  // }
}

AccessoryState getState(const String& accessoryId, const AccessorySetting& setting) {
  auto accessory = BaseAccessory::findAccessoryById(accessoryId);
  if (accessory) {
    return accessory->getState();
  }
}
void setState(const String& accessoryId, const AccessorySetting& setting, AccessoryState& state) {
  auto accessory = BaseAccessory::findAccessoryById(accessoryId);
  if (accessory) {
    accessory->setState(state);
  }
}

void timesOut() {
  console.log("times out!");
}

void onButtonClick(const String& accessoryId, int times) {
  if (times == 1) {
    auto accessory = BaseAccessory::findAccessoryById(accessoryId);
    if (accessory) {
      AccessoryState state = accessory->getState();
      state.boolValue = !state.boolValue;
      accessory->setState(state);
    }
  }
}

void onToggle(const String& accessoryId, bool isOn) {
  console.log("toggle " + String(isOn ? "ON" : "OFF"));
}

void onStateChange(const AccessoryState& state) {
  ledOn();
  timesTrigger.count();
  console.log("boolean value " + String(state.boolValue));
  console.log("integer value " + String(state.intValue));
  ledOff();
}

void setup(void) {
  console.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  ledOn();

  configStore = new ConfigStore();
  auto settings = loadSettings();
  if (settings.size() > 0) {
    auto pair = settings.begin();
    auto id = pair->first;
    auto setting = pair->second;
    // outputs
    auto outputPin = setting.outputIO;
    if (outputPin > -1) {
      pinMode(outputPin, OUTPUT);
      if (setting.outputLevel > -1) {
        digitalWrite(outputPin, setting.outputLevel);
      }
      if (setting.type == BooleanAccessoryType) {
        auto booleanAccessory = new BooleanAccessory(id, outputPin);
        booleanAccessory->onStateChange = onStateChange;
      } else if (setting.type == IntegerAccessoryType) {
        //TODO:
      }
    }
    // inputs
    auto inputPin = setting.inputIO;
    if (inputPin > -1) {
      pinMode(inputPin, INPUT_PULLUP);
      if (setting.inputLevel > -1) {
        digitalWrite(inputPin, setting.inputLevel);
      }
      if (inputEvents) {
        delete inputEvents;
        inputEvents = NULL;
      }
      inputEvents = new ButtonEvents(id, inputPin);
      inputEvents->onClick = onButtonClick;
      onOffEvents = new OnOffEvents(id, inputPin);
      onOffEvents->onToggle = onToggle;
    }
  }

  webServer.onLoadSettings = loadSettings;
  webServer.onSaveSetting = saveSetting;
  webServer.onDeleteSetting = deleteSetting;
  webServer.onGetState = getState;
  webServer.onSetState = setState;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onResetAccessory = [](){ BaseAccessory::resetAll(); };
  webServer.setup();

  timesTrigger.onTimesOut = timesOut;
  timer.setInterval(60 * 1000, [](){ BaseAccessory::heartbeatAll(); });

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  console.log("Firmware Version: " + firmwareVersion);
  console.log("Setup Complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  webServer.loop();
  BaseAccessory::loopAll();
  if (inputEvents) {
    inputEvents->loop();
  }
  if (onOffEvents) {
    onOffEvents->loop();
  }
}
