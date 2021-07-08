#include <RadioStorage.h>

namespace Victoria::Components {

  RadioStorage::RadioStorage() {
    _filePath = "/radio.json";
  }

  void RadioStorage::_serializeTo(const RadioModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    doc[0] = model.inputPin;
  }

  void RadioStorage::_deserializeFrom(RadioModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) {
    model.inputPin = doc[0].as<int>();
  }

  // global
  RadioStorage radioStorage;

} // namespace Victoria::Components
