#include <ESP8266WebServer.h>

struct PurlWebServerState {
  bool isSwitchOn;
};
typedef void (*TPurlEvent)();
typedef void (*TPurlStateEvent)(PurlWebServerState);

class PurlWebServer {
  public:
    PurlWebServer(int port, String productName, String hostName);
    ~PurlWebServer();
    void setup();
    void loop();
    PurlWebServerState currentState;
    TPurlStateEvent onSetState;
    TPurlStateEvent onGetState;
    TPurlEvent onRequestStart;
    TPurlEvent onRequestEnd;
    TPurlEvent onResetAccessory;

  private:
    String _productName;
    String _hostName;
    ESP8266WebServer* _server;
    String _formatPage(String htmlBody);
    void _redirectTo(String url);
    void _dispatchGetState();
    void _dispatchSetState();
    void _dispatchRequestStart();
    void _dispatchRequestEnd();
    void _dispatchResetAccessory();
    void _handleRoot();
    void _handleSelectWiFi();
    void _handleControl();
    void _handleReset();
    void _handleCrossOrigin();
    void _handleNotFound();
};
