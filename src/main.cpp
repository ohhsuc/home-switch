#include <Arduino.h>
#include <ESP8266mDNS.h>
#include "BuiltinLed.h"
#include "VictoriaOTA.h"
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

BuiltinLed* builtinLed;
WebPortal webPortal(80);
RadioPortal radioPortal;
TimesTrigger timesTrigger(10, 5 * 1000);

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

void setSwitchAction(const String& serviceId, const int& action) {
  auto service = HomeKitMain::findServiceById(serviceId);
  if (service) {
    auto state = service->getState();
    switch (action) {
      case 0: {
        state.boolValue = false;
        break;
      }
      case 1: {
        state.boolValue = true;
        break;
      }
      case 2: {
        state.boolValue = !state.boolValue;
        break;
      }
      default: {
        break;
      }
    }
    service->setState(state);
  }
}

void setRadioAction(const RadioRule& rule) {
  if (rule.serviceId) {
    int action = rule.action == RadioActionFalse ? 0
      : rule.action == RadioActionTrue ? 1
      : rule.action == RadioActionToggle ? 2 : -1;
    setSwitchAction(rule.serviceId, action);
  }
}

void setRadioCommand(const RadioCommandParsed& command) {
  if (command.serviceId && command.entry == EntryBoolean && command.action == EntryBooleanSet) {
    int action = command.parameters == "false" ? 0
      : command.parameters == "true" ? 1
      : command.parameters == "toggle" ? 2 : -1;
    setSwitchAction(command.serviceId, action);
  }
}

void onStateChange(const ServiceState& state) {
  builtinLed->flash();
  timesTrigger.count();
  console.log("boolean value " + String(state.boolValue));
  console.log("integer value " + String(state.intValue));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error("LittleFS mount failed");
  }

  builtinLed = new BuiltinLed();
  builtinLed->turnOn();

  webPortal.onDeleteService = deleteService;
  webPortal.onGetServiceState = getServiceState;
  webPortal.onSetServiceState = setServiceState;
  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onResetAccessory = []() { HomeKitMain::reset(); };
  webPortal.setup();

  radioPortal.onMessage = [](const RadioMessage& message) {
    console.log("[Radio] > received [" + message.id + "!" + message.value + "] from channel [" + String(message.channel) + "]");
    builtinLed->flash();
  };
  radioPortal.onAction = setRadioAction;
  radioPortal.onCommand = setRadioCommand;
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

  VictoriaOTA::setup();

  console.log("setup complete");
  builtinLed->turnOff();
}

void loop(void) {
  timer.loop();
  webPortal.loop();
  radioPortal.loop();
  HomeKitMain::loop();
}
