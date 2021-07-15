#ifndef ButtonEvents_h
#define ButtonEvents_h

#include <functional>
#include <Arduino.h>

// Migrate from
// https://www.sigmdel.ca/michel/program/esp8266/arduino/switch_debouncing_en.html

#define DEFAULT_DEBOUNCE_PRESS_TIME   15
#define DEFAULT_DEBOUNCE_RELEASE_TIME 30
#define DEFAULT_MULTI_CLICK_TIME     400
#define DEFAULT_HOLD_TIME           2000

namespace Victoria::Events {
  class ButtonEvents {
    typedef std::function<void(int)> TClickHandler;

   public:
    ButtonEvents(uint8_t inputPin);
    TClickHandler onClick;
    void loop();

   private:
    uint8_t _inputPin;
    unsigned int _debouncePressTime   = DEFAULT_DEBOUNCE_PRESS_TIME;
    unsigned int _debounceReleaseTime = DEFAULT_DEBOUNCE_RELEASE_TIME;
    unsigned int _multiClickTime      = DEFAULT_MULTI_CLICK_TIME;
    unsigned int _holdTime            = DEFAULT_HOLD_TIME;
    // states
    enum ButtonState { AWAIT_PRESS, DEBOUNCE_PRESS, AWAIT_RELEASE, DEBOUNCE_RELEASE, AWAIT_MULTI_PRESS, DEBOUNCE_MULTI_PRESS };
    ButtonState _buttonState = AWAIT_PRESS;
    unsigned long _eventTime = 0;
    // status, number of clicks since last update
    // -1 = button held, 0 = button up, 1, 2, ... number of times button clicked
    int _loadState();
    int _lastState = 0;
  };
} // namespace Victoria::Events

#endif // ButtonEvents_h
