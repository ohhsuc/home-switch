#ifndef Commons_h
#define Commons_h

#include <vector>
#include "Arduino.h"
#include <ArduinoJson.h>

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
    uint8_t outputIO;
    uint8_t inputIO;
    bool boolValue;
    int intValue;
  };

  struct SettingModel {
    std::vector<AccessoryState> states;
    // ... other items

    void deserializeFrom(StaticJsonDocument<256> doc) {
      auto statesDoc = doc["s"];
      if (statesDoc) {
        int index = -1;
        while (true) {
          auto item = statesDoc[++index];
          if (!item || !item[0]) {
            break;
          }
          int type = item[2];
          states.push_back({
            id: item[0],
            name: item[1],
            type: AccessoryType(type), // convert int to enum
            outputIO: item[3],
            inputIO: item[4],
          });
        }
      }
    }

    void serializeTo(StaticJsonDocument<256>& doc) {
      for (int i = 0; i < states.size(); i++) {
        AccessoryState item = states[i];
        int type = item.type; // convert enum to int
        doc["s"][i][0] = item.id;
        doc["s"][i][1] = item.name;
        doc["s"][i][2] = type;
        doc["s"][i][3] = item.outputIO;
        doc["s"][i][4] = item.inputIO;
      }
    }
  };

}

#endif //Commons_h
