#ifndef WebPortal_h
#define WebPortal_h

#include <map>
#include <functional>
#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Commons.h"
#include "RadioStorage.h"
#include "ServiceStorage.h"

namespace Victoria::Components {
  class WebPortal {
    typedef std::function<void(const String&, const ServiceSetting&)> TServiceSettingHandler;
    typedef std::function<ServiceState(const String&, const ServiceSetting&)> TGetServiceStateHandler;
    typedef std::function<void(const String&, const ServiceSetting&, ServiceState&)> TSetServiceStateHandler;
    typedef std::function<void()> TServerEventHandler;

   public:
    WebPortal(int port);
    ~WebPortal();
    void setup();
    void loop();
    // service events
    TServiceSettingHandler onSaveService;
    TServiceSettingHandler onDeleteService;
    TGetServiceStateHandler onGetServiceState;
    TSetServiceStateHandler onSetServiceState;
    // server events
    TServerEventHandler onRequestStart;
    TServerEventHandler onRequestEnd;
    TServerEventHandler onResetAccessory;
    // static
    static String getHostName(bool includeVersion);

   private:
    ESP8266WebServer* _server = NULL;
    std::pair<bool, ServiceSetting> _getService(const String& serviceId);
    void _saveService(const String& serviceId, const ServiceSetting& service);
    void _deleteService(const String& serviceId, const ServiceSetting& service);
    void _redirectTo(const String& url);
    void _send200(const String& bodyHtml);
    void _send404(const String& bodyHtml);
    void _sendHints(const String& title, const String& message);
    String _formatPage(const String& bodyHtml);
    void _dispatchRequestStart();
    void _dispatchRequestEnd();
    void _handleRoot();
    void _handleSystem();
    void _handleSystemFile();
    void _handleWifiList();
    void _handleWifiJoin();
    void _handleRadio();
    void _handleNewService();
    void _handleService();
    void _handleServiceState();
    static String _getCheckedAttr(bool checked);
    static String _renderTable(const TableModel& model);
    static String _renderSelectionList(std::vector<std::vector<String>> list);
    static String _getTypeHtml(const ServiceSetting& service);
    static String _getIOHtml(const ServiceSetting& service);
    static String _getLevelHtml(const String& name, const int& level);
    static String _getBooleanHtml(const ServiceState& state);
    static String _getIntegerHtml(const ServiceState& state);
    static void _onWifiEvent(WiFiEvent_t event);
    void _handleReset();
    void _handleNotFound();
    void _handleCrossOrigin();
  };
} // namespace Victoria::Components

#endif // WebPortal_h
