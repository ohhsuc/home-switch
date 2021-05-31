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
        unsigned int setTimeout(unsigned short delayMillis, TCallback callback);
        unsigned int setInterval(unsigned short intervalMillis, TCallback callback);
        bool clearTimeout(unsigned int id);
        bool clearInterval(unsigned int id);
        void loop();
      private:
        struct Config {
          bool type; // true for setTimeout, otherwise setInterval
          unsigned short ms; // delay or interval in milliseconds
          TCallback cb; // callback function
          unsigned long time; // timestamp when timer registered
        };
        unsigned int _idSeed = 0;
        std::map<unsigned int, Config> _configs;
        unsigned int _addConfig(bool type, unsigned short milliseconds, TCallback callback);
        bool _removeConfig(unsigned int id);
    };
  }
}

#endif //Timer_h
