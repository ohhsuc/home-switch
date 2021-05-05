#include "Arduino.h"
#include "ShakeEvents.h"

#define DEFAULT_DEBOUNCE_MILLIS 2000

namespace Purl {
  namespace Events {

    ShakeEvents::ShakeEvents(uint8_t pin) {
      _pin = pin;
      pinMode(_pin, INPUT); 
      _state = digitalRead(_pin);
    }

    void ShakeEvents::loop() {
      int state = digitalRead(_pin);
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
