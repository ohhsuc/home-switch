
#ifndef FileStorage_h
#define FileStorage_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Commons.h"

#define DEFAULT_FILE_SIZE 1024

namespace Victoria::Components {

  template<class TModel>
  class FileStorage {
    public:
      FileStorage();
      TModel load();
      bool save(TModel model);
    protected:
      String _filePath;
      virtual void _serializeTo(const TModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc);
      virtual void _deserializeFrom(TModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc);
  };

  template <class TModel>
  FileStorage<TModel>::FileStorage() {}

  template <class TModel>
  TModel FileStorage<TModel>::load() {
    // default result
    TModel model;
    // begin
    if (LittleFS.begin()) {
      if (LittleFS.exists("/Victoria.json")) {
        LittleFS.remove("/Victoria.json");
      }
      // check exists
      if (LittleFS.exists(_filePath)) {
        // open file
        File file = LittleFS.open(_filePath, "r");
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
        console.error("File notfound " + _filePath);
      }
    } else {
      console.error("Failed to mount file system");
    }
    // end
    LittleFS.end();
    return model;
  }

  template<class TModel>
  bool FileStorage<TModel>::save(TModel model) {
    // convert
    // DynamicJsonDocument doc(DEFAULT_FILE_SIZE); // Store data in the heap - Dynamic Memory Allocation
    StaticJsonDocument<DEFAULT_FILE_SIZE> doc; // Store data in the stack - Fixed Memory Allocation
    _serializeTo(model, doc);
    bool success = false;
    // begin
    if (LittleFS.begin()) {
      // open file
      File file = LittleFS.open(_filePath, "w");
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

  template<class TModel>
  void FileStorage<TModel>::_serializeTo(const TModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) { }

  template<class TModel>
  void FileStorage<TModel>::_deserializeFrom(TModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) { }

} // namespace Victoria::Components

#endif // FileStorage_h
