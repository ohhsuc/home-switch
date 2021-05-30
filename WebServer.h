#ifndef WebServer_h
#define WebServer_h

#include <map>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Commons.h"

namespace Victoria {
  namespace Components {
    class WebServer {
      public:
        WebServer(int port, const String& productName, const String& hostName);
        ~WebServer();
        void setup();
        void loop();
        // accessory events
        typedef std::map<String, AccessorySetting> (*LoadAccessorySettingsEvent)(void);
        typedef void (*UpdateAccessorySettingEvent)(const String&, const AccessorySetting&);
        LoadAccessorySettingsEvent onLoadSettings;
        UpdateAccessorySettingEvent onSaveSetting;
        UpdateAccessorySettingEvent onDeleteSetting;
        typedef void (*AccessoryStateEvent)(const String&, const AccessorySetting&, AccessoryState&);
        AccessoryStateEvent onGetState;
        AccessoryStateEvent onSetState;
        // server events
        typedef void (*ServerEvent)();
        ServerEvent onRequestStart;
        ServerEvent onRequestEnd;
        ServerEvent onResetAccessory;
      private:
        String _productName;
        String _hostName;
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
        static String _getTypeHtml(const AccessorySetting& setting);
        static String _getIOHtml(const AccessorySetting& setting);
        static String _getBooleanHtml(const AccessoryState& state);
        static String _getIntegerHtml(const AccessoryState& state);
        void _handleReset();
        void _handleCrossOrigin();
        void _handleNotFound();
    };
  }
}

#endif //WebServer_h
