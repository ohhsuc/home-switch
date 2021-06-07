#ifndef WebServer_h
#define WebServer_h

#include <map>
#include <functional>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Commons.h"

namespace Victoria {
  namespace Components {
    class WebServer {
      typedef std::function<std::map<String, AccessorySetting>()> TLoadAccessorySettingsHandler;
      typedef std::function<void(const String&, const AccessorySetting&)> TAccessorySettingHandler;
      typedef std::function<AccessoryState(const String&, const AccessorySetting&)> TGetAccessoryStateHandler;
      typedef std::function<void(const String&, const AccessorySetting&, AccessoryState&)> TSetAccessoryStateHandler;
      typedef std::function<void()> TServerEventHandler;
      public:
        WebServer(int port, const String& productName, const String& hostName, const String& firmwareVersion);
        ~WebServer();
        void setup();
        void loop();
        // accessory events
        TLoadAccessorySettingsHandler onLoadSettings;
        TAccessorySettingHandler onSaveSetting;
        TAccessorySettingHandler onDeleteSetting;
        TGetAccessoryStateHandler onGetState;
        TSetAccessoryStateHandler onSetState;
        // server events
        TServerEventHandler onRequestStart;
        TServerEventHandler onRequestEnd;
        TServerEventHandler onResetAccessory;
      private:
        String _productName;
        String _hostName;
        String _firmwareVersion;
        ESP8266WebServer* _server;
        void _redirectTo(const String& url);
        void _send200(const String& bodyHtml);
        void _send404(const String& bodyHtml);
        String _formatPage(const String& bodyHtml);
        void _dispatchRequestStart();
        void _dispatchRequestEnd();
        void _handleRoot();
        void _handleWifiList();
        void _handleWifiConnect();
        void _handleNewAccessory();
        void _handleAccessory();
        void _handleAccessoryState();
        std::pair<bool, AccessorySetting> _getAccessorySetting(const String& id);
        static String _getCheckedAttr(bool checked);
        static String _renderTable(const TableModel& model);
        static String _getTypeHtml(const AccessorySetting& setting);
        static String _getIOHtml(const AccessorySetting& setting);
        static String _getLevelHtml(const String name, const short int level);
        static String _getBooleanHtml(const AccessoryState& state);
        static String _getIntegerHtml(const AccessoryState& state);
        void _handleReset();
        void _handleCrossOrigin();
        void _handleNotFound();
    };
  }
}

#endif //WebServer_h
