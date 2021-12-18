#include <Arduino.h>
#include <ESP8266mDNS.h>

#include "BuiltinLed.h"
#include "VictorOTA.h"
#include "VictorWifi.h"
#include "VictorRadio.h"
#include "TimesTrigger.h"

#include "WebPortal.h"
#include "HomeKit/HomeKitMain.h"

using namespace Victor;
using namespace Victor::Events;
using namespace Victor::Components;
using namespace Victor::HomeKit;

BuiltinLed* builtinLed;
VictorRadio radioPortal;
WebPortal webPortal(80);
TimesTrigger timesTrigger(10, 5 * 1000);
HomeKitMain homeKitMain;

void deleteService(const String& serviceId, const ServiceSetting& setting) {
  homeKitMain.removeService(serviceId);
}

ServiceState getServiceState(const String& serviceId, const ServiceSetting& setting) {
  auto service = homeKitMain.findServiceById(serviceId);
  if (service) {
    return service->getState();
  }
  return {};
}

void setServiceState(const String& serviceId, const ServiceSetting& setting, ServiceState& state) {
  auto service = homeKitMain.findServiceById(serviceId);
  if (service) {
    service->setState(state);
  }
}

void setSwitchAction(const String& serviceId, const int& action) {
  auto service = homeKitMain.findServiceById(serviceId);
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

bool setRadioAction(const RadioRule& rule) {
  if (rule.serviceId) {
    int action = rule.action == RadioActionFalse ? 0
      : rule.action == RadioActionTrue ? 1
      : rule.action == RadioActionToggle ? 2 : -1;
    setSwitchAction(rule.serviceId, action);
    return true;
  }
  return false;
}

bool setRadioCommand(const RadioCommandParsed& command) {
  if (command.serviceId && command.entry == EntryBoolean) {
    int action = command.action == EntryBooleanToggle ? 2
      : (command.action == EntryBooleanSet && command.parameters == "true") ? 1
      : (command.action == EntryBooleanSet && command.parameters == "false") ? 0 : -1;
    setSwitchAction(command.serviceId, action);
    return true;
  }
  return false;
}

void onStateChange(const ServiceState& state) {
  builtinLed->flash();
  timesTrigger.count();
  console.log().type(F("Service"))
    .write(F(" boolean:")).write(String(state.boolValue))
    .write(F(" integer:")).write(String(state.intValue))
    .newline();
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error(F("[LittleFS] mount failed"));
  }

  builtinLed = new BuiltinLed();
  builtinLed->turnOn();

  victorOTA.setup();
  victorWifi.setup();

  radioPortal.onAction = setRadioAction;
  radioPortal.onCommand = setRadioCommand;
  radioPortal.onEmit = [](const RadioEmit& emit) {
    builtinLed->flash();
    // emit via your radio tool
    console.log().type(F("Radio"))
      .write(F(" sent [")).write(emit.value).write(F("]"))
      .write(F(" via channel [")).write(String(emit.channel)).write(F("]")).newline();
  };

  webPortal.onDeleteService = deleteService;
  webPortal.onGetServiceState = getServiceState;
  webPortal.onSetServiceState = setServiceState;
  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onResetAccessory = []() { homeKitMain.reset(); };
  webPortal.onCountClients = []() { return homeKitMain.countClients(); };
  webPortal.setup();

  timesTrigger.onTimesOut = []() { console.log(F("times out!")); };

  homeKitMain.clear();
  auto model = serviceStorage.load();
  if (model.services.size() > 0) {
    for (const auto& pair : model.services) {
      auto serviceId = pair.first;
      auto serviceSetting = pair.second;
      auto service = homeKitMain.createService(serviceId, serviceSetting);
      if (service) {
        service->onStateChange = onStateChange;
      }
    }
    auto hostName = victorWifi.getLocalHostName();
    homeKitMain.setup(hostName);
  }

  console.log(F("[setup] complete"));
  builtinLed->flash();
}

void loop(void) {
  webPortal.loop();
  homeKitMain.loop();
  // receive from your radio tool
  if (false) {
    String value = "";
    int channel = 1;
    radioPortal.receive(value, channel);
    console.log().type(F("Radio"))
      .write(F(" received [")).write(value).write(F("]"))
      .write(F(" from channel [")).write(String(channel)).write(F("]")).newline();
    builtinLed->flash();
  }
}
