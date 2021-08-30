#include "WebPortal.h"

namespace Victoria::Components {

  WebPortal::WebPortal(int port)
  : VictoriaWeb(port) {}

  WebPortal::~WebPortal() {}

  void WebPortal::_registerHandlers() {
    VictoriaWeb::_registerHandlers();
    _server->on("/radio", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/radio", HTTP_ANY, std::bind(&WebPortal::_handleRadio, this));
    _server->on("/radio/rule", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/radio/rule", HTTP_ANY, std::bind(&WebPortal::_handleRadioRule, this));
    _server->on("/radio/command", HTTP_OPTIONS, std::bind(&WebPortal::_handleCrossOrigin, this));
    _server->on("/radio/command", HTTP_ANY, std::bind(&WebPortal::_handleRadioCommand, this));
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
      auto inputPin = _server->arg("InputPin");
      model.inputPin = inputPin.toInt();
      radioStorage.save(model);
      _redirectTo(_server->uri());
    } else {
      auto lastReceived = radioStorage.getLastReceived();
      auto hasReceived = lastReceived.value.length() > 0;
      TableModel receivedTable = {
        .header = {},
        .rows = {
          { "Value", hasReceived ? lastReceived.value : "-" },
          { "Channel", hasReceived ? String(lastReceived.channel) : "-" },
        },
      };
      _send200("\
        <p>\
          <a href=\"/\">&lt; Home</a> |\
          <a href=\"/radio/rule\">Rules</a> |\
          <a href=\"/radio/command\">Commands</a>\
        </p>\
        <h3>Radio</h3>\
        <form method=\"post\">\
          <p>\
            <label>Last received " + (hasReceived ? GlobalHelpers::timeSince(lastReceived.timestamp) + " ago" : "-") + "</label>\
            " + _renderTable(receivedTable) + "\
          </p>\
          <p>\
            <label for=\"txtInputPin\">Input Pin</label>\
            <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" min=\"-1\" max=\"100\" value=\"" + String(model.inputPin) + "\" />\
          </p>\
          <p>\
            <button type=\"submit\" name=\"Submit\" value=\"Save\" class=\"btn\">Save</button>\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioRule() {
    _dispatchRequestStart();
    auto model = radioStorage.load();
    if (_server->method() == HTTP_POST) {
      auto submit = _server->arg("Submit");
      if (submit == "Add") {
        auto lastReceived = radioStorage.getLastReceived();
        model.rules.push_back({
          .value = lastReceived.value,
          .channel = lastReceived.channel,
          .press = PressStateClick,
          .action = RadioActionNone,
          .serviceId = "",
        });
      } else if (submit.startsWith("Remove")) {
        submit.remove(0, 6);
        int removeIndex = submit.toInt();
        int loopIndex = -1;
        for (auto it = model.rules.begin(); it != model.rules.end(); it++) {
          if (++loopIndex == removeIndex) {
            model.rules.erase(it);
            break;
          }
        }
      } else {
        std::vector<String> values;
        std::vector<String> channels;
        std::vector<String> pressIds;
        std::vector<String> actionIds;
        std::vector<String> serviceIds;
        for (uint8_t i = 0; i < _server->args(); i++) {
          auto argValue = _server->arg(i);
          auto argName = _server->argName(i);
          if (argName == "Value") {
            values.push_back(argValue);
          } else if (argName == "Channel") {
            channels.push_back(argValue);
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
            .value = values[i],
            .channel = strtoul(channels[i].c_str(), NULL, 10),
            .press = RadioPressState(pressIds[i].toInt()),
            .action = RadioAction(actionIds[i].toInt()),
            .serviceId = serviceIds[i],
          });
        }
      }
      radioStorage.save(model);
      _redirectTo(_server->uri());
    } else {
      auto serviceOptionJson = String("['','None'],");
      auto serviceModel = serviceStorage.load();
      for (const auto& pair : serviceModel.services) {
        serviceOptionJson += "['" + pair.first + "','" + pair.second.name + "'],";
      }
      auto radioCommandsJson = String("");
      for (const auto& rule : model.rules) {
        radioCommandsJson += "{value:'" + rule.value + "',channel:" + String(rule.channel) + ",press:" + String(rule.press) + ",action:" + String(rule.action) + ",service:'" + rule.serviceId + "'},";
      }
      _send200("\
        <p><a href=\"/radio\">&lt; Radio</a></p>\
        <h3>Radio Rules</h3>\
        <form method=\"post\">\
          <p id=\"radioRules\"></p>\
          <script type=\"text/x-tmpl\" id=\"radio-rules\">\
            <table>\
              <tr>\
                <th class=\"lt\"></th>\
                <th class=\"lt\">Value</th>\
                <th class=\"lt\">Channel</th>\
                <th class=\"lt\">Press</th>\
                <th class=\"lt\">Action</th>\
                <th class=\"lt\">Service</th>\
              </tr>\
              {% for (var i=0; i<o.radioRules.length; i++) { var rule=o.radioRules[i]; %}\
              <tr>\
                <td><button type=\"submit\" name=\"Submit\" value=\"Remove{%=i%}\" class=\"btn confirm\">Remove</button></td>\
                <td><input type=\"text\" name=\"Value\" value=\"{%=rule.value%}\" maxlength=\"8\" /></td>\
                <td><input type=\"number\" name=\"Channel\" min=\"-1\" max=\"100\" value=\"{%=rule.channel%}\" /></td>\
                <td>{% include('html-select',{name:'PressId',value:rule.press,options:o.pressOptions}); %}</td>\
                <td>{% include('html-select',{name:'ActionId',value:rule.action,options:o.actionOptions}); %}</td>\
                <td>{% include('html-select',{name:'ServiceId',value:rule.service,options:o.serviceOptions}); %}</td>\
              </tr>\
              {% } %}\
            </table>\
          </script>\
          <script>\
          vic(() => {\
            var radioRules = document.querySelector('#radioRules');\
            if (radioRules) {\
              radioRules.innerHTML = tmpl('radio-rules', {\
                radioRules: [" + radioCommandsJson + "],\
                serviceOptions: vic.arr2opts([" + serviceOptionJson + "]),\
                pressOptions: vic.arr2opts([[1,'Click'],[2,'Double Click'],[3,'Long Press']]),\
                actionOptions: vic.arr2opts([[0,'None'],[1,'True'],[2,'False'],[3,'Toggle'],[4,'WiFi STA'],[5,'WiFi STA+AP'],[6,'WiFi Reset'],[7,'ESP Restart']]),\
              });\
            }\
          });\
          </script>\
          <p>\
            <button type=\"submit\" name=\"Submit\" value=\"Add\" class=\"btn\">Add+</button>\
            <button type=\"submit\" name=\"Submit\" value=\"Save\" class=\"btn\">Save</button>\
          </p>\
        </form>\
      ");
    }
    _dispatchRequestEnd();
  }

  void WebPortal::_handleRadioCommand() {
    _dispatchRequestStart();
    auto model = radioStorage.load();
    if (_server->method() == HTTP_POST) {
      auto submit = _server->arg("Submit");
      if (submit == "Add") {
        model.commands.push_back({
          .entry = EntryNone,
          .action = -1,
          .press = PressStateClick,
          .serviceId = "",
        });
      } else if (submit.startsWith("Remove")) {
        submit.remove(0, 6);
        int removeIndex = submit.toInt();
        int loopIndex = -1;
        for (auto it = model.commands.begin(); it != model.commands.end(); it++) {
          if (++loopIndex == removeIndex) {
            model.commands.erase(it);
            break;
          }
        }
      } else {
        std::vector<String> entryIds;
        std::vector<String> actionIds;
        std::vector<String> pressIds;
        std::vector<String> serviceIds;
        for (uint8_t i = 0; i < _server->args(); i++) {
          auto argValue = _server->arg(i);
          auto argName = _server->argName(i);
          if (argName == "EntryIdActionId") {
            auto idParts = GlobalHelpers::splitString(argValue, "-");
            entryIds.push_back(idParts[0]);
            actionIds.push_back(idParts[1]);
          } else if(argName == "PressId") {
            pressIds.push_back(argValue);
          } else if (argName == "ServiceId") {
            serviceIds.push_back(argValue);
          }
        }
        model.commands.clear();
        for (size_t i = 0; i < entryIds.size(); i++) {
          model.commands.push_back({
            .entry = RadioCommandEntry(entryIds[i].toInt()),
            .action = actionIds[i].toInt(),
            .press = RadioPressState(pressIds[i].toInt()),
            .serviceId = serviceIds[i],
          });
        }
      }
      radioStorage.save(model);
      _redirectTo(_server->uri());
    } else {
      auto serviceOptionJson = String("['','None'],");
      auto serviceModel = serviceStorage.load();
      for (const auto& pair : serviceModel.services) {
        serviceOptionJson += "['" + pair.first + "','" + pair.second.name + "'],";
      }
      auto radioCommandsJson = String("");
      for (const auto& command : model.commands) {
        radioCommandsJson += "{entry:" + String(command.entry) + ",action:" + String(command.action) + ",press:" + String(command.press) + ",service:'" + command.serviceId + "'},";
      }
      _send200("\
        <p><a href=\"/radio\">&lt; Radio</a></p>\
        <h3>Radio Commands</h3>\
        <form method=\"post\">\
          <p id=\"radioCommands\"></p>\
          <script type=\"text/x-tmpl\" id=\"radio-commands\">\
            <table>\
              <tr>\
                <th class=\"lt\"></th>\
                <th class=\"lt\">Entry</th>\
                <th class=\"lt\">Press</th>\
                <th class=\"lt\">Service</th>\
              </tr>\
              {% for (var i=0; i<o.radioCommands.length; i++) { var command=o.radioCommands[i]; %}\
              <tr>\
                <td><button type=\"submit\" name=\"Submit\" value=\"Remove{%=i%}\" class=\"btn confirm\">Remove</button></td>\
                <td>{% include('html-select',{name:'EntryIdActionId',value:command.entry+'-'+command.action,options:o.entryActionOptions}); %}</td>\
                <td>{% include('html-select',{name:'PressId',value:command.press,options:o.pressOptions}); %}</td>\
                <td>{% include('html-select',{name:'ServiceId',value:command.service,options:o.serviceOptions}); %}</td>\
              </tr>\
              {% } %}\
            </table>\
          </script>\
          <script>\
          vic(() => {\
            var radioCommands = document.querySelector('#radioCommands');\
            if (radioCommands) {\
              radioCommands.innerHTML = tmpl('radio-commands', {\
                radioCommands: [" + radioCommandsJson + "],\
                serviceOptions: vic.arr2opts([" + serviceOptionJson + "]),\
                entryActionOptions: vic.arr2opts([['0-0','None'],['1-1','wifi-join'],['1-2','wifi-mode'],['1-3','wifi-reset'],['2-1','app-name'],['2-2','app-ota'],['3-1','esp-restart'],['4-1','boolean-set'],['4-2','boolean-toggle']]),\
                pressOptions: vic.arr2opts([[1,'Click'],[2,'Double Click'],[3,'Long Press']]),\
              });\
            }\
          });\
          </script>\
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
      auto inputPin = _server->arg("InputPin");
      auto outputPin = _server->arg("OutputPin");
      auto inputTrueValue = _server->arg("InputTrueValue");
      auto outputTrueValue = _server->arg("OutputTrueValue");
      auto submit = _server->arg("Submit");
      if (submit == "Delete") {
        _deleteService(serviceId, service);
        _redirectTo("/");
      } else {
        service.name = serviceName;
        service.type =
          serviceType == "boolean" ? BooleanServiceType :
          serviceType == "integer" ? IntegerServiceType : EmptyServiceType;
        service.inputPin = inputPin.toInt();
        service.outputPin = outputPin.toInt();
        service.inputTrueValue = inputTrueValue.toInt();
        service.outputTrueValue = outputTrueValue.toInt();
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
            <button type=\"submit\" name=\"Submit\" value=\"Delete\" class=\"btnWeak confirm\">Delete</button>\
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
          <label for=\"txtInputPin\">Input</label>\
          <input type=\"number\" id=\"txtInputPin\" name=\"InputPin\" min=\"-1\" max=\"100\" value=\"" + String(service.inputPin) + "\" />\
          " + _getTrueValueHtml("InputTrueValue", service.inputTrueValue) + "\
        </p>\
        <p>\
          <label for=\"txtOutputPin\">Output</label>\
          <input type=\"number\" id=\"txtOutputPin\" name=\"OutputPin\" min=\"-1\" max=\"100\" value=\"" + String(service.outputPin) + "\" />\
          " + _getTrueValueHtml("OutputTrueValue", service.outputTrueValue) + "\
        </p>\
      </fieldset>\
    ");
    return html;
  }

  String WebPortal::_getTrueValueHtml(const String& name, const int& trueValue) {
    return String("\
      <span>Use</span>\
      <label for=\"txt" + name + "High\">\
        <span>HIGH</span>\
        <input type=\"radio\" id=\"txt" + name + "High\" name=\"" + name + "\" value=\"1\"" + _getCheckedAttr(trueValue == 1) + " />\
      </label>\
      <label for=\"txt" + name + "Low\">\
        <span>LOW</span>\
        <input type=\"radio\" id=\"txt" + name + "Low\" name=\"" + name + "\" value=\"0\"" + _getCheckedAttr(trueValue == 0) + " />\
      </label>\
      <span>as true value</span>\
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
