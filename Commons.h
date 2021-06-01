#ifndef Commons_h
#define Commons_h

#include <map>
#include <Arduino.h>

namespace Victoria {

  enum VEnvironment {
    VTest,
    VProd,
  };

  const VEnvironment VEnv = VTest;

  enum AccessoryType {
    EmptyAccessoryType = 0,
    BooleanAccessoryType = 1,
    IntegerAccessoryType = 2,
  };

  struct AccessorySetting {
    String name;
    AccessoryType type;
    short int outputIO;
    short int inputIO;
    short int outputLevel;
    short int inputLevel;
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

  // const int led = LED_BUILTIN;
  const uint8_t V_GPIO0 = 0; // GPIO-0
  const uint8_t V_GPIO2 = 2; // GPIO-2 (Led Builtin)
  const uint8_t V_TXD = 1; // TXD (Transmitter)
  const uint8_t V_RXD = 3; // RXD (Receiver)
}

#endif //Commons_h
