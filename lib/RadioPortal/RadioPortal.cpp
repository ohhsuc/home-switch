#include "RadioPortal.h"

namespace Victoria::Components {

  RadioPortal::RadioPortal() { }

  RadioPortal::~RadioPortal() {
    if (_rf) {
      delete _rf;
      _rf = NULL;
    }
  }

  void RadioPortal::setup() {
    auto model = radioStorage.load();
    if (model.inputPin > 0) {
      pinMode(model.inputPin, INPUT);
      _rf = new RCSwitch();
      _rf->enableReceive(model.inputPin);
    }
  }

  void RadioPortal::loop() {
    if (_rf && _rf->available()) {
      // payload
      unsigned int protocol = _rf->getReceivedProtocol();
      unsigned long value = _rf->getReceivedValue();
      unsigned int bits = _rf->getReceivedBitlength();
      // logs
      auto received = String(protocol) + "/" + String(value) + "/" + String(bits) + "bits";
      console.log("[RadioPortal] received " + received);
      // handle message
      RadioMessage message = {
        .value = value,
        .bits = bits,
        .protocol = protocol,
        .timestamp = millis(),
      };
      radioStorage.broadcast(message);
      _handleMessage(message);
      // reset state
      _rf->resetAvailable();
    }
  }

  void RadioPortal::_handleMessage(RadioMessage message) {
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

  void RadioPortal::_proceedAction(RadioRule rule) {
    switch (rule.action) {
      case RadioActionWiFiSta:
        WiFi.mode(WIFI_STA);
        console.log("[RadioPortal] set mode WIFI_STA");
        break;
      case RadioActionWiFiApSta:
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
