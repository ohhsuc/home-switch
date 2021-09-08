#ifndef HomeKitAccessory_h
#define HomeKitAccessory_h

#include <map>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "Commons.h"
#include "HomeKitBuildFlags.h"
#include "HomeKitService.h"
#include "BooleanHomeKitService.h"

namespace Victor::HomeKit {
  class HomeKitMain {
   public:
    static HomeKitService* createService(const String& serviceId, const ServiceSetting& serviceSetting);
    static HomeKitService* findServiceById(const String& serviceId);
    static void removeService(const String& serviceId);
    static void heartbeat();
    static void clear();
    static void setup(String hostName);
    static void loop();
    static void reset();
  };

} // namespace Victor::HomeKit

#endif // HomeKitAccessory_h
