#ifndef OnOffEvents_h
#define OnOffEvents_h

#define DEFAULT_DEBOUNCE_DELAY 50

namespace Victoria {
  namespace Events {
    class OnOffEvents {
      public:
        OnOffEvents(uint8_t inputPin);
        typedef void (*ToggleEvent)(bool);
        ToggleEvent onToggle;
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
