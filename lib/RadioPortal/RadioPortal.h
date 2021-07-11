#ifndef RadioPortal_h
#define RadioPortal_h

#include <map>
#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include "Commons.h"
#include "RadioStorage.h"

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
      RCSwitch* _rf;
      void _handleMessage(RadioMessage message);
      void _proceedAction(RadioRule rule);
  };
} // namespace Victoria::Components

#endif // RadioPortal_h