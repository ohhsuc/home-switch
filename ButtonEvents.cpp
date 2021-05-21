#include "Arduino.h"
#include "ButtonEvents.h"

namespace Victoria {
  namespace Events {

    ButtonEvents::ButtonEvents(uint8_t inputPin) {
      _inputPin = inputPin;
      pinMode(_inputPin, INPUT_PULLUP);
      digitalWrite(_inputPin, HIGH);
    }

    void ButtonEvents::loop() {
      int state = _loadState();
      if (state != _lastState) {
        _lastState = state;
        if (onClick) {
          onClick(state);
        }
      }
    }

    int ButtonEvents::_loadState() {
      if (_buttonState == AWAIT_PRESS) {
        _clicks = 0;
        if (digitalRead(_inputPin) == LOW) {
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
        if (digitalRead(_inputPin) == HIGH) {
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
        if (digitalRead(_inputPin) == LOW) {
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

  }
}
