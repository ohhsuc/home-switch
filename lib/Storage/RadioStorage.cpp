#include <RadioStorage.h>

namespace Victoria::Components {

  RadioStorage::RadioStorage() {
    _filePath = "/radio.json";
  }

  void RadioStorage::broadcast(RadioMessage message) {
    _lastReceived = message;
  }

  RadioMessage RadioStorage::getLastReceived() {
    return _lastReceived;
  }

  void RadioStorage::_serializeTo(const RadioModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    doc["i"] = model.inputPin;
    JsonArray items = doc.createNestedArray("r");
    for (const auto& rule : model.rules) {
      JsonArray item = items.createNestedArray();
      item[0] = rule.value;
      item[1] = rule.protocol;
      item[2] = rule.serviceId;
      item[3] = rule.action;
    }
  }

  void RadioStorage::_deserializeFrom(RadioModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    model.inputPin = doc["i"];
    auto items = doc["r"];
    for (size_t i = 0; i < items.size(); i++) {
      auto item = items[i];
      RadioRule rule = {
        .value = item[0],
        .protocol = item[1],
        .serviceId = item[2],
        .action = item[3],
      };
      model.rules.push_back(rule);
    }
  }

  // global
  RadioStorage radioStorage;

} // namespace Victoria::Components
