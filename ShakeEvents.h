#ifndef ShakeEvents_h
#define ShakeEvents_h

#include "Arduino.h"

namespace Victoria {
  namespace Events {
    class ShakeEvents {
      public:
        ShakeEvents(uint8_t inputPin);
        typedef void (*ShakeEvent)();
        ShakeEvent onShake;
        void loop();
      private:
        uint8_t _inputPin;
        int _state = 0;
        bool _shaked = false;
        unsigned long _lastFire;
    };
  }
}

#endif //ShakeEvents_h
