#include <Arduino.h>
#include <RH_ASK.h>
#include <arduino_homekit_server.h>

#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorRadio.h>
#include <VictorWeb.h>

#include "SwitchStorage.h"
#include "SwitchIO.h"
#include "TimesTrigger.h"

using namespace Victor;
using namespace Victor::Events;
using namespace Victor::Components;

RH_ASK* ask;
BuiltinLed* builtinLed;
VictorRadio radioPortal;
VictorWeb webPortal(80);
SwitchIO* switchIO;
TimesTrigger timesTrigger(10, 5 * 1000);
String hostName;

extern "C" homekit_characteristic_t switchState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_server_config_t serverConfig;

String parseStateName(bool state) {
  return state ? "On" : "Off";
}

void switchStateSetter(const homekit_value_t value) {
  builtinLed->flash();
  timesTrigger.count();
  switchState.value.bool_value = value.bool_value;
  switchIO->outputState(value.bool_value);
  console.log().section(F("switch"), parseStateName(value.bool_value));
}

void setSwitchState(const bool value) {
  switchState.value.bool_value = value;
  homekit_characteristic_notify(&switchState, switchState.value);
  switchIO->outputState(value);
  console.log().section(F("switch"), parseStateName(value));
}

void setSwitchAction(const int& action) {
  const auto value = action == 0 ? false
    : action == 1 ? true
    : action == 2 ? !switchState.value.bool_value
    : switchState.value.bool_value;
  setSwitchState(value);
}

bool setRadioAction(const RadioRule& rule) {
  const int action = rule.action == RadioActionFalse ? 0
    : rule.action == RadioActionTrue ? 1
    : rule.action == RadioActionToggle ? 2 : -1;
  setSwitchAction(action);
  return true;
}

bool setRadioCommand(const RadioCommandParsed& command) {
  if (command.entry == EntryBoolean) {
    const int action = command.action == EntryBooleanToggle ? 2
      : (command.action == EntryBooleanSet && command.parameters == "true") ? 1
      : (command.action == EntryBooleanSet && command.parameters == "false") ? 0 : -1;
    setSwitchAction(action);
    return true;
  }
  return false;
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

  const auto radioJson = radioStorage.load();
  ask = new RH_ASK(2000, radioJson.inputPin, radioJson.outputPin, 0);
  if (!ask->init()) {
    console.error(F("RH_ASK init failed"));
  }

  // setup radio
  radioPortal.onAction = setRadioAction;
  radioPortal.onCommand = setRadioCommand;
  radioPortal.onEmit = [](const RadioEmit& emit) {
    auto value = emit.name + F("!") + emit.value;
    const char* payload = value.c_str();
    ask->send((uint8_t *)payload, strlen(payload));
    ask->waitPacketSent();
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("sent"), value)
      .section(F("via channel"), String(emit.channel));
  };

  // setup web
  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onRadioEmit = [](int index) { radioPortal.emit(index); };
  webPortal.onResetService = []() { homekit_server_reset(); };
  webPortal.onGetServiceState = [](std::vector<KeyValueModel>& items) {
    const auto stateName = parseStateName(switchState.value.bool_value);
    const auto count = arduino_homekit_connected_clients_count();
    items.push_back({ .key = "Switch", .value = stateName });
    items.push_back({ .key = "Clients", .value = String(count) });
  };
  webPortal.setup();

  // setup homekit server
  hostName = victorWifi.getHostName();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  switchState.setter = switchStateSetter;
  arduino_homekit_setup(&serverConfig);

    // setup switch io
  const auto switchJson = switchStorage.load();
  switchIO = new SwitchIO(switchJson);
  switchIO->onStateChange = setSwitchState;

  timesTrigger.onTimesOut = []() {
    console.log(F("times out!"));
  };

  builtinLed->flash();
  console.log(F("setup complete"));
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
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
  }
}
