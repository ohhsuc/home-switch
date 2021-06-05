#ifndef OnOffEvents_h
#define OnOffEvents_h

#include <functional>
#include <Arduino.h>

namespace Victoria {
  namespace Events {
    class OnOffEvents {
      typedef std::function<void(const String&, bool)> TToggleHandler;
      public:
        OnOffEvents(String accessoryId, uint8_t inputPin);
        TToggleHandler onToggle;
        void loop();
      private:
        String _accessoryId;
        uint8_t _inputPin;
        bool _lastState = false;
        unsigned long _lastTimeRead = 0;
    };
  }
}

#endif //OnOffEvents_h
