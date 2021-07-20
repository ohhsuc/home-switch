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
    int outputPin = -1;
    int inputPin = -1;
    int outputLevel = -1;
    int inputLevel = -1;
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
