#include <LittleFS.h>
#include "WebServer.h"

namespace Victoria::Components {

  WebServer::WebServer(int port, const String& productName, const String& hostName, const String& firmwareVersion) {
    _server = new ESP8266WebServer(port);
    _productName = productName;
    _hostName = hostName;
    _firmwareVersion = firmwareVersion;
  }

  WebServer::~WebServer() {
    _server->stop();
    delete _server;
    _server = NULL;
  }

  void WebServer::setup() {
    WiFiMode_t wifiMode = WiFi.getMode();
    bool apEnabled = ((wifiMode & WIFI_AP) != 0);
    bool staEnabled = ((wifiMode & WIFI_STA) != 0);
    if (!apEnabled && !staEnabled) {
      WiFi.mode(WIFI_AP_STA);
      wifiMode = WIFI_AP_STA;
    }

    bool isApEnabled = ((wifiMode & WIFI_AP) != 0);
    if (isApEnabled) {
      // IPAddress apIp(192, 168, 1, 33);
      // IPAddress apSubnet(255, 255, 255, 0);
      // WiFi.softAPConfig(apIp, apIp, apSubnet);
      WiFi.softAP(_hostName);
      IPAddress currentApIp = WiFi.softAPIP();
      if (currentApIp) {
        console.log("AP Address: " + currentApIp.toString());
      }
    }

    WiFi.hostname(_hostName);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin();

    _server->on("/", HTTP_GET, std::bind(&WebServer::_handleRoot, this));
    _server->on("/system", HTTP_GET, std::bind(&WebServer::_handleSystem, this));
    _server->on("/system/file", HTTP_OPTIONS, std::bind(&WebServer::_handleCrossOrigin, this));
    _server->on("/system/file", HTTP_ANY, std::bind(&WebServer::_handleSystemFile, this));
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
    _server->sendHeader("Location", url, true);
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
            html, body { background: transparent; font-family: Arial, Sans-Serif; margin: 0; padding: 0; border: 0; color: #4f4e4e; }\
            h1, h2, h3 { font-weight: 400; }\
            a { color: #00979d; text-decoration: none; }\
            fieldset { border: 1px solid #00979c; }\
            ul { padding: 0; list-style-type: none; }\
            td { border-bottom: 1px solid #d5e9e9; }\
            .container { padding: 0 10px; font-size: 14px; }\
            .title { background: #008184; color: #ffffff; text-align: center; margin: 0; padding: 5px 0; }\
            .button { background-color: #005c5f; border: 1px solid #005c5f; color: #ffffff; border-radius: 5px; padding: 5px 10px; box-shadow: none; }\
          </style>\
        </head>\
        <body>\
          <h2 class=\"title\">" + _productName + "</h2>\
          <div class=\"container\">\
            " + bodyHtml + "\
          </div>\
        </body>\
      </html>\
    ";
  }

  void WebServer::_dispatchRequestStart() {
    // set cross origin
    _server->sendHeader("Access-Control-Allow-Origin", "*");
    _server->sendHeader("Access-Control-Max-Age", "600"); // 10 minutes
    _server->sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    _server->sendHeader("Access-Control-Allow-Headers", "*");
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
          | <a href=\"" + url + "\">" + pair.second.name + "</a>\
        ";
      }
    }
    TableModel table = {
      .header = {},
      .rows = {
        { "Wifi Mode", strWifiMode },
        { "AP Address", strApIP != "" ? "<a href=\"http://" + strApIP + "\">" + strApIP + "</a>" : "-" },
        { "IP Address", strLocalIP != "" ? "<a href=\"http://" + strLocalIP + "\">" + strLocalIP + "</a>" : "-" },
        { "MAC Address", macAddr },
        { "Firmware Version", _firmwareVersion },
      },
    };
    // content
    _send200("\
      <p>\
        <a href=\"/wifi/list\">Wifi</a> |\
        <a href=\"/system\">System</a> |\
        <a href=\"/reset\">Reset</a>\
      </p>\
      <h3>Home</h3>\
      <p>\
        " + _renderTable(table) + "\
      </p>\
      <h3>Accessories</h3>\
      <p>\
        " + accessoryLinks + "\
      </p>\
    ");
    _dispatchRequestEnd();
  }

  // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
  void WebServer::_handleSystem() {
    _dispatchRequestStart();
    if (LittleFS.begin()) {
      FSInfo fsInfo;
      if (LittleFS.info(fsInfo)) {
        TableModel infoTable = {
          .header = {},
          .rows = {
            { "Total Bytes", String(fsInfo.totalBytes) },
            { "Used Bytes", String(fsInfo.usedBytes) },
            { "Max Path Length", String(fsInfo.maxPathLength) },
            { "Max Open Files", String(fsInfo.maxOpenFiles) },
            { "Block Size", String(fsInfo.blockSize) },
            { "Page Size", String(fsInfo.pageSize) },
          },
        };
        TableModel filesTable = {
          .header = { "Name", "Bytes" },
        };
        // loop file method 1
        // std::function<void(File)> loopFile;
        // loopFile = [&](File file)->void {
        //   File next = file.openNextFile();
        //   file.close();
        //   if (!next) {
        //     return;
        //   }
        //   if (next.isFile()) {
        //     String name = String(next.fullName());
        //     String uri = "/system/file?name=" + name;
        //     filesTable.rows.push_back({
        //       "<a href=\"" + uri + "\">" + name + "</a>",
        //       String(next.size()),
        //     });
        //   }
        //   loopFile(next);
        // };
        // loopFile(LittleFS.open("/", "r"));

        // loop file method 2
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
          if (dir.fileSize()) {
            File file = dir.openFile("r");
            String name = String(file.fullName());
            String uri = "/system/file?name=" + name;
            filesTable.rows.push_back({
              "<a href=\"" + uri + "\">" + name + "</a>",
              String(file.size()),
            });
          }
        }
        _send200("\
          <p><a href=\"/\">&lt; Home</a></p>\
          <h3>System</h3>\
          <p>\
            " + _renderTable(infoTable) + "\
          </p>\
          <h3>Files</h3>\
          <p>\
            " + _renderTable(filesTable) + "\
          </p>\
        ");
      } else {
        console.error("Read fs info failed");
      }
    } else {
      console.error("Failed to mount file system");
    }
    LittleFS.end();
    _dispatchRequestEnd();
  }

  void WebServer::_handleSystemFile() {
    _dispatchRequestStart();
    if (LittleFS.begin()) {
      String fileName = _server->arg("name");
      File file = LittleFS.open("/" + fileName, "r");
      if (file) {
        size_t size = file.size();
        String content = file.readString();
        file.close();
        _send200("\
          <p><a href=\"/system\">&lt; System</a></p>\
          <h3>" + fileName + " " + String(size) + " bytes</h3>\
          <p>\
            <textarea name=\"Content\" cols=\"50\" rows=\"10\">" + content + "</textarea>\
          </p>\
        ");
      } else {
        console.error("Failed to open file " + fileName);
      }
    } else {
      console.error("Failed to mount file system");
    }
    LittleFS.end();
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
      <p><a href=\"/\">&lt; Home</a></p>\
      <h3>Connect WiFi</h3>\
      <form method=\"post\" action=\"/wifi/connect\">\
        <ul>" + list + "</ul>\
        <p>\
          <label for=\"txtPassword\">Password:</label>\
          <input type=\"text\" id=\"txtPassword\" name=\"password\" length=\"64\" />\
        </p>\
        <p><input type=\"submit\" class=\"button\" value=\"Connect\" /></p>\
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
        <p><a href=\"/\">&lt; Home</a></p>\
        <fieldset>\
          <legend>Ignore</legend>\
          <p>Ignore network setup</p>\
        </fieldset>\
      ");
      _dispatchRequestEnd();
      return;
    }

    console.log("SSID: " + ssid);
    console.log("PASSWORD: " + password);
    WiFi.begin(ssid, password);
    // Wait for connection
    int checkTimes = 30;
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

    bool isConnected = WiFi.status() == WL_CONNECTED;
    if (isConnected) {
      _send200("\
        <p><a href=\"/\">&lt; Home</a></p>\
        <fieldset>\
          <legend>Success</legend>\
          <p>Connected to <b>" + ssid + "</b></p>\
          <p>IP address <b>" + WiFi.localIP().toString() + "</b></p>\
        </fieldset>\
      ");
    } else {
      _send200("\
        <p><a href=\"/\">&lt; Home</a></p>\
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
          <a href=\"/\">&lt; Home</a> |\
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
            <input type=\"submit\" class=\"button\" name=\"Submit\" value=\"Save\" />\
            <input type=\"submit\" class=\"button\" name=\"Submit\" value=\"Delete\" />\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebServer::_handleAccessoryState() {
    _dispatchRequestStart();
    String accessoryId = _server->arg("id");
    String backUrl = "/accessory?id=" + accessoryId;
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
        state = onGetState(accessoryId, setting);
      }
      String stateHtml =
        setting.type == BooleanAccessoryType ? _getBooleanHtml(state) :
        setting.type == IntegerAccessoryType ? _getIntegerHtml(state) : "";
      _send200("\
        <p>\
          <a href=\"" + backUrl + "\">&lt; Setting (" + setting.name + ")</a>\
        </p>\
        <h3>State (" + setting.name + ")</h3>\
        <form method=\"post\" action=\"" + currentUrl + "\">\
          " + stateHtml + "\
          <p>\
            <input type=\"submit\" class=\"button\" name=\"Submit\" value=\"Save\" />\
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
        <p><a href=\"/\">&lt; Home</a></p>\
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

  String WebServer::_renderTable(const TableModel& model) {
    String tableHeader = "";
    if (model.header.size() > 0) {
      tableHeader += "<tr>";
      for (const auto& headerCell : model.header) {
        tableHeader += "<th>" + headerCell + "</th>";
      }
      tableHeader += "</tr>";
    }
    String tableRows = "";
    for (const auto& row : model.rows) {
      tableRows += "<tr>";
      for (const auto rowCell : row) {
        tableRows += "<td>" + rowCell + "</td>";
      }
      tableRows += "</tr>";
    }
    String html = "\
      <table>\
        " + tableHeader + "\
        " + tableRows + "\
      </table>\
    ";
    return html;
  }

  String WebServer::_renderSelectionList(std::vector<std::vector<String>> list) {
    String html = "";
    for (const auto& item : list) {
      html += "\
        <p>\
          <input type=\"" + item[3] + "\" id=\"id" + item[1] + item[2] + "\" name=\"" + item[1] + "\" value=\"" + item[2] + "\"" + item[4] + " />\
          <label for=\"id" + item[1] + item[2] + "\">" + item[0] + "</label>\
        </p>\
      ";
    }
    return html;
  }

  String WebServer::_getTypeHtml(const AccessorySetting& setting) {
    String booleanAttribute = _getCheckedAttr(setting.type == BooleanAccessoryType);
    String integerAttribute = _getCheckedAttr(setting.type == IntegerAccessoryType);
    String html = "\
      <fieldset>\
        <legend>Accessory Type</legend>\
        " + _renderSelectionList({
          { "Boolean - Accessory with boolean value such as switcher(on/off), shake sensor(yes/no)", "AccessoryType", "boolean", "radio", booleanAttribute },
          { "Integer - Accessory with integer value such as temperature, humidness", "AccessoryType", "integer", "radio", integerAttribute },
        }) + "\
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
        " + _renderSelectionList({
          { "On/Yes/True", "BooleanValue", "true", "radio", trueAttribute },
          { "Off/No/False", "BooleanValue", "false", "radio", falseAttribute },
        }) + "\
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
      String wifiReset = _server->arg("WifiReset");
      if (wifiReset == "1") {
        // wifi_config_reset();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP_STA);
        console.log("Wifi mode: WIFI_AP_STA");
      }
      if (_server->arg("AccessoryReset") == "1" && onResetAccessory) {
        onResetAccessory();
      }
      if (_server->arg("EspRestart") == "1") {
        // sdk_system_restart();
        ESP.restart();
      }
      if (_server->arg("EspReset") == "1") {
        ESP.reset();
      }
      if (_server->arg("EspEraseConfig") == "1") {
        ESP.eraseConfig();
      }
      _redirectTo("/");
    } else {
      _send200("\
        <p><a href=\"/\">&lt; Home</a></p>\
        <h3>Reset</h3>\
        <form method=\"post\" action=\"/reset\">\
          " + _renderSelectionList({
            { "Reset Wifi", "WifiReset", "1", "checkbox", "" },
            { "Accessory Reset", "AccessoryReset", "1", "checkbox", "" },
            { "ESP Restart", "EspRestart", "1", "checkbox", "" },
            { "ESP Reset", "EspReset", "1", "checkbox", "" },
            { "ESP Erase Config", "EspEraseConfig", "1", "checkbox", "" },
          }) + "\
          <p><input type=\"submit\" class=\"button\" value=\"Submit\" /></p>\
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
      <p><a href=\"/\">&lt; Home</a></p>\
      <fieldset>\
        <legend>Oops...</legend>\
        <p>Resource Not Found</p>\
        <p>URI: " + _server->uri() + "</p>\
        <p>Method: " + method + "</p>\
      </fieldset>\
    ");
    _dispatchRequestEnd();
  }

} // namespace Victoria::Components
