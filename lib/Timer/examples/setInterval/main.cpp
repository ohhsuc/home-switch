#include <Arduino.h>
#include <Timer.h">

using namespace Victoria::Events;

void setup() {
  timer.setInterval(2 * 1000, []() {
    Serial.println("Func is fired every 2 seconds")
  });
}

void loop() {
  timer.loop();
}
