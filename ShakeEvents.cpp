#include "ShakeEvents.h"

#define DEFAULT_DEBOUNCE_MILLIS 2000

namespace Victoria {
  namespace Events {

    ShakeEvents::ShakeEvents(uint8_t inputPin) {
      _inputPin = inputPin;
      pinMode(_inputPin, INPUT);
      _state = digitalRead(_inputPin);
    }

    void ShakeEvents::loop() {
      int state = digitalRead(_inputPin);
      if (_state != state) {
        _state = state;
        _shaked = true;
      }
      if (_shaked) {
        unsigned long now = millis();
        if (now - _lastFire > DEFAULT_DEBOUNCE_MILLIS) {
          _lastFire = now;
          _shaked = false;
          if (onShake) {
            onShake();
          }
        }
      }
    }

  }
}
