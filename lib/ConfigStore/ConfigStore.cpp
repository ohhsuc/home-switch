#include <LittleFS.h>
#include "ConfigStore.h"

namespace Victoria::Components {

  ConfigStore::ConfigStore() {}

  SettingModel ConfigStore::load() {
    // default result
    SettingModel model;
    // begin
    if (LittleFS.begin()) {
      // check exists
      if (LittleFS.exists(CONFIG_FILE_PATH)) {
        // open file
        File file = LittleFS.open(CONFIG_FILE_PATH, "r");
        if (file) {
          // validate size
          size_t size = file.size();
          // read file
          // Allocate a buffer to store contents of the file.
          char buffer[size];
          // We don't use String here because ArduinoJson library requires the input
          // buffer to be mutable. If you don't use ArduinoJson, you may as well
          // use file.readString instead.
          file.readBytes(buffer, size);
          // close
          file.close();
          // deserialize
          if (size <= DEFAULT_FILE_SIZE) {
            // https://arduinojson.org/
            // https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html
            // DynamicJsonDocument doc(DEFAULT_FILE_SIZE); // Store data in the heap - Dynamic Memory Allocation
            StaticJsonDocument<DEFAULT_FILE_SIZE> doc; // Store data in the stack - Fixed Memory Allocation
            auto error = deserializeJson(doc, buffer);
            if (!error) {
              // convert
              _deserializeFrom(model, doc);
            } else {
              console.error("Failed to parse config file");
              console.error(error.f_str());
            }
          } else {
            console.error("Config file size is too large");
          }
        } else {
          console.error("Failed to open config file");
        }
      } else {
        console.error("File notfound " + String(CONFIG_FILE_PATH));
      }
    } else {
      console.error("Failed to mount file system");
    }
    // end
    LittleFS.end();
    return model;
  }

  bool ConfigStore::save(SettingModel model) {
    // convert
    // DynamicJsonDocument doc(DEFAULT_FILE_SIZE); // Store data in the heap - Dynamic Memory Allocation
    StaticJsonDocument<DEFAULT_FILE_SIZE> doc; // Store data in the stack - Fixed Memory Allocation
    _serializeTo(model, doc);
    bool success = false;
    // begin
    if (LittleFS.begin()) {
      // open file
      File file = LittleFS.open(CONFIG_FILE_PATH, "w");
      if (file) {
        // write
        serializeJson(doc, file);
        // close
        file.close();
        success = true;
      } else {
        console.error("Failed to open config file for writing");
      }
    } else {
      console.error("Failed to mount file system");
    }
    // end
    LittleFS.end();
    return success;
  }

  void ConfigStore::_serializeTo(const SettingModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    JsonArray items = doc.createNestedArray("s");
    for (const auto& pair : model.services) {
      ServiceSetting setting = pair.second;
      int type = setting.type; // convert enum to int
      JsonArray item = items.createNestedArray();
      item[0] = pair.first; // id
      item[1] = setting.name;
      item[2] = type;
      item[3] = setting.outputPin;
      item[4] = setting.inputPin;
      item[5] = setting.outputLevel;
      item[6] = setting.inputLevel;
      item[7] = setting.rfInputPin;
    }
  }

  void ConfigStore::_deserializeFrom(SettingModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    auto items = doc["s"];
    if (items) {
      int index = -1;
      while (true) {
        auto item = items[++index];
        if (!item) {
          break;
        }
        String id = item[0];
        if (!id) {
          break;
        }
        ServiceSetting setting = {
          .name = item[1],
          .type = ServiceType(item[2].as<int>()), // convert int to enum
          .outputPin = item[3].as<int>(),
          .inputPin = item[4].as<int>(),
          .outputLevel = item[5].as<int>(),
          .inputLevel = item[6].as<int>(),
          .rfInputPin = item[7].as<int>(),
        };
        model.services[id] = setting;
      }
    }
  }

} // namespace Victoria::Components
