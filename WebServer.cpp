#include "WebServer.h"

namespace Victoria {
  namespace Components {

    WebServer::WebServer(int port, const String& productName, const String& hostName) {
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
          Serial.println("AP Address: " + currentApIp.toString());
        }
      }

      _server->on("/", HTTP_GET, std::bind(&WebServer::_handleRoot, this));
      _server->on("/wifi/list", HTTP_GET, std::bind(&WebServer::_handleWifiList, this));
      _server->on("/wifi/connect", HTTP_POST, std::bind(&WebServer::_handleWifiConnect, this));
      _server->on("/accessory/new", HTTP_GET, std::bind(&WebServer::_handleNewAccessory, this));
      _server->on("/accessory", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
      _server->on("/accessory", HTTP_ANY, std::bind(&WebServer::_handleAccessory, this));
      _server->on("/accessory/state", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
      _server->on("/accessory/state", HTTP_ANY, std::bind(&WebServer::_handleAccessoryState, this));
      _server->on("/reset", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
      _server->on("/reset", HTTP_ANY, std::bind(&WebServer::_handleReset, this));
      _server->onNotFound(std::bind(&WebServer::_handleNotFound, this));
      _server->begin();
    }

    void WebServer::loop() {
      _server->handleClient();
    }

    void WebServer::_redirectTo(const String& url) {
      _server->sendHeader(F("Location"), url, true);
      _server->send(302, "text/plain", "");
    }

    void WebServer::_send200(const String& bodyHtml) {
      _server->send(200, "text/html", _formatPage(bodyHtml));
    }

    void WebServer::_send404(const String& bodyHtml) {
      _server->send(404, "text/html", _formatPage(bodyHtml));
    }

    String WebServer::_formatPage(const String& bodyHtml) {
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
            " + bodyHtml + "\
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
      String strLocalIP = "";
      bool isStaEnabled = ((wifiMode & WIFI_STA) != 0);
      if (isStaEnabled) {
        IPAddress localIP = WiFi.localIP();
        if (localIP) {
          strLocalIP = localIP.toString();
        }
      }
      // ap
      String strApIP = "";
      bool isApEnabled = ((wifiMode & WIFI_AP) != 0);
      if (isApEnabled) {
        IPAddress apIP = WiFi.softAPIP();
        if (apIP) {
          strApIP = apIP.toString();
        }
      }
      // mac
      String macAddr = WiFi.macAddress();
      // settings
      String accessoryLinks = "";
      if (onLoadSettings) {
        std::map<String, AccessorySetting> settings = onLoadSettings();
        String randomId = CommonHelpers::randomString(4);
        String newAccessoryUrl = "/accessory/new?id=" + randomId + "&index=" + String(settings.size() + 1);
        accessoryLinks += "\
          <a href=\"" + newAccessoryUrl + "\">Add+</a>\
        ";
        for (const auto& pair : settings) {
          String url = ("/accessory?id=" + pair.first);
          accessoryLinks += "\
            <a href=\"" + url + "\">" + pair.second.name + "</a>\
          ";
        }
      }
      // content
      _send200("\
        <p>\
          <a href=\"/wifi/list\">Wifi</a>\
          <a href=\"/reset\">Reset</a>\
        </p>\
        <h3>Accessories</h3>\
        <p>\
          " + accessoryLinks + "\
        </p>\
        <h3>Home</h3>\
        <table>\
          <tr>\
            <td>Wifi Mode</td>\
            <td>" + strWifiMode + "</td>\
          </tr>\
          <tr>\
            <td>AP Address</td>\
            <td><a href=\"http://" + strApIP + "\">" + strApIP + "</a></td>\
          </tr>\
          <tr>\
            <td>IP Address</td>\
            <td><a href=\"http://" + strLocalIP + "\">" + strLocalIP + "</a></td>\
          </tr>\
          <tr>\
            <td>MAC Address</td>\
            <td>" + macAddr + "</td>\
          </tr>\
        </table>\
      ");
      _dispatchRequestEnd();
    }

    void WebServer::_handleWifiList() {
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
      _send200("\
        <p><a href=\"/\">Home</a></p>\
        <h3>Connect WiFi</h3>\
        <form method=\"post\" action=\"/wifi/connect\">\
          <ul>" + list + "</ul>\
          <p>\
            <label for=\"txtPassword\">Password:</label>\
            <input type=\"text\" id=\"txtPassword\" name=\"password\" length=\"64\" />\
          </p>\
          <p><input type=\"submit\" value=\"Connect\" /></p>\
        </form>\
      ");
      _dispatchRequestEnd();
    }

    void WebServer::_handleWifiConnect() {
      _dispatchRequestStart();
      String ssid = _server->arg("ssid");
      String password = _server->arg("password");

      if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        _send200("\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Ignore</legend>\
            <p>Ignore network setup</p>\
          </fieldset>\
        ");
        _dispatchRequestEnd();
        return;
      }

      Serial.println("SSID: " + ssid);
      Serial.println("PASSWORD: " + password);
      WiFi.begin(ssid, password);
      // Wait for connection
      int checkTimes = 30;
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
        _send200("\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Success</legend>\
            <p>Connected to <b>" + ssid + "</b></p>\
            <p>IP address <b>" + WiFi.localIP().toString() + "</b></p>\
          </fieldset>\
        ");
      } else {
        _send200("\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Failed</legend>\
            <p>Connect <b>" + ssid + "</b> failed</p>\
          </fieldset>\
        ");
      }
      _dispatchRequestEnd();
    }

