#ifndef Commons_h
#define Commons_h

#include <vector>
#include "Arduino.h"

namespace Victoria {

  enum AccessoryType {
    EmptyAccessoryType,
    BooleanAccessoryType,
    IntegerAccessoryType,
  };

  struct AccessoryState {
    String id;
    String name;
    AccessoryType type;
    bool boolValue;
    int intValue;
  };

}

#endif //Commons_h
