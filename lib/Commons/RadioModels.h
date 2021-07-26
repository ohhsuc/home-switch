#ifndef RadioModels_h
#define RadioModels_h

#include <vector>
#include <Arduino.h>

namespace Victoria {

  struct RadioMessage {
    String value;
    unsigned int channel = 0;
    unsigned long timestamp = 0;
  };

  enum RadioAction {
    RadioActionNone = 0,
    RadioActionTrue = 1,
    RadioActionFalse = 2,
    RadioActionToggle = 3,
    RadioActionWiFiSta = 4,
    RadioActionWiFiStaAp = 5,
    RadioActionWiFiReset = 6,
    RadioActionEspRestart = 7,
  };

  enum RadioPressState {
    PressStateAwait = 0,
    PressStateClick = 1,
    PressStateDoubleClick = 2,
    PressStateLongPress = 3,
  };

  struct RadioRule {
    String value;
    unsigned int channel = 0;
    RadioPressState press = PressStateClick;
    RadioAction action = RadioActionNone;
    String serviceId;
  };

  struct RadioModel {
    int inputPin = -1;
    std::vector<RadioRule> rules;
  };

} // namespace Victoria

#endif // RadioModels_h
