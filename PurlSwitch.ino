#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <arduino_homekit_server.h>

// create web server on port 80
ESP8266WebServer server(80);

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
  Serial.println("LED -> ON");
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
  String html = "\
    <p>\
      <a href=\"/control\">Control</a>\
      <a href=\"/reset\">Reset</a>\
    </p>\
    <h2>Select WiFi</h2>\
    <form enctype=\"text/html\" method=\"post\" action=\"/select\">"
      + list +
      "<p><label>Password: </label><input name=\"pass\" length=\"64\" /></p>\
      <p><input type=\"submit\" /></p>\
    </form>";
  return formatPage(html);
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
    String html = "\
      <p><a href=\"/\">Back</a></p>\
      <h2>Control Switch</h2>\
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
    server.send(200, "text/html", formatPage(html));
  }
  delay(100);
  ledOff();
}

void handleReset() {
  ledOn();
  bool redirected = false;
  if (server.method() == HTTP_POST) {
    String confirmed = server.arg("confirmed");
    if (confirmed == "yes") {
      WiFi.disconnect(true);
      redirectTo("/");
      redirected = true;
    }
  }
  if (!redirected) {
    String html = "\
      <p><a href=\"/\">Back</a></p>\
      <h2>Reset WiFi</h2>\
      <form enctype=\"text/html\" method=\"post\" action=\"/reset\">\
        <p>\
          <label for=\"isConfirmed\">Confirm</label>\
          <input type=\"checkbox\" id=\"isConfirmed\" name=\"confirmed\" value=\"yes\" />\
        </p>\
        <p><input type=\"submit\" /></p>\
      </form>";
    server.send(200, "text/html", formatPage(html));
  }
  delay(100);
  ledOff();
}

void handleNotFound() {
  ledOn();
  String method = (server.method() == HTTP_GET) ? "GET" : "POST";
  String bodyHtml = "<h2>File Not Found</h2>\
    <p>URI: " + server.uri() + "</p>\
    <p>Method: " + method + "</p>";
  server.send(404, "text/html", formatPage(bodyHtml));
  delay(100);
  ledOff();
}

void setup(void) {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  ledOff();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  IPAddress apIp(192, 168, 1, 33);
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIp, apIp, apSubnet);
  WiFi.softAP("Purl-Switch-001");

  IPAddress currentApIp = WiFi.softAPIP();
  Serial.println("AP IP address: " + currentApIp.toString());

  arduino_homekit_setup(&config);
  cha_switch.getter = cha_switch_getter;
  cha_switch.setter = cha_switch_setter;

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/select", handleSelectWiFi);
  server.on("/control", handleControl);
  server.on("/reset", handleReset);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  arduino_homekit_loop();
}
