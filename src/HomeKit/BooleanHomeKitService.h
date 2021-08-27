#ifndef BooleanHomeKitService_h
#define BooleanHomeKitService_h

#include <Arduino.h>
#include "HomeKitService.h"
#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "ButtonEvents.h"

using namespace Victoria::Events;
using namespace Victoria::Components;

namespace Victoria::HomeKit {
  class BooleanHomeKitService : public HomeKitService {
   public:
    BooleanHomeKitService(const String& id, const ServiceSetting& setting);
    ~BooleanHomeKitService();
    void setup() override;
    void loop() override;
    ServiceState getState() override;
    void setState(const ServiceState& state) override;

   private:
    DigitalInput* _inputPin;
    DigitalOutput* _outputPin;
    ButtonEvents* _buttonEvents = NULL;
    static void _notifyCallback(homekit_characteristic_t *ch, homekit_value_t value, void *context);
  };
} // namespace Victoria::HomeKit

#endif // BooleanHomeKitService_h
