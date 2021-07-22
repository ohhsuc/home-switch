#include "WebPortal.h"

namespace Victoria::Components {

  WebPortal::WebPortal(int port = 80)
  : VictoriaWeb(port) {}

  WebPortal::~WebPortal() {}

  void WebPortal::_registerHandlers() {
    VictoriaWeb::_registerHandlers();
    _server->on("/radio", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/radio", HTTP_ANY, std::bind(&WebPortal::_handleRadio, this));
    _server->on("/service/new", HTTP_GET, std::bind(&WebPortal::_handleNewService, this));
    _server->on("/service", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/service", HTTP_ANY, std::bind(&WebPortal::_handleService, this));
    _server->on("/service/state", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/service/state", HTTP_ANY, std::bind(&WebPortal::_handleServiceState, this));
  }

  std::pair<bool, ServiceSetting> WebPortal::_getService(const String& serviceId) {
    auto found = false;
    auto model = serviceStorage.load();
    ServiceSetting service;
    if (model.services.count(serviceId) > 0) {
      service = model.services[serviceId];
      found = true;
    }
    if (!found) {
      _sendHints("Service Not Found", "\
        <p>Service ID: " + serviceId + "</p>\
      ");
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

  String WebPortal::_appendHomeBody() {
    auto body = VictoriaWeb::_appendHomeBody();
    // services
    auto model = serviceStorage.load();
    auto randomId = GlobalHelpers::randomString(4);
    auto newServiceUrl = "/service/new?id=" + randomId + "&index=" + String(model.services.size() + 1);
    auto serviceLinks = String("\
      <a href=\"" + newServiceUrl + "\">Add+</a>\
    ");
    for (const auto& pair : model.services) {
      auto url = String("/service?id=" + pair.first);
      serviceLinks += "\
        | <a href=\"" + url + "\">" + pair.second.name + "</a>\
      ";
    }
    body += "\
      <h3>Services</h3>\
      <p>\
        " + serviceLinks + "\
      </p>\
      <h3>Radio</h3>\
      <p>\
        <a href=\"/radio\">Setup</a>\
      </p>\
    ";
    return body;
  }

  void WebPortal::_handleRadio() {
    _dispatchRequestStart();
    auto model = radioStorage.load();
    if (_server->method() == HTTP_POST) {
      auto submit = _server->arg("Submit");
      if (submit == "Add") {
        auto lastReceived = radioStorage.getLastReceived();
        model.rules.push_back({
          .value = lastReceived.value,
          .protocol = lastReceived.protocol,
          .press = PressStateClick,
          .action = RadioActionNone,
          .serviceId = "",
        });
      } else if (submit.startsWith("Remove")) {
        submit.remove(0, 6);
        int removeIndex = submit.toInt();
        int ruleIndex = -1;
        for (auto it = model.rules.begin(); it != model.rules.end(); it++) {
          if (++ruleIndex == removeIndex) {
            model.rules.erase(it);
            break;
          }
        }
      } else {
        // pin
        auto inputPin = _server->arg("InputPin");
        model.inputPin = inputPin.toInt();
        // rules
        std::vector<String> values;
        std::vector<String> protocols;
        std::vector<String> pressIds;
        std::vector<String> actionIds;
        std::vector<String> serviceIds;
        for (uint8_t i = 0; i < _server->args(); i++) {
          auto argValue = _server->arg(i);
          auto argName = _server->argName(i);
          if (argName == "Value") {
            values.push_back(argValue);
          } else if (argName == "Protocol") {
            protocols.push_back(argValue);
          } else if(argName == "PressId") {
            pressIds.push_back(argValue);
          } else if (argName == "ActionId") {
            actionIds.push_back(argValue);
          } else if (argName == "ServiceId") {
            serviceIds.push_back(argValue);
          }
        }
        model.rules.clear();
        for (size_t i = 0; i < values.size(); i++) {
          model.rules.push_back({
            .value = strtoul(values[i].c_str(), NULL, 10),
            .protocol = strtoul(protocols[i].c_str(), NULL, 10),
            .press = RadioPressState(pressIds[i].toInt()),
            .action = RadioAction(actionIds[i].toInt()),
            .serviceId = serviceIds[i],
          });
        }
      }
      radioStorage.save(model);
      _redirectTo(_server->uri());
    } else {
      auto lastReceived = radioStorage.getLastReceived();
      auto hasReceived = lastReceived.value > 0;
      TableModel receivedTable = {
        .header = {},
        .rows = {
          { "Value", hasReceived ? String(lastReceived.value) : "-" },
          { "Bits", hasReceived ? String(lastReceived.bits) : "-" },
          { "Protocol", hasReceived ? String(lastReceived.protocol) : "-" },
        },
      };
      TableModel rulesTable = {
        .header = { "Rule", "Value", "Protocol", "Press", "Action", "Service" },
        .rows = {},
      };
      std::vector<SelectOption> pressOptions = {
        // { .value = "0", .text = "Await" },
        { .value = "1", .text = "Click" },
        { .value = "2", .text = "Double Click" },
        { .value = "3", .text = "Long Press" },
      };
      std::vector<SelectOption> actionOptions = {
        { .value = "0", .text = "None" },
        { .value = "1", .text = "True" },
        { .value = "2", .text = "False" },
        { .value = "3", .text = "Toggle" },
        { .value = "4", .text = "WiFi STA" },
        { .value = "5", .text = "WiFi STA+AP" },
        { .value = "6", .text = "WiFi Reset" },
        { .value = "7", .text = "ESP Restart" },
      };
      std::vector<SelectOption> serviceOptions = {
        { .value = "", .text = "None" },
      };
      auto serviceModel = serviceStorage.load();
      for (const auto& pair : serviceModel.services) {
        serviceOptions.push_back({
          .value = pair.first,
          .text = pair.second.name,
        });
      }
      int ruleIndex = -1;
      for (const auto& rule : model.rules) {
        ruleIndex++;
        SelectModel pressSelect = {
          .name = "PressId",
          .value = String(rule.press),
          .options = pressOptions,
        };
        SelectModel actionSelect = {
          .name = "ActionId",
          .value = String(rule.action),
          .options = actionOptions,
        };
        SelectModel serviceSelect = {
          .name = "ServiceId",
          .value = rule.serviceId,
          .options = serviceOptions,
        };
        rulesTable.rows.push_back({
          "<button type=\"submit\" name=\"Submit\" value=\"Remove" + String(ruleIndex) + "\" class=\"btn\" onclick=\"return confirm('Are you sure you want to remove?')\">Remove</button>",
          "<input type=\"number\" name=\"Value\" min=\"-1\" max=\"99999999\" value=\"" + String(rule.value) + "\" />",
          "<input type=\"number\" name=\"Protocol\" min=\"-1\" max=\"100\" value=\"" + String(rule.protocol) + "\" />",
          _renderSelect(pressSelect),
          _renderSelect(actionSelect),
          _renderSelect(serviceSelect),
        });
      }
      _send200("\
        <p><a href=\"/\">&lt; Home</a></p>\
        <h3>Radio</h3>\
        <form method=\"post\">\
          <p>\
            <label for=\"txtInputPin\">Input Pin</label>\
            <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" min=\"-1\" max=\"100\" value=\"" + String(model.inputPin) + "\" />\
          </p>\
          <p>\
            <label>Last received " + (hasReceived ? GlobalHelpers::timeSince(lastReceived.timestamp) + " ago" : "-") + "</label>\
            " + _renderTable(receivedTable) + "\
          </p>\
          <p>\
            " + _renderTable(rulesTable) + "\
          </p>\
          <p>\
            <button type=\"submit\" name=\"Submit\" value=\"Add\" class=\"btn\">Add+</button>\
            <button type=\"submit\" name=\"Submit\" value=\"Save\" class=\"btn\">Save</button>\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleNewService() {
    _dispatchRequestStart();
    auto serviceId = _server->arg("id");
    auto serviceIndex = _server->arg("index");
    // new
    ServiceSetting newSetting = {
      .name = "New" + serviceIndex,
      .type = BooleanServiceType,
    };
    _saveService(serviceId, newSetting);
    // redirect
    auto url = String("/service?id=" + serviceId);
    _redirectTo(url);
    _dispatchRequestEnd();
  }

  void WebPortal::_handleService() {
    _dispatchRequestStart();
    auto serviceId = _server->arg("id");
    auto currentUrl = String("/service?id=" + serviceId);
    auto found = _getService(serviceId);
    if (!found.first) {
      _dispatchRequestEnd();
      return;
    }
    ServiceSetting service = found.second;
    if (_server->method() == HTTP_POST) {
      auto serviceName = _server->arg("ServiceName");
      auto serviceType = _server->arg("ServiceType");
      auto outputPin = _server->arg("OutputPin");
      auto inputPin = _server->arg("InputPin");
      auto outputLevel = _server->arg("OutputLevel");
      auto inputLevel = _server->arg("InputLevel");
      auto submit = _server->arg("Submit");
      if (submit == "Delete") {
        _deleteService(serviceId, service);
        _redirectTo("/");
      } else {
        service.name = serviceName;
        service.type =
          serviceType == "boolean" ? BooleanServiceType :
          serviceType == "integer" ? IntegerServiceType : EmptyServiceType;
        service.outputPin = outputPin.toInt();
        service.inputPin = inputPin.toInt();
        service.outputLevel = outputLevel.toInt();
        service.inputLevel = inputLevel.toInt();
        _saveService(serviceId, service);
        _redirectTo(currentUrl);
      }
    } else {
      _send200("\
        <p>\
          <a href=\"/\">&lt; Home</a> |\
          <a href=\"/service/state?id=" + serviceId + "\">State</a>\
        </p>\
        <h3>Setting (" + service.name + ")</h3>\
        <form method=\"post\">\
          <p>\
            <label for=\"txtServiceName\">Name</label>\
            <input type=\"text\" id=\"txtServiceName\" name=\"ServiceName\" maxlength=\"20\" value=\"" + service.name + "\" />\
          </p>\
          " + _getTypeHtml(service) + "\
          " + _getIOHtml(service) + "\
          <p>\
            <button type=\"submit\" name=\"Submit\" value=\"Save\" class=\"btn\">Save</button>\
            <button type=\"submit\" name=\"Submit\" value=\"Delete\" class=\"btnWeak\" onclick=\"return confirm('Are you sure you want to delete?')\">Delete</button>\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleServiceState() {
    _dispatchRequestStart();
    auto serviceId = _server->arg("id");
    auto backUrl = String("/service?id=" + serviceId);
    auto currentUrl = String("/service/state?id=" + serviceId);
    auto found = _getService(serviceId);
    if (!found.first) {
      _dispatchRequestEnd();
      return;
    }
    ServiceSetting service = found.second;
    ServiceState state = {
      .boolValue = false,
      .intValue = 0,
    };
    if (_server->method() == HTTP_POST) {
      if (_server->hasArg("BooleanValue")) {
        auto booleanValue = _server->arg("BooleanValue");
        state.boolValue = (booleanValue == "1");
      }
      if (_server->hasArg("IntegerValue")) {
        auto integerValue = _server->arg("IntegerValue");
        state.intValue = integerValue.toInt();
      }
      if (onSetServiceState) {
        onSetServiceState(serviceId, service, state);
      }
      _redirectTo(currentUrl);
    } else {
      if (onGetServiceState) {
        state = onGetServiceState(serviceId, service);
      }
      auto stateHtml =
        service.type == BooleanServiceType ? _getBooleanHtml(state) :
        service.type == IntegerServiceType ? _getIntegerHtml(state) : String("");
      _send200("\
        <p>\
          <a href=\"" + backUrl + "\">&lt; Setting (" + service.name + ")</a>\
        </p>\
        <h3>State (" + service.name + ")</h3>\
        <form method=\"post\">\
          " + stateHtml + "\
          <p>\
            <button type=\"submit\" class=\"btn\">Save</button>\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  String WebPortal::_getTypeHtml(const ServiceSetting& service) {
    auto html = String("\
      <fieldset>\
        <legend>Service Type</legend>\
        " + _renderSelectionList({
          { .inputType = "radio", .inputName = "ServiceType", .inputValue = "boolean", .isChecked = (service.type == BooleanServiceType), .labelText = "Boolean - Service with boolean value such as switcher(on/off), shake sensor(yes/no)" },
          { .inputType = "radio", .inputName = "ServiceType", .inputValue = "integer", .isChecked = (service.type == IntegerServiceType), .labelText = "Integer - Service with integer value such as temperature, humidness" },
        }) + "\
      </fieldset>\
    ");
    return html;
  }

  String WebPortal::_getIOHtml(const ServiceSetting& service) {
    auto html = String("\
      <fieldset>\
        <legend>IO Pins</legend>\
        <p>\
          <label for=\"txtOutputPin\">Output</label>\
          <input type=\"number\" id=\"txtOutputPin\" name=\"OutputPin\" min=\"-1\" max=\"100\" value=\"" + String(service.outputPin) + "\" />\
          " + _getLevelHtml("OutputLevel", service.outputLevel) + "\
        </p>\
        <p>\
          <label for=\"txtInputPin\">Input</label>\
          <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" min=\"-1\" max=\"100\" value=\"" + String(service.inputPin) + "\" />\
          " + _getLevelHtml("InputLevel", service.inputLevel) + "\
        </p>\
      </fieldset>\
    ");
    return html;
  }

  String WebPortal::_getLevelHtml(const String& name, const int& level) {
    return String("\
      <label for=\"txt" + name + "High\">High</label>\
      <input type=\"radio\" id=\"txt" + name + "High\" name=\"" + name + "\" value=\"1\"" + _getCheckedAttr(level == 1) + " />\
      <label for=\"txt" + name + "Low\">Low</label>\
      <input type=\"radio\" id=\"txt" + name + "Low\" name=\"" + name + "\" value=\"0\"" + _getCheckedAttr(level == 0) + " />\
      <label for=\"txt" + name + "No\">No</label>\
      <input type=\"radio\" id=\"txt" + name + "No\" name=\"" + name + "\" value=\"-1\"" + _getCheckedAttr(level == -1) + " />\
    ");
  }

  String WebPortal::_getBooleanHtml(const ServiceState& state) {
    auto html = String("\
      <fieldset>\
        <legend>Boolean Value</legend>\
        " + _renderSelectionList({
          { .inputType = "radio", .inputName = "BooleanValue", .inputValue = "1", .isChecked = state.boolValue, .labelText = "On/Yes/True" },
          { .inputType = "radio", .inputName = "BooleanValue", .inputValue = "0", .isChecked = !state.boolValue, .labelText = "Off/No/False" },
        }) + "\
      </fieldset>\
    ");
    return html;
  }

  String WebPortal::_getIntegerHtml(const ServiceState& state) {
    auto html = String("\
      <fieldset>\
        <legend>Integer Value</legend>\
        <p>\
          <label for=\"txtIntegerValue\">Value</label>\
          <input type=\"number\" id=\"txtIntegerValue\" name=\"IntegerValue\" value=\"" + String(state.intValue) + "\"/>\
        </p>\
      </fieldset>\
    ");
    return html;
  }

  std::vector<SelectionOptions> WebPortal::_getResetList() {
    auto list = VictoriaWeb::_getResetList();
    list.push_back({
      .inputType = "checkbox",
      .inputName = "AccessoryReset",
      .inputValue = "1",
      .isChecked = false,
      .labelText = "Reset Accessory",
    });
    return list;
  }

  void WebPortal::_handleResetPost() {
    VictoriaWeb::_handleResetPost();
    if (_server->arg("AccessoryReset") == "1" && onResetAccessory) {
      onResetAccessory();
    }
  }

} // namespace Victoria::Components
