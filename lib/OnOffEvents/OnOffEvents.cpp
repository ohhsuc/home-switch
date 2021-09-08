#include "OnOffEvents.h"

#define DEBOUNCE_TIMESPAN 500

namespace Victor::Events {

  OnOffEvents::OnOffEvents(uint8_t inputPin) {
    _inputPin = inputPin;
  }

  void OnOffEvents::loop() {
    auto now = millis();
    if (now - _lastTime > DEBOUNCE_TIMESPAN) {
      auto state = digitalRead(_inputPin) == LOW;
      if (state != _lastState) {
        _lastState = state;
        _lastTime = now;
        if (onChange) {
          onChange(state);
        }
      }
    }
  }

} // namespace Victor::Events
