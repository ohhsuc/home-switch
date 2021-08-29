#ifndef ServiceModels_h
#define ServiceModels_h

#include <map>
#include <Arduino.h>

namespace Victoria {

  enum ServiceType {
    EmptyServiceType = 0,
    BooleanServiceType = 1,
    IntegerServiceType = 2,
  };

  struct ServiceSetting {
    String name;
    ServiceType type;
    int8_t inputPin = -1;
    int8_t outputPin = -1;
    uint8_t inputTrueValue = 0;  // LOW
    uint8_t outputTrueValue = 0; // LOW
  };

  struct ServiceState {
    bool boolValue = false;
    int intValue = 0;
  };

  struct ServicesModel {
    std::map<String, ServiceSetting> services;
  };

} // namespace Victoria

#endif // ServiceModels_h
