#ifndef WebPortalBase_h
#define WebPortalBase_h

#include <functional>
#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Commons.h"

namespace Victoria::Components {
  class VictoriaWeb {
   public:
    VictoriaWeb(int port);
    ~VictoriaWeb();
    void setup();
    void loop();
    // server events
    typedef std::function<void()> TServerEventHandler;
    TServerEventHandler onRequestStart;
    TServerEventHandler onRequestEnd;
    // static
    static String getHostName(bool includeVersion);

   protected:
    ESP8266WebServer* _server = NULL;
    virtual void _registerHandlers();
    void _redirectTo(const String& url);
    void _send200(const String& bodyHtml);
    void _send404(const String& bodyHtml);
    void _sendHints(const String& title, const String& message);
    String _formatPage(const String& bodyHtml);
    void _dispatchRequestStart();
    void _dispatchRequestEnd();
    void _handleNotFound();
    void _handleCrossOrigin();
    void _handleHome();
    virtual String _appendHomeBody();
    void _handleSystem();
    void _handleSystemFile();
    void _handleWifiList();
    void _handleWifiJoin();
    void _handleReset();
    virtual std::vector<SelectionOptions> _getResetList();
    virtual void _handleResetPost();
    // static helpers
    static String _getCheckedAttr(bool checked);
    static String _renderTable(const TableModel& model);
    static String _renderSelect(const SelectModel& model);
    static String _renderSelectionList(const std::vector<SelectionOptions>& list);
    static void _onWifiEvent(WiFiEvent_t event);
  };
} // namespace Victoria::Components

#endif // WebPortalBase_h
