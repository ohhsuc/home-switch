#ifndef ConfigStore_h
#define ConfigStore_h

#include "Commons.h"

namespace Victoria {
  namespace Components {
    class ConfigStore {
      public:
        ConfigStore();
        SettingModel load();
        bool save(SettingModel model);
    };
  }
}

#endif //ConfigStore_h
