#include "OnOffEvents.h"

#define DEFAULT_DEBOUNCE_DELAY 50

namespace Victoria::Events {

  OnOffEvents::OnOffEvents(String serviceId, uint8_t inputPin) {
    _serviceId = serviceId;
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
        if (onToggle) {
          onToggle(_serviceId, isOn);
        }
      }
    }
  }

} // namespace Victoria::Events
