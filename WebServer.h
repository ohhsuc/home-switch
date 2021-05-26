#ifndef WebServer_h
#define WebServer_h

#include <map>
#include <ESP8266WebServer.h>
#include "Commons.h"

namespace Victoria {
  namespace Components {
    class WebServer {
      public:
        WebServer(int port, String productName, String hostName);
        ~WebServer();
        void setup();
        void loop();
        // accessory events
        typedef std::map<String, AccessorySetting> (*LoadAccessorySettingsEvent)(void);
        typedef void (*UpdateAccessorySettingEvent)(String, AccessorySetting&);
        LoadAccessorySettingsEvent onLoadSettings;
        UpdateAccessorySettingEvent onSaveSetting;
        UpdateAccessorySettingEvent onDeleteSetting;
        // server events
        typedef void (*ServerEvent)();
        ServerEvent onRequestStart;
        ServerEvent onRequestEnd;
        ServerEvent onResetAccessory;
      private:
        String _productName;
        String _hostName;
        ESP8266WebServer* _server;
        String _formatPage(String htmlBody);
        void _redirectTo(String url);
        void _dispatchRequestStart();
        void _dispatchRequestEnd();
        void _handleRoot();
        void _handleListWifi();
        void _handleConnectWifi();
        void _handleNewAccessory();
        void _handleAccessory();
        static String _randomString(int length);
        static String _getCheckedAttr(bool checked);
        static String _getTypeHtml(AccessorySetting setting);
        static String _getIOHtml(AccessorySetting setting);
        static String _getBooleanHtml(AccessorySetting setting);
        static String _getIntegerHtml(AccessorySetting setting);
        void _handleReset();
        void _handleCrossOrigin();
        void _handleNotFound();
    };
  }
}

#endif //WebServer_h
