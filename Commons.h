#ifndef Commons_h
#define Commons_h

#include <map>
#include <Arduino.h>

namespace Victoria {

  enum AccessoryType {
    EmptyAccessoryType,
    BooleanAccessoryType,
    IntegerAccessoryType,
  };

  struct AccessorySetting {
    String name;
    AccessoryType type;
    uint8_t outputIO;
    uint8_t inputIO;
  };

  struct AccessoryState {
    bool boolValue;
    int intValue;
  };

  struct SettingModel {
    std::map<String, AccessorySetting> settings;
    // ... other items
  };

  class CommonHelpers {
    public:
      static String randomString(int length) {
        String result = "";
        int generated = 0;
        while (generated < length) {
          byte randomValue = random(0, 26);
          char letter = randomValue + 'a';
          if (randomValue > 26) {
            letter = (randomValue - 26);
          }
          result += letter;
          generated++;
        }
        return result;
      }
  };
}

#endif //Commons_h
