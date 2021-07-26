#include "RadioPortal.h"

namespace Victoria::Components {

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
        console.error("[RadioHead] init failed");
      }
    }
  }

  void RadioPortal::loop() {
    auto now = millis();
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t bufLength = sizeof(buf);
    if (_rf->recv(buf, &bufLength)) {
      // payload
      auto value = String((char*)buf);
      value = value.substring(0, bufLength);
      // message
      RadioMessage message = {
        .value = value,
        .channel = 1,
        .timestamp = now,
      };
      // log
      auto received = String(message.channel) + "/" + message.value;
      console.log("[RadioPortal] > received " + received);
      // press
      if (now - _lastMessage.timestamp > RESET_PRESS_TIMESPAN) {
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
        now - _lastMessage.timestamp >= DOUBLE_CLICK_TIMESPAN_FROM &&
        now - _lastMessage.timestamp < DOUBLE_CLICK_TIMESPAN_TO
      ) {
        _handleMessage(_lastMessage, PressStateDoubleClick);
      } else if (
        _lastPressState != PressStateLongPress &&
        (now - _lastMessage.timestamp >= LONG_PRESS_TIMESPAN)
      ) {
        _handleMessage(_lastMessage, PressStateLongPress);
      }
      // broadcase state
      radioStorage.broadcast(message);
    }
  }

  void RadioPortal::_handleMessage(const RadioMessage& message, RadioPressState press) {
    // log states
    _lastMessage = message;
    _lastPressState = press;
    console.log("[RadioPortal] > press " + String(press));
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
  }

  void RadioPortal::_proceedAction(const RadioRule& rule) {
    switch (rule.action) {
      case RadioActionWiFiSta:
        WiFi.mode(WIFI_STA);
        console.log("[RadioPortal] WiFi STA");
        break;
      case RadioActionWiFiStaAp:
        WiFi.mode(WIFI_AP_STA);
        console.log("[RadioPortal] WiFi STA+AP");
        break;
      case RadioActionWiFiReset:
        WiFi.disconnect(true);
        console.log("[RadioPortal] WiFi Reset");
        break;
      case RadioActionEspRestart:
        ESP.restart();
        console.log("[RadioPortal] ESP Restart");
        break;
      default:
        break;
    }
    if (onAction) {
      onAction(rule);
    }
  }

} // namespace Victoria::Components
