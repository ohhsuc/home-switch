#ifndef TimesTrigger_h
#define TimesTrigger_h

#include <functional>
#include <Arduino.h>

namespace Victoria::Events {
  class TimesTrigger {
    typedef std::function<void()> TTimesOutHandler;

   public:
    TimesTrigger(int times, unsigned long resetMillis);
    TTimesOutHandler onTimesOut;
    void count();

   private:
    int _times;
    unsigned long _resetMillis;
    // state
    int _count = 0;
    unsigned long _lastTime = 0;
  };
} // namespace Victoria::Events

#endif // TimesTrigger_h
