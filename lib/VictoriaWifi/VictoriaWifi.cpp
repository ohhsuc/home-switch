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
        console.log("[Wifi] AP Address > " + currentApIp.toString());
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
    console.log("[Wifi] mode > WIFI_AP_STA");
  }

  void VictoriaWifi::join(String ssid, String password, bool waitForConnecting = true) {
    console.log("[Wifi] ssid > " + ssid);
    console.log("[Wifi] password > " + password);
    WiFi.persistent(true);
    WiFi.begin(ssid, password);
    if (waitForConnecting) {
      auto checkTimes = 60;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        console.write(".");
        if (checkTimes == 0) {
          break;
        } else {
          checkTimes--;
        }
      }
      console.newline();
    }
  }

  String VictoriaWifi::getHostName(bool includeVersion = false) {
    auto id = WiFi.macAddress();
    id.replace(":", "");
    id.toUpperCase();
    id = id.substring(id.length() - 6);

    auto version = String(FirmwareVersion);
    version.replace(".", "");

    auto model = appStorage.load();
    auto productName = model.name.length() > 0
      ? model.name
      : FirmwareName;

    auto hostName = includeVersion
      ? productName + "-" + id + "-" + version
      : productName + "-" + id;

    return hostName;
  }

  void VictoriaWifi::_onWifiEvent(WiFiEvent_t event) {
    switch (event) {
      case WiFiEvent::WIFI_EVENT_STAMODE_CONNECTED: {
        console.log("[Wifi] event > STA connected");
        break;
      }
      case WiFiEvent::WIFI_EVENT_STAMODE_DISCONNECTED: {
        console.log("[Wifi] event > STA disconnected");
        break;
      }
      case WiFiEvent::WIFI_EVENT_STAMODE_GOT_IP: {
        console.log("[Wifi] event > STA got ip");
        break;
      }
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STACONNECTED: {
        console.log("[Wifi] event > AP connected");
        break;
      }
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STADISCONNECTED: {
        console.log("[Wifi] event > AP disconnected");
        break;
      }
      default: {
        break;
      }
    }
  }

} // namespace Victoria::Components
