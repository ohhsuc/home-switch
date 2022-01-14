#include <Arduino.h>
#include <RH_ASK.h>
#include <arduino_homekit_server.h>

#include <Console.h>
#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorRadio.h>
#include <VictorWeb.h>

#include "TimesCounter.h"
#include "SwitchStorage.h"
#include "SwitchIO.h"

using namespace Victor;
using namespace Victor::Events;
using namespace Victor::Components;

RH_ASK* ask;
VictorRadio radioPortal;
VictorWeb webPortal(80);
TimesCounter times(10, 5 * 1000);
SwitchIO* switchIO;
String hostName;

extern "C" homekit_characteristic_t switchState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_server_config_t serverConfig;

String parseStateName(bool state) {
  return state ? F("On") : F("Off");
}

String parseYesNo(bool state) {
  return state ? F("Yes") : F("No");
}

void switchStateSetter(const homekit_value_t value) {
  builtinLed.flash();
  times.count();
  switchState.value.bool_value = value.bool_value;
  switchIO->outputState(value.bool_value);
  console.log()
    .bracket(F("switch"))
    .section(F("state"), parseStateName(value.bool_value));
}

void setSwitchState(const bool value) {
  switchState.value.bool_value = value;
  homekit_characteristic_notify(&switchState, switchState.value);
  switchIO->outputState(value);
  console.log()
    .bracket(F("switch"))
    .section(F("state"), parseStateName(value));
}

void setSwitchAction(const int& action) {
  const bool value = action == 0 ? false
    : action == 1 ? true
    : action == 2 ? !switchState.value.bool_value
    : switchState.value.bool_value;
  setSwitchState(value);
}

bool setRadioAction(const RadioRule& rule) {
  const uint8_t action = rule.action == RadioActionFalse ? 0
    : rule.action == RadioActionTrue ? 1
    : rule.action == RadioActionToggle ? 2 : -1;
  setSwitchAction(action);
  return true;
}

bool setRadioCommand(const RadioCommandParsed& command) {
  if (command.entry == EntryBoolean) {
    const uint8_t action = command.action == EntryBooleanToggle ? 2
      : (command.action == EntryBooleanSet && command.parameters == F("true")) ? 1
      : (command.action == EntryBooleanSet && command.parameters == F("false")) ? 0 : -1;
    setSwitchAction(action);
    return true;
  }
  return false;
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  builtinLed.setup();
  builtinLed.turnOn();

  // setup radio
  const auto radioJson = radioStorage.load();
  ask = new RH_ASK(2000, radioJson.inputPin, radioJson.outputPin, 0);
  if (!ask->init()) {
    console.error()
      .bracket(F("radio"))
      .section(F("init failed"));
  }
  radioPortal.onAction = setRadioAction;
  radioPortal.onCommand = setRadioCommand;
  radioPortal.onEmit = [](const RadioEmit& emit) {
    auto value = emit.name + F("!") + emit.value;
    const char* payload = value.c_str();
    ask->send((uint8_t *)payload, strlen(payload));
    ask->waitPacketSent();
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("sent"), value)
      .section(F("via channel"), String(emit.channel));
  };

  // setup web
  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.onRadioEmit = [](const uint8_t index) { radioPortal.emit(index); };
  webPortal.onServiceGet = [](std::vector<KeyValueModel>& items) {
    items.push_back({ .key = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    items.push_back({ .key = F("State"),   .value = parseStateName(switchState.value.bool_value) });
    items.push_back({ .key = F("Paired"),  .value = parseYesNo(homekit_is_paired()) });
    items.push_back({ .key = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
  };
  webPortal.onServicePost = [](const String type) {
    if (type == F("reset")) {
      homekit_server_reset();
    }
  };
  webPortal.setup();

  // setup switch io
  const auto switchJson = switchStorage.load();
  switchIO = new SwitchIO(switchJson);
  switchIO->onStateChange = setSwitchState;

  // setup homekit server
  hostName = victorWifi.getHostName();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  switchState.setter = switchStateSetter;
  arduino_homekit_setup(&serverConfig);

  // counter
  times.onOut = []() {
    console.log(F("times out!"));
  };

  // setup wifi
  victorOTA.setup();
  victorWifi.setup();

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  webPortal.loop();
  switchIO->loop();
  arduino_homekit_loop();
  // loop radio
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  if (ask->recv(buf, &buflen)) {
    auto value = String((char*)buf);
    value = value.substring(0, buflen);
    auto channel = 1;
    radioPortal.receive(value, channel);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
  }
}
