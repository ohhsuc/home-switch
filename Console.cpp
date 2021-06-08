#include "Console.h"

namespace Victoria {

  Console::Console() { }

  void Console::begin(unsigned long baud) {
    Serial.begin(baud);
  }

  void Console::log(String log) {
    write(_format("log", log), true);
  }

  void Console::error(String error) {
    write(_format("error", error), true);
  }

  void Console::debug(String debug) {
    write(_format("debug", debug), true);
  }

  void Console::write(String message) {
    write(message, false);
  }

  void Console::write(String message, bool newline) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
  }

  String Console::_format(String type, String message) {
    auto now = millis();
    auto content = "[" + String(now) + "][" + type + "] " + message;
    return content;
  }

  // global
  Console console;

}
