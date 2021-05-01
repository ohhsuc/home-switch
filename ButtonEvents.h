// Migrate from
// https://www.sigmdel.ca/michel/program/esp8266/arduino/switch_debouncing_en.html

#define DEFAULT_DEBOUNCE_PRESS_TIME      30
#define DEFAULT_DEBOUNCE_RELEASE_TIME    60
#define DEFAULT_MULTI_CLICK_TIME        400
#define DEFAULT_HOLD_TIME              2000

class ButtonEvents {
  public:
    ButtonEvents(uint8_t pin);
    enum ButtonState { NONE_STATE, AWAIT_PRESS, DEBOUNCE_PRESS, AWAIT_RELEASE, DEBOUNCE_RELEASE, AWAIT_MULTI_PRESS };
    typedef void (*ButtonEvent)(ButtonState);
    ButtonEvent onTrigger;
    void loop();

  private:
    uint8_t _pin;
    uint16_t _debouncePressTime   = DEFAULT_DEBOUNCE_PRESS_TIME;
    uint16_t _debounceReleaseTime = DEFAULT_DEBOUNCE_RELEASE_TIME;
    uint16_t _multiClickTime      = DEFAULT_MULTI_CLICK_TIME;
    uint16_t _holdTime            = DEFAULT_HOLD_TIME;
    // states
    ButtonState _buttonState      = AWAIT_PRESS;
    ButtonState _lastButtonState  = NONE_STATE;
    long _eventTime               = 0;
    int _clicks                   = 0;
    // status, number of clicks since last update
    // -1 = button held, 0 = button up, 1, 2, ... number of times button clicked
    int _loadState();
};
