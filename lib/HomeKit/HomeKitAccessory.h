#ifndef HomeKitAccessory_h
#define HomeKitAccessory_h

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "Commons.h"

namespace Victoria::HomeKit {
  class HomeKitAccessory {
   public:
    HomeKitAccessory();
    static void setup(const String& hostName);
    static void loop();
    static void reset();
  };

} // namespace Victoria::HomeKit

#endif // HomeKitAccessory_h
