#include "OnOffEvents.h"

#define DEFAULT_DEBOUNCE_DELAY 50

namespace Victoria::Events {

  OnOffEvents::OnOffEvents(String accessoryId, uint8_t inputPin) {
    _accessoryId = accessoryId;
    _inputPin = inputPin;
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
          onToggle(_accessoryId, isOn);
        }
      }
    }
  }

} // namespace Victoria::Events
