#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <arduino_homekit_server.h>

// create web server on port 80
ESP8266WebServer server(80);
const String hostName = "Purl-Switch-001";

// access your HomeKit characteristics defined
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch;

const int led = LED_BUILTIN;
void ledOn() {
  digitalWrite(led, LOW);
  Serial.println("LED -> ON");
}
void ledOff() {
  digitalWrite(led, HIGH);
  Serial.println("LED -> OFF");
}

void switchOn() {
	cha_switch.value.bool_value = true;
  gpio_output_set(0, BIT0, BIT0, 0);
  Serial.println("SWITCH -> ON");
}
void switchOff() {
  cha_switch.value.bool_value = false;
  gpio_output_set(BIT0, 0, BIT0, 0);
  Serial.println("SWITCH -> OFF");
}

homekit_value_t cha_switch_getter() {
  return cha_switch.value;
}

void cha_switch_setter(const homekit_value_t value) {
  ledOn();

  if (value.bool_value) {
    switchOn();
  } else {
    switchOff();
  }

  delay(100);
  ledOff();
}

void redirectTo(String url) {
  server.sendHeader("Location", url, true);
  server.send(302, "text/plain", "");
}

String formatPage(String htmlBody) {
  return "\
    <!DOCTYPE HTML>\
    <html>\
      <head>\
        <title>Purl Switch</title>\
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

String listSsid() {
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
    <h3>Select WiFi</h3>\
    <form enctype=\"text/html\" method=\"post\" action=\"/select\">"
      + list +
      "<p><label>Password: </label><input name=\"pass\" length=\"64\" /></p>\
      <p><input type=\"submit\" /></p>\
    </form>";
  return formatPage(htmlBody);
}

void handleRoot() {
  ledOn();
  server.send(200, "text/html", listSsid());
  delay(100);
  ledOff();
}

void handleSelectWiFi() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
    String ignoreMsg = "\
      <p><a href=\"/\">Back</a></p>\
      <p>Ignore network setup</p>";
    server.send(200, "text/html", formatPage(ignoreMsg));
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
    server.send(200, "text/html", formatPage(successMessage));
  } else {
    String failedMessage = "\
      <p><a href=\"/\">Back</a></p>\
      <p>Connect to: " + ssid + " failed</p>";
    server.send(200, "text/html", formatPage(failedMessage));
  }
}

void handleControl() {
  ledOn();
  if (server.method() == HTTP_POST) {
    String state = server.arg("state");
    if (state == "on") {
      switchOn();
    } else if (state == "off") {
      switchOff();
    }
    redirectTo("/control");
  } else {
    bool isOn = cha_switch.value.bool_value;
    String onAttribute = isOn ? " checked=\"checked\"" : "";
    String offAttribute = isOn ? "" : " checked=\"checked\"";
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
    server.send(200, "text/html", formatPage(htmlBody));
  }
  delay(100);
  ledOff();
}

void handleReset() {
  ledOn();
  if (server.method() == HTTP_POST) {
    String resetWifi = server.arg("ResetWifi");
    if (resetWifi == "yes") {
      WiFi.disconnect(true);
    }
    String resetHomekit = server.arg("ResetHomekit");
    if (resetHomekit == "yes") {
      homekit_server_reset();
    }
    redirectTo("/");
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
          <label for=\"chkResetHomekit\">Confirm reset homekit</label>\
          <input type=\"checkbox\" id=\"chkResetHomekit\" name=\"ResetHomekit\" value=\"yes\" />\
        </p>\
        <p><input type=\"submit\" /></p>\
      </form>";
    server.send(200, "text/html", formatPage(htmlBody));
  }
  delay(100);
  ledOff();
}

void handleNotFound() {
  ledOn();
  String method = (server.method() == HTTP_GET) ? "GET" : "POST";
  String bodyHtml = "\
    <h3>File Not Found</h3>\
    <p>URI: " + server.uri() + "</p>\
    <p>Method: " + method + "</p>";
  server.send(404, "text/html", formatPage(bodyHtml));
  delay(100);
  ledOff();
}

void setup(void) {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  ledOn();

  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(hostName);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // IPAddress apIp(192, 168, 1, 33);
  // IPAddress apSubnet(255, 255, 255, 0);
  // WiFi.softAPConfig(apIp, apIp, apSubnet);
  WiFi.softAP(hostName);

  IPAddress currentApIp = WiFi.softAPIP();
  Serial.println("AP IP address: " + currentApIp.toString());

  server.on("/", handleRoot);
  server.on("/select", handleSelectWiFi);
  server.on("/control", handleControl);
  server.on("/reset", handleReset);
  server.onNotFound(handleNotFound);
  server.begin();

  cha_switch.getter = cha_switch_getter;
  cha_switch.setter = cha_switch_setter;
  arduino_homekit_setup(&config);

  Serial.println("Setup complete!");
  ledOff();
}

void loop(void) {
  server.handleClient();
  arduino_homekit_loop();
}
