#ifndef RadioPortal_h
#define RadioPortal_h

#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include "Commons.h"
#include "RadioStorage.h"

#define AVAILABLE_THROTTLE_TIMESPAN 100 // millianseconds
#define MESSAGE_THROTTLE_TIMESPAN 500 // millianseconds

namespace Victoria::Components {
  class RadioPortal {
    typedef std::function<void(const RadioRule&)> TRadioAction;

   public:
    RadioPortal();
    ~RadioPortal();
    void setup();
    void loop();
    TRadioAction onAction;

   private:
    RCSwitch* _rf = NULL;
    unsigned long _lastAvailable = 0;
    void _handleMessage(const RadioMessage& message);
    void _proceedAction(const RadioRule& rule);
  };
} // namespace Victoria::Components

#endif // RadioPortal_h
