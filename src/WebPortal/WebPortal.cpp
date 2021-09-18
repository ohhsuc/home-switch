#include "WebPortal.h"

namespace Victor::Components {

  WebPortal::WebPortal(int port)
  : VictorWeb(port) {}

  WebPortal::~WebPortal() {}

  void WebPortal::_registerHandlers() {
    VictorWeb::_registerHandlers();
    _server->serveStatic("/fav.svg", LittleFS, "/web/fav.svg", "max-age=43200");
    _server->serveStatic("/victoria.min.js", LittleFS, "/web/victoria.min.js");
    _server->on(F("/service/list"), HTTP_GET, std::bind(&WebPortal::_handleServiceList, this));
    _server->on(F("/service/add"), HTTP_POST, std::bind(&WebPortal::_handleServiceAdd, this));
    _server->on(F("/service/reset"), HTTP_POST, std::bind(&WebPortal::_handleServiceReset, this));
    _server->on(F("/service"), HTTP_GET, std::bind(&WebPortal::_handleServiceGet, this));
    _server->on(F("/service"), HTTP_POST, std::bind(&WebPortal::_handleServiceSave, this));
    _server->on(F("/service"), HTTP_DELETE, std::bind(&WebPortal::_handleServiceDelete, this));
    _server->on(F("/service/state"), HTTP_GET, std::bind(&WebPortal::_handleServiceStateGet, this));
    _server->on(F("/service/state"), HTTP_POST, std::bind(&WebPortal::_handleServiceStateSave, this));
    _server->on(F("/radio"), HTTP_GET, std::bind(&WebPortal::_handleRadioGet, this));
    _server->on(F("/radio"), HTTP_POST, std::bind(&WebPortal::_handleRadioSave, this));
    _server->on(F("/radio/rule"), HTTP_GET, std::bind(&WebPortal::_handleRadioRuleGet, this));
    _server->on(F("/radio/rule"), HTTP_POST, std::bind(&WebPortal::_handleRadioRuleSave, this));
    _server->on(F("/radio/command"), HTTP_GET, std::bind(&WebPortal::_handleRadioCommandGet, this));
    _server->on(F("/radio/command"), HTTP_POST, std::bind(&WebPortal::_handleRadioCommandSave, this));
  }

