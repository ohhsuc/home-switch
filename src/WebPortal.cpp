#include "WebPortal.h"

namespace Victor::Components {

  WebPortal::WebPortal(int port)
  : VictorWeb(port) {}

  WebPortal::~WebPortal() {}

  void WebPortal::_registerHandlers() {
    VictorWeb::_registerHandlers();
    _server->serveStatic("/fav.svg", LittleFS, "/web/fav.svg", "max-age=43200");
    _server->serveStatic("/service.min.js", LittleFS, "/web/service.min.js");
    _server->on(F("/service/list"), HTTP_GET, std::bind(&WebPortal::_handleServiceList, this));
    _server->on(F("/service/add"), HTTP_POST, std::bind(&WebPortal::_handleServiceAdd, this));
    _server->on(F("/service/reset"), HTTP_POST, std::bind(&WebPortal::_handleServiceReset, this));
    _server->on(F("/service"), HTTP_GET, std::bind(&WebPortal::_handleServiceGet, this));
    _server->on(F("/service"), HTTP_POST, std::bind(&WebPortal::_handleServiceSave, this));
    _server->on(F("/service"), HTTP_DELETE, std::bind(&WebPortal::_handleServiceDelete, this));
    _server->on(F("/service/state"), HTTP_GET, std::bind(&WebPortal::_handleServiceStateGet, this));
    _server->on(F("/service/state"), HTTP_POST, std::bind(&WebPortal::_handleServiceStateSave, this));
  }

  void WebPortal::_solvePageTokens(String& html) {
    html.replace(F("{appendHead}"), F("\
      <link rel=\"icon\" href=\"fav.svg?v={version}\" type=\"image/svg+xml\">\
      <script src=\"service.min.js?v={version}\"></script>\
    "));
    VictorWeb::_solvePageTokens(html);
  }

  std::pair<bool, ServiceSetting> WebPortal::_getService(const String& serviceId) {
    auto found = false;
    auto model = serviceStorage.load();
    ServiceSetting service;
    if (model.services.count(serviceId) > 0) {
      service = model.services[serviceId];
      found = true;
    }
    return std::make_pair(found, service);
  }

  void WebPortal::_saveService(const String& serviceId, const ServiceSetting& service) {
    auto model = serviceStorage.load();
    model.services[serviceId] = service;
    serviceStorage.save(model);
    if (onSaveService) {
      onSaveService(serviceId, service);
    }
  }

  void WebPortal::_deleteService(const String& serviceId, const ServiceSetting& service) {
    auto model = serviceStorage.load();
    model.services.erase(serviceId);
    serviceStorage.save(model);
    if (onDeleteService) {
      onDeleteService(serviceId, service);
    }
  }

  void WebPortal::_handleServiceList() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    res[F("clientNumber")] = onCountClients ? onCountClients() : -1;
    const JsonArray serviceArr = res.createNestedArray(F("services"));
    const auto model = serviceStorage.load();
    for (const auto& pair : model.services) {
      const JsonObject serviceObj = serviceArr.createNestedObject();
      serviceObj[F("id")] = pair.first;
      serviceObj[F("name")] = pair.second.name;
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceAdd() {
    _dispatchRequestStart();
    const auto serviceId = GlobalHelpers::randomString(4);
    const ServiceSetting newSetting = {
      .name = F("New-") + serviceId,
      .type = BooleanServiceType,
    };
    _saveService(serviceId, newSetting);
    DynamicJsonDocument res(64);
    res[F("id")] = serviceId;
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceReset() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    if (onResetAccessory) {
      onResetAccessory();
      res[F("message")] = String(F("success"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceGet() {
    _dispatchRequestStart();
    const auto id = _server->arg(F("id"));
    const auto found = _getService(id);
    DynamicJsonDocument res(512);
    const JsonObject serviceObj = res.createNestedObject(F("service"));
    if (found.first) {
      const auto service = found.second;
      serviceObj[F("id")] = id;
      serviceObj[F("name")] = service.name;
      serviceObj[F("type")] = service.type;
      serviceObj[F("inputPin")] = service.inputPin;
      serviceObj[F("outputPin")] = service.outputPin;
      serviceObj[F("inputTrueValue")] = service.inputTrueValue;
      serviceObj[F("outputTrueValue")] = service.outputTrueValue;
    } else {
      res[F("error")] = String(F("Can't find the service"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceSave() {
    _dispatchRequestStart();
    DynamicJsonDocument res(64);
    const auto id = _server->arg(F("id"));
    const auto found = _getService(id);
    if (found.first) {
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(512);
      deserializeJson(payload, payloadJson);
      // read
      const auto name = String(payload[F("name")]);
      const auto type = String(payload[F("type")]);
      const auto inputPin = String(payload[F("inputPin")]);
      const auto outputPin = String(payload[F("outputPin")]);
      const auto inputTrueValue = String(payload[F("inputTrueValue")]);
      const auto outputTrueValue = String(payload[F("outputTrueValue")]);
      // save
      auto service = found.second;
      service.name = name;
      service.type = ServiceType(type.toInt());
      service.inputPin = inputPin.toInt();
      service.outputPin = outputPin.toInt();
      service.inputTrueValue = inputTrueValue.toInt();
      service.outputTrueValue = outputTrueValue.toInt();
      _saveService(id, service);
      res[F("message")] = String(F("success"));
    } else {
      res[F("error")] = String(F("Can't find the service"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceDelete() {
    _dispatchRequestStart();
    DynamicJsonDocument res(64);
    const auto id = _server->arg(F("id"));
    const auto found = _getService(id);
    if (found.first) {
      _deleteService(id, found.second);
      res[F("message")] = String(F("success"));
    } else {
      res[F("error")] = String(F("Can't find the service"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceStateGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    const auto id = _server->arg(F("id"));
    const auto found = _getService(id);
    if (found.first) {
      const ServiceSetting service = found.second;
      ServiceState state = {
        .boolValue = false,
        .intValue = 0,
      };
      if (onGetServiceState) {
        state = onGetServiceState(id, service);
      }
      const JsonObject serviceObj = res.createNestedObject(F("service"));
      serviceObj[F("id")] = id;
      serviceObj[F("name")] = service.name;
      serviceObj[F("type")] = service.type;
      const JsonObject valueObj = res.createNestedObject(F("value"));
      valueObj[F("boolValue")] = state.boolValue;
      valueObj[F("intValue")] = state.intValue;
      res[F("message")] = String(F("success"));
    } else {
      res[F("error")] = String(F("Can't find the service"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceStateSave() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    const auto id = _server->arg(F("id"));
    const auto found = _getService(id);
    if (found.first) {
      const ServiceSetting service = found.second;
      ServiceState state = {
        .boolValue = false,
        .intValue = 0,
      };
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(64);
      deserializeJson(payload, payloadJson);
      // action
      if (service.type == BooleanServiceType) {
        const auto boolValue = String(payload[F("boolValue")]);
        state.boolValue = (boolValue == F("true"));
      }
      if (service.type == IntegerServiceType) {
        const auto intValue = String(payload[F("intValue")]);
        state.intValue = intValue.toInt();
      }
      if (onSetServiceState) {
        onSetServiceState(id, service, state);
      }
      res[F("message")] = String(F("success"));
    } else {
      res[F("error")] = String(F("Can't find the service"));
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

} // namespace Victor::Components
