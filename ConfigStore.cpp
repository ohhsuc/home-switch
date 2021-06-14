#include <LittleFS.h>
#include "ConfigStore.h"

namespace Victoria::Components {

  ConfigStore::ConfigStore() { }

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
      doc["s"][i][5] = setting.outputLevel;
      doc["s"][i][6] = setting.inputLevel;
      i++;
    }
  }

  void ConfigStore::_deserializeFrom(SettingModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    auto settingsDoc = doc["s"];
    if (settingsDoc) {
      int index = -1;
      while (true) {
        auto item = settingsDoc[++index];
        if (!item) {
          break;
        }
        String id = item[0];
        if (!id) {
          break;
        }
        int type = item[2];
        AccessorySetting setting = {
          .name = item[1],
          .type = AccessoryType(type), // convert int to enum
          .outputIO = item[3],
          .inputIO = item[4],
          .outputLevel = item[5],
          .inputLevel = item[6],
        };
        model.settings[id] = setting;
      }
    }
  }

}
