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
    if (_rf && _rf->available()) {
      // payload
      unsigned long value = _rf->getReceivedValue();
      unsigned int bits = _rf->getReceivedBitlength();
      unsigned int protocol = _rf->getReceivedProtocol();
      // logs
      auto received = String(protocol) + "/" + String(value) + "/" + String(bits) + "bits";
      console.log("[RadioPortal] received " + received);
      // message
      RadioMessage message = {
        .value = value,
        .bits = bits,
        .protocol = protocol,
        .timestamp = millis(),
      };
      // throttle
      if (!_isThrottled(message)) {
        radioStorage.broadcast(message);
        _handleMessage(message);
      }
      // reset state
      _rf->resetAvailable();
    }
  }

  bool RadioPortal::_isThrottled(const RadioMessage& message) {
    auto lastMessage = radioStorage.getLastReceived();
    return (
      lastMessage.value == message.value &&
      lastMessage.protocol == message.protocol &&
      millis() - lastMessage.timestamp < THROTTLE_TIMESPAN
    );
  }

  void RadioPortal::_handleMessage(const RadioMessage& message) {
    auto model = radioStorage.load();
    for (const auto& rule : model.rules) {
      if (
        rule.value == message.value &&
        rule.protocol == message.protocol
      ) {
        _proceedAction(rule);
      }
    }
  }

  void RadioPortal::_proceedAction(const RadioRule& rule) {
    switch (rule.action) {
      case RadioActionWiFiSta:
        WiFi.mode(WIFI_STA);
        console.log("[RadioPortal] set mode WIFI_STA");
        break;
      case RadioActionWiFiStaAp:
        WiFi.mode(WIFI_AP_STA);
        console.log("[RadioPortal] set mode WIFI_AP_STA");
        break;
      default:
        break;
    }
    if (onAction) {
      onAction(rule);
    }
  }

} // namespace Victoria::Components
