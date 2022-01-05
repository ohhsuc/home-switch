#ifndef TimesCounter_h
#define TimesCounter_h

#include <functional>
#include <Arduino.h>

namespace Victor::Events {
  class TimesCounter {
    typedef std::function<void()> TTimesOutHandler;

   public:
    TimesCounter(int times, unsigned long resetMillis);
    TTimesOutHandler onOut;
    void count();

   private:
    // args
    int _times;
    unsigned long _resetMillis;
    // state
    int _count = 0;
    unsigned long _lastTime = 0;
  };
} // namespace Victor::Events

#endif // TimesCounter_h
