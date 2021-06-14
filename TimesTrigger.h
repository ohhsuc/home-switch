#ifndef TimesTrigger_h
#define TimesTrigger_h

#include <functional>
#include <Arduino.h>

namespace Victoria::Events {
  class TimesTrigger {
    typedef std::function<void()> TTimesOutHandler;
    public:
      TimesTrigger(int times, int resetMillis);
      TTimesOutHandler onTimesOut;
      void count();
    private:
      int _times;
      int _resetMillis;
      // state
      int _count = 0;
      unsigned long _lastTime = 0;
  };
}

#endif //TimesTrigger_h
