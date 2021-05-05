#include "Arduino.h"
#include "Timer.h"

namespace Purl {
  namespace Events {

    Timer::Timer() {
    }

    int Timer::setTimeout(int delayMillis, Callback callback) {
      return _addConfig(true, delayMillis, callback);
    }

    int Timer::setInterval(int intervalMillis, Callback callback) {
      return _addConfig(false, intervalMillis, callback);
    }

    bool Timer::clearTimeout(int id) {
      return _removeConfig(id);
    }

    bool Timer::clearInterval(int id) {
      return _removeConfig(id);
    }

    void Timer::loop() {
      unsigned long now = millis();
      for (auto it = _config.begin(); it != _config.end(); ++it) {
        int id = it->first;
        Config config = it->second;
        if (now - config.time > config.ms) {
          if (config.type) {
            _removeConfig(id);
            _fireCallback(config.cb);
          } else {
            config.time = now;
            _fireCallback(config.cb);
          }
        }
      }
    }

    int Timer::_addConfig(bool type, int milliseconds, Callback callback) {
      Config config = {
        type: type,
        ms: milliseconds,
        cb: callback,
        time: millis(),
      };
      int id = _idSeed++;
      _config[id] = config;
      return id;
    }

    bool Timer::_removeConfig(int id) {
      if (_config.count(id) > 0) {
        _config.erase(id);
        return true;
      }
      return false;
    }

    void Timer::_fireCallback(Callback callback) {
      if (callback) {
        (*callback)();
      }
    }

  }
}
