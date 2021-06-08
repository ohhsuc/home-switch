#ifndef Console_h
#define Console_h

#include <Arduino.h>

namespace Victoria {
  class Console {
    public:
      Console();
      void begin(unsigned long baud);
      void log(String log);
      void error(String error);
      void debug(String debug);
      void write(String message);
      void write(String message, bool newline);
    private:
      static String _format(String type, String message);
  };

  // global
  extern Console console;
}

#endif //Console_h
