#include "ServiceStorage.h"

namespace Victoria::Components {

  ServiceStorage::ServiceStorage() {
    _filePath = "/service.json";
  }

  void ServiceStorage::_serializeTo(const ServicesModel& model, DynamicJsonDocument& doc) {
    JsonArray items = doc.createNestedArray("s");
    for (const auto& pair : model.services) {
      ServiceSetting service = pair.second;
      JsonArray item = items.createNestedArray();
      item[0] = pair.first; // id
      item[1] = service.name;
      item[2] = service.type;
      item[3] = service.inputPin;
      item[4] = service.outputPin;
      item[5] = service.inputTrueValue;
      item[6] = service.outputTrueValue;
    }
  }

  void ServiceStorage::_deserializeFrom(ServicesModel& model, const DynamicJsonDocument& doc) {
    auto items = doc["s"];
    for (size_t i = 0; i < items.size(); i++) {
      auto item = items[i];
      ServiceSetting service = {
        .name = item[1],
        .type = item[2],
        .inputPin = item[3],
        .outputPin = item[4],
        .inputTrueValue = item[5],
        .outputTrueValue = item[6],
      };
      auto id = String(item[0]);
      model.services[id] = service;
    }
  }

  // global
  ServiceStorage serviceStorage;

} // namespace Victoria::Components
