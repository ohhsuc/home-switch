#include <Arduino.h>
#include <RH_ASK.h>

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

RH_ASK* ask;
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
  console.log().bracket(F("Service"))
    .section(F("boolean")).section(String(state.boolValue))
    .section(F("integer")).section(String(state.intValue));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error(F("fs mount failed"));
  }

  builtinLed = new BuiltinLed();
  builtinLed->turnOn();

  victorOTA.setup();
  victorWifi.setup();

  auto radioJson = radioStorage.load();
  ask = new RH_ASK(2000, radioJson.inputPin, radioJson.outputPin, 0);
  if (!ask->init()) {
    console.error(F("RH_ASK init failed"));
  }

  radioPortal.onAction = setRadioAction;
  radioPortal.onCommand = setRadioCommand;
  radioPortal.onEmit = [](const RadioEmit& emit) {
    const char* payload = emit.value.c_str();
    ask->send((uint8_t *)payload, strlen(payload));
    ask->waitPacketSent();
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("sent")).section(emit.value)
      .section(F("via channel")).section(String(emit.channel));
  };

  webPortal.onDeleteService = deleteService;
  webPortal.onGetServiceState = getServiceState;
  webPortal.onSetServiceState = setServiceState;
  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onResetAccessory = []() { homeKitMain.reset(); };
  webPortal.onCountClients = []() { return homeKitMain.countClients(); };
  webPortal.onRadioEmit = [](int index) { radioPortal.emit(index); };
  webPortal.setup();

  homeKitMain.clear();
  auto serviceJson = serviceStorage.load();
  if (serviceJson.services.size() > 0) {
    for (const auto& pair : serviceJson.services) {
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

  timesTrigger.onTimesOut = []() {
    console.log(F("times out!"));
  };

  builtinLed->flash();
  console.log(F("setup complete"));
}

void loop(void) {
  webPortal.loop();
  homeKitMain.loop();
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  if (ask->recv(buf, &buflen)) {
    auto value = String((char*)buf);
    value = value.substring(0, buflen);
    auto channel = 1;
    radioPortal.receive(value, channel);
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("received")).section(value)
      .section(F("from channel")).section(String(channel));
  }
}
