#ifndef WebServer_h
#define WebServer_h

#include <vector>
#include <ESP8266WebServer.h>

namespace Victoria {
  enum AccessoryType {
    EmptyAccessoryType,
    BooleanAccessoryType,
    IntegerAccessoryType,
  };

  struct AccessoryState {
    String id;
    String name;
    AccessoryType type;
    bool boolValue;
    int intValue;
  };

  namespace Components {
    class WebServer {
      public:
        WebServer(int port, String productName, String hostName);
        ~WebServer();
        void setup();
        void loop();
        // accessory events
        typedef std::vector<AccessoryState> (*LoadAccessoryStatesEvent)(void);
        typedef void (*UpdateAccessoryStateEvent)(AccessoryState&);
        LoadAccessoryStatesEvent onLoadStates;
        UpdateAccessoryStateEvent onSaveState;
        UpdateAccessoryStateEvent onDeleteState;
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
        void _handleAccessory();
        static String _getCheckedAttr(bool checked);
        static String _getTypeHtml(AccessoryState state);
        static String _getBooleanHtml(AccessoryState state);
        static String _getIntegerHtml(AccessoryState state);
        void _handleReset();
        void _handleCrossOrigin();
        void _handleNotFound();
    };
  }
}

#endif //WebServer_h
