#ifndef ConfigStore_h
#define ConfigStore_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Commons.h"

#define CONFIG_FILE_PATH "/Victoria.json"
#define DEFAULT_FILE_SIZE 1024

namespace Victoria {
  namespace Components {
    class ConfigStore {
      public:
        ConfigStore();
        SettingModel load();
        bool save(SettingModel model);
      private:
        static void _serializeTo(const SettingModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc);
        static void _deserializeFrom(SettingModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc);
    };
  }
}

#endif //ConfigStore_h
