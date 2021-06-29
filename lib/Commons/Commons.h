#ifndef Commons_h
#define Commons_h

#include <map>
#include <vector>
#include <Arduino.h>
#include "Console.h"
#include "BuildFlags.h"

namespace Victoria {

  enum VEnvironment {
    VTest,
    VProd,
  };

  struct TableModel {
    std::vector<String> header;
    std::vector<std::vector<String>> rows;
  };

  enum ServiceType {
    EmptyServiceType = 0,
    BooleanServiceType = 1,
    IntegerServiceType = 2,
  };

  struct ServiceSetting {
    String name;
    ServiceType type;
    int outputIO;
    int inputIO;
    int outputLevel;
    int inputLevel;
  };

  struct ServiceState {
    bool boolValue;
    int intValue;
  };

  struct SettingModel {
    std::map<String, ServiceSetting> services;
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
  const uint8_t V_TXD = 1;   // TXD (Transmitter)
  const uint8_t V_RXD = 3;   // RXD (Receiver)

  // globals
  const VEnvironment VEnv = VTest;
} // namespace Victoria

#endif // Commons_h
