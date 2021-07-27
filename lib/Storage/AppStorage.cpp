#include <AppStorage.h>

namespace Victoria::Components {

  AppStorage::AppStorage() {
    _filePath = "/app.json";
    _maxSize = 256;
  }

  void AppStorage::_serializeTo(const AppModel& model, DynamicJsonDocument& doc) {
    doc["n"] = model.name;
  }

  void AppStorage::_deserializeFrom(AppModel& model, const DynamicJsonDocument& doc) {
    const char* name = doc["n"];
    model.name = String(name);
  }

  // global
  AppStorage appStorage;

} // namespace Victoria::Components
