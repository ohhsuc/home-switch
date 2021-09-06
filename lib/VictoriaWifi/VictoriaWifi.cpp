#include "VictoriaWifi.h"

namespace Victoria::Components {

  void VictoriaWifi::setup() {
    auto wifiMode = WiFi.getMode();
    auto apEnabled = ((wifiMode & WIFI_AP) != 0);
    auto staEnabled = ((wifiMode & WIFI_STA) != 0);
    if (!apEnabled && !staEnabled) {
      WiFi.mode(WIFI_AP_STA);
      wifiMode = WIFI_AP_STA;
    }

    auto hostName = getHostName(true);
    auto isApEnabled = ((wifiMode & WIFI_AP) != 0);
    if (isApEnabled) {
      // IPAddress apIp(192, 168, 1, 33);
      // IPAddress apSubnet(255, 255, 255, 0);
      // WiFi.softAPConfig(apIp, apIp, apSubnet);
      WiFi.softAP(hostName); // name which is displayed on AP list
      auto currentApIp = WiFi.softAPIP();
      if (currentApIp) {
        console.log().write(F("[Wifi] AP Address > ")).write(currentApIp.toString()).newline();
      }
    }

    WiFi.hostname(hostName); // name which is displayed on router
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin();
    WiFi.onEvent(VictoriaWifi::_onWifiEvent, WiFiEvent::WIFI_EVENT_ANY);
  }

  void VictoriaWifi::reset() {
    // wifi_config_reset();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP_STA);
    console.log(F("[Wifi] mode > WIFI_AP_STA"));
  }

  void VictoriaWifi::join(String ssid, String password, bool waitForConnecting) {
    console.log().write(F("[Wifi] ssid > ")).write(ssid).newline();
    console.log().write(F("[Wifi] password > ")).write(password).newline();
    WiFi.persistent(true);
    WiFi.begin(ssid, password);
    if (waitForConnecting) {
      auto checkTimes = 60;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        console.write(F("."));
        if (checkTimes == 0) {
          break;
        } else {
          checkTimes--;
        }
      }
      console.newline();
    }
  }

  String VictoriaWifi::getHostId() {
    auto id = WiFi.macAddress();
    id.replace(F(":"), F(""));
    id.toUpperCase();
    id = id.substring(id.length() - 6);
    return id;
  }

  String VictoriaWifi::getHostName(bool includeVersion) {
    auto id = getHostId();

    auto version = String(FirmwareVersion);
    version.replace(F("."), F(""));

    auto model = appStorage.load();
    auto productName = model.name.length() > 0
      ? model.name
      : FirmwareName;

    auto hostName = includeVersion
      ? productName + F("-") + id + F("-") + version
      : productName + F("-") + id;

    return hostName;
  }

  void VictoriaWifi::_onWifiEvent(WiFiEvent_t event) {
    switch (event) {
      case WiFiEvent::WIFI_EVENT_STAMODE_CONNECTED: {
        console.log(F("[Wifi] event > STA connected"));
        break;
      }
      case WiFiEvent::WIFI_EVENT_STAMODE_DISCONNECTED: {
        console.log(F("[Wifi] event > STA disconnected"));
        break;
      }
      case WiFiEvent::WIFI_EVENT_STAMODE_GOT_IP: {
        console.log(F("[Wifi] event > STA got ip"));
        break;
      }
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STACONNECTED: {
        console.log(F("[Wifi] event > AP connected"));
        break;
      }
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STADISCONNECTED: {
        console.log(F("[Wifi] event > AP disconnected"));
        break;
      }
      default: {
        break;
      }
    }
  }

} // namespace Victoria::Components
