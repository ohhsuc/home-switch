#ifndef VictoriaOTA_h
#define VictoriaOTA_h

#include "Commons.h"

namespace Victoria::Components {
  class VictoriaOTA {
   public:
    static String getCurrentVersion();
    static String checkNewVersion();
    static void update(String version, VOtaType type);
    static void trigger(VOtaType type);
  };
} // namespace Victoria::Components

#endif // VictoriaOTA_h
