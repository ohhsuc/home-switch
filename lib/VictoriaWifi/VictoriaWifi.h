#ifndef VictoriaWifi_h
#define VictoriaWifi_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Commons.h"
#include "AppStorage.h"

namespace Victoria::Components {
  class VictoriaWifi {
   public:
    static void setup();
    static void reset();
    static void join(String ssid, String password, bool waitForConnecting);
    static String getHostId();
    static String getHostName(bool includeVersion);

   private:
    static void _onWifiEvent(WiFiEvent_t event);
  };
} // namespace Victoria::Components

#endif // VictoriaWifi_h
