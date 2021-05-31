#ifndef OnOffEvents_h
#define OnOffEvents_h

#include <functional>
#include <Arduino.h>

#define DEFAULT_DEBOUNCE_DELAY 50

namespace Victoria {
  namespace Events {
    class OnOffEvents {
      typedef std::function<void(bool)> TToggleHandler;
      public:
        OnOffEvents(uint8_t inputPin);
        TToggleHandler onToggle;
        void loop();
      private:
        uint8_t _inputPin;
        bool _lastState = false;
        unsigned long _lastTimeRead = 0;
        uint8_t _debounceDelay = DEFAULT_DEBOUNCE_DELAY;
    };
  }
}

#endif //OnOffEvents_h
