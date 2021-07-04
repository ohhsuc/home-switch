#include "RfPortal.h"

namespace Victoria::Components {

  RfPortal::RfPortal(String serviceId, uint8_t inputPin) {
    _serviceId = serviceId;
    pinMode(inputPin, INPUT);
    _rf = new RCSwitch();
    _rf->enableReceive(inputPin);
  }

  RfPortal::~RfPortal() {
    if (_rf) {
      delete _rf;
      _rf = NULL;
    }
  }

  void RfPortal::loop() {
    if (_rf->available()) {
      unsigned int protocol = _rf->getReceivedProtocol();
      unsigned long value = _rf->getReceivedValue();
      unsigned int bitLength = _rf->getReceivedBitlength();
      console.log("[RfPortal] received " + String(protocol) + "/" + String(value) + "/" + String(bitLength) + "bit");
      if (protocol == RfProtocol) {
        _handleMessage(value, bitLength);
      }
      _rf->resetAvailable();
    }
  }

  void RfPortal::_handleMessage(unsigned long value, unsigned int bitLength) {
    switch (value) {
      case 10835154: {
        WiFi.mode(WIFI_STA);
        console.log("[RfPortal] set mode WIFI_STA");
        break;
      }
      case 10835156: {
        WiFi.mode(WIFI_AP_STA);
        console.log("[RfPortal] set mode WIFI_AP_STA");
        break;
      }
      case 9958609: {
        if (onToggleState) {
          onToggleState(_serviceId);
        }
        break;
      }
      default:
        break;
    }
  }

} // namespace Victoria::Components
