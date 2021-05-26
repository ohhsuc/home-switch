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

  struct AccessoryState {
    String name;
    AccessoryType type;
    uint8_t outputIO;
    uint8_t inputIO;
    bool boolValue;
    int intValue;
  };

  struct SettingModel {
    std::map<String, AccessoryState> states;
    // ... other items

    void deserializeFrom(StaticJsonDocument<256>& doc) {
      auto statesDoc = doc["s"];
      if (statesDoc) {
        int index = -1;
        while (true) {
          auto item = statesDoc[++index];
          if (!item || !item[0]) {
            break;
          }
          int type = item[2];
          AccessoryState state = {
            name: item[1],
            type: AccessoryType(type), // convert int to enum
            outputIO: item[3],
            inputIO: item[4],
          };
          String id = item[0];
          states[id] = state;
        }
      }
    }

    void serializeTo(StaticJsonDocument<256>& doc) {
      int i = 0;
      for (auto pair : states) {
        String id = pair.first;
        AccessoryState state = pair.second;
        int type = state.type; // convert enum to int
        doc["s"][i][0] = id;
        doc["s"][i][1] = state.name;
        doc["s"][i][2] = type;
        doc["s"][i][3] = state.outputIO;
        doc["s"][i][4] = state.inputIO;
        i++;
      }
    }
  };

}

#endif //Commons_h
