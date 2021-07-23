#include "VictoriaWeb.h"

namespace Victoria::Components {

  VictoriaWeb::VictoriaWeb(int port = 80) {
    _server = new ESP8266WebServer(port);
  }

  VictoriaWeb::~VictoriaWeb() {
    if (_server) {
      _server->stop();
      delete _server;
      _server = NULL;
    }
  }

  void VictoriaWeb::setup() {
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
    WiFi.onEvent(VictoriaWeb::_onWifiEvent, WiFiEvent::WIFI_EVENT_ANY);

    _registerHandlers();
    _server->begin();
  }

  void VictoriaWeb::loop() {
    _server->handleClient();
  }

  void VictoriaWeb::_registerHandlers() {
    _server->onNotFound(std::bind(&VictoriaWeb::_handleNotFound, this));
    _server->on("/", HTTP_GET, std::bind(&VictoriaWeb::_handleHome, this));
    _server->on("/system", HTTP_GET, std::bind(&VictoriaWeb::_handleSystem, this));
    _server->on("/system/file", HTTP_OPTIONS, std::bind(&VictoriaWeb::_handleCrossOrigin, this));
    _server->on("/system/file", HTTP_ANY, std::bind(&VictoriaWeb::_handleSystemFile, this));
    _server->on("/wifi/list", HTTP_GET, std::bind(&VictoriaWeb::_handleWifiList, this));
    _server->on("/wifi/join", HTTP_POST, std::bind(&VictoriaWeb::_handleWifiJoin, this));
    _server->on("/reset", HTTP_OPTIONS, std::bind(&VictoriaWeb::_handleCrossOrigin, this));
    _server->on("/reset", HTTP_ANY, std::bind(&VictoriaWeb::_handleReset, this));
    _server->serveStatic("/fav", LittleFS, "/fav.ico", "max-age=43200");
    _server->serveStatic("/css", LittleFS, "/style.css", "max-age=43200");
    _server->serveStatic("/tmpl", LittleFS, "/tmpl.min.js", "max-age=43200");
    _server->serveStatic("/js", LittleFS, "/app.js", "max-age=43200");
  }

  String VictoriaWeb::getHostName(bool includeVersion = false) {
    auto id = WiFi.macAddress();
    id.replace(":", "");
    id.toUpperCase();
    id = id.substring(id.length() - 6);

    auto version = String(FirmwareVersion);
    version.replace(".", "");

    auto productName = FirmwareName;
    auto hostName = includeVersion
      ? productName + "-" + id + "-" + version
      : productName + "-" + id;

    return hostName;
  }

  void VictoriaWeb::_redirectTo(const String& url) {
    _server->sendHeader("Location", url, true);
    _server->send(302, "text/plain", "");
  }

  void VictoriaWeb::_send200(const String& bodyHtml) {
    _server->send(200, "text/html", _formatPage(bodyHtml));
  }

  void VictoriaWeb::_send404(const String& bodyHtml) {
    _server->send(404, "text/html", _formatPage(bodyHtml));
  }

  void VictoriaWeb::_sendHints(const String& title, const String& message) {
    _send200("\
      <p><a href=\"/\">&lt; Home</a></p>\
      <fieldset>\
        <legend>" + title + "</legend>\
        " + message + "\
      </fieldset>\
    ");
  }

  String VictoriaWeb::_formatPage(const String& bodyHtml) {
    auto productName = FirmwareName;
    return "\
      <!DOCTYPE HTML>\
      <html>\
        <head>\
          <title>" + productName + "</title>\
          <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
          <link rel=\"icon\" href=\"/fav\">\
          <link rel=\"stylesheet\" href=\"/css\">\
          <script src=\"/tmpl\"></script>\
          <script src=\"/js\"></script>\
        </head>\
        <body>\
          <script type=\"text/x-tmpl\" id=\"html-select\">\
            <select name=\"{%=o.name%}\">\
              {% for (var i=0; i<o.options.length; i++) { var opt=o.options[i]; %}\
              <option value=\"{%=opt.ov%}\"{% if(o.value===opt.ov)print(' selected');%}>{%=opt.ot%}</option>\
              {% } %}\
            </select>\
          </script>\
          <h2 class=\"title\">" + productName + "</h2>\
          <div class=\"main\">\
            " + bodyHtml + "\
          </div>\
        </body>\
      </html>\
    ";
  }

