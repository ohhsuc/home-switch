#include "Console.h"

namespace Victoria {

  Console::Console() {}

  void Console::begin(unsigned long baud) {
    Serial.begin(baud);
  }

  Console Console::log() {
    ts().writeType(F("log"));
    return console;
  }

  Console Console::log(const String& message) {
    log().write(message).newline();
    return console;
  }

  Console Console::error() {
    ts().writeType(F("error"));
    return console;
  }

  Console Console::error(const String& message) {
    error().write(message).newline();
    return console;
  }

  Console Console::debug() {
    ts().writeType(F("debug"));
    return console;
  }

  Console Console::debug(const String& message) {
    debug().write(message).newline();
    return console;
  }

  Console Console::ts() {
    auto now = millis();
    write(F("["));
    write(String(now));
    write(F("]"));
    return console;
  }

  Console Console::newline() {
    Serial.println(F(""));
    return console;
  }

  Console Console::writeType(const String& type) {
    write(F("["));
    write(type);
    write(F("]"));
    return console;
  }

  Console Console::write(const String& message) {
    Serial.print(message);
    return console;
  }

  // global
  Console console;

} // namespace Victoria
