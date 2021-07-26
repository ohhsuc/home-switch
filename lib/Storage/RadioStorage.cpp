#include <RadioStorage.h>

namespace Victoria::Components {

  RadioStorage::RadioStorage() {
    _filePath = "/radio.json";
    _lastReceived = {}; // empty value
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
      item[1] = rule.channel;
      item[2] = rule.press;
      item[3] = rule.action;
      item[4] = rule.serviceId;
    }
  }

  void RadioStorage::_deserializeFrom(RadioModel& model, const DynamicJsonDocument& doc) {
    model.inputPin = doc["i"];
    auto items = doc["r"];
    for (size_t i = 0; i < items.size(); i++) {
      auto item = items[i];
      RadioRule rule = {
        .value = item[0],
        .channel = item[1],
        .press = item[2],
        .action = item[3],
        .serviceId = item[4],
      };
      model.rules.push_back(rule);
    }
  }

  // global
  RadioStorage radioStorage;

} // namespace Victoria::Components
