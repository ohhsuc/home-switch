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
      _rf = new RCSwitch();
      _rf->enableReceive(model.inputPin);
    }
  }

  void RadioPortal::loop() {
    auto now = millis();
    if (
      now - _lastAvailable > AVAILABLE_THROTTLE_TIMESPAN &&
      _rf && _rf->available()
    ) {
      // payload
      unsigned long value = _rf->getReceivedValue();
      unsigned int bits = _rf->getReceivedBitlength();
      unsigned int protocol = _rf->getReceivedProtocol();
      // message
      RadioMessage message = {
        .value = value,
        .bits = bits,
        .protocol = protocol,
        .timestamp = now,
      };
      if (now - _lastMessage.timestamp > RESET_PRESS_TIMESPAN) {
        RadioMessage empty {};
        _lastMessage = empty;
        _lastPressState = PressStateAwait;
      }
      if (
        _lastPressState != PressStateClick &&
        (_lastMessage.value != message.value || _lastMessage.protocol != message.protocol)
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
      // reset state
      _rf->resetAvailable();
      _lastAvailable = now;
      radioStorage.broadcast(message);
    }
  }

  void RadioPortal::_handleMessage(const RadioMessage& message, RadioPressState press) {
    // logs
    auto received = String(message.protocol) + "/" + String(message.value) + "/" + String(message.bits) + "bits press " + String(press);
    console.log("[RadioPortal] received " + received);
    // log states
    _lastMessage = message;
    _lastPressState = press;
    // check rules
    auto model = radioStorage.load();
    for (const auto& rule : model.rules) {
      if (
        rule.value == message.value &&
        rule.protocol == message.protocol &&
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
