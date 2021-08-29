#include "AppStorage.h"

namespace Victoria::Components {

  AppStorage::AppStorage() {
    _filePath = "/app.json";
    _maxSize = 256;
  }

  void AppStorage::_serializeTo(const AppModel& model, DynamicJsonDocument& doc) {
    doc["name"] = model.name;
    doc["led"][0] = model.ledPin;
    doc["led"][1] = model.ledOnValue;
  }

  void AppStorage::_deserializeFrom(AppModel& model, const DynamicJsonDocument& doc) {
    const char* name = doc["name"];
    model.name = String(name);
    model.ledPin = doc["led"][0];
    model.ledOnValue = doc["led"][1];
  }

  // global
  AppStorage appStorage;

} // namespace Victoria::Components
