#include <map>

namespace Victoria {
  namespace Events {
    class Timer {
      public:
        Timer();
        typedef void (*Callback)();
        struct Config {
          bool type; // true for setTimeout, otherwise setInterval
          unsigned short ms; // delay or interval in milliseconds
          Callback cb; // callback function
          unsigned long time; // timestamp when timer registered
        };
        unsigned int setTimeout(unsigned short delayMillis, Callback callback);
        unsigned int setInterval(unsigned short intervalMillis, Callback callback);
        bool clearTimeout(unsigned int id);
        bool clearInterval(unsigned int id);
        void loop();
      private:
        unsigned int _idSeed = 0;
        std::map<unsigned int, Config> _configs;
        unsigned int _addConfig(bool type, unsigned short milliseconds, Callback callback);
        bool _removeConfig(unsigned int id);
        void _fireCallback(Callback callback);
    };
  }
}
