#ifndef WebPortal_h
#define WebPortal_h

#include <functional>
#include <Arduino.h>
#include "Commons.h"
#include "VictoriaWeb.h"
#include "RadioStorage.h"
#include "ServiceStorage.h"

namespace Victoria::Components {
  class WebPortal : public VictoriaWeb {
    typedef std::function<void(const String&, const ServiceSetting&)> TServiceSettingHandler;
    typedef std::function<ServiceState(const String&, const ServiceSetting&)> TGetServiceStateHandler;
    typedef std::function<void(const String&, const ServiceSetting&, ServiceState&)> TSetServiceStateHandler;

   public:
    WebPortal(int port);
    ~WebPortal();
    // service events
    TServiceSettingHandler onSaveService;
    TServiceSettingHandler onDeleteService;
    TGetServiceStateHandler onGetServiceState;
    TSetServiceStateHandler onSetServiceState;
    // server events
    TServerEventHandler onResetAccessory;

   private:
    void _registerHandlers() override;
    std::pair<bool, ServiceSetting> _getService(const String& serviceId);
    void _saveService(const String& serviceId, const ServiceSetting& service);
    void _deleteService(const String& serviceId, const ServiceSetting& service);
    String _appendHomeBody() override;
    void _handleRadio();
    void _handleNewService();
    void _handleService();
    void _handleServiceState();
    std::vector<SelectionOptions> _getResetList() override;
    void _handleResetPost() override;
    static String _getTypeHtml(const ServiceSetting& service);
    static String _getIOHtml(const ServiceSetting& service);
    static String _getLevelHtml(const String& name, const int& level);
    static String _getBooleanHtml(const ServiceState& state);
    static String _getIntegerHtml(const ServiceState& state);
  };
} // namespace Victoria::Components

#endif // WebPortal_h
