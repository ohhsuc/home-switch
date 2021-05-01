#include <ESP8266WiFi.h>
#include "ButtonEvents.h"

ButtonEvents::ButtonEvents(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT_PULLUP);
  digitalWrite(_pin, HIGH);
}

void ButtonEvents::loop() {
  _loadState();
  if (_buttonState != _lastButtonState) {
    _lastButtonState = _buttonState;
    if (onTrigger) {
      onTrigger(_buttonState);
    }
  }
}

int ButtonEvents::_loadState() {
  if (_buttonState == AWAIT_PRESS) {
    _clicks = 0;
    if (digitalRead(_pin) == LOW) {
      _buttonState = DEBOUNCE_PRESS;
      _eventTime = millis();
    }
  }

  else if (_buttonState == DEBOUNCE_PRESS) {
    if ((millis() - _eventTime) > _debouncePressTime) {
      _buttonState = AWAIT_RELEASE;
      _eventTime = millis();
    }
  }

  else if (_buttonState == AWAIT_RELEASE) {
    if (digitalRead(_pin) == HIGH) {
      if ((millis() - _eventTime) > _holdTime) {
        _clicks = -1;
      }
      _buttonState = DEBOUNCE_RELEASE;
      _eventTime = millis();
    }
  }

  else if (_buttonState == DEBOUNCE_RELEASE) {
    if ((millis() - _eventTime) > _debounceReleaseTime) {
      if (_clicks < 0) {
        _buttonState = AWAIT_PRESS;
        return -1;
      }
      _clicks += 1;
      _buttonState = AWAIT_MULTI_PRESS;
      _eventTime = millis();
    }
  }

  else {   // (_buttonState == AWAIT_MULTI_PRESS)
    if (digitalRead(_pin) == LOW) {
      _buttonState = DEBOUNCE_PRESS;
      _eventTime = millis();
    }
    else if ((millis() - _eventTime) > _multiClickTime) {
      _buttonState = AWAIT_PRESS;
      return _clicks;
    }
  }

  return 0;
}
