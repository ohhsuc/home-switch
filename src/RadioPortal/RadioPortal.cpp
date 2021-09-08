#include "RadioPortal.h"

namespace Victor::Components {

  RadioPortal::RadioPortal() {}

  RadioPortal::~RadioPortal() {
    if (_rf) {
      delete _rf;
      _rf = NULL;
    }
  }

  void RadioPortal::setup() {
    auto model = radioStorage.load();
    if (model.inputPin > -1) {
      pinMode(model.inputPin, INPUT);
      // ESP8266 or ESP32: do not use pin 11 or 2
      _rf = new RH_ASK(2000, model.inputPin);
      if (!_rf->init()) {
        console.error(F("[RadioHead] init failed"));
      }
    }
  }

  void RadioPortal::loop() {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t bufLength = sizeof(buf);
    if (_rf->recv(buf, &bufLength)) {
      // payload
      auto value = String((char*)buf);
      value = value.substring(0, bufLength);
      // read id
      auto id = String(F("none"));
      if (value.indexOf(F("!")) == 4) {
        id = value.substring(0, 4);
        value = value.substring(5);
      }
      // message
      RadioMessage message = {
        .id = id,
        .value = value,
        .channel = 1,
        .timestamp = millis(),
      };
      // press
      auto timespan = message.timestamp - _lastMessage.timestamp;
      if (
        _lastMessage.id != message.id  ||
        timespan > RESET_PRESS_TIMESPAN
      ) {
        RadioMessage empty {};
        _lastMessage = empty;
        _lastPressState = PressStateAwait;
      }
      if (
        _lastPressState != PressStateClick &&
        (_lastMessage.value != message.value || _lastMessage.channel != message.channel)
      ) {
        _handleMessage(message, PressStateClick);
      } else if (
        _lastPressState != PressStateDoubleClick &&
        timespan >= DOUBLE_CLICK_TIMESPAN_FROM &&
        timespan < DOUBLE_CLICK_TIMESPAN_TO
      ) {
        _handleMessage(_lastMessage, PressStateDoubleClick);
      } else if (
        _lastPressState != PressStateLongPress &&
        (timespan >= LONG_PRESS_TIMESPAN)
      ) {
        _handleMessage(_lastMessage, PressStateLongPress);
      }
      // broadcase state
      radioStorage.broadcast(message);
      if (onMessage) {
        onMessage(message);
      }
    }
  }

  void RadioPortal::_handleMessage(const RadioMessage& message, RadioPressState press) {
    // log states
    _lastMessage = message;
    _lastPressState = press;
    console.log().write(F("[RadioPortal] > detected pressed ")).write(String(press)).newline();
    // check rules
    auto model = radioStorage.load();
    for (const auto& rule : model.rules) {
      if (
        rule.value == message.value &&
        rule.channel == message.channel &&
        rule.press == press
      ) {
        _proceedAction(rule);
      }
    }
    // check commands
    auto parsedCommand = _parseCommand(message);
    for (const auto& command : model.commands) {
      if (
        command.entry == parsedCommand.entry &&
        command.action == parsedCommand.action &&
        command.press == press
      ) {
        parsedCommand.serviceId = command.serviceId;
        _proceedCommand(parsedCommand);
      }
    }
  }

  void RadioPortal::_proceedAction(const RadioRule& rule) {
    switch (rule.action) {
      case RadioActionWiFiSta: {
        WiFi.mode(WIFI_STA);
        break;
      }
      case RadioActionWiFiStaAp: {
        WiFi.mode(WIFI_AP_STA);
        break;
      }
      case RadioActionWiFiReset: {
        VictorWifi::reset();
        break;
      }
      case RadioActionEspRestart: {
        ESP.restart();
        break;
      }
      default: {
        break;
      }
    }
    if (onAction) {
      onAction(rule);
    }
  }

  void RadioPortal::_proceedCommand(const RadioCommandParsed& command) {
    switch (command.entry) {
      case EntryWifi: {
        switch (command.action) {
          case EntryWifiJoin: {
            auto credential = GlobalHelpers::splitString(command.parameters, F("/"));
            if (credential.size() == 2) {
              auto ssid = credential[0];
              auto password = credential[1];
              VictorWifi::join(ssid, password, false);
            }
            break;
          }
          case EntryWifiMode: {
            auto hasAP = command.parameters.indexOf(F("ap")) > -1;
            auto hasSTA = command.parameters.indexOf(F("sta")) > -1;
            auto isOff = command.parameters == F("off");
            if (hasAP && hasSTA) { WiFi.mode(WIFI_AP_STA); }
            else if (hasAP) { WiFi.mode(WIFI_AP); }
            else if (hasSTA) { WiFi.mode(WIFI_STA); }
            else if (isOff) { WiFi.mode(WIFI_OFF); }
            break;
          }
          case EntryWifiReset: {
            VictorWifi::reset();
            break;
          }
          case EntryWifiNone:
          default: {
            break;
          }
        }
        break;
      }

      case EntryApp: {
        switch (command.action) {
          case EntryAppName: {
            auto model = appStorage.load();
            model.name = command.parameters;
            appStorage.save(model);
            break;
          }
          case EntryAppOTA: {
            auto otaType =
              command.parameters == F("all") ? VOta_All :
              command.parameters == F("fs") ? VOta_FileSystem :
              command.parameters == F("sketch") ? VOta_Sketch : VOta_Sketch;
            VictorOTA::trigger(otaType);
            break;
          }
          case EntryAppNone:
          default: {
            break;
          }
        }
        break;
      }

      case EntryEsp: {
        switch (command.action) {
          case EntryEspRestart: {
            ESP.restart();
            break;
          }
          case EntryEspNone:
          default: {
            break;
          }
        }
        break;
      }

      case EntryNone:
      default: {
        break;
      }
    }
    if (onCommand) {
      onCommand(command);
    }
  }

  RadioCommandParsed RadioPortal::_parseCommand(const RadioMessage& message) {
    RadioCommandParsed command;
    auto value = String(message.value); // clone
    value.replace(F(":"), F("-"));
    auto parts = GlobalHelpers::splitString(value, F("-"));
    if (parts.size() >= 2) {
      auto entry = parts[0];
      auto action = parts[1];
      if (entry == F("wifi")) {
        command.entry = EntryWifi;
        if (action == F("join")) {
          command.action = EntryWifiJoin;
        } else if (action == F("mode")) {
          command.action = EntryWifiMode;
        } else if (action == F("reset")) {
          command.action = EntryWifiReset;
        }
      } else if (entry == F("app")) {
        command.entry = EntryApp;
        if (action == F("name")) {
          command.action = EntryAppName;
        } else if (action == F("ota")) {
          command.action = EntryAppOTA;
        }
      } else if (entry == F("esp")) {
        command.entry = EntryEsp;
        if (action == F("restart")) {
          command.action = EntryEspRestart;
        }
      } else if (entry == F("boolean")) {
        command.entry = EntryBoolean;
        if (action == F("set")) {
          command.action = EntryBooleanSet;
        } else if (action == F("toggle")) {
          command.action = EntryBooleanToggle;
        }
      }
      if (parts.size() >= 3) {
        auto parameters = parts[2];
        command.parameters = parameters;
      }
    }
    return command;
  }

} // namespace Victor::Components
