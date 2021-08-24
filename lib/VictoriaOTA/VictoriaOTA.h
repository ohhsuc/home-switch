#ifndef VictoriaOTA_h
#define VictoriaOTA_h

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include "Commons.h"

namespace Victoria::Components {
  class VictoriaOTA {
   public:
    static void setup();
    static String getCurrentVersion();
    static String checkNewVersion();
    static void update(String version, VOtaType type);
    static void trigger(VOtaType type);

   private:
    static void _updateSketch();
    static void _updateFileSystem();
    static void _onStart();
    static void _onEnd();
    static void _onProgress(int progress, int total);
    static void _onError(int error);
  };
} // namespace Victoria::Components

#endif // VictoriaOTA_h
