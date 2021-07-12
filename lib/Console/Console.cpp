#include "Console.h"

namespace Victoria {

  Console::Console() {}

  void Console::begin(unsigned long baud) {
    Serial.begin(baud);
  }

  void Console::log(const String& log) {
    write(_format("log", log), true);
  }

  void Console::error(const String& error) {
    write(_format("error", error), true);
  }

  void Console::debug(const String& debug) {
    write(_format("debug", debug), true);
  }

  void Console::write(const String& message) {
    write(message, false);
  }

  void Console::newline() {
    write("", true);
  }

  void Console::write(const String& message, bool newline) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
  }

  String Console::_format(const String& type, const String& message) {
    auto now = millis();
    auto content = "[" + String(now) + "][" + type + "] " + message;
    return content;
  }

  // global
  Console console;

} // namespace Victoria
