#ifndef BooleanHomeKitService_h
#define BooleanHomeKitService_h

#include <Arduino.h>
#include "HomeKitService.h"

namespace Victoria::Components {
  class BooleanHomeKitService : public HomeKitService {
    public:
      BooleanHomeKitService(String id, uint8_t outputPin);
      ServiceState getState() override;
      void setState(const ServiceState& state) override;
    private:
      void _innerSetState(const ServiceState& state, bool notify);
      static void _setter_ex(homekit_characteristic_t* ch, const homekit_value_t value);
  };
} // namespace Victoria::Components

#endif // BooleanHomeKitService_h
