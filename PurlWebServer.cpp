#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "PurlWebServer.h";

PurlWebServer::PurlWebServer(int port, String productName, String hostName) {
  _server = new ESP8266WebServer(port);
  _productName = productName;
  _hostName = hostName;
  currentState = PurlWebServerState {
    // .isSwitchOn = false,
    isSwitchOn: false,
  };
}

PurlWebServer::~PurlWebServer() {
  _server->stop();
}

void PurlWebServer::setup() {
  WiFiMode_t wifiMode = WiFi.getMode();
  bool apEnabled = ((wifiMode & WIFI_AP) != 0);
  bool staEnabled = ((wifiMode & WIFI_STA) != 0);
  if (!apEnabled && !staEnabled) {
    WiFi.mode(WIFI_AP_STA);
    Serial.println("Wifi mode: WIFI_AP_STA");
  }

  WiFi.hostname(_hostName);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  bool isApEnabled = ((WiFi.getMode() & WIFI_AP) != 0);
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

  _server->on("/", HTTP_GET, std::bind(&PurlWebServer::_handleRoot, this));
  _server->on("/connect", HTTP_POST, std::bind(&PurlWebServer::_handleConnectSsid, this));
  _server->on("/control", HTTP_OPTIONS, std::bind(&PurlWebServer::_handleCrossOrigin, this));
  _server->on("/control", HTTP_ANY, std::bind(&PurlWebServer::_handleControl, this));
  _server->on("/reset", HTTP_OPTIONS, std::bind(&PurlWebServer::_handleCrossOrigin, this));
  _server->on("/reset", HTTP_ANY, std::bind(&PurlWebServer::_handleReset, this));
  _server->onNotFound(std::bind(&PurlWebServer::_handleNotFound, this));
  _server->begin();
}

void PurlWebServer::loop() {
  _server->handleClient();
}

void PurlWebServer::_redirectTo(String url) {
  _server->sendHeader(F("Location"), url, true);
  _server->send(302, "text/plain", "");
}

String PurlWebServer::_formatPage(String htmlBody) {
  return "\
    <!DOCTYPE HTML>\
    <html>\
      <head>\
        <title>" + _productName + "</title>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Sans-Serif; }\
        </style>\
      </head>\
      <body>\
        <h1>Purl Switch</h1>"
      + htmlBody +
      "</body>\
    </html>";
}

void PurlWebServer::_dispatchSetState() {
  if (onSetState) {
    onSetState(currentState);
  }
}

void PurlWebServer::_dispatchGetState() {
  if (onGetState) {
    onGetState(currentState);
  }
}

void PurlWebServer::_dispatchRequestStart() {
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

void PurlWebServer::_dispatchRequestEnd() {
  if (onRequestEnd) {
    onRequestEnd();
  }
}

void PurlWebServer::_dispatchResetAccessory() {
  if (onResetAccessory) {
    onResetAccessory();
  }
}

void PurlWebServer::_handleRoot() {
  _dispatchRequestStart();
  String list = "";
  int count = WiFi.scanNetworks();
  String current = WiFi.SSID();
  for (int i = 0; i < count; ++i) {
    String ssid = WiFi.SSID(i);
    String checked = ssid == current ? " checked=\"checked\"" : "";
    list += "<li>";
    list += "<input type=\"radio\" id=\"ssid_" + String(i) + "\" name=\"ssid\" value=\"" + ssid + "\"" + checked + " />";
    list += "<label for=\"ssid_" + String(i) + "\">";
    list += ssid;
    list += "</label>";
    list += "</li>";
  }
  list += "</ul>";
  String htmlBody = "\
    <p>\
      <a href=\"/control\">Control</a>\
      <a href=\"/reset\">Reset</a>\
    </p>\
    <h3>Connect WiFi</h3>\
    <form enctype=\"text/html\" method=\"post\" action=\"/connect\">"
      + list +
      "<p><label>Password: </label><input name=\"pass\" length=\"64\" /></p>\
      <p><input type=\"submit\" /></p>\
    </form>";
  _server->send(200, "text/html", _formatPage(htmlBody));
  _dispatchRequestEnd();
}

void PurlWebServer::_handleConnectSsid() {
  _dispatchRequestStart();
  String ssid = _server->arg("ssid");
  String pass = _server->arg("pass");

  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
    String ignoreMsg = "\
      <p><a href=\"/\">Back</a></p>\
      <p>Ignore network setup</p>";
    _server->send(200, "text/html", _formatPage(ignoreMsg));
    _dispatchRequestEnd();
    return;
  }

  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);
  WiFi.begin(ssid, pass);
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
      <p><a href=\"/\">Back</a></p>\
      <p>Connected to: " + ssid + "</p>\
      <p>IP address: " + WiFi.localIP().toString() + "</p>";
    _server->send(200, "text/html", _formatPage(successMessage));
  } else {
    String failedMessage = "\
      <p><a href=\"/\">Back</a></p>\
      <p>Connect to: " + ssid + " failed</p>";
    _server->send(200, "text/html", _formatPage(failedMessage));
  }
  _dispatchRequestEnd();
}

