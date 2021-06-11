#ifndef Console_h
#define Console_h

#include <Arduino.h>

namespace Victoria {
  class Console {
    public:
      Console();
      void begin(unsigned long baud);
      void log(const String& log);
      void error(const String& error);
      void debug(const String& debug);
      void newline();
      void write(const String& message);
      void write(const String& message, bool newline);
    private:
      static String _format(const String& type, const String& message);
  };

  // global
  extern Console console;
}

#endif //Console_h