    void WebServer::_handleNewAccessory() {
      _dispatchRequestStart();
      String accessoryId = _server->arg("id");
      String accessoryIndex = _server->arg("index");
      if (onSaveSetting) {
        // save
        AccessorySetting newSetting = {
          .name = "New" + accessoryIndex,
          .type = BooleanAccessoryType,
          .outputIO = -1,
          .inputIO = -1,
          .outputLevel = -1,
          .inputLevel = -1,
        };
        onSaveSetting(accessoryId, newSetting);
        // redirect
        String accessoryUrl = "/accessory?id=" + accessoryId;
        _redirectTo(accessoryUrl);
      }
      _dispatchRequestEnd();
    }

    void WebServer::_handleAccessory() {
      _dispatchRequestStart();
      String accessoryId = _server->arg("id");
      String currentUrl = "/accessory?id=" + accessoryId;
      std::pair<bool, AccessorySetting> found = _getAccessorySetting(accessoryId);
      AccessorySetting setting = found.second;
      if (!found.first) {
        _dispatchRequestEnd();
        return;
      }
      if (_server->method() == HTTP_POST) {
        String accessoryName = _server->arg("AccessoryName");
        String accessoryType = _server->arg("AccessoryType");
        String outputIO = _server->arg("OutputIO");
        String inputIO = _server->arg("InputIO");
        String outputLevel = _server->arg("OutputLevel");
        String inputLevel = _server->arg("InputLevel");
        String submit = _server->arg("Submit");
        if (submit == "Delete") {
          if (onDeleteSetting) {
            onDeleteSetting(accessoryId, setting);
          }
          _redirectTo("/");
        } else {
          setting.name = accessoryName;
          setting.type =
            accessoryType == "boolean" ? BooleanAccessoryType :
            accessoryType == "integer" ? IntegerAccessoryType : EmptyAccessoryType;
          setting.outputIO = outputIO.toInt();
          setting.inputIO = inputIO.toInt();
          setting.outputLevel = outputLevel.toInt();
          setting.inputLevel = inputLevel.toInt();
          if (onSaveSetting) {
            onSaveSetting(accessoryId, setting);
          }
          _redirectTo(currentUrl);
        }
      } else {
        _send200("\
          <p>\
            <a href=\"/\">Home</a>\
            <a href=\"/accessory/state?id=" + accessoryId + "\">State</a>\
          </p>\
          <h3>Setting (" + setting.name + ")</h3>\
          <form method=\"post\" action=\"" + currentUrl + "\">\
            <p>\
              <label for=\"txtAccessoryName\">Name</label>\
              <input type=\"text\" id=\"txtAccessoryName\" name=\"AccessoryName\" value=\"" + setting.name + "\" />\
            </p>\
            " + _getTypeHtml(setting) + "\
            " + _getIOHtml(setting) + "\
            <p>\
              <input type=\"submit\" name=\"Submit\" value=\"Save\" />\
              <input type=\"submit\" name=\"Submit\" value=\"Delete\" />\
            </p>\
          </form>\
        ");
      }
      _dispatchRequestEnd();
    }

    void WebServer::_handleAccessoryState() {
      _dispatchRequestStart();
      String accessoryId = _server->arg("id");
      String currentUrl = "/accessory/state?id=" + accessoryId;
      std::pair<bool, AccessorySetting> found = _getAccessorySetting(accessoryId);
      AccessorySetting setting = found.second;
      if (!found.first) {
        _dispatchRequestEnd();
        return;
      }
      AccessoryState state = {
        .boolValue = false,
        .intValue = 0,
      };
      if (_server->method() == HTTP_POST) {
        if (_server->hasArg("BooleanValue")) {
          String booleanValue = _server->arg("BooleanValue");
          state.boolValue = (booleanValue == "true");
        }
        if (_server->hasArg("IntegerValue")) {
          String integerValue = _server->arg("IntegerValue");
          state.intValue = integerValue.toInt();
        }
        if (onSetState) {
          onSetState(accessoryId, setting, state);
        }
        _redirectTo(currentUrl);
      } else {
        if (onGetState) {
          onGetState(accessoryId, setting, state);
        }
        String stateHtml =
          setting.type == BooleanAccessoryType ? _getBooleanHtml(state) :
          setting.type == IntegerAccessoryType ? _getIntegerHtml(state) : "";
        _send200("\
          <p><a href=\"/\">Home</a></p>\
          <h3>State (" + setting.name + ")</h3>\
          <form method=\"post\" action=\"" + currentUrl + "\">\
            " + stateHtml + "\
            <p>\
              <input type=\"submit\" name=\"Submit\" value=\"Save\" />\
            </p>\
          </form>\
        ");
      }
      _dispatchRequestEnd();
    }

