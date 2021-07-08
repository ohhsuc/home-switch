#include <ServiceStorage.h>

namespace Victoria::Components {

  ServiceStorage::ServiceStorage() {
    _filePath = "/services.json";
  }

  void ServiceStorage::_serializeTo(const ServicesModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
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

  void ServiceStorage::_deserializeFrom(ServicesModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
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

  // global
  ServiceStorage serviceStorage;

} // namespace Victoria::Components
