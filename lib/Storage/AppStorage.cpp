#include "AppStorage.h"

namespace Victoria::Components {

  AppStorage::AppStorage() {
    _filePath = F("/app.json");
    _maxSize = 256;
  }

  void AppStorage::_serializeTo(const AppModel& model, DynamicJsonDocument& doc) {
    doc[F("name")] = model.name;
    doc[F("led")][0] = model.ledPin;
    doc[F("led")][1] = model.ledOnValue;
    doc[F("otw")] = model.overTheWeb ? 1 : 0;
  }

  void AppStorage::_deserializeFrom(AppModel& model, const DynamicJsonDocument& doc) {
    const char* name = doc[F("name")];
    model.name = String(name);
    model.ledPin = doc[F("led")][0];
    model.ledOnValue = doc[F("led")][1];
    model.overTheWeb = doc[F("otw")] == 1;
  }

  // global
  AppStorage appStorage;

} // namespace Victoria::Components
