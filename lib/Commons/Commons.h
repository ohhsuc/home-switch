#ifndef Commons_h
#define Commons_h

#include <map>
#include <vector>
#include <Arduino.h>
#include "Console.h"
#include "BuildFlags.h"
#include "GlobalHelpers.h"

namespace Victoria {

  enum VEnvironment {
    VTest,
    VProd,
  };

  struct TableModel {
    std::vector<String> header;
    std::vector<std::vector<String>> rows;
  };

  struct SelectOption {
    String value;
    String text;
  };

  struct SelectModel {
    String name;
    String value;
    std::vector<SelectOption> options;
  };

  struct RadioMessage {
    unsigned long value;
    unsigned int bits;
    unsigned int protocol;
    unsigned long timestamp;
  };

  enum RadioAction {
    RadioActionNone = 0,
    RadioActionTrue = 1,
    RadioActionFalse = 2,
    RadioActionToggle = 3,
    RadioActionWiFiSta = 4,
    RadioActionWiFiStaAp = 5,
    RadioActionWiFiReset = 6,
    RadioActionEspRestart = 7,
  };

  enum PressType {
    PressTypeClick = 0,
    PressTypeDoubleClick = 1,
    PressTypeLongPress = 2,
  };

  struct RadioRule {
    unsigned long value;
    unsigned int protocol;
    PressType press;
    RadioAction action;
    String serviceId;
  };

  struct RadioModel {
    int inputPin = -1;
    std::vector<RadioRule> rules;
  };

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
    bool boolValue;
    int intValue;
  };

  struct ServicesModel {
    std::map<String, ServiceSetting> services;
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
