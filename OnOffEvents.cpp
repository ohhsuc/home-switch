#include "OnOffEvents.h"

#define DEFAULT_DEBOUNCE_DELAY 50

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
      if (millis() - _lastTimeRead > DEFAULT_DEBOUNCE_DELAY) {
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
