#include <map>

namespace Purl {
  namespace Events {
    class Timer {
      public:
        Timer();
        typedef void (*Callback)();
        struct Config {
          bool type; // true for setTimeout, otherwise setInterval
          int ms; // delay or interval in milliseconds
          Callback cb; // callback function
          unsigned long time; // timestamp when timer registered
        };
        int setTimeout(int delayMillis, Callback callback);
        int setInterval(int intervalMillis, Callback callback);
        bool clearTimeout(int id);
        bool clearInterval(int id);
        void loop();
      private:
        int _idSeed = 0;
        std::map<int, Config> _config;
        int _addConfig(bool type, int milliseconds, Callback callback);
        bool _removeConfig(int id);
        void _fireCallback(Callback callback);
    };
  }
}
