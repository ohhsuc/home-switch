#include <vector>
#include "Timer.h"

namespace Victoria {
  namespace Events {

    Timer::Timer() {
    }

    unsigned int Timer::setTimeout(unsigned short delayMillis, TCallback callback) {
      return _addConfig(true, delayMillis, callback);
    }

    unsigned int Timer::setInterval(unsigned short intervalMillis, TCallback callback) {
      return _addConfig(false, intervalMillis, callback);
    }

    bool Timer::clearTimeout(unsigned int id) {
      return _removeConfig(id);
    }

    bool Timer::clearInterval(unsigned int id) {
      return _removeConfig(id);
    }

    void Timer::loop() {
      std::vector<unsigned int> hitIds;
      unsigned long now = millis();
      for (const auto& item : _configs) {
        Config config = item.second;
        if (now - config.time > config.ms) {
          hitIds.push_back(item.first);
        }
      }
      for (const unsigned int id : hitIds) {
        Config& config = _configs[id];
        if (config.type) {
          _removeConfig(id);
        } else {
          config.time = now;
        }
        if (config.cb) {
          config.cb();
        }
      }
    }

    unsigned int Timer::_addConfig(bool type, unsigned short milliseconds, TCallback callback) {
      Config config = {
        .type = type,
        .ms = milliseconds,
        .cb = callback,
        .time = millis(),
      };
      _idSeed = _idSeed + 1;
      _configs[_idSeed] = config;
      return _idSeed;
    }

    bool Timer::_removeConfig(unsigned int id) {
      if (_configs.count(id) > 0) {
        _configs.erase(id);
        return true;
      }
      return false;
    }

  }
}
