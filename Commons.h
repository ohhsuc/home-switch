#ifndef Commons_h
#define Commons_h

#include <map>
#include "Arduino.h"
#include <ArduinoJson.h>

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

    void deserializeFrom(const StaticJsonDocument<256>& doc) {
      auto settingsDoc = doc["s"];
      if (settingsDoc) {
        int index = -1;
        while (true) {
          auto item = settingsDoc[++index];
          if (!item || !item[0]) {
            break;
          }
          int type = item[2];
          AccessorySetting setting = {
            .name = item[1],
            .type = AccessoryType(type), // convert int to enum
            .outputIO = item[3],
            .inputIO = item[4],
          };
          String id = item[0];
          settings[id] = setting;
        }
      }
    }

    void serializeTo(StaticJsonDocument<256>& doc) {
      int i = 0;
      for (const auto& pair : settings) {
        String id = pair.first;
        AccessorySetting setting = pair.second;
        int type = setting.type; // convert enum to int
        doc["s"][i][0] = id;
        doc["s"][i][1] = setting.name;
        doc["s"][i][2] = type;
        doc["s"][i][3] = setting.outputIO;
        doc["s"][i][4] = setting.inputIO;
        i++;
      }
    }
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
