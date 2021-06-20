#include "TimesTrigger.h"

namespace Victoria::Events {

  TimesTrigger::TimesTrigger(int times, int resetMillis) {
    _times = times;
    _resetMillis = resetMillis;
  }

  void TimesTrigger::count() {
    // https://www.arduino.cc/reference/en/language/functions/time/millis/
    // Returns the number of milliseconds passed since the Arduino board began running the current program.
    // This number will overflow (go back to zero), after approximately 50 days.
    unsigned long now = millis();
    if (now - _lastTime >= _resetMillis) {
      _count = 0;
    }
    _count++;
    _lastTime = now;
    if (_count >= _times) {
      _count = 0;
      if (onTimesOut) {
        onTimesOut();
      }
    }
  }

} // namespace Victoria::Events
