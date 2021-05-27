#include "OnOffEvents.h"

namespace Victoria {
  namespace Events {

    OnOffEvents::OnOffEvents(uint8_t inputPin) {
      _inputPin = inputPin;
      pinMode(_inputPin, INPUT_PULLUP);
      digitalWrite(_inputPin, HIGH);
    }

    void OnOffEvents::loop() {
      bool isOn = digitalRead(_inputPin) == LOW;
      if (isOn != _lastState) {
        _lastTimeRead = millis();
        return;
      }
      if (millis() - _lastTimeRead > _debounceDelay) {
        if (isOn != _lastState) {
          _lastState = isOn;
          if (onToggle) {
            onToggle(isOn);
          }
        }
      }
    }

  }
}
