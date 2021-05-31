#include <LittleFS.h>
#include "ConfigStore.h"

#define CONFIG_FILE_PATH "/Victoria.json"

namespace Victoria {
  namespace Components {

    ConfigStore::ConfigStore() {
      if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
      }
    }

    SettingModel ConfigStore::load() {
      // [default result]
      SettingModel model;

      // [open file]
      File file = LittleFS.open(CONFIG_FILE_PATH, "r");
      if (!file) {
        Serial.println("Failed to open config file");
        return model;
      }
      size_t size = file.size();
      if (size > 1024) {
        Serial.println("Config file size is too large");
        return model;
      }

      // [read file]
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      // We don't use String here because ArduinoJson library requires the input
      // buffer to be mutable. If you don't use ArduinoJson, you may as well
      // use file.readString instead.
      file.readBytes(buf.get(), size);

      // [deserialize]
      // DynamicJsonDocument doc(1024); // Store data in the heap - Dynamic Memory Allocation
      StaticJsonDocument<256> doc; // Store data in the stack - Fixed Memory Allocation
      auto error = deserializeJson(doc, buf.get());
      if (error) {
        Serial.println("Failed to parse config file");
        return model;
      }

      // [convert]
      _deserializeFrom(model, doc);
      return model;
    }

    bool ConfigStore::save(SettingModel model) {
      // [convert]
      // DynamicJsonDocument doc(1024); // Store data in the heap - Dynamic Memory Allocation
      StaticJsonDocument<256> doc; // Store data in the stack - Fixed Memory Allocation
      _serializeTo(model, doc);

      // [open file]
      File file = LittleFS.open(CONFIG_FILE_PATH, "w");
      if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
      }

      // [write]
      serializeJson(doc, file);
      return true;
    }

    void ConfigStore::_serializeTo(const SettingModel& model, StaticJsonDocument<256>& doc) {
      int i = 0;
      for (const auto& pair : model.settings) {
        String id = pair.first;
        AccessorySetting setting = pair.second;
        int type = setting.type; // convert enum to int
        doc["s"][i][0] = id;
        doc["s"][i][1] = setting.name;
        doc["s"][i][2] = type;
        doc["s"][i][3] = setting.outputIO;
        doc["s"][i][4] = setting.inputIO;
        i++;
      }
    }

    void ConfigStore::_deserializeFrom(SettingModel& model, const StaticJsonDocument<256>& doc) {
      auto settingsDoc = doc["s"];
      if (settingsDoc) {
        int index = -1;
        while (true) {
          auto item = settingsDoc[++index];
          if (!item || !item[0]) {
            break;
          }
          int type = item[2];
          AccessorySetting setting = {
            .name = item[1],
            .type = AccessoryType(type), // convert int to enum
            .outputIO = item[3],
            .inputIO = item[4],
          };
          String id = item[0];
          model.settings[id] = setting;
        }
      }
    }

  }
}
