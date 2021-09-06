#ifndef VictoriaWeb_h
#define VictoriaWeb_h

#include <functional>
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "Commons.h"
#include "AppStorage.h"
#include "VictoriaOTA.h"
#include "VictoriaWifi.h"

namespace Victoria::Components {
  class VictoriaWeb {
   public:
    VictoriaWeb(int port = 80);
    virtual ~VictoriaWeb();
    void setup();
    void loop();
    // server events
    typedef std::function<void()> TServerEventHandler;
    TServerEventHandler onRequestStart;
    TServerEventHandler onRequestEnd;

   protected:
    ESP8266WebServer* _server = NULL;
    ESP8266HTTPUpdateServer* _httpUpdater = NULL;
    virtual void _registerHandlers();
    virtual void _solvePageTokens(String& html);
    void _sendHtml(const String& html);
    void _sendJson(DynamicJsonDocument doc);
    void _dispatchRequestStart();
    void _dispatchRequestEnd();
    void _handleIndex();
    void _handleHome();
    void _handleFileSystem();
    void _handleFileGet();
    void _handleFileSave();
    void _handleFileDelete();
    void _handleWifi();
    void _handleWifiJoin();
    void _handleOta();
    void _handleOtaFire();
    void _handleReset();
    void _handleNotFound();
  };
} // namespace Victoria::Components

#endif // VictoriaWeb_h
