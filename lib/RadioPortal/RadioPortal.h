#ifndef RadioPortal_h
#define RadioPortal_h

#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include "Commons.h"
#include "RadioStorage.h"

#define THROTTLE_TIMESPAN 500 // millianseconds

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
    bool _isThrottled(const RadioMessage& message);
    void _handleMessage(const RadioMessage& message);
    void _proceedAction(const RadioRule& rule);
  };
} // namespace Victoria::Components

#endif // RadioPortal_h
