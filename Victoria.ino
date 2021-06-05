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

auto timer = Timer();
auto timesTrigger = TimesTrigger(10, 5 * 1000);
auto webServer = WebServer(80, productName, hostName);
ConfigStore* configStore;
ButtonEvents* inputEvents;
OnOffEvents* onOffEvents;
BooleanAccessory* booleanAccessory;

void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("led -> ON");
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("led -> OFF");
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
}

AccessoryState getState(const String& accessoryId, const AccessorySetting& setting) {
  BaseAccessory* accessory = BaseAccessory::findAccessoryById(accessoryId);
  if (accessory) {
    return accessory->getState();
  }
}
void setState(const String& accessoryId, const AccessorySetting& setting, AccessoryState& state) {
  BaseAccessory* accessory = BaseAccessory::findAccessoryById(accessoryId);
  if (accessory) {
    accessory->setState(state);
  }
}

void timesOut() {
  Serial.println("times out!");
}

void buttonClick(const String& accessoryId, int times) {
  if (times == 1 && booleanAccessory) {
    AccessoryState state = booleanAccessory->getState();
    state.boolValue = !state.boolValue;
    booleanAccessory->setState(state);
  }
}

void heartbeat() {
  if (booleanAccessory) {
    booleanAccessory->heartbeat();
  }
}

void inputToggle(const String& accessoryId, bool isOn) {
  Serial.print("toggle ");
  Serial.println(isOn ? "ON" : "OFF");
}

void onAccessoryChange(const AccessoryState& state) {
  ledOn();
  timesTrigger.count();
  Serial.println("boolean value " + String(state.boolValue));
  Serial.println("integer value " + String(state.intValue));
  ledOff();
}

void resetAccessory() {
  if (booleanAccessory) {
    booleanAccessory->reset();
  }
}

void setup(void) {
  Serial.begin(115200);
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
    if (outputPin > 0) {
      pinMode(outputPin, OUTPUT);
      if (setting.outputLevel > 0) {
        digitalWrite(outputPin, setting.outputLevel);
      }
      if (booleanAccessory) {
        delete booleanAccessory;
        booleanAccessory = NULL;
      }
      if (setting.type == BooleanAccessoryType) {
        booleanAccessory = new BooleanAccessory(id, outputPin);
        booleanAccessory->onChange = onAccessoryChange;
      } else if(setting.type == IntegerAccessoryType) {
        //TODO:
      }
    }
    // inputs
    auto inputPin = setting.inputIO;
    if (inputPin > 0) {
      pinMode(inputPin, INPUT_PULLUP);
      if (setting.inputLevel > 0) {
        digitalWrite(inputPin, setting.inputLevel);
      }
      if (inputEvents) {
        delete inputEvents;
        inputEvents = NULL;
      }
      inputEvents = new ButtonEvents(id, inputPin);
      inputEvents->onClick = buttonClick;
      if (onOffEvents) {
        delete onOffEvents;
        onOffEvents = NULL;
      }
      onOffEvents = new OnOffEvents(id, inputPin);
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

  timesTrigger.onTimesOut = timesOut;
  timer.setInterval(60 * 1000, heartbeat);

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  Serial.println("Setup complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  webServer.loop();
  if (booleanAccessory) {
    booleanAccessory->loop();
  }
  if (inputEvents) {
    inputEvents->loop();
  }
  if (onOffEvents) {
    onOffEvents->loop();
  }
}
