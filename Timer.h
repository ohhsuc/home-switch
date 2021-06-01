#ifndef Timer_h
#define Timer_h

#include <map>
#include <functional>
#include <Arduino.h>

namespace Victoria {
  namespace Events {
    class Timer {
      typedef std::function<void()> TCallback;
      public:
        Timer();
        unsigned int setTimeout(unsigned short timespan, TCallback callback);
        unsigned int setInterval(unsigned short timespan, TCallback callback);
        bool clearTimeout(unsigned int id);
        bool clearInterval(unsigned int id);
        void loop();
      private:
        struct Config {
          bool repeat; // true for setInterval, otherwise setTimeout
          unsigned short timespan; // delay or interval in milliseconds
          TCallback callback; // callback function
          unsigned long timestamp; // timestamp when timer registered
        };
        unsigned int _idSeed = 0;
        std::map<unsigned int, Config> _configs;
        unsigned int _addConfig(bool repeat, unsigned short timespan, TCallback callback);
        bool _removeConfig(unsigned int id);
    };
  }
}

#endif //Timer_h