void PurlWebServer::_handleControl() {
  _dispatchRequestStart();
  if (_server->method() == HTTP_POST) {
    String state = _server->arg("state");
    currentState.isSwitchOn = (state == "on");
    _dispatchSetState();
    _redirectTo("/control");
  } else {
    _dispatchGetState();
    String onAttribute = currentState.isSwitchOn ? " checked=\"checked\"" : "";
    String offAttribute = currentState.isSwitchOn ? "" : " checked=\"checked\"";
    String htmlBody = "\
      <p><a href=\"/\">Back</a></p>\
      <h3>Control Switch</h3>\
      <form enctype=\"text/html\" method=\"post\" action=\"/control\">\
        <p>\
          <label for=\"stateOn\">ON</label>\
          <input type=\"radio\" id=\"stateOn\" name=\"state\" value=\"on\"" + onAttribute + " />\
        </p>\
        <p>\
          <label for=\"stateOff\">OFF</label>\
          <input type=\"radio\" id=\"stateOff\" name=\"state\" value=\"off\"" + offAttribute + " />\
        </p>\
        <p><input type=\"submit\" /></p>\
      </form>";
    _server->send(200, "text/html", _formatPage(htmlBody));
  }
  _dispatchRequestEnd();
}

void PurlWebServer::_handleReset() {
  _dispatchRequestStart();
  if (_server->method() == HTTP_POST) {
    String resetWifi = _server->arg("ResetWifi");
    if (resetWifi == "yes") {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_AP_STA);
      Serial.println("Wifi mode: WIFI_AP_STA");
    }
    String resetAccessory = _server->arg("ResetAccessory");
    if (resetAccessory == "yes") {
      _dispatchResetAccessory();
    }
    String restartESP = _server->arg("RestartESP");
    if (restartESP == "yes") {
      ESP.restart();
    }
    _redirectTo("/");
  } else {
    String htmlBody = "\
      <p><a href=\"/\">Back</a></p>\
      <h3>Reset</h3>\
      <form enctype=\"text/html\" method=\"post\" action=\"/reset\">\
        <p>\
          <label for=\"chkResetWifi\">Confirm reset wifi</label>\
          <input type=\"checkbox\" id=\"chkResetWifi\" name=\"ResetWifi\" value=\"yes\" />\
        </p>\
        <p>\
          <label for=\"chkResetAccessory\">Confirm reset accessory</label>\
          <input type=\"checkbox\" id=\"chkResetAccessory\" name=\"ResetAccessory\" value=\"yes\" />\
        </p>\
        <p>\
          <label for=\"chkRestartESP\">Restart ESP</label>\
          <input type=\"checkbox\" id=\"chkRestartESP\" name=\"RestartESP\" value=\"yes\" />\
        </p>\
        <p><input type=\"submit\" /></p>\
      </form>";
    _server->send(200, "text/html", _formatPage(htmlBody));
  }
  _dispatchRequestEnd();
}

void PurlWebServer::_handleCrossOrigin() {
    _dispatchRequestStart();
    _server->send(204);
    _dispatchRequestEnd();
}

void PurlWebServer::_handleNotFound() {
  _dispatchRequestStart();
  String method = (_server->method() == HTTP_GET) ? "GET" : "POST";
  String bodyHtml = "\
    <h3>File Not Found</h3>\
    <p>URI: " + _server->uri() + "</p>\
    <p>Method: " + method + "</p>";
  _server->send(404, "text/html", _formatPage(bodyHtml));
  _dispatchRequestEnd();
}
