#ifndef SystemHelpers_h
#define SystemHelpers_h

#include <Arduino.h>

namespace Victoria {

  class GlobalHelpers {
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

    static unsigned char h2int(char c) {
      if (c >= '0' && c <= '9') {
        return ((unsigned char)c - '0');
      }
      if (c >= 'a' && c <= 'f') {
        return ((unsigned char)c - 'a' + 10);
      }
      if (c >= 'A' && c <= 'F') {
        return ((unsigned char)c - 'A' + 10);
      }
      return (0);
    }

    static String urlDecode(String str) {
      auto encodedString = String("");
      char c;
      char code0;
      char code1;
      for (unsigned int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == '+') {
          encodedString += ' ';
        } else if (c == '%') {
          i++;
          code0 = str.charAt(i);
          i++;
          code1 = str.charAt(i);
          c = (h2int(code0) << 4) | h2int(code1);
          encodedString += c;
        } else {
          encodedString += c;
        }
        yield();
      }
      return encodedString;
    }

    static String urlEncode(String str) {
      auto encodedString = String("");
      char c;
      char code0;
      char code1;
      // char code2;
      for (unsigned int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
          encodedString += '+';
        } else if (isalnum(c)) {
          encodedString += c;
        } else {
          code1 = (c & 0xf) + '0';
          if ((c & 0xf) > 9) {
            code1 = (c & 0xf) - 10 + 'A';
          }
          c = (c >> 4) & 0xf;
          code0 = c + '0';
          if (c > 9) {
            code0 = c - 10 + 'A';
          }
          // code2 = '\0';
          encodedString += '%';
          encodedString += code0;
          encodedString += code1;
          // encodedString += code2;
        }
        yield();
      }
      return encodedString;
    }
  };

} // namespace Victoria

#endif // SystemHelpers_h
