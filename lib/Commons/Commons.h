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
    RadioActionWiFiApSta = 5,
  };

  struct RadioRule {
    unsigned long value;
    unsigned int protocol;
    RadioAction action;
    String serviceId;
  };

  struct RadioModel {
    int inputPin;
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
    int outputPin;
    int inputPin;
    int outputLevel;
    int inputLevel;
  };

  struct ServiceState {
    bool boolValue;
    int intValue;
  };

  struct ServicesModel {
    std::map<String, ServiceSetting> services;
  };

  class CommonHelpers {
    public:
      static String randomString(int length) {
        auto result = String("");
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
      static String timeSince(unsigned long timestamp) {
        float s = (millis() - timestamp) / 1000;
        float timespan;
        // y
        timespan = s / 31536000;
        if (timespan > 1) {
          int years = floor(timespan);
          return String(years) + " years";
        }
        // m
        timespan = s / 2592000;
        if (timespan > 1) {
          int months = floor(timespan);
          return String(months) + " months";
        }
        // d
        timespan = s / 86400;
        if (timespan > 1) {
          int days = floor(timespan);
          return String(days) + " days";
        }
        // h
        timespan = s / 3600;
        if (timespan > 1) {
          int hours = floor(timespan);
          return String(hours) + " hours";
        }
        // m
        timespan = s / 60;
        if (timespan > 1) {
          int minutes = floor(timespan);
          return String(minutes) + " minutes";
        }
        // s
        int seconds = floor(s);
        return String(seconds) + " seconds";
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
