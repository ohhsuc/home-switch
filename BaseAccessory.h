#ifndef BaseAccessory_h
#define BaseAccessory_h

#include <map>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

namespace Victoria {
  namespace Components {
    class BaseAccessory {
      public:
        BaseAccessory(String id, uint8_t outputPin, homekit_server_config_t* serverConfig, homekit_characteristic_t* mainCharacteristic);
        ~BaseAccessory();
        void loop();
        void reset();
        void heartbeat();
        String accessoryId;
      protected:
        uint8_t _outputPin;
        homekit_server_config_t* _serverConfig;
        homekit_characteristic_t* _mainCharacteristic;
        void _init();
        void _notify();
        static BaseAccessory* _findAccessory(homekit_characteristic_t* mainCharacteristic);
    };
  }
}

#endif //BaseAccessory_h
