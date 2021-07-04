#ifndef RfPortal_h
#define RfPortal_h

#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include "Commons.h"

namespace Victoria::Components {
  class RfPortal {
    typedef std::function<void(const String&)> TToggleHandler;
    public:
      RfPortal(String serviceId, uint8_t inputPin);
      ~RfPortal();
      void loop();
      TToggleHandler onToggleState;
    private:
      String _serviceId;
      RCSwitch* _rf;
      void _handleMessage(unsigned long value, unsigned int bitLength);
  };
} // namespace Victoria::Components

#endif // RfPortal_h
