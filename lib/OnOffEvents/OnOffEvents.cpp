#include "OnOffEvents.h"

#define DEFAULT_DEBOUNCE_DELAY 50

namespace Victoria::Events {

  OnOffEvents::OnOffEvents(uint8_t inputPin) {
    _inputPin = inputPin;
  }

  void OnOffEvents::loop() {
    auto isOn = digitalRead(_inputPin) == LOW;
    if (isOn != _lastState) {
      _lastTimeRead = millis();
      return;
    }
    if (millis() - _lastTimeRead > DEFAULT_DEBOUNCE_DELAY) {
      if (isOn != _lastState) {
        _lastState = isOn;
        if (onChange) {
          onChange(isOn);
        }
      }
    }
  }

} // namespace Victoria::Events
