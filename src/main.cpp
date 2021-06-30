#include <map>
#include <Arduino.h>
#include <ESP8266mDNS.h>
#include "Commons.h"
#include "ConfigStore.h"
#include "WebServer.h"
#include "Timer.h"
#include "TimesTrigger.h"
#include "ButtonEvents.h"
#include "OnOffEvents.h"
#include "Mesher.h"
#include "HomeKitAccessory.h"
#include "HomeKitService.h"
#include "BooleanHomeKitService.h"

using namespace Victoria;
using namespace Victoria::Events;
using namespace Victoria::Components;
using namespace Victoria::HomeKit;

Timer timer;
ConfigStore configStore;
TimesTrigger timesTrigger(10, 5 * 1000);
WebServer webServer(&configStore, 80);
ButtonEvents* inputEvents;
OnOffEvents* onOffEvents;

void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void deleteService(const String& serviceId, const ServiceSetting& setting) {
  // auto service = HomeKitService::findServiceById(serviceId);
  // if (service) {
  //   delete service;
  //   service = NULL;
  // }
}

ServiceState getServiceState(const String& serviceId, const ServiceSetting& setting) {
  auto service = HomeKitService::findServiceById(serviceId);
  if (service) {
    return service->getState();
  }
}

void setServiceState(const String& serviceId, const ServiceSetting& setting, ServiceState& state) {
  auto service = HomeKitService::findServiceById(serviceId);
  if (service) {
    service->setState(state);
  }
}

void onButtonClick(const String& serviceId, int times) {
  if (times == 1) {
    auto service = HomeKitService::findServiceById(serviceId);
    if (service) {
      ServiceState state = service->getState();
      state.boolValue = !state.boolValue;
      service->setState(state);
    }
  }
}

void onToggle(const String& serviceId, bool isOn) {
  console.log("toggle " + String(isOn ? "ON" : "OFF"));
}

void onStateChange(const ServiceState& state) {
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

  webServer.onDeleteService = deleteService;
  webServer.onGetServiceState = getServiceState;
  webServer.onSetServiceState = setServiceState;
  webServer.onRequestStart = ledOn;
  webServer.onRequestEnd = ledOff;
  webServer.onResetAccessory = []() { HomeKitAccessory::reset(); };
  webServer.setup();

  timesTrigger.onTimesOut = []() { console.log("times out!"); };
  timer.setInterval(10 * 60 * 1000, []() { MDNS.announce(); });
  timer.setInterval(30 * 60 * 1000, []() { HomeKitService::heartbeat(); });

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  auto model = configStore.load();
  if (model.services.size() > 0) {
    auto pair = model.services.begin();
    auto id = pair->first;
    auto setting = pair->second;
    // outputs
    auto outputPin = setting.outputIO;
    if (outputPin > -1) {
      pinMode(outputPin, OUTPUT);
      if (setting.outputLevel > -1) {
        digitalWrite(outputPin, setting.outputLevel);
      }
      if (setting.type == BooleanServiceType) {
        auto booleanService = new BooleanHomeKitService(id, outputPin);
        booleanService->onStateChange = onStateChange;
      } else if (setting.type == IntegerServiceType) {
        // TODO:
      }
    }
    // inputs
    auto inputPin = setting.inputIO;
    if (inputPin > -1) {
      pinMode(inputPin, INPUT_PULLUP);
      if (setting.inputLevel > -1) {
        digitalWrite(inputPin, setting.inputLevel);
      }
      inputEvents = new ButtonEvents(id, inputPin);
      inputEvents->onClick = onButtonClick;
      onOffEvents = new OnOffEvents(id, inputPin);
      onOffEvents->onToggle = onToggle;
    }
    // setup
    HomeKitAccessory::setup(webServer.getHostName(false));
  }

  console.log("Setup Complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  webServer.loop();
  HomeKitAccessory::loop();
  if (inputEvents) {
    inputEvents->loop();
  }
  if (onOffEvents) {
    onOffEvents->loop();
  }
}
