#include "WebPortal.h"

namespace Victoria::Components {

  WebPortal::WebPortal(int port) {
    _server = new ESP8266WebServer(port);
  }

  WebPortal::~WebPortal() {
    if (_server) {
      _server->stop();
      delete _server;
      _server = NULL;
    }
  }

  void WebPortal::setup() {
    WiFiMode_t wifiMode = WiFi.getMode();
    bool apEnabled = ((wifiMode & WIFI_AP) != 0);
    bool staEnabled = ((wifiMode & WIFI_STA) != 0);
    if (!apEnabled && !staEnabled) {
      WiFi.mode(WIFI_AP_STA);
      wifiMode = WIFI_AP_STA;
    }

    String hostName = getHostName(true);
    bool isApEnabled = ((wifiMode & WIFI_AP) != 0);
    if (isApEnabled) {
      // IPAddress apIp(192, 168, 1, 33);
      // IPAddress apSubnet(255, 255, 255, 0);
      // WiFi.softAPConfig(apIp, apIp, apSubnet);
      WiFi.softAP(hostName); // name which is displayed on AP list
      IPAddress currentApIp = WiFi.softAPIP();
      if (currentApIp) {
        console.log("Wifi > AP Address > " + currentApIp.toString());
      }
    }

    WiFi.hostname(hostName); // name which is displayed on router
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin();
    WiFi.onEvent(WebPortal::_onWifiEvent, WiFiEvent::WIFI_EVENT_ANY);

    _server->on("/", HTTP_GET, std::bind(&WebPortal::_handleRoot, this));
    _server->on("/system", HTTP_GET, std::bind(&WebPortal::_handleSystem, this));
    _server->on("/system/file", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/system/file", HTTP_ANY, std::bind(&WebPortal::_handleSystemFile, this));
    _server->on("/wifi/list", HTTP_GET, std::bind(&WebPortal::_handleWifiList, this));
    _server->on("/wifi/join", HTTP_POST, std::bind(&WebPortal::_handleWifiJoin, this));
    _server->on("/radio", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/radio", HTTP_ANY, std::bind(&WebPortal::_handleRadio, this));
    _server->on("/service/new", HTTP_GET, std::bind(&WebPortal::_handleNewService, this));
    _server->on("/service", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/service", HTTP_ANY, std::bind(&WebPortal::_handleService, this));
    _server->on("/service/state", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/service/state", HTTP_ANY, std::bind(&WebPortal::_handleServiceState, this));
    _server->on("/reset", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/reset", HTTP_ANY, std::bind(&WebPortal::_handleReset, this));
    _server->onNotFound(std::bind(&WebPortal::_handleNotFound, this));
    _server->begin();
  }

  void WebPortal::loop() {
    _server->handleClient();
  }

  String WebPortal::getHostName(bool includeVersion) {
    String id = WiFi.macAddress();
    id.replace(":", "");
    id.toUpperCase();
    id = id.substring(id.length() - 5);

    String version = String(FirmwareVersion);
    version.replace(".", "");

    String productName = FirmwareName;
    String hostName = includeVersion
      ? productName + "-" + id + "-" + version
      : productName + "-" + id;

    return hostName;
  }

  std::pair<bool, ServiceSetting> WebPortal::_getService(const String& serviceId) {
    bool found = false;
    auto model = serviceStorage.load();
    ServiceSetting service;
    if (model.services.count(serviceId) > 0) {
      service = model.services[serviceId];
      found = true;
    }
    if (!found) {
      _sendHints("Service Not Found", "\
        <p>Service ID: " + serviceId + "</p>\
      ");
    }
    return std::make_pair(found, service);
  }

  void WebPortal::_saveService(const String& serviceId, const ServiceSetting& service) {
    auto model = serviceStorage.load();
    model.services[serviceId] = service;
    serviceStorage.save(model);
    if (onSaveService) {
      onSaveService(serviceId, service);
    }
  }

  void WebPortal::_deleteService(const String& serviceId, const ServiceSetting& service) {
    auto model = serviceStorage.load();
    model.services.erase(serviceId);
    serviceStorage.save(model);
    if (onDeleteService) {
      onDeleteService(serviceId, service);
    }
  }

  void WebPortal::_redirectTo(const String& url) {
    _server->sendHeader("Location", url, true);
    _server->send(302, "text/plain", "");
  }

  void WebPortal::_send200(const String& bodyHtml) {
    _server->send(200, "text/html", _formatPage(bodyHtml));
  }

  void WebPortal::_send404(const String& bodyHtml) {
    _server->send(404, "text/html", _formatPage(bodyHtml));
  }

  void WebPortal::_sendHints(const String& title, const String& message) {
    _send200("\
      <p><a href=\"/\">&lt; Home</a></p>\
      <fieldset>\
        <legend>" + title + "</legend>\
        " + message + "\
      </fieldset>\
    ");
  }

  String WebPortal::_formatPage(const String& bodyHtml) {
    String productName = FirmwareName;
    return "\
      <!DOCTYPE HTML>\
      <html>\
        <head>\
          <title>" + productName + "</title>\
          <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
          <style>\
            html, body { background: transparent; font-family: Arial, Sans-Serif; margin: 0; padding: 0; border: 0; color: #4f4e4e; }\
            h1, h2, h3 { font-weight: 400; }\
            a { color: #00979d; text-decoration: none; }\
            b { color: #005c5f; }\
            fieldset { border: 1px solid #00979c; }\
            ul { padding: 0; list-style-type: none; }\
            td { border-bottom: 1px solid #d5e9e9; }\
            .main { padding: 0 10px; font-size: 14px; }\
            .title { background: #008184; color: #ffffff; text-align: center; margin: 0; padding: 5px 0; }\
            .btn { background-color: #005c5f; border: 1px solid #005c5f; color: #ffffff; border-radius: 5px; padding: 5px 10px; box-shadow: none; }\
            .btnWeak { background: none; border: none; padding: 0; color: #00979d; }}\
          </style>\
        </head>\
        <body>\
          <h2 class=\"title\">" + productName + "</h2>\
          <div class=\"main\">\
            " + bodyHtml + "\
          </div>\
        </body>\
      </html>\
    ";
  }

  void WebPortal::_dispatchRequestStart() {
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

  void WebPortal::_dispatchRequestEnd() {
    if (onRequestEnd) {
      onRequestEnd();
    }
  }

  void WebPortal::_handleRoot() {
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
    // ssid
    String ssidJoined = WiFi.SSID();
    // mac
    String macAddr = WiFi.macAddress();
    // services
    auto model = serviceStorage.load();
    String randomId = CommonHelpers::randomString(4);
    String newServiceUrl = "/service/new?id=" + randomId + "&index=" + String(model.services.size() + 1);
    String serviceLinks = "\
      <a href=\"" + newServiceUrl + "\">Add+</a>\
    ";
    for (const auto& pair : model.services) {
      String url = ("/service?id=" + pair.first);
      serviceLinks += "\
        | <a href=\"" + url + "\">" + pair.second.name + "</a>\
      ";
    }
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
        <a href=\"/radio\">Radio</a> |\
        <a href=\"/system\">System</a> |\
        <a href=\"/reset\">Reset</a>\
      </p>\
      <h3>Home</h3>\
      <p>\
        " + _renderTable(table) + "\
      </p>\
      <h3>Services</h3>\
      <p>\
        " + serviceLinks + "\
      </p>\
    ");
    _dispatchRequestEnd();
  }

  // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
  void WebPortal::_handleSystem() {
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
        console.error("LittleFS > Read fs info failed");
      }
    } else {
      console.error("LittleFS > Failed to mount file system");
    }
    LittleFS.end();
    _dispatchRequestEnd();
  }

  void WebPortal::_handleSystemFile() {
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
        console.error("LittleFS > Failed to open file " + fileName);
      }
    } else {
      console.error("LittleFS > Failed to mount file system");
    }
    LittleFS.end();
    _dispatchRequestEnd();
  }

  void WebPortal::_handleWifiList() {
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
      <h3>Join WiFi</h3>\
      <form method=\"post\" action=\"/wifi/join\">\
        <ul>" + list + "</ul>\
        <p>\
          <label for=\"txtPassword\">Password:</label>\
          <input type=\"text\" id=\"txtPassword\" name=\"password\" length=\"64\" />\
        </p>\
        <p><input type=\"submit\" class=\"btn\" value=\"Join\" /></p>\
      </form>\
    ");
    _dispatchRequestEnd();
  }

  void WebPortal::_handleWifiJoin() {
    _dispatchRequestStart();
    String ssid = _server->arg("ssid");
    String password = _server->arg("password");

    if (!ssid || ssid == "") {
      _sendHints("Failed", "\
        <p>Please select wifi to join</p>\
      ");
      _dispatchRequestEnd();
      return;
    }

    console.log("Wifi > SSID > " + ssid);
    console.log("Wifi > Password > " + password);
    WiFi.persistent(true);
    WiFi.begin(ssid, password);
    // Wait for connecting
    int checkTimes = 60;
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
    console.log("Wifi > Connected > " + String(isConnected));
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

  void WebPortal::_handleRadio() {
    _dispatchRequestStart();
    auto model = radioStorage.load();
    if (_server->method() == HTTP_POST) {
      String inputPin = _server->arg("InputPin");
      model.inputPin = inputPin.toInt();
      radioStorage.save(model);
      _redirectTo(_server->uri());
    } else {
      RadioMessage lastReceived = radioStorage.getLastReceived();
      bool hasValue = lastReceived.value > 0;
      TableModel table = {
        .header = {},
        .rows = {
          { "Value", hasValue ? String(lastReceived.value) : "-" },
          { "Bits", hasValue ? String(lastReceived.bits) : "-" },
          { "Protocol", hasValue ? String(lastReceived.protocol) : "-" },
        },
      };
      _send200("\
        <p>\
          <a href=\"/\">&lt; Home</a>\
        </p>\
        <h3>Radio</h3>\
        <form method=\"post\">\
          <p>\
            <label for=\"txtInputPin\">Input Pin</label>\
            <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" value=\"" + String(model.inputPin) + "\" />\
          </p>\
          <p>\
            <label>Last received " + (hasValue ? CommonHelpers::timeSince(lastReceived.timestamp) + " ago" : "-") + "</label>\
            " + _renderTable(table) + "\
          </p>\
          <p>\
            <input type=\"submit\" class=\"btn\" name=\"Submit\" value=\"Save\" />\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleNewService() {
    _dispatchRequestStart();
    String serviceId = _server->arg("id");
    String serviceIndex = _server->arg("index");
    // new
    ServiceSetting newSetting = {
      .name = "New" + serviceIndex,
      .type = BooleanServiceType,
      .outputPin = -1,
      .inputPin = -1,
      .outputLevel = -1,
      .inputLevel = -1,
    };
    _saveService(serviceId, newSetting);
    // redirect
    String url = "/service?id=" + serviceId;
    _redirectTo(url);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleService() {
    _dispatchRequestStart();
    String serviceId = _server->arg("id");
    String currentUrl = "/service?id=" + serviceId;
    std::pair<bool, ServiceSetting> found = _getService(serviceId);
    ServiceSetting service = found.second;
    if (!found.first) {
      _dispatchRequestEnd();
      return;
    }
    if (_server->method() == HTTP_POST) {
      String serviceName = _server->arg("ServiceName");
      String serviceType = _server->arg("ServiceType");
      String outputPin = _server->arg("OutputPin");
      String inputPin = _server->arg("InputPin");
      String outputLevel = _server->arg("OutputLevel");
      String inputLevel = _server->arg("InputLevel");
      String submit = _server->arg("Submit");
      if (submit == "Delete") {
        _deleteService(serviceId, service);
        _redirectTo("/");
      } else {
        service.name = serviceName;
        service.type =
          serviceType == "boolean" ? BooleanServiceType :
          serviceType == "integer" ? IntegerServiceType : EmptyServiceType;
        service.outputPin = outputPin.toInt();
        service.inputPin = inputPin.toInt();
        service.outputLevel = outputLevel.toInt();
        service.inputLevel = inputLevel.toInt();
        _saveService(serviceId, service);
        _redirectTo(currentUrl);
      }
    } else {
      _send200("\
        <p>\
          <a href=\"/\">&lt; Home</a> |\
          <a href=\"/service/state?id=" + serviceId + "\">State</a>\
        </p>\
        <h3>Setting (" + service.name + ")</h3>\
        <form method=\"post\">\
          <p>\
            <label for=\"txtServiceName\">Name</label>\
            <input type=\"text\" id=\"txtServiceName\" name=\"ServiceName\" value=\"" + service.name + "\" />\
          </p>\
          " + _getTypeHtml(service) + "\
          " + _getIOHtml(service) + "\
          <p>\
            <input type=\"submit\" class=\"btn\" name=\"Submit\" value=\"Save\" />\
            <input type=\"submit\" class=\"btnWeak\" name=\"Submit\" value=\"Delete\" onclick=\"return confirm('Are you sure you want to delete?')\" />\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceState() {
    _dispatchRequestStart();
    String serviceId = _server->arg("id");
    String backUrl = "/service?id=" + serviceId;
    String currentUrl = "/service/state?id=" + serviceId;
    std::pair<bool, ServiceSetting> found = _getService(serviceId);
    ServiceSetting service = found.second;
    if (!found.first) {
      _dispatchRequestEnd();
      return;
    }
    ServiceState state = {
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
      if (onSetServiceState) {
        onSetServiceState(serviceId, service, state);
      }
      _redirectTo(currentUrl);
    } else {
      if (onGetServiceState) {
        state = onGetServiceState(serviceId, service);
      }
      String stateHtml =
        service.type == BooleanServiceType ? _getBooleanHtml(state) :
        service.type == IntegerServiceType ? _getIntegerHtml(state) : "";
      _send200("\
        <p>\
          <a href=\"" + backUrl + "\">&lt; Setting (" + service.name + ")</a>\
        </p>\
        <h3>State (" + service.name + ")</h3>\
        <form method=\"post\">\
          " + stateHtml + "\
          <p>\
            <input type=\"submit\" class=\"btn\" name=\"Submit\" value=\"Save\" />\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  String WebPortal::_getCheckedAttr(bool checked) {
    return checked ? " checked=\"checked\"" : "";
  }

  String WebPortal::_renderTable(const TableModel& model) {
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

  String WebPortal::_renderSelectionList(std::vector<std::vector<String>> list) {
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

  String WebPortal::_getTypeHtml(const ServiceSetting& service) {
    String booleanAttribute = _getCheckedAttr(service.type == BooleanServiceType);
    String integerAttribute = _getCheckedAttr(service.type == IntegerServiceType);
    String html = "\
      <fieldset>\
        <legend>Service Type</legend>\
        " + _renderSelectionList({
          { "Boolean - Service with boolean value such as switcher(on/off), shake sensor(yes/no)", "ServiceType", "boolean", "radio", booleanAttribute },
          { "Integer - Service with integer value such as temperature, humidness", "ServiceType", "integer", "radio", integerAttribute },
        }) + "\
      </fieldset>\
    ";
    return html;
  }

  String WebPortal::_getIOHtml(const ServiceSetting& service) {
    String html = "\
      <fieldset>\
        <legend>IO Pins</legend>\
        <p>\
          <label for=\"txtOutputPin\">Output</label>\
          <input type=\"number\" id=\"txtOutputPin\" name=\"OutputPin\" value=\"" + String(service.outputPin) + "\" />\
          " + _getLevelHtml("OutputLevel", service.outputLevel) + "\
        </p>\
        <p>\
          <label for=\"txtInputPin\">Input</label>\
          <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" value=\"" + String(service.inputPin) + "\" />\
          " + _getLevelHtml("InputLevel", service.inputLevel) + "\
        </p>\
      </fieldset>\
    ";
    return html;
  }

  String WebPortal::_getLevelHtml(const String name, const short int level) {
    return "\
      <label for=\"txt" + name + "High\">High</label>\
      <input type=\"radio\" id=\"txt" + name + "High\" name=\"" + name + "\" value=\"1\"" + _getCheckedAttr(level == 1) + " />\
      <label for=\"txt" + name + "Low\">Low</label>\
      <input type=\"radio\" id=\"txt" + name + "Low\" name=\"" + name + "\" value=\"0\"" + _getCheckedAttr(level == 0) + " />\
      <label for=\"txt" + name + "No\">No</label>\
      <input type=\"radio\" id=\"txt" + name + "No\" name=\"" + name + "\" value=\"-1\"" + _getCheckedAttr(level == -1) + " />\
    ";
  }

  String WebPortal::_getBooleanHtml(const ServiceState& state) {
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

  String WebPortal::_getIntegerHtml(const ServiceState& state) {
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

  void WebPortal::_onWifiEvent(WiFiEvent_t event) {
    switch (event) {
      case WiFiEvent::WIFI_EVENT_STAMODE_CONNECTED:
        console.log("Wifi > Event > STA connected");
        break;
      case WiFiEvent::WIFI_EVENT_STAMODE_DISCONNECTED:
        console.log("Wifi > Event > STA disconnected");
        break;
      case WiFiEvent::WIFI_EVENT_STAMODE_GOT_IP:
        console.log("Wifi > Event > STA got ip");
        break;
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STACONNECTED:
        console.log("Wifi > Event > AP connected");
        break;
      case WiFiEvent::WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
        console.log("Wifi > Event > AP disconnected");
        break;
      default:
        break;
    }
  }

  void WebPortal::_handleReset() {
    _dispatchRequestStart();
    if (_server->method() == HTTP_POST) {
      String wifiReset = _server->arg("WifiReset");
      if (wifiReset == "1") {
        // wifi_config_reset();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP_STA);
        console.log("Wifi > Mode > WIFI_AP_STA");
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
        <form method=\"post\">\
          " + _renderSelectionList({
            { "Reset Wifi", "WifiReset", "1", "checkbox", "" },
            { "Reset Accessory", "AccessoryReset", "1", "checkbox", "" },
            { "ESP Restart", "EspRestart", "1", "checkbox", "" },
            { "ESP Reset", "EspReset", "1", "checkbox", "" },
            { "ESP Erase Config", "EspEraseConfig", "1", "checkbox", "" },
          }) + "\
          <p><input type=\"submit\" class=\"btn\" value=\"Submit\" /></p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleNotFound() {
    _dispatchRequestStart();
    String method = (_server->method() == HTTP_GET) ? "GET" : "POST";
    _sendHints("Resource Not Found", "\
      <p>URI: " + _server->uri() + "</p>\
      <p>Method: " + method + "</p>\
    ");
    _dispatchRequestEnd();
  }

  void WebPortal::_handleCrossOrigin() {
    _dispatchRequestStart();
    _server->send(204);
    _dispatchRequestEnd();
  }

} // namespace Victoria::Components
