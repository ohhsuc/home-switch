#ifndef BooleanAccessory_h
#define BooleanAccessory_h

#include <map>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

namespace Victoria {
  namespace Components {
    class BooleanAccessory {
      typedef std::function<void(bool)> TChangeHandler;
      public:
        BooleanAccessory(uint8_t outputPin);
        ~BooleanAccessory();
        void setup();
        void reset();
        void loop();
        void setValue(bool value);
        bool getValue();
        void heartbeat();
        TChangeHandler onChange;
      private:
        uint8_t _outputPin;
        homekit_server_config_t* _serverConfig;
        homekit_characteristic_t* _boolCharacteristic;
        void _notify();
        static void _setter_ex(homekit_characteristic_t *ch, const homekit_value_t value);
    };
  }
}

#endif //BooleanAccessory_h
