#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <arduino_homekit_server.h>

ESP8266WebServer server(80);

const int led = LED_BUILTIN;
void ledOn() {
  digitalWrite(led, LOW);
  Serial.println("LED -> ON");
}
void ledOff() {
  digitalWrite(led, HIGH);
  Serial.println("LED -> ON");
}

int switchPin = 1;
void switchOn() {
  gpio_output_set(0, BIT0, BIT0, 0);
  Serial.println("SWITCH -> ON");
}
void switchOff() {
  gpio_output_set(BIT0, 0, BIT0, 0);
  Serial.println("SWITCH -> OFF");
}

// access your HomeKit characteristics defined
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

void cha_switch_on_setter(const homekit_value_t value) {
  ledOn();

	bool on = value.bool_value;
	cha_switch_on.value.bool_value = on; //sync the value

  if (on) {
    switchOn();
  } else {
    switchOff();
  }

  delay(100);
  ledOff();
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
      <body>"
      + htmlBody +
      "</body>\
    </html>";
}

String listSSID() {
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
    <form enctype=\"text/html\" method=\"post\" action=\"select\">\
      <h1>Purl Switch</h1>"
      + list +
      "<p><label>Password: </label><input name=\"pass\" length=\"64\" /></p>\
      <p>ON<input type=\"radio\" name=\"gpio\" value=\"on\" checked=\"checked\" /></p>\
      <p>OFF<input type=\"radio\" name=\"gpio\" value=\"off\" /></p>\
      <p><input type=\"submit\" /></p>\
    </form>";
  return formatPage(html);
}

void select() {
  if (server.method() != HTTP_POST) {
    ledOn();
    server.send(405, "text/html", formatPage("<h2>Method Not Allowed</h2>"));
    delay(100);
    ledOff();
    return;
  }

  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  String gpio = server.arg("gpio");

  if (gpio == "on") {
    switchOn();
  } else if (gpio == "off") {
    switchOff();
  }

  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
    String ignoreMsg = "<h2>Ignore network setup</h2><a href=\"/\">Back</a>";
    server.send(200, "text/html", formatPage(ignoreMsg));
    return;
  }

  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String htmlMsg = "<p>Connected to: " + ssid + "</p>\
    <p>IP address: " + WiFi.localIP().toString() + "</p>\
    <a href=\"/\">Back</a>";
  server.send(200, "text/html", formatPage(htmlMsg));
}

void handleRoot() {
  ledOn();
  server.send(200, "text/html", listSSID());
  delay(100);
  ledOff();
}

void handleNotFound() {
  ledOn();
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", formatPage(message));
  delay(100);
  ledOff();
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);

  cha_switch_on.setter = cha_switch_on_setter;
	arduino_homekit_setup(&config);

  pinMode(led, OUTPUT);
  ledOff();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/select", select);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  arduino_homekit_loop();
}