  void WebPortal::_solvePageTokens(String& html) {
    VictorWeb::_solvePageTokens(html);
    html.replace(F("{appendHead}"), F("\
      <link rel=\"icon\" href=\"fav.svg\" type=\"image/svg+xml\">\
      <script src=\"victoria.min.js\"></script>\
    "));
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
    JsonArray serviceArr = res.createNestedArray(F("services"));
    auto model = serviceStorage.load();
    for (const auto& pair : model.services) {
      JsonObject serviceObj = serviceArr.createNestedObject();
      serviceObj[F("id")] = pair.first;
      serviceObj[F("name")] = pair.second.name;
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceAdd() {
    _dispatchRequestStart();
    auto serviceId = GlobalHelpers::randomString(4);
    ServiceSetting newSetting = {
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
    auto id = _server->arg(F("id"));
    auto found = _getService(id);
    DynamicJsonDocument res(512);
    JsonObject serviceObj = res.createNestedObject(F("service"));
    if (found.first) {
      auto service = found.second;
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
    auto id = _server->arg(F("id"));
    auto found = _getService(id);
    if (found.first) {
      // payload
      auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(512);
      deserializeJson(payload, payloadJson);
      // read
      auto name = String(payload[F("name")]);
      auto type = String(payload[F("type")]);
      auto inputPin = String(payload[F("inputPin")]);
      auto outputPin = String(payload[F("outputPin")]);
      auto inputTrueValue = String(payload[F("inputTrueValue")]);
      auto outputTrueValue = String(payload[F("outputTrueValue")]);
      // save
      ServiceSetting service = found.second;
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
    auto id = _server->arg(F("id"));
    auto found = _getService(id);
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
    auto id = _server->arg(F("id"));
    auto found = _getService(id);
    if (found.first) {
      ServiceSetting service = found.second;
      ServiceState state = {
        .boolValue = false,
        .intValue = 0,
      };
      if (onGetServiceState) {
        state = onGetServiceState(id, service);
      }
      JsonObject serviceObj = res.createNestedObject(F("service"));
      serviceObj[F("id")] = id;
      serviceObj[F("name")] = service.name;
      serviceObj[F("type")] = service.type;
      JsonObject valueObj = res.createNestedObject(F("value"));
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
    auto id = _server->arg(F("id"));
    auto found = _getService(id);
    if (found.first) {
      ServiceSetting service = found.second;
      ServiceState state = {
        .boolValue = false,
        .intValue = 0,
      };
      // payload
      auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(64);
      deserializeJson(payload, payloadJson);
      // action
      if (service.type == BooleanServiceType) {
        auto boolValue = String(payload[F("boolValue")]);
        state.boolValue = (boolValue == F("true"));
      }
      if (service.type == IntegerServiceType) {
        auto intValue = String(payload[F("intValue")]);
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

  void WebPortal::_handleRadioGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    auto model = radioStorage.load();
    auto lastReceived = radioStorage.getLastReceived();
    res[F("millis")] = millis();
    res[F("inputPin")] = model.inputPin;
    JsonObject lastReceivedObj = res.createNestedObject(F("lastReceived"));
    lastReceivedObj[F("value")] = lastReceived.value;
    lastReceivedObj[F("channel")] = lastReceived.channel;
    lastReceivedObj[F("timestamp")] = lastReceived.timestamp;
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioSave() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(64);
    deserializeJson(payload, payloadJson);
    // read
    auto inputPin = String(payload[F("inputPin")]);
    // action
    auto model = radioStorage.load();
    model.inputPin = inputPin.toInt();
    radioStorage.save(model);
    // res
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioRuleGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(1024);
    // rules
    auto model = radioStorage.load();
    JsonArray ruleArr = res.createNestedArray(F("rules"));
    for (const auto& rule : model.rules) {
      JsonObject ruleObj = ruleArr.createNestedObject();
      ruleObj[F("value")] = rule.value;
      ruleObj[F("channel")] = rule.channel;
      ruleObj[F("press")] = rule.press;
      ruleObj[F("action")] = rule.action;
      ruleObj[F("serviceId")] = rule.serviceId;
    }
    // service
    auto serviceModel = serviceStorage.load();
    JsonArray serviceArr = res.createNestedArray(F("services"));
    for (const auto& pair : serviceModel.services) {
      JsonObject serviceObj = serviceArr.createNestedObject();
      serviceObj[F("id")] = pair.first;
      serviceObj[F("name")] = pair.second.name;
    }
    // last received
    auto lastReceived = radioStorage.getLastReceived();
    JsonObject lastReceivedObj = res.createNestedObject(F("lastReceived"));
    lastReceivedObj[F("value")] = lastReceived.value;
    lastReceivedObj[F("channel")] = lastReceived.channel;
    // end
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioRuleSave() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(1024);
    deserializeJson(payload, payloadJson);
    // read
    auto ruleItems = payload["rules"];
    // save
    auto model = radioStorage.load();
    model.rules.clear();
    for (size_t i = 0; i < ruleItems.size(); i++) {
      auto item = ruleItems[i];
      model.rules.push_back({
        .value = String(item[F("value")]),
        .channel = strtoul(item[F("channel")], NULL, 10),
        .press = RadioPressState(String(item[F("press")]).toInt()),
        .action = RadioAction(String(item[F("action")]).toInt()),
        .serviceId = String(item[F("serviceId")]),
      });
    }
    radioStorage.save(model);
    // res
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioCommandGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(1024);
    // commands
    auto model = radioStorage.load();
    JsonArray commandArr = res.createNestedArray(F("commands"));
    for (const auto& command : model.commands) {
      JsonObject commandObj = commandArr.createNestedObject();
      commandObj[F("entry")] = command.entry;
      commandObj[F("action")] = command.action;
      commandObj[F("press")] = command.press;
      commandObj[F("serviceId")] = command.serviceId;
    }
    // service
    auto serviceModel = serviceStorage.load();
    JsonArray serviceArr = res.createNestedArray(F("services"));
    for (const auto& pair : serviceModel.services) {
      JsonObject serviceObj = serviceArr.createNestedObject();
      serviceObj[F("id")] = pair.first;
      serviceObj[F("name")] = pair.second.name;
    }
    // end
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioCommandSave() {
    _dispatchRequestStart();
    // payload
    auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(1024);
    deserializeJson(payload, payloadJson);
    // read
    auto commandItems = payload["commands"];
    // save
    auto model = radioStorage.load();
    model.commands.clear();
    for (size_t i = 0; i < commandItems.size(); i++) {
      auto item = commandItems[i];
      model.commands.push_back({
        .entry = RadioCommandEntry(String(item[F("entry")]).toInt()),
        .action = String(item[F("action")]).toInt(),
        .press = RadioPressState(String(item[F("press")]).toInt()),
        .serviceId = String(item[F("serviceId")]),
      });
    }
    radioStorage.save(model);
    // res
    DynamicJsonDocument res(64);
    res[F("message")] = String(F("success"));
    _sendJson(res);
    _dispatchRequestEnd();
  }

} // namespace Victor::Components
