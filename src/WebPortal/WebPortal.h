#ifndef WebPortal_h
#define WebPortal_h

#include <functional>
#include <Arduino.h>
#include "Commons.h"
#include "VictorWeb.h"
#include "Models/ServiceModels.h"
#include "RadioStorage/RadioStorage.h"
#include "ServiceStorage/ServiceStorage.h"

namespace Victor::Components {
  class WebPortal : public VictorWeb {
    typedef std::function<void(const String&, const ServiceSetting&)> TServiceSettingHandler;
    typedef std::function<ServiceState(const String&, const ServiceSetting&)> TGetServiceStateHandler;
    typedef std::function<void(const String&, const ServiceSetting&, ServiceState&)> TSetServiceStateHandler;

   public:
    WebPortal(int port = 80);
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
    void _solvePageTokens(String& html) override;
    std::pair<bool, ServiceSetting> _getService(const String& serviceId);
    void _saveService(const String& serviceId, const ServiceSetting& service);
    void _deleteService(const String& serviceId, const ServiceSetting& service);
    void _handleServiceList();
    void _handleServiceAdd();
    void _handleServiceReset();
    void _handleServiceGet();
    void _handleServiceSave();
    void _handleServiceDelete();
    void _handleServiceStateGet();
    void _handleServiceStateSave();
    void _handleRadioGet();
    void _handleRadioSave();
    void _handleRadioRuleGet();
    void _handleRadioRuleSave();
    void _handleRadioCommandGet();
    void _handleRadioCommandSave();
  };
} // namespace Victor::Components

#endif // WebPortal_h
