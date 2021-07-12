#ifndef HomeKitService_h
#define HomeKitService_h

#include <map>
#include <Arduino.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "Commons.h"

namespace Victoria::HomeKit {
  class HomeKitService {
    typedef std::function<void(const ServiceState&)> TStateChangeHandler;

   public:
    HomeKitService(String id, uint8_t outputPin, homekit_characteristic_t* mainCharacteristic);
    ~HomeKitService();
    String serviceId;
    TStateChangeHandler onStateChange;
    virtual ServiceState getState();
    virtual void setState(const ServiceState& state);
    static HomeKitService* findServiceById(const String& serviceId);
    static void heartbeat();

   protected:
    uint8_t _outputPin;
    homekit_characteristic_t* _mainCharacteristic;
    void _notify();
    static HomeKitService* _findService(homekit_characteristic_t* mainCharacteristic);
  };
} // namespace Victoria::HomeKit

#endif // HomeKitService_h
