#include <Arduino.h>
#include <ESP8266mDNS.h>
#include "WebPortal.h"
#include "RadioPortal.h"
#include "Timer.h"
#include "TimesTrigger.h"
#include "Mesher.h"
#include "HomeKitMain.h"

using namespace Victoria;
using namespace Victoria::Events;
using namespace Victoria::Components;
using namespace Victoria::HomeKit;

WebPortal webPortal(80);
RadioPortal radioPortal;
TimesTrigger timesTrigger(10, 5 * 1000);

void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(100); // at least light for some time
}
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void deleteService(const String& serviceId, const ServiceSetting& setting) {
  HomeKitMain::removeService(serviceId);
}

ServiceState getServiceState(const String& serviceId, const ServiceSetting& setting) {
  auto service = HomeKitMain::findServiceById(serviceId);
  if (service) {
    return service->getState();
  }
  return {};
}

void setServiceState(const String& serviceId, const ServiceSetting& setting, ServiceState& state) {
  auto service = HomeKitMain::findServiceById(serviceId);
  if (service) {
    service->setState(state);
  }
}

void setSwitchAction(const String& serviceId, const RadioAction& radioAction) {
  auto service = HomeKitMain::findServiceById(serviceId);
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

void onStateChange(const ServiceState& state) {
  ledOn();
  timesTrigger.count();
  console.log("boolean value " + String(state.boolValue));
  console.log("integer value " + String(state.intValue));
  ledOff();
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error("LittleFS mount failed");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  ledOn();

  webPortal.onDeleteService = deleteService;
  webPortal.onGetServiceState = getServiceState;
  webPortal.onSetServiceState = setServiceState;
  webPortal.onRequestStart = ledOn;
  webPortal.onRequestEnd = ledOff;
  webPortal.onResetAccessory = []() { HomeKitMain::reset(); };
  webPortal.setup();

  radioPortal.onAction = setRadioAction;
  radioPortal.setup();

  timesTrigger.onTimesOut = []() { console.log("times out!"); };
  timer.setInterval(30 * 60 * 1000, []() { HomeKitMain::heartbeat(); });
  timer.setInterval(1 * 60 * 1000, []() {
    if (MDNS.isRunning()) {
      MDNS.announce();
      console.log("MDNS announced");
    }
  });

  auto mesher = Mesher();
  auto loader = RadioFrequencyMeshLoader(10);
  mesher.setLoader(&loader);

  auto model = serviceStorage.load();
  if (model.services.size() > 0) {
    for (const auto& pair : model.services) {
      auto serviceId = pair.first;
      auto serviceSetting = pair.second;
      auto service = HomeKitMain::createService(serviceId, serviceSetting);
      if (service) {
        service->onStateChange = onStateChange;
      }
    }
    auto hostName = webPortal.getHostName(false);
    HomeKitMain::setup(hostName);
  }

  console.log("setup complete");
  ledOff();
}

void loop(void) {
  timer.loop();
  webPortal.loop();
  radioPortal.loop();
  HomeKitMain::loop();
}
