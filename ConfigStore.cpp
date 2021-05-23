#include <ArduinoJson.h>
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
      std::vector<AccessoryState> states;
      SettingModel model = {
        states: states,
      };

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
      model.deserializeFrom(doc);
      return model;
    }

    bool ConfigStore::save(SettingModel model) {
      // [convert]
      // DynamicJsonDocument doc(1024); // Store data in the heap - Dynamic Memory Allocation
      StaticJsonDocument<256> doc; // Store data in the stack - Fixed Memory Allocation
      model.serializeTo(doc);

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

  }
}
