#include "SwitchStorage.h"

namespace Victor::Components {

  SwitchStorage::SwitchStorage() {
    _filePath = "/switch.json";
  }

  void SwitchStorage::_serializeTo(const SwitchSetting& model, DynamicJsonDocument& doc) {
    const JsonArray setting = doc.createNestedArray(F("s"));
    setting[0] = model.inputPin;
    setting[1] = model.outputPin;
    setting[2] = model.inputTrueValue;
    setting[3] = model.outputTrueValue;
  }

  void SwitchStorage::_deserializeFrom(SwitchSetting& model, const DynamicJsonDocument& doc) {
    const auto setting = doc[F("s")];
    model.inputPin = setting[0];
    model.outputPin = setting[1];
    model.inputTrueValue = setting[2];
    model.outputTrueValue = setting[3];
  }

  // global
  SwitchStorage switchStorage;

} // namespace Victor::Components
