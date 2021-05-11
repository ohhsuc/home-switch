#define DEFAULT_DEBOUNCE_DELAY 50

namespace Purl {
  namespace Events {
    class OnOffEvents {
      public:
        OnOffEvents(uint8_t pin);
        typedef void (*ToggleEvent)(bool);
        ToggleEvent onToggle;
        void loop();
      private:
        uint8_t _pin;
        bool _lastState = false;
        unsigned long _lastTimeRead = 0;
        uint8_t _debounceDelay = DEFAULT_DEBOUNCE_DELAY;
    };
  }
}
