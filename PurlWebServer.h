#include <ESP8266WebServer.h>

class PurlWebServer;
  struct PurlWebServerState {
    bool isSwitchOn;
  };
  typedef void (*TEvent)();
  typedef void (*TSyncEvent)(PurlWebServerState);

class PurlWebServer {
  public:
    PurlWebServer(int port, String productName, String hostName);
    ~PurlWebServer();
    void setup();
    void loop();
    PurlWebServerState currentState;
    TSyncEvent onSetState;
    TSyncEvent onGetState;
    TEvent onRequestStart;
    TEvent onRequestEnd;
    TEvent onResetAccessory;

  private:
    String _productName;
    String _hostName;
    ESP8266WebServer* _server;
    String _formatPage(String htmlBody);
    void _redirectTo(String url);
    void _triggerRequestStart();
    void _triggerRequestEnd();
    void _triggerResetAccessory();
    void _handleRoot();
    void _handleSelectWiFi();
    void _handleControl();
    void _handleReset();
    void _handleNotFound();
};
