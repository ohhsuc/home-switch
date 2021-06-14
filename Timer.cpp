#include <vector>
#include "Timer.h"

namespace Victoria::Events {

  Timer::Timer() {
  }

  unsigned int Timer::setTimeout(int timespan, TCallback callback) {
    return _addConfig(false, timespan, callback);
  }

  unsigned int Timer::setInterval(int timespan, TCallback callback) {
    return _addConfig(true, timespan, callback);
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
      if (now - config.timestamp > config.timespan) {
        hitIds.push_back(item.first);
      }
    }
    for (const unsigned int id : hitIds) {
      Config& config = _configs[id];
      if (config.repeat) {
        config.timestamp = now;
      } else {
        _removeConfig(id);
      }
      if (config.callback) {
        config.callback();
      }
    }
  }

  unsigned int Timer::_addConfig(bool repeat, int timespan, TCallback callback) {
    Config config = {
      .repeat = repeat,
      .timespan = timespan,
      .callback = callback,
      .timestamp = millis(),
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

} // namespace Victoria::Events
