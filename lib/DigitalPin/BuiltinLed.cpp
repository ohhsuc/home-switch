#include "BuiltinLed.h"

namespace Victoria::Components {

  BuiltinLed::BuiltinLed() {
    auto model = appStorage.load();
    auto trueValue = model.ledOnValue == 0 ? LOW : HIGH;
    _outputPin = new DigitalOutput(model.ledPin, trueValue);
  }

  BuiltinLed::~BuiltinLed() {
    delete _outputPin;
    _outputPin = NULL;
  }

  void BuiltinLed::turnOn() {
    _outputPin->setValue(true);
  }

  void BuiltinLed::turnOff() {
    _outputPin->setValue(false);
  }

  void BuiltinLed::flash() {
    turnOn();
    delay(100); // at least light for some time
    turnOff();
  }

} // namespace Victoria::Components
