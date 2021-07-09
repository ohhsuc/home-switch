#include <map>
#include <Arduino.h>
#include <ESP8266mDNS.h>
#include "WebPortal.h"
#include "RadioPortal.h"
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
TimesTrigger timesTrigger(10, 5 * 1000);
WebPortal webPortal(80);
RadioPortal radioPortal;
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
  return {};
}

void setServiceState(const String& serviceId, const ServiceSetting& setting, ServiceState& state) {
  auto service = HomeKitService::findServiceById(serviceId);
  if (service) {
    service->setState(state);
  }
}

void setSwitchAction(const String& serviceId, const RadioAction& radioAction) {
  auto service = HomeKitService::findServiceById(serviceId);
  if (service) {
    auto state = service->getState();
    switch (radioAction) {
      case RadioActionToggle:
        state.boolValue = !state.boolValue;
        break;
      case RadioActionTrue:
        state.boolValue = true;
        break;
      case RadioActionFalse:
        state.boolValue = false;
        break;
      default:
        break;
    }
    service->setState(state);
  }
}

void setRadioAction(const RadioRule& rule) {
  if (rule.serviceId) {
    setSwitchAction(rule.serviceId, rule.action);
  }
}

void onButtonClick(const String& serviceId, int times) {
  if (times == 1) {
    setSwitchAction(serviceId, RadioActionToggle);
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

  webPortal.onDeleteService = deleteService;
  webPortal.onGetServiceState = getServiceState;
  webPortal.onSetServiceState = setServiceState;
  webPortal.onRequestStart = ledOn;
  webPortal.onRequestEnd = ledOff;
  webPortal.onResetAccessory = []() { HomeKitAccessory::reset(); };
  webPortal.setup();

  radioPortal.onAction = setRadioAction;
  radioPortal.setup();

  timesTrigger.onTimesOut = []() { console.log("times out!"); };
  timer.setInterval(10 * 60 * 1000, []() { MDNS.announce(); });
  timer.setInterval(30 * 60 * 1000, []() { HomeKitService::heartbeat(); });

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  auto model = serviceStorage.load();
  if (model.services.size() > 0) {
    auto pair = model.services.begin();
    auto serviceId = pair->first;
    auto service = pair->second;
    // outputs
    auto outputPin = service.outputPin;
    if (outputPin > -1) {
      pinMode(outputPin, OUTPUT);
      if (service.outputLevel > -1) {
        digitalWrite(outputPin, service.outputLevel);
      }
      if (service.type == BooleanServiceType) {
        auto booleanService = new BooleanHomeKitService(serviceId, outputPin);
        booleanService->onStateChange = onStateChange;
      } else if (service.type == IntegerServiceType) {
        // TODO:
      }
    }
    // inputs
    auto inputPin = service.inputPin;
    if (inputPin > -1) {
      pinMode(inputPin, INPUT_PULLUP);
      if (service.inputLevel > -1) {
        digitalWrite(inputPin, service.inputLevel);
      }
      inputEvents = new ButtonEvents(serviceId, inputPin);
      inputEvents->onClick = onButtonClick;
      onOffEvents = new OnOffEvents(serviceId, inputPin);
      onOffEvents->onToggle = onToggle;
    }
    // setup
    HomeKitAccessory::setup(webPortal.getHostName(false));
  }

  console.log("Setup Complete!");
  ledOff();
}

void loop(void) {
  timer.loop();
  radioPortal.loop();
  webPortal.loop();
  HomeKitAccessory::loop();
  if (inputEvents) {
    inputEvents->loop();
  }
  if (onOffEvents) {
    onOffEvents->loop();
  }
}
