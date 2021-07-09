#include <ServiceStorage.h>

namespace Victoria::Components {

  ServiceStorage::ServiceStorage() {
    _filePath = "/services.json";
  }

  void ServiceStorage::_serializeTo(const ServicesModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    JsonArray items = doc.createNestedArray("s");
    for (const auto& pair : model.services) {
      ServiceSetting service = pair.second;
      JsonArray item = items.createNestedArray();
      item[0] = pair.first; // id
      item[1] = service.name;
      item[2] = service.type;
      item[3] = service.outputPin;
      item[4] = service.inputPin;
      item[5] = service.outputLevel;
      item[6] = service.inputLevel;
    }
  }

  void ServiceStorage::_deserializeFrom(ServicesModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    auto items = doc["s"];
    for (size_t i = 0; i < items.size(); i++) {
      auto item = items[i];
      ServiceSetting service = {
        .name = item[1],
        .type = item[2],
        .outputPin = item[3],
        .inputPin = item[4],
        .outputLevel = item[5],
        .inputLevel = item[6],
      };
      String id = item[0];
      model.services[id] = service;
    }
  }

  // global
  ServiceStorage serviceStorage;

} // namespace Victoria::Components
