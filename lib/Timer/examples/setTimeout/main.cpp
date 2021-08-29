#include <Arduino.h>
#include <Timer.h">

using namespace Victoria::Events;

void setup() {
  timer.setTimeout(2 * 1000, []() {
    Serial.println("Func is fired after 2 seconds")
  });
}

void loop() {
  timer.loop();
}