    std::pair<bool, AccessorySetting> WebServer::_getAccessorySetting(const String& id) {
      bool foundSetting = false;
      AccessorySetting setting;
      if (onLoadSettings) {
        std::map<String, AccessorySetting> settings = onLoadSettings();
        if (settings.count(id) > 0) {
          setting = settings[id];
          foundSetting = true;
        }
      }
      if (!foundSetting) {
        _send200("\
          <p><a href=\"/\">Home</a></p>\
          <fieldset>\
            <legend>Oops...</legend>\
            <p>Accessory Not Found</p>\
            <p>Accessory ID: " + id + "</p>\
          </fieldset>\
        ");
      }
      return std::make_pair(foundSetting, setting);
    }

    String WebServer::_getCheckedAttr(bool checked) {
      return checked ? " checked=\"checked\"" : "";
    }

    String WebServer::_getTypeHtml(const AccessorySetting& setting) {
      String booleanAttribute = _getCheckedAttr(setting.type == BooleanAccessoryType);
      String integerAttribute = _getCheckedAttr(setting.type == IntegerAccessoryType);
      String html = "\
        <fieldset>\
          <legend>Accessory Type</legend>\
          <p>\
            <input type=\"radio\" id=\"rdoBooleanType\" name=\"AccessoryType\" value=\"boolean\"" + booleanAttribute + " />\
            <label for=\"rdoBooleanType\">Boolean - Accessory with boolean value such as switcher(on/off), shake sensor(yes/no)</label>\
          </p>\
          <p>\
            <input type=\"radio\" id=\"rdoIntegerType\" name=\"AccessoryType\" value=\"integer\"" + integerAttribute + " />\
            <label for=\"rdoIntegerType\">Integer - Accessory with integer value such as temperature, humidness</label>\
          </p>\
        </fieldset>\
      ";
      return html;
    }

    String WebServer::_getIOHtml(const AccessorySetting& setting) {
      String html = "\
        <fieldset>\
          <legend>IO Pins</legend>\
          <p>\
            <label for=\"txtOutputIO\">Output</label>\
            <input type=\"number\" id=\"txtOutputIO\" name=\"OutputIO\" value=\"" + String(setting.outputIO) + "\" />\
            " + _getLevelHtml("OutputLevel", setting.outputLevel) + "\
          </p>\
          <p>\
            <label for=\"txtInputIO\">Input</label>\
            <input type=\"number\" id=\"txtInputIO\" name=\"InputIO\" value=\"" + String(setting.inputIO) + "\" />\
            " + _getLevelHtml("InputLevel", setting.inputLevel) + "\
          </p>\
        </fieldset>\
      ";
      return html;
    }

    String WebServer::_getLevelHtml(const String name, const short int level) {
      return "\
        <label for=\"txt" + name + "High\">High</label>\
        <input type=\"radio\" id=\"txt" + name + "High\" name=\"" + name + "\" value=\"1\"" + _getCheckedAttr(level == 1) + " />\
        <label for=\"txt" + name + "Low\">Low</label>\
        <input type=\"radio\" id=\"txt" + name + "Low\" name=\"" + name + "\" value=\"0\"" + _getCheckedAttr(level == 0) + " />\
        <label for=\"txt" + name + "No\">No</label>\
        <input type=\"radio\" id=\"txt" + name + "No\" name=\"" + name + "\" value=\"-1\"" + _getCheckedAttr(level == -1) + " />\
      ";
    }

    String WebServer::_getBooleanHtml(const AccessoryState& state) {
      String trueAttribute = _getCheckedAttr(state.boolValue);
      String falseAttribute = _getCheckedAttr(!state.boolValue);
      String html = "\
        <fieldset>\
          <legend>Boolean Value</legend>\
          <p>\
            <input type=\"radio\" id=\"rdoBooleanTrue\" name=\"BooleanValue\" value=\"true\"" + trueAttribute + " />\
            <label for=\"rdoBooleanTrue\">On/Yes/True</label>\
          </p>\
          <p>\
            <input type=\"radio\" id=\"rdoBooleanFalse\" name=\"BooleanValue\" value=\"false\"" + falseAttribute + " />\
            <label for=\"rdoBooleanFalse\">Off/No/False</label>\
          </p>\
        </fieldset>\
      ";
      return html;
    }

    String WebServer::_getIntegerHtml(const AccessoryState& state) {
      String html = "\
        <fieldset>\
          <legend>Integer Value</legend>\
          <p>\
            <label for=\"txtIntegerValue\">Value</label>\
            <input type=\"number\" id=\"txtIntegerValue\" name=\"IntegerValue\" value=\"" + String(state.intValue) + "\"/>\
          </p>\
        </fieldset>\
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
        _send200("\
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
        ");
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
      _send404("\
        <p><a href=\"/\">Home</a></p>\
        <fieldset>\
          <legend>Oops...</legend>\
          <p>Resource Not Found</p>\
          <p>URI: " + _server->uri() + "</p>\
          <p>Method: " + method + "</p>\
        </fieldset>\
      ");
      _dispatchRequestEnd();
    }

  }
}
