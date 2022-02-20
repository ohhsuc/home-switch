#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <Console.h>
#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorWeb.h>

#include "TimesCounter.h"
#include "SwitchStorage.h"
#include "SwitchIO.h"

using namespace Victor;
using namespace Victor::Events;
using namespace Victor::Components;

VictorWeb webPortal(80);
TimesCounter times(10, 5 * 1000);
SwitchIO* switchIO;
String hostName;

extern "C" homekit_characteristic_t switchState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_server_config_t serverConfig;

String toYesNoName(bool state) {
  return state ? F("Yes") : F("No");
}

String toSwitchStateName(bool state) {
  return state ? F("On") : F("Off");
}

void setSwitchState(const bool value) {
  ESP.wdtFeed();
  builtinLed.flash();
  times.count();
  switchState.value.bool_value = value;
  homekit_characteristic_notify(&switchState, switchState.value);
  switchIO->outputState(value);
  console.log()
    .bracket(F("switch"))
    .section(F("state"), toSwitchStateName(value));
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

  // setup web
  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.onRadioEmit = [](uint8_t index) { };
  webPortal.onServiceGet = [](std::vector<KeyValueModel>& items) {
    items.push_back({ .key = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    items.push_back({ .key = F("State"),   .value = toSwitchStateName(switchState.value.bool_value) });
    items.push_back({ .key = F("Paired"),  .value = toYesNoName(homekit_is_paired()) });
    items.push_back({ .key = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
  };
  webPortal.onServicePost = [](const String& type) {
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
  switchState.setter = [](const homekit_value_t value) { setSwitchState(value.bool_value); };
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
  arduino_homekit_loop();
  webPortal.loop();
  switchIO->loop();
}
