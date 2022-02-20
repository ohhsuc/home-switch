#include "SwitchIO.h"

namespace Victor::Components {

  SwitchIO::SwitchIO(SwitchSetting model) {
    _input = new DigitalInput(model.inputPin, model.inputTrueValue);
    _output = new DigitalOutput(model.outputPin, model.outputTrueValue);
    _syncMode = model.syncMode;
    _lastState = readState();
  }

  void SwitchIO::loop() {
    const auto now = millis();
    if (now - _lastLoop > 100) {
      _lastLoop = now;
      const auto state = readState();
      if (state != _lastState) {
        _lastState = state;
        if (onStateChange) {
          if (_syncMode) {
            // sync mode
            onStateChange(state);
          } else if (state) {
            // toggle mode
            const auto outputState = _output->lastValue();
            onStateChange(!outputState);
          }
        }
      }
    }
  }

  bool SwitchIO::readState() {
    return _input->getValue();
  }

  void SwitchIO::outputState(bool state) {
    _output->setValue(state);
  }

} // namespace Victor::Components
