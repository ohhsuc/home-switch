#include "VictoriaWeb.h"

namespace Victoria::Components {

  VictoriaWeb::VictoriaWeb(int port) {
    _server = new ESP8266WebServer(port);
  }

  VictoriaWeb::~VictoriaWeb() {
    if (_server) {
      _server->stop();
      delete _server;
      _server = NULL;
    }
    if (_httpUpdater) {
      delete _httpUpdater;
      _httpUpdater = NULL;
    }
  }

  void VictoriaWeb::setup() {
    _registerHandlers();
    auto model = appStorage.load();
    if (model.overTheWeb) {
      _httpUpdater = new ESP8266HTTPUpdateServer();
      _httpUpdater->setup(_server, F("/update"));
    }
    _server->begin();
  }

  void VictoriaWeb::loop() {
    _server->handleClient();
  }

  void VictoriaWeb::_registerHandlers() {
    _server->serveStatic("/fav.ico", LittleFS, "/web/fav.ico", "max-age=43200");
    _server->serveStatic("/style.min.css", LittleFS, "/web/style.min.css", "max-age=43200");
    _server->serveStatic("/mithril.min.js", LittleFS, "/web/mithril.min.js", "max-age=43200");
    _server->serveStatic("/app.min.js", LittleFS, "/web/app.min.js");
    _server->on(F("/"), HTTP_GET, std::bind(&VictoriaWeb::_handleIndex, this));
    _server->on(F("/home"), HTTP_GET, std::bind(&VictoriaWeb::_handleHome, this));
    _server->on(F("/fs"), HTTP_GET, std::bind(&VictoriaWeb::_handleFileSystem, this));
    _server->on(F("/file"), HTTP_GET, std::bind(&VictoriaWeb::_handleFileGet, this));
    _server->on(F("/file"), HTTP_POST, std::bind(&VictoriaWeb::_handleFileSave, this));
    _server->on(F("/file"), HTTP_DELETE, std::bind(&VictoriaWeb::_handleFileDelete, this));
    _server->on(F("/wifi"), HTTP_GET, std::bind(&VictoriaWeb::_handleWifi, this));
    _server->on(F("/wifi/join"), HTTP_POST, std::bind(&VictoriaWeb::_handleWifiJoin, this));
    _server->on(F("/ota"), HTTP_GET, std::bind(&VictoriaWeb::_handleOta, this));
    _server->on(F("/ota/fire"), HTTP_POST, std::bind(&VictoriaWeb::_handleOtaFire, this));
    _server->on(F("/reset"), HTTP_POST, std::bind(&VictoriaWeb::_handleReset, this));
    _server->onNotFound(std::bind(&VictoriaWeb::_handleNotFound, this));
  }

  void VictoriaWeb::_solvePageTokens(String& html) {
    auto productName = FirmwareName;
    html.replace(F("{productName}"), productName);
  }

  void VictoriaWeb::_sendHtml(const String& html) {
    _server->send(200, F("text/html"), html);
  }

  void VictoriaWeb::_sendJson(DynamicJsonDocument res) {
    String buf;
    serializeJson(res, buf);
    _server->send(200, F("application/json"), buf);
  }