  void VictoriaWeb::_dispatchRequestStart() {
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

  void VictoriaWeb::_dispatchRequestEnd() {
    if (onRequestEnd) {
      onRequestEnd();
    }
  }

  void VictoriaWeb::_handleNotFound() {
    _dispatchRequestStart();
    auto method = (_server->method() == HTTP_GET) ? String("GET") : String("POST");
    _sendHints("Resource Not Found", "\
      <p>URI: " + _server->uri() + "</p>\
      <p>Method: " + method + "</p>\
    ");
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleCrossOrigin() {
    _dispatchRequestStart();
    _server->send(204);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleHome() {
    _dispatchRequestStart();
    // mode
    auto wifiMode = WiFi.getMode();
    auto strWifiMode = String("WIFI_OFF");
    if (wifiMode == WIFI_STA) {
      strWifiMode = "WIFI_STA";
    } else if (wifiMode == WIFI_AP) {
      strWifiMode = "WIFI_AP";
    } else if (wifiMode == WIFI_AP_STA) {
      strWifiMode = "WIFI_AP_STA";
    }
    // ip
    auto strLocalIP = String("");
    auto isStaEnabled = ((wifiMode & WIFI_STA) != 0);
    if (isStaEnabled) {
      IPAddress localIP = WiFi.localIP();
      if (localIP) {
        strLocalIP = localIP.toString();
      }
    }
    // ap
    auto strApIP = String("");
    auto isApEnabled = ((wifiMode & WIFI_AP) != 0);
    if (isApEnabled) {
      IPAddress apIP = WiFi.softAPIP();
      if (apIP) {
        strApIP = apIP.toString();
      }
    }
    // ssid
    auto ssidJoined = WiFi.SSID();
    // mac
    auto macAddr = WiFi.macAddress();
    // table
    TableModel table = {
      .header = {},
      .rows = {
        { "Wifi Mode", strWifiMode },
        { "Joined", ssidJoined != "" ? ssidJoined : "-" },
        { "IP Address", strLocalIP != "" ? "<a href=\"http://" + strLocalIP + "\">" + strLocalIP + "</a>" : "-" },
        { "MAC Address", macAddr },
        { "AP Address", strApIP != "" ? "<a href=\"http://" + strApIP + "\">" + strApIP + "</a>" : "-" },
        { "Firmware Version", FirmwareVersion },
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
      " + _appendHomeBody() + "\
    ");
    _dispatchRequestEnd();
  }

  String VictoriaWeb::_appendHomeBody() {
    return "";
  }

  // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
  void VictoriaWeb::_handleSystem() {
    _dispatchRequestStart();
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
        .header = { "File", "Bytes" },
      };
      // loop file method 1
      // std::function<void(File)> loopFile;
      // loopFile = [&](File file)->void {
      //   auto next = file.openNextFile();
      //   file.close();
      //   if (!next) {
      //     return;
      //   }
      //   if (next.isFile()) {
      //     auto path = "/" + String(next.fullName());
      //     auto url = String("/system/file?path=" + GlobalHelpers::urlEncode(path));
      //     filesTable.rows.push_back({
      //       "<a href=\"" + url + "\">" + path + "</a>",
      //       String(next.size()),
      //     });
      //   }
      //   loopFile(next);
      // };
      // loopFile(LittleFS.open("/", "r"));

      // loop file method 2
      auto dir = LittleFS.openDir("/");
      while (dir.next()) {
        if (dir.fileSize()) {
          auto file = dir.openFile("r");
          auto path = "/" + String(file.fullName());
          auto url = String("/system/file?path=" + GlobalHelpers::urlEncode(path));
          filesTable.rows.push_back({
            "<a href=\"" + url + "\">" + path + "</a>",
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
        <p>\
          " + _renderTable(filesTable) + "\
        </p>\
      ");
    } else {
      console.error("read fs info failed");
    }
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleSystemFile() {
    _dispatchRequestStart();
    auto path = _server->arg("path");
    if (_server->method() == HTTP_POST) {
      auto submit = _server->arg("Submit");
      if (submit == "Delete") {
        LittleFS.remove(path);
        _redirectTo("/system");
      } else {
        auto file = LittleFS.open(path, "w");
        if (file) {
          auto content = _server->arg("Content");
          file.print(content);
          file.close();
        }
        _redirectTo("/system/file?path=" + GlobalHelpers::urlEncode(path));
      }
    } else {
      auto file = LittleFS.open(path, "r");
      if (file) {
        auto size = file.size();
        auto name = String(file.name());
        auto content = file.readString();
        file.close();
        _send200("\
          <p><a href=\"/system\">&lt; System</a></p>\
          <h3>" + name + " " + String(size) + " bytes</h3>\
          <form method=\"post\">\
            <p>\
              <textarea name=\"Content\" cols=\"50\" rows=\"10\">" + content + "</textarea>\
            </p>\
            <p>\
              <button type=\"submit\" name=\"Submit\" value=\"Save\" class=\"btn\">Save</button>\
              <button type=\"submit\" name=\"Submit\" value=\"Delete\" class=\"btnWeak confirm\">Delete</button>\
            </p>\
          </form>\
        ");
      } else {
        console.error("failed to open file " + path);
      }
    }
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleWifiList() {
    _dispatchRequestStart();
    auto list = String("");
    auto current = WiFi.SSID();
    auto count = WiFi.scanNetworks();
    for (int8_t i = 0; i < count; ++i) {
      auto ssid = WiFi.SSID(i);
      auto checked = _getCheckedAttr(ssid == current);
      list += "\
        <li>\
          <input type=\"radio\" id=\"rdoSsid" + String(i) + "\" name=\"ssid\" value=\"" + ssid + "\"" + checked + " />\
          <label for=\"rdoSsid" + String(i) + "\">" + ssid + "</label>\
        </li>\
      ";
    }
    _send200("\
      <p><a href=\"/\">&lt; Home</a></p>\
      <h3>Join WiFi</h3>\
      <form method=\"post\" action=\"/wifi/join\">\
        <ul>" + list + "</ul>\
        <p>\
          <label for=\"txtPassword\">Password:</label>\
          <input type=\"text\" id=\"txtPassword\" name=\"password\" maxlength=\"32\" />\
        </p>\
        <p><button type=\"submit\" class=\"btn\">Join</button></p>\
      </form>\
    ");
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleWifiJoin() {
    _dispatchRequestStart();
    auto ssid = _server->arg("ssid");
    auto password = _server->arg("password");

    if (!ssid || ssid == "") {
      _sendHints("Failed", "\
        <p>Please select wifi to join</p>\
      ");
      _dispatchRequestEnd();
      return;
    }

    console.log("[Wifi] ssid > " + ssid);
    console.log("[Wifi] password > " + password);
    WiFi.persistent(true);
    WiFi.begin(ssid, password);
    // Wait for connecting
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
    auto isConnected = WiFi.status() == WL_CONNECTED;
    console.log("[Wifi] connected > " + String(isConnected));
    if (isConnected) {
      _sendHints("Success", "\
        <p>Joined to <b>" + ssid + "</b></p>\
        <p>IP address <b>" + WiFi.localIP().toString() + "</b></p>\
      ");
    } else {
      _sendHints("Failed", "\
        <p>Joining to <b>" + ssid + "</b> failed</p>\
      ");
    }
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleReset() {
    _dispatchRequestStart();
    if (_server->method() == HTTP_POST) {
      _handleResetPost();
      _redirectTo("/");
    } else {
      std::vector<SelectionOptions> items = _getResetList();
      _send200("\
        <p><a href=\"/\">&lt; Home</a></p>\
        <h3>Reset</h3>\
        <form method=\"post\">\
          " + _renderSelectionList(items) + "\
          <p><button type=\"submit\" class=\"btn\">Submit</button></p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  std::vector<SelectionOptions> VictoriaWeb::_getResetList() {
    return {
      { .inputType = "checkbox", .inputName = "EspRestart", .inputValue = "1", .isChecked = false, .labelText = "ESP Restart" },
      { .inputType = "checkbox", .inputName = "EspReset", .inputValue = "1", .isChecked = false, .labelText = "ESP Reset" },
      { .inputType = "checkbox", .inputName = "EspEraseConfig", .inputValue = "1", .isChecked = false, .labelText = "ESP Erase Config" },
      { .inputType = "checkbox", .inputName = "WifiReset", .inputValue = "1", .isChecked = false, .labelText = "Reset Wifi" },
    };
  }

  void VictoriaWeb::_handleResetPost() {
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
    if (_server->arg("WifiReset") == "1") {
      // wifi_config_reset();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_AP_STA);
      console.log("[Wifi] mode > WIFI_AP_STA");
    }
  }

  String VictoriaWeb::_getCheckedAttr(bool checked) {
    return checked ? String(" checked=\"checked\"") : String("");
  }

  String VictoriaWeb::_renderTable(const TableModel& model) {
    auto tableHeader = String("");
    if (model.header.size() > 0) {
      tableHeader += "<tr>";
      for (const auto& headerCell : model.header) {
        tableHeader += "<th class=\"lt\">" + headerCell + "</th>";
      }
      tableHeader += "</tr>";
    }
    auto tableRows = String("");
    for (const auto& row : model.rows) {
      tableRows += "<tr>";
      for (const auto rowCell : row) {
        tableRows += "<td>" + rowCell + "</td>";
      }
      tableRows += "</tr>";
    }
    auto html = String("\
      <table>\
        " + tableHeader + "\
        " + tableRows + "\
      </table>\
    ");
    return html;
  }

  String VictoriaWeb::_renderSelect(const SelectModel& model) {
    auto options = String("");
    for (const auto& option : model.options) {
      auto selected = option.value == model.value ? " selected" : "";
      options += "<option value=\"" + option.value + "\"" + selected + ">" + option.text + "</option>";
    }
    auto html = "<select name=\"" + model.name + "\">" + options + "</select>";
    return html;
  }

  String VictoriaWeb::_renderSelectionList(const std::vector<SelectionOptions>& list) {
    auto html = String("");
    for (const auto& item : list) {
      html += String("\
        <p>\
          <input type=\"" + item.inputType + "\" id=\"id" + item.inputName + item.inputValue + "\" name=\"" + item.inputName + "\" value=\"" + item.inputValue + "\"" + _getCheckedAttr(item.isChecked) + " />\
          <label for=\"id" + item.inputName + item.inputValue + "\">" + item.labelText + "</label>\
        </p>\
      ");
    }
    return html;
  }

  void VictoriaWeb::_onWifiEvent(WiFiEvent_t event) {
    switch (event) {
      case WiFiEvent::WIFI_EVENT_STAMODE_CONNECTED:
        console.log("[Wifi] event > STA connected");
        break;
      case WiFiEvent::WIFI_EVENT_STAMODE_DISCONNECTED:
        console.log("[Wifi] event > STA disconnected");
        break;
      case WiFiEvent::WIFI_EVENT_STAMODE_GOT_IP:
        console.log("[Wifi] event > STA got ip");
        break;
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STACONNECTED:
        console.log("[Wifi] event > AP connected");
        break;
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
        console.log("[Wifi] event > AP disconnected");
        break;
      default:
        break;
    }
  }

} // namespace Victoria::Components
