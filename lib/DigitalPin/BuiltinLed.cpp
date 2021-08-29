#include "BuiltinLed.h"

namespace Victoria::Components {

  BuiltinLed::BuiltinLed() {
    auto model = appStorage.load();
    if (model.ledPin > -1) {
      auto trueValue = model.ledOnValue == 0 ? LOW : HIGH;
      _outputPin = new DigitalOutput(model.ledPin, trueValue);
    }
  }

  BuiltinLed::~BuiltinLed() {
    if (_outputPin) {
      delete _outputPin;
      _outputPin = NULL;
    }
  }

  void BuiltinLed::turnOn() {
    if (_outputPin) {
      _outputPin->setValue(true);
    }
  }

  void BuiltinLed::turnOff() {
    if (_outputPin) {
      _outputPin->setValue(false);
    }
  }

  void BuiltinLed::flash() {
    turnOn();
    delay(100); // at least light for some time
    turnOff();
  }

} // namespace Victoria::Components
