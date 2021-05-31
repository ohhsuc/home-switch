#ifndef ConfigStore_h
#define ConfigStore_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Commons.h"

namespace Victoria {
  namespace Components {
    class ConfigStore {
      public:
        ConfigStore();
        SettingModel load();
        bool save(SettingModel model);
      private:
        static void _serializeTo(const SettingModel& model, StaticJsonDocument<256>& doc);
        static void _deserializeFrom(SettingModel& model, const StaticJsonDocument<256>& doc);
    };
  }
}

#endif //ConfigStore_h
