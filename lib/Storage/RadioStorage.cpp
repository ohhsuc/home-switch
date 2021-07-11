#include <RadioStorage.h>

namespace Victoria::Components {

  RadioStorage::RadioStorage() {
    _filePath = "/radio.json";
    // empty value
    _lastReceived = {
      .value = 0,
      .bits = 0,
      .protocol = 0,
      .timestamp = 0,
    };
  }

  void RadioStorage::broadcast(RadioMessage message) {
    _lastReceived = message;
  }

  RadioMessage RadioStorage::getLastReceived() {
    return _lastReceived;
  }

  void RadioStorage::_serializeTo(const RadioModel& model, DynamicJsonDocument& doc) {
    doc["i"] = model.inputPin;
    JsonArray items = doc.createNestedArray("r");
    for (const auto& rule : model.rules) {
      JsonArray item = items.createNestedArray();
      item[0] = rule.value;
      item[1] = rule.protocol;
      item[2] = rule.action;
      item[3] = rule.serviceId;
    }
  }

  void RadioStorage::_deserializeFrom(RadioModel& model, const DynamicJsonDocument& doc) {
    model.inputPin = doc["i"];
    auto items = doc["r"];
    for (size_t i = 0; i < items.size(); i++) {
      auto item = items[i];
      RadioRule rule = {
        .value = item[0],
        .protocol = item[1],
        .action = item[2],
        .serviceId = item[3],
      };
      model.rules.push_back(rule);
    }
  }

  // global
  RadioStorage radioStorage;

} // namespace Victoria::Components
