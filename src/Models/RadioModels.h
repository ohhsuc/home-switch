#ifndef RadioModels_h
#define RadioModels_h

#include <vector>
#include <Arduino.h>

namespace Victor {

  struct RadioMessage {
    String id;
    String value;
    unsigned int channel = 0;
    unsigned long timestamp = 0;
  };

  enum RadioAction {
    RadioActionNone = 0,
    RadioActionTrue = 1,
    RadioActionFalse = 2,
    RadioActionToggle = 3,
    RadioActionWiFiSta = 4,
    RadioActionWiFiStaAp = 5,
    RadioActionWiFiReset = 6,
    RadioActionEspRestart = 7,
  };

  enum RadioPressState {
    PressStateAwait = 0,
    PressStateClick = 1,
    PressStateDoubleClick = 2,
    PressStateLongPress = 3,
  };

  struct RadioRule {
    String value;
    unsigned int channel = 0;
    RadioPressState press = PressStateClick;
    RadioAction action = RadioActionNone;
    String serviceId;
  };

  enum RadioCommandEntry {
    EntryNone = 0,
    EntryWifi = 1,
    EntryApp = 2,
    EntryEsp = 3,
    EntryBoolean = 4,
  };

  enum EntryWifiAction {
    EntryWifiNone = 0,
    EntryWifiJoin = 1, // parameter: {ssid}/{password}
    EntryWifiMode = 2, // parameter: off, sta, ap, ap-sta
    EntryWifiReset = 3, // parameter: NONE
  };

  enum EntryAppAction {
    EntryAppNone = 0,
    EntryAppName = 1, // parameter: {name}
    EntryAppOTA = 2, // parameter: fs, sketch(default)
  };

  enum EntryEspAction {
    EntryEspNone = 0,
    EntryEspRestart = 1, // parameter: NONE
  };

  enum EntryBooleanAction {
    EntryBooleanNone = 0,
    EntryBooleanSet = 1, // parameter: true, false
    EntryBooleanToggle = 2,
  };

  struct RadioCommand {
    RadioCommandEntry entry = EntryNone;
    int action = -1;
    RadioPressState press = PressStateClick;
    String serviceId;
  };

  struct RadioCommandParsed :RadioCommand {
    String parameters;
  };

  struct RadioModel {
    int8_t inputPin = -1;
    int8_t outputPin = -1;
    std::vector<RadioRule> rules;
    std::vector<RadioCommand> commands;
  };

} // namespace Victor

#endif // RadioModels_h