  void VictoriaWeb::_dispatchRequestStart() {
    _server->sendHeader(F("Connection"), F("keep-alive")); // close / keep-alive
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

  void VictoriaWeb::_dispatchRequestEnd() {
    if (onRequestEnd) {
      onRequestEnd();
    }
  }

  void VictoriaWeb::_handleIndex() {
    _dispatchRequestStart();
    auto path = String(F("/web/index.htm"));
    auto file = LittleFS.open(path, "r");
    auto html = file.readString();
    file.close();
    _solvePageTokens(html);
    html.replace(F("{productName}"), "");
    html.replace(F("{appendHead}"), "");
    html.replace(F("{appendBody}"), "");
    _sendHtml(html);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleHome() {
    _dispatchRequestStart();
    // wifi
    auto ssidJoined = WiFi.SSID();
    auto wifiMode = WiFi.getMode();
    auto strWifiMode = String(F("WIFI_OFF"));
    if (wifiMode == WIFI_STA) {
      strWifiMode = F("WIFI_STA");
    } else if (wifiMode == WIFI_AP) {
      strWifiMode = F("WIFI_AP");
    } else if (wifiMode == WIFI_AP_STA) {
      strWifiMode = F("WIFI_AP_STA");
    }
    // station
    auto staAddress = String("");
    auto staMacAddress = WiFi.macAddress();
    auto isStaEnabled = ((wifiMode & WIFI_STA) != 0);
    if (isStaEnabled) {
      IPAddress localIP = WiFi.localIP();
      if (localIP) {
        staAddress = localIP.toString();
      }
    }
    // access point
    auto apAddress = String("");
    auto apMacAddress = WiFi.softAPmacAddress();
    auto isApEnabled = ((wifiMode & WIFI_AP) != 0);
    if (isApEnabled) {
      IPAddress apIP = WiFi.softAPIP();
      if (apIP) {
        apAddress = apIP.toString();
      }
    }
    // send
    DynamicJsonDocument res(512);
    res[F("running")] = GlobalHelpers::timeSince(0);
    res[F("wifiMode")] = strWifiMode;
    res[F("joined")] = ssidJoined;
    res[F("staAddress")] = staAddress;
    res[F("staMacAddress")] = staMacAddress;
    res[F("apAddress")] = apAddress;
    res[F("apMacAddress")] = apMacAddress;
    res[F("firmwareVersion")] = FirmwareVersion;
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleFileSystem() {
    _dispatchRequestStart();
    FSInfo fsInfo;
    DynamicJsonDocument res(512);
    if (LittleFS.info(fsInfo)) {
      res[F("totalBytes")] = fsInfo.totalBytes;
      res[F("usedBytes")] = fsInfo.usedBytes;
      res[F("maxPathLength")] = fsInfo.maxPathLength;
      res[F("maxOpenFiles")] = fsInfo.maxOpenFiles;
      res[F("blockSize")] = fsInfo.blockSize;
      res[F("pageSize")] = fsInfo.pageSize;
      // files
      // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
      JsonObject filesObj = res.createNestedObject(F("files"));
      std::function<void(String)> loopFile;
      loopFile = [&](String path)->void {
        Dir dir = LittleFS.openDir(path);
        while(dir.next()) {
          if (dir.isDirectory()) {
            loopFile(path + dir.fileName() + F("/"));
          } else if (dir.isFile()) {
            auto fullName = path + dir.fileName();
            auto size = dir.fileSize();
            filesObj[fullName] = size;
          }
        }
      };
      loopFile(F("/"));
    } else {
      auto error = String(F("read fs info failed"));
      res[F("error")] = error;
      console.error(error);
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleFileGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(2048); // 2k
    auto path = _server->arg(F("path"));
    auto file = LittleFS.open(path, "r");
    if (file) {
      res[F("size")] = file.size();
      res[F("name")] = String(file.name());
      res[F("content")] = file.readString();
      file.close();
    } else {
      auto error = String(F("failed to open file"));
      res[F("error")] = error;
      console.error(error);
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleFileSave() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(2048); // 2k
    deserializeJson(payload, payloadJson);
    // write
    DynamicJsonDocument res(64);
    auto path = _server->arg(F("path"));
    auto file = LittleFS.open(path, "w");
    if (file) {
      auto content = String(payload[F("content")]);
      file.print(content);
      file.close();
      res[F("message")] = String(F("success"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleFileDelete() {
    _dispatchRequestStart();
    auto path = _server->arg(F("path"));
    LittleFS.remove(path);
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleWifi() {
    _dispatchRequestStart();
    DynamicJsonDocument res(1024);
    res[F("connected")] = WiFi.SSID();
    JsonArray apArr = res.createNestedArray(F("founds"));
    auto count = WiFi.scanNetworks();
    for (int8_t i = 0; i < count; ++i) {
      JsonObject apObj = apArr.createNestedObject();
      apObj[F("ssid")] = WiFi.SSID(i);
      apObj[F("rssi")] = WiFi.RSSI(i);
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleWifiJoin() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(128);
    deserializeJson(payload, payloadJson);
    // read
    auto ssid = String(payload[F("ssid")]);
    auto password = String(payload[F("password")]);
    // res
    DynamicJsonDocument res(64);
    if (!ssid || ssid == "") {
      res[F("error")] = String(F("Please select wifi to join"));
    } else {
      VictoriaWifi::join(ssid, password, true);
      auto isConnected = WiFi.status() == WL_CONNECTED;
      console.log(F("[Wifi] connected > ") + String(isConnected));
      if (isConnected) {
        res[F("ip")] = WiFi.localIP().toString();
      } else {
        res[F("error")] = String(F("failed"));
      }
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleOta() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    res[F("chipId")] = ESP.getChipId();
    res[F("chipSize")] = ESP.getFlashChipRealSize();
    res[F("sketchSize")] = ESP.getSketchSize();
    res[F("sketchFreeSpace")] = ESP.getFreeSketchSpace();
    res[F("sketchMD5")] = ESP.getSketchMD5();
    res[F("sdkVersion")] = ESP.getSdkVersion();
    res[F("otaVersion")] = VictoriaOTA::getCurrentVersion();
    res[F("otaNewVersion")] = VictoriaOTA::checkNewVersion();
    res[F("overTheWeb")] = _httpUpdater != NULL;
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleOtaFire() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(64);
    deserializeJson(payload, payloadJson);
    // read
    auto version = String(payload[F("version")]);
    auto otaType = String(payload[F("otaType")]);
    auto type =
      otaType == F("all") ? VOta_All :
      otaType == F("fs") ? VOta_FileSystem :
      otaType == F("sketch") ? VOta_Sketch : VOta_Sketch;
    // update
    VictoriaOTA::update(version, type);
    // res
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleReset() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(64);
    deserializeJson(payload, payloadJson);
    // read
    auto values = String(payload[F("values")]);
    // action
    if (values.indexOf(F("EspRestart")) >= 0) {
      // sdk_system_restart();
      ESP.restart();
    }
    if (values.indexOf(F("EspReset")) >= 0) {
      ESP.reset();
    }
    if (values.indexOf(F("EspEraseCfg")) >= 0) {
      ESP.eraseConfig();
    }
    if (values.indexOf(F("WifiReset")) >= 0) {
      VictoriaWifi::reset();
    }
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictoriaWeb::_handleNotFound() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    res[F("method")] = (_server->method() == HTTP_GET) ? String(F("GET")) : String(F("POST"));
    res[F("uri")] = _server->uri();
    res[F("error")] = String(F("Resource Not Found"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

} // namespace Victoria::Components
