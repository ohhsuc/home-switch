#ifndef HomeKitMain_h
#define HomeKitMain_h

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
    HomeKitService* createService(const String& serviceId, const ServiceSetting& serviceSetting);
    HomeKitService* findServiceById(const String& serviceId);
    void removeService(const String& serviceId);
    void heartbeat();
    void clear();
    void setup(String hostName);
    void loop();
    void reset();

   private:
    String _hostName;
    std::map<String, HomeKitService*> _idServiceMap;
  };

} // namespace Victor::HomeKit

#endif // HomeKitMain_h
