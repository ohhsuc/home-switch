#include <AppStorage.h>

namespace Victoria::Components {

  AppStorage::AppStorage() {
    _filePath = "/app.json";
    _maxSize = 256;
  }

  void AppStorage::_serializeTo(const AppModel& model, DynamicJsonDocument& doc) {
    doc["n"] = model.name;
    doc["l"][0] = model.ledPin;
    doc["l"][1] = model.ledOnValue;
  }

  void AppStorage::_deserializeFrom(AppModel& model, const DynamicJsonDocument& doc) {
    const char* name = doc["n"];
    model.name = String(name);
    model.ledPin = doc["l"][0];
    model.ledOnValue = doc["l"][1];
  }

  // global
  AppStorage appStorage;

} // namespace Victoria::Components
