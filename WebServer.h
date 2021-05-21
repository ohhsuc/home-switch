#include <ESP8266WebServer.h>

namespace Victoria {
  enum AccessoryType {
    EmptyAccessoryType,
    BooleanAccessoryType,
    IntegerAccessoryType,
  };

  struct AccessoryState {
    bool booleanValue;
    int integerValue;
    String stringValue;
    AccessoryType accessoryType;
  };

  namespace Components {
    class WebServer {
      public:
        WebServer(int port, String productName, String hostName);
        ~WebServer();
        void setup();
        void loop();
        // accessory events
        typedef void (*AccessoryStateEvent)(AccessoryState&);
        AccessoryStateEvent onSetState;
        AccessoryStateEvent onGetState;
        // server events
        typedef void (*ServerEvent)();
        ServerEvent onRequestStart;
        ServerEvent onRequestEnd;
        ServerEvent onResetAccessory;
      private:
        String _productName;
        String _hostName;
        ESP8266WebServer* _server;
        AccessoryState _currentState = {
          // .booleanValue = false,
          booleanValue: false,
        };
        String _formatPage(String htmlBody);
        void _redirectTo(String url);
        void _dispatchGetState();
        void _dispatchSetState();
        void _dispatchRequestStart();
        void _dispatchRequestEnd();
        void _dispatchResetAccessory();
        void _handleRoot();
        void _handleListWifi();
        void _handleConnectWifi();
        void _handleAccessory();
        String _getTypeHtml();
        String _getBooleanHtml();
        String _getIntegerHtml();
        void _handleReset();
        void _handleCrossOrigin();
        void _handleNotFound();
    };
  }
}
