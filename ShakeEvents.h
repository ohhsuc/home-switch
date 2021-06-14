#ifndef ShakeEvents_h
#define ShakeEvents_h

#include <functional>
#include <Arduino.h>

namespace Victoria::Events {
  class ShakeEvents {
    typedef std::function<void()> TShakeHandler;
    public:
      ShakeEvents(uint8_t inputPin);
      TShakeHandler onShake;
      void loop();
    private:
      uint8_t _inputPin;
      int _inputState = 0;
      bool _shaked = false;
      unsigned long _lastFire;
  };
} // namespace Victoria::Events

#endif // ShakeEvents_h
