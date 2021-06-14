#ifndef BooleanAccessory_h
#define BooleanAccessory_h

#include <Arduino.h>
#include "BaseAccessory.h"

namespace Victoria::Components {
  class BooleanAccessory: public BaseAccessory {
    public:
      BooleanAccessory(String id, uint8_t outputPin);
      AccessoryState getState() override;
      void setState(const AccessoryState& state) override;
    private:
      void _innerSetState(const AccessoryState& state, bool notify);
      static void _setter_ex(homekit_characteristic_t *ch, const homekit_value_t value);
  };
}

#endif //BooleanAccessory_h
