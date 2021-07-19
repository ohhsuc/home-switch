#ifndef RadioPortal_h
#define RadioPortal_h

#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include "Commons.h"
#include "RadioStorage.h"

// timespan in millianseconds
#define AVAILABLE_THROTTLE_TIMESPAN 100
#define DOUBLE_CLICK_TIMESPAN_FROM 300
#define DOUBLE_CLICK_TIMESPAN_TO 500
#define LONG_PRESS_TIMESPAN 2000
#define RESET_PRESS_TIMESPAN 2500

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
    RadioMessage _lastMessage = {};
    RadioPressState _lastPressState = PressStateAwait;
    unsigned long _lastAvailable = 0;
    void _handleMessage(const RadioMessage& message, RadioPressState press);
    void _proceedAction(const RadioRule& rule);
  };
} // namespace Victoria::Components

#endif // RadioPortal_h
