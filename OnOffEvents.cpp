#include <ESP8266WiFi.h>
#include "OnOffEvents.h"

namespace Purl {
  namespace Events {

    OnOffEvents::OnOffEvents(uint8_t pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
      digitalWrite(_pin, HIGH);
    }

    void OnOffEvents::loop() {
      bool isOn = digitalRead(_pin) == LOW;
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
