#include <list>
#include "Arduino.h"
#include "Timer.h"

namespace Victoria {
  namespace Events {

    Timer::Timer() {
    }

    unsigned int Timer::setTimeout(unsigned short delayMillis, Callback callback) {
      return _addConfig(true, delayMillis, callback);
    }

    unsigned int Timer::setInterval(unsigned short intervalMillis, Callback callback) {
      return _addConfig(false, intervalMillis, callback);
    }

    bool Timer::clearTimeout(unsigned int id) {
      return _removeConfig(id);
    }

    bool Timer::clearInterval(unsigned int id) {
      return _removeConfig(id);
    }

    void Timer::loop() {
      std::list<unsigned int> hitIds;
      unsigned long now = millis();
      for (auto it = _configs.begin(); it != _configs.end(); ++it) {
        unsigned int id = it->first;
        Config& config = it->second;
        if (now - config.time > config.ms) {
          hitIds.push_back(id);
        }
      }
      for (unsigned int id : hitIds) {
        Config& config = _configs[id];
        if (config.type) {
          _removeConfig(id);
        } else {
          config.time = now;
        }
        _fireCallback(config.cb);
      }
    }

    unsigned int Timer::_addConfig(bool type, unsigned short milliseconds, Callback callback) {
      Config config = {
        type: type,
        ms: milliseconds,
        cb: callback,
        time: millis(),
      };
      unsigned int id = _idSeed++;
      _configs[id] = config;
      return id;
    }

    bool Timer::_removeConfig(unsigned int id) {
      if (_configs.count(id) > 0) {
        _configs.erase(id);
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
