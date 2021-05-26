#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WebServer.h"

namespace Victoria {
  namespace Components {

    WebServer::WebServer(int port, String productName, String hostName) {
      _server = new ESP8266WebServer(port);
      _productName = productName;
      _hostName = hostName;
    }

    WebServer::~WebServer() {
      _server->stop();
    }

    void WebServer::setup() {
      WiFiMode_t wifiMode = WiFi.getMode();
      bool apEnabled = ((wifiMode & WIFI_AP) != 0);
      bool staEnabled = ((wifiMode & WIFI_STA) != 0);
      if (!apEnabled && !staEnabled) {
        WiFi.mode(WIFI_AP_STA);
        wifiMode = WIFI_AP_STA;
      }
      Serial.println("Wifi mode: WIFI_AP_STA");

      WiFi.hostname(_hostName);
      WiFi.setAutoConnect(true);
      WiFi.setAutoReconnect(true);

      bool isApEnabled = ((wifiMode & WIFI_AP) != 0);
      if (isApEnabled) {
        // IPAddress apIp(192, 168, 1, 33);
        // IPAddress apSubnet(255, 255, 255, 0);
        // WiFi.softAPConfig(apIp, apIp, apSubnet);
        WiFi.softAP(_hostName);

        IPAddress currentApIp = WiFi.softAPIP();
        if (currentApIp) {
          Serial.println("AP IP address: " + currentApIp.toString());
        }
      }

      _server->on("/", HTTP_GET, std::bind(&WebServer::_handleRoot, this));
      _server->on("/list-wifi", HTTP_GET, std::bind(&WebServer::_handleListWifi, this));
      _server->on("/connect-wifi", HTTP_POST, std::bind(&WebServer::_handleConnectWifi, this));
      _server->on("/accessory", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
      _server->on("/accessory", HTTP_ANY, std::bind(&WebServer::_handleAccessory, this));
      _server->on("/reset", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
      _server->on("/reset", HTTP_ANY, std::bind(&WebServer::_handleReset, this));
      _server->onNotFound(std::bind(&WebServer::_handleNotFound, this));
      _server->begin();
    }

    void WebServer::loop() {
      _server->handleClient();
    }

    void WebServer::_redirectTo(String url) {
      _server->sendHeader(F("Location"), url, true);
      _server->send(302, "text/plain", "");
    }

    String WebServer::_formatPage(String htmlBody) {
      return "\
        <!DOCTYPE HTML>\
        <html>\
          <head>\
            <title>" + _productName + "</title>\
            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
            <style>\
              body { background-color: #cccccc; font-family: Arial, Sans-Serif; }\
              ul { padding: 0; list-style-type: none; }\
              td { border-bottom: 1px solid #aaaaaa; }\
            </style>\
          </head>\
          <body>\
            <h1>" + _productName + "</h1>\
            " + htmlBody + "\
          </body>\
        </html>\
      ";
    }

    void WebServer::_dispatchRequestStart() {
      // set cross origin
      _server->sendHeader(F("Access-Control-Allow-Origin"), F("*"));
      _server->sendHeader(F("Access-Control-Max-Age"), F("600")); // 10 minutes
      _server->sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
      _server->sendHeader(F("Access-Control-Allow-Headers"), F("*"));
      // fire event
      if (onRequestStart) {
        onRequestStart();
      }
    }

    void WebServer::_dispatchRequestEnd() {
      if (onRequestEnd) {
        onRequestEnd();
      }
    }

    void WebServer::_handleRoot() {
      _dispatchRequestStart();
      // mode
      WiFiMode_t wifiMode = WiFi.getMode();
      String strWifiMode = "WIFI_OFF";
      if (wifiMode == WIFI_STA) {
        strWifiMode = "WIFI_STA";
      } else if (wifiMode == WIFI_AP) {
        strWifiMode = "WIFI_AP";
      } else if (wifiMode == WIFI_AP_STA) {
        strWifiMode = "WIFI_AP_STA";
      }
      // ip
      IPAddress localIP = WiFi.localIP();
      String strLocalIP = "";
      if (localIP) {
        strLocalIP = localIP.toString();
      }
      // ap
      bool isApEnabled = ((wifiMode & WIFI_AP) != 0);
      String strApEnabled = isApEnabled ? "YES" : "NO";
      String strApIP = "";
      IPAddress apIP = WiFi.softAPIP();
      if (apIP) {
        strApIP = apIP.toString();
      }
      // mac
      String macAddr = WiFi.macAddress();
      // settings
      String accessoryLinks = "";
      if (onLoadSettings) {
        std::map<String, AccessorySetting> settings = onLoadSettings();
        for (const auto& pair : settings) {
          String url = ("/accessory?id=" + pair.first);
          accessoryLinks += "\
            <a href=\"" + url + "\">Accessory (" + pair.second.name + ")</a>\
          ";
        }
      }
      // content
      String htmlBody = "\
        <p>\
          " + accessoryLinks + "\
          <a href=\"/list-wifi\">Wifi</a>\
          <a href=\"/reset\">Reset</a>\
        </p>\
        <h3>Home</h3>\
        <table>\
          <tr>\
            <td>Wifi Mode</td>\
            <td>" + strWifiMode + "</td>\
          </tr>\
          <tr>\
            <td>IP Address</td>\
            <td>" + strLocalIP + "</td>\
          </tr>\
          <tr>\
            <td>AP Enabled</td>\
            <td>" + strApEnabled + "</td>\
          </tr>\
          <tr>\
            <td>AP Address</td>\
            <td>" + strApIP + "</td>\
          </tr>\
          <tr>\
            <td>MAC Address</td>\
            <td>" + macAddr + "</td>\
          </tr>\
        </table>\
      ";
      _server->send(200, "text/html", _formatPage(htmlBody));
      _dispatchRequestEnd();
    }

    void WebServer::_handleListWifi() {
      _dispatchRequestStart();
      String list = "";
      int count = WiFi.scanNetworks();
      String current = WiFi.SSID();
      for (int i = 0; i < count; ++i) {
        String ssid = WiFi.SSID(i);
        String checked = _getCheckedAttr(ssid == current);
        list += "\
          <li>\
            <input type=\"radio\" id=\"rdoSsid" + String(i) + "\" name=\"ssid\" value=\"" + ssid + "\"" + checked + " />\
            <label for=\"rdoSsid" + String(i) + "\">" + ssid + "</label>\
          </li>\
        ";
      }
      String htmlBody = "\
        <p><a href=\"/\">Home</a></p>\
        <h3>Connect WiFi</h3>\
        <form method=\"post\" action=\"/connect-wifi\">\
          <ul>" + list + "</ul>\
          <p>\
            <label for=\"txtPassword\">Password:</label>\
            <input type=\"text\" id=\"txtPassword\" name=\"password\" length=\"64\" />\
          </p>\
          <p><input type=\"submit\" value=\"Connect\" /></p>\
        </form>\
      ";
      _server->send(200, "text/html", _formatPage(htmlBody));
      _dispatchRequestEnd();
    }

    void WebServer::_handleConnectWifi() {
      _dispatchRequestStart();
      String ssid = _server->arg("ssid");
      String password = _server->arg("password");

      if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        String ignoreMessage = "\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Ignore</legend>\
            <p>Ignore network setup</p>\
          </fieldset>\
        ";
        _server->send(200, "text/html", _formatPage(ignoreMessage));
        _dispatchRequestEnd();
        return;
      }

      Serial.println("SSID: " + ssid);
      Serial.println("PASSWORD: " + password);
      WiFi.begin(ssid, password);
      // Wait for connection
      int checkTimes = 5;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (checkTimes == 0) {
          break;
        } else {
          checkTimes--;
        }
      }

      bool isConnected = WiFi.status() == WL_CONNECTED;
      if (isConnected) {
        String successMessage = "\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Success</legend>\
            <p>Connected to <b>" + ssid + "</b></p>\
            <p>IP address <b>" + WiFi.localIP().toString() + "</b></p>\
          </fieldset>\
        ";
        _server->send(200, "text/html", _formatPage(successMessage));
      } else {
        String failedMessage = "\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Failed</legend>\
            <p>Connect <b>" + ssid + "</b> failed</p>\
          </fieldset>\
        ";
        _server->send(200, "text/html", _formatPage(failedMessage));
      }
      _dispatchRequestEnd();
    }

    void WebServer::_handleAccessory() {
      _dispatchRequestStart();
      String accessoryId = _server->arg("id");
      String currentUrl = "/accessory?id=" + accessoryId;
      AccessorySetting currentSetting;
      if (onLoadSettings) {
        std::map<String, AccessorySetting> settings = onLoadSettings();
        if (settings.count(accessoryId) > 0) {
          currentSetting = settings[accessoryId];
        }
      }
      if (!currentSetting.name) {
        String notfound = "\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Oops...</legend>\
            <p>Accessory Not Found</p>\
            <p>Accessory ID: " + accessoryId + "</p>\
          </fieldset>\
        ";
        _server->send(200, "text/html", _formatPage(notfound));
      } else {
        if (_server->method() == HTTP_POST) {
          String submit = _server->arg("Submit");
          String accessoryName = _server->arg("AccessoryName");
          String accessoryType = _server->arg("AccessoryType");
          String outputIO = _server->arg("OutputIO");
          String inputIO = _server->arg("InputIO");
          String booleanValue = _server->arg("BooleanValue");
          String integerValue = _server->arg("IntegerValue");
          if (submit == "Delete") {
            if (onDeleteSetting) {
              onDeleteSetting(accessoryId, currentSetting);
            }
            _redirectTo("/");
          } else {
            currentSetting.name = accessoryName;
            currentSetting.type =
              accessoryType == "boolean" ? BooleanAccessoryType :
              accessoryType == "integer" ? IntegerAccessoryType : EmptyAccessoryType;
            currentSetting.outputIO = outputIO.toInt();
            currentSetting.inputIO = inputIO.toInt();
            currentSetting.boolValue = (booleanValue == "true");
            currentSetting.intValue = integerValue.toInt();
            if (onSaveSetting) {
              onSaveSetting(accessoryId, currentSetting);
            }
            _redirectTo(currentUrl);
          }
        } else {
          String htmlBody = "\
            <p><a href=\"/\">Home</a></p>\
            <h3>Accessory (" + currentSetting.name + ")</h3>\
            <form method=\"post\" action=\"" + currentUrl + "\">\
              <p>\
                <label for=\"txtAccessoryName\">Name</label>\
                <input type=\"text\" id=\"txtAccessoryName\" name=\"AccessoryName\" value=\"" + currentSetting.name + "\" />\
              </p>\
              <p>\
                <label for=\"txtOutputIO\">Output IO</label>\
                <input type=\"text\" id=\"txtOutputIO\" name=\"OutputIO\" value=\"" + currentSetting.outputIO + "\" />\
              </p>\
              <p>\
                <label for=\"txtInputIO\">Input IO</label>\
                <input type=\"text\" id=\"txtInputIO\" name=\"InputIO\" value=\"" + currentSetting.inputIO + "\" />\
              </p>\
              <fieldset>\
                <legend>Accessory Type</legend>\
                " + _getTypeHtml(currentSetting) + "\
              </fieldset>\
              <fieldset>\
                <legend>Boolean Value</legend>\
                " + _getBooleanHtml(currentSetting) + "\
              </fieldset>\
              <fieldset>\
                <legend>Integer Value</legend>\
                " + _getIntegerHtml(currentSetting) + "\
              </fieldset>\
              <p>\
                <input type=\"submit\" name=\"Submit\" value=\"Save\" />\
                <input type=\"submit\" name=\"Submit\" value=\"Delete\" />\
              </p>\
            </form>\
          ";
          _server->send(200, "text/html", _formatPage(htmlBody));
        }
      }
      _dispatchRequestEnd();
    }

    String WebServer::_getCheckedAttr(bool checked) {
      return checked ? " checked=\"checked\"" : "";
    }

    String WebServer::_getTypeHtml(AccessorySetting setting) {
      String booleanAttribute = _getCheckedAttr(setting.type == BooleanAccessoryType);
      String integerAttribute = _getCheckedAttr(setting.type == IntegerAccessoryType);
      String html = "\
        <p>\
          <input type=\"radio\" id=\"rdoBooleanType\" name=\"AccessoryType\" value=\"boolean\"" + booleanAttribute + " />\
          <label for=\"rdoBooleanType\">Boolean - Accessory with boolean value such as switcher(on/off), shake sensor(yes/no)</label>\
        </p>\
        <p>\
          <input type=\"radio\" id=\"rdoIntegerType\" name=\"AccessoryType\" value=\"integer\"" + integerAttribute + " />\
          <label for=\"rdoIntegerType\">Integer - Accessory with integer value such as temperature, humidness</label>\
        </p>\
      ";
      return html;
    }

    String WebServer::_getBooleanHtml(AccessorySetting setting) {
      String trueAttribute = _getCheckedAttr(setting.boolValue);
      String falseAttribute = _getCheckedAttr(setting.boolValue);
      String html = "\
        <p>\
          <input type=\"radio\" id=\"rdoBooleanTrue\" name=\"BooleanValue\" value=\"true\"" + trueAttribute + " />\
          <label for=\"rdoBooleanTrue\">On/Yes/True</label>\
        </p>\
        <p>\
          <input type=\"radio\" id=\"rdoBooleanFalse\" name=\"BooleanValue\" value=\"false\"" + falseAttribute + " />\
          <label for=\"rdoBooleanFalse\">Off/No/False</label>\
        </p>\
      ";
      return html;
    }

    String WebServer::_getIntegerHtml(AccessorySetting setting) {
      String value = String(setting.intValue);
      String html = "\
        <p>\
          <label for=\"txtIntegerValue\">Value</label>\
          <input type=\"number\" id=\"txtIntegerValue\" name=\"IntegerValue\" value=\"" + value + "\"/>\
        </p>\
      ";
      return html;
    }

    void WebServer::_handleReset() {
      _dispatchRequestStart();
      if (_server->method() == HTTP_POST) {
        String resetWifi = _server->arg("ResetWifi");
        if (resetWifi == "yes") {
          // wifi_config_reset();
          WiFi.disconnect(true);
          WiFi.mode(WIFI_AP_STA);
          Serial.println("Wifi mode: WIFI_AP_STA");
        }
        String resetAccessory = _server->arg("ResetAccessory");
        if (resetAccessory == "yes" && onResetAccessory) {
          onResetAccessory();
        }
        String restartESP = _server->arg("RestartESP");
        if (restartESP == "yes") {
          // sdk_system_restart();
          ESP.restart();
        }
        _redirectTo("/");
      } else {
        String htmlBody = "\
          <p><a href=\"/\">Home</a></p>\
          <h3>Reset</h3>\
          <form method=\"post\" action=\"/reset\">\
            <p>\
              <input type=\"checkbox\" id=\"chkResetWifi\" name=\"ResetWifi\" value=\"yes\" />\
              <label for=\"chkResetWifi\">Reset Wifi</label>\
            </p>\
            <p>\
              <input type=\"checkbox\" id=\"chkResetAccessory\" name=\"ResetAccessory\" value=\"yes\" />\
              <label for=\"chkResetAccessory\">Reset Accessory</label>\
            </p>\
            <p>\
              <input type=\"checkbox\" id=\"chkRestartESP\" name=\"RestartESP\" value=\"yes\" />\
              <label for=\"chkRestartESP\">Restart ESP</label>\
            </p>\
            <p><input type=\"submit\" value=\"Submit\" /></p>\
          </form>\
        ";
        _server->send(200, "text/html", _formatPage(htmlBody));
      }
      _dispatchRequestEnd();
    }

    void WebServer::_handleCrossOrigin() {
      _dispatchRequestStart();
      _server->send(204);
      _dispatchRequestEnd();
    }

    void WebServer::_handleNotFound() {
      _dispatchRequestStart();
      String method = (_server->method() == HTTP_GET) ? "GET" : "POST";
      String bodyHtml = "\
        <p><a href=\"/\">Home</a></p>\
        <fieldset>\
          <legend>Oops...</legend>\
          <p>Resource Not Found</p>\
          <p>URI: " + _server->uri() + "</p>\
          <p>Method: " + method + "</p>\
        </fieldset>\
      ";
      _server->send(404, "text/html", _formatPage(bodyHtml));
      _dispatchRequestEnd();
    }

  }
}
