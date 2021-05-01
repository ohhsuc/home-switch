#include <ESP8266WiFi.h>
#include "TimesTrigger.h"

TimesTrigger::TimesTrigger(int times, int resetMillis) {
  _times = times;
  _resetMillis = resetMillis;
  _count = 0;
  _lastTime = 0;
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
  if (_count >= _times && onTimesOut) {
    _count = 0;
    onTimesOut();
  }
}
