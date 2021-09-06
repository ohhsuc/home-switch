#include "RadioStorage.h"

namespace Victoria::Components {

  RadioStorage::RadioStorage() {
    _filePath = F("/radio.json");
    _lastReceived = {}; // empty value
  }

  void RadioStorage::broadcast(RadioMessage message) {
    _lastReceived = message;
  }

  RadioMessage RadioStorage::getLastReceived() {
    return _lastReceived;
  }

  void RadioStorage::_serializeTo(const RadioModel& model, DynamicJsonDocument& doc) {
    doc[F("i")] = model.inputPin;
    JsonArray ruleItems = doc.createNestedArray(F("rules"));
    JsonArray commandItems = doc.createNestedArray(F("commands"));
    for (const auto& rule : model.rules) {
      JsonArray item = ruleItems.createNestedArray();
      item[0] = rule.value;
      item[1] = rule.channel;
      item[2] = rule.press;
      item[3] = rule.action;
      item[4] = rule.serviceId;
    }
    for (const auto& command : model.commands) {
      JsonArray item = commandItems.createNestedArray();
      item[0] = command.entry;
      item[1] = command.action;
      item[2] = command.press;
      item[3] = command.serviceId;
    }
  }

  void RadioStorage::_deserializeFrom(RadioModel& model, const DynamicJsonDocument& doc) {
    model.inputPin = doc[F("i")];
    auto ruleItems = doc[F("rules")];
    auto commandItems = doc[F("commands")];
    for (size_t i = 0; i < ruleItems.size(); i++) {
      auto item = ruleItems[i];
      RadioRule rule = {
        .value = item[0],
        .channel = item[1],
        .press = item[2],
        .action = item[3],
        .serviceId = item[4],
      };
      model.rules.push_back(rule);
    }
    for (size_t i = 0; i < commandItems.size(); i++) {
      auto item = commandItems[i];
      RadioCommand command = {
        .entry = item[0],
        .action = item[1],
        .press = item[2],
        .serviceId = item[3],
      };
      model.commands.push_back(command);
    }
  }

  // global
  RadioStorage radioStorage;

} // namespace Victoria::Components
