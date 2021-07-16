#ifndef HomeKitService_h
#define HomeKitService_h

#include <Arduino.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "Commons.h"

namespace Victoria::HomeKit {
  class HomeKitService {
    typedef std::function<void(const ServiceState&)> TStateChangeHandler;

   public:
    HomeKitService(const String& id, const ServiceSetting& setting, homekit_characteristic_t* characteristic);
    ~HomeKitService();
    String serviceId;
    ServiceSetting serviceSetting;
    homekit_characteristic_t* serviceCharacteristic = NULL;
    TStateChangeHandler onStateChange;
    virtual void setup();
    virtual void loop();
    virtual ServiceState getState();
    virtual void setState(const ServiceState& state);
    virtual void notifyState();

   protected:
    void _fireStateChange(const ServiceState& state);
  };
} // namespace Victoria::HomeKit

#endif // HomeKitService_h
