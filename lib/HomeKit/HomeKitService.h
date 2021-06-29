#ifndef HomeKitService_h
#define HomeKitService_h

#include <map>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "Commons.h"

namespace Victoria::Components {
  class HomeKitService {
    typedef std::function<void(const ServiceState&)> TStateChangeHandler;
    public:
      HomeKitService(String id, uint8_t outputPin, homekit_server_config_t* serverConfig, homekit_characteristic_t* mainCharacteristic);
      ~HomeKitService();
      String serviceId;
      TStateChangeHandler onStateChange;
      virtual ServiceState getState();
      virtual void setState(const ServiceState& state);
      static HomeKitService* findServiceById(const String& serviceId);
      static void heartbeatAll();
      static void loopAll();
      static void resetAll();
    protected:
      uint8_t _outputPin;
      homekit_server_config_t* _serverConfig;
      homekit_characteristic_t* _mainCharacteristic;
      void _init();
      void _notify();
      static HomeKitService* _findService(homekit_characteristic_t* mainCharacteristic);
  };
} // namespace Victoria::Components

#endif // HomeKitService_h
