#ifndef TimesCounter_h
#define TimesCounter_h

#include <functional>
#include <Arduino.h>

namespace Victor::Events {
  class TimesCounter {
    typedef std::function<void()> TTimesOutHandler;

   public:
    TimesCounter(uint8_t times, unsigned long resetMillis);
    TTimesOutHandler onOut;
    void count();

   private:
    // args
    uint8_t _times;
    unsigned long _resetMillis;
    // state
    uint8_t _count = 0;
    unsigned long _lastTime = 0;
  };
} // namespace Victor::Events

#endif // TimesCounter_h
