const RadioView = (() => {
  const state = {
    loading: true,
    millis: 0,
    inputPin: -1,
    lastReceived: {
      value: "",
      channel: 0,
      timestamp: 1,
    },
  };
  const save = () => {
    const inputPin = vic.query("#txtInputPin").value;
    const outputPin = vic.query("#txtOutputPin").value;
    m.request({
      method: "POST",
      url: "/radio",
      body: { inputPin, outputPin },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        oninit();
      }
    });
  };
  const oninit = () => {
    state.loading = true;
    m.request({
      method: "GET",
      url: "/radio",
    }).then((res) => {
      state.loading = false;
      state.millis = res.millis;
      state.inputPin = res.inputPin;
      state.outputPin = res.outputPin;
      state.lastReceived = res.lastReceived;
      m.redraw();
    });
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      return [
        vic.getNav(),
        m("h3", "Radio"),
        m("p", [
          m(m.route.Link, { href: "/radio/rule" }, "Rules"),
          m("span", " | "),
          m(m.route.Link, { href: "/radio/command" }, "Commands"),
        ]),
        vic.mTable({
          header: null,
          rows: [
            [
              "Last Received",
              state.lastReceived.value
                ? vic.ago(state.millis, state.lastReceived.timestamp) + " ago"
                : "-",
            ],
            [
              "Value",
              state.lastReceived.value ? state.lastReceived.value : "-",
            ],
            [
              "Channel",
              state.lastReceived.value ? state.lastReceived.channel : "-",
            ],
          ],
        }),
        m("div.form", [
          m("p", [
            m("label", { for: "txtInputPin" }, "Input Pin"),
            m("input[type=number]", {
              id: "txtInputPin",
              min: -1,
              max: 100,
              value: state.inputPin,
            }),
          ]),
          m("p", [
            m("label", { for: "txtOutputPin" }, "Output Pin"),
            m("input[type=number]", {
              id: "txtOutputPin",
              min: -1,
              max: 100,
              value: state.outputPin,
            }),
          ]),
          m("p", [m("button.btn", { onclick: save }, "Save")]),
        ]),
      ];
    },
  };
})();

const RadioRuleView = (() => {
  const state = {
    loading: true,
    rules: [{ value: "", channel: 0, press: 1, action: 1, serviceId: "" }],
    services: [{ id: "", name: "" }],
    lastReceived: { value: "", channel: 0 },
    pressOptions: [
      [1, "Click"],
      [2, "Double Click"],
      [3, "Long Press"],
    ],
    actionOptions: [
      [0, "None"],
      [1, "True"],
      [2, "False"],
      [3, "Toggle"],
      [4, "WiFi STA"],
      [5, "WiFi STA+AP"],
      [6, "WiFi Reset"],
      [7, "ESP Restart"],
    ],
  };
  const add = () => {
    state.rules.push({
      value: state.lastReceived.value,
      channel: state.lastReceived.channel,
      press: state.pressOptions[0][0],
      action: state.actionOptions[0][0],
      serviceId: "",
    });
    m.redraw();
  };
  const remove = (ev) => {
    if (vic.confirm()) {
      const index = parseInt(ev.target.value, 10);
      state.rules.splice(index, 1);
      m.redraw();
    }
  };
  const save = () => {
    const valueEls = vic.queryAll("input[name=Value]");
    const channelEls = vic.queryAll("input[name=Channel]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const actionIdEls = vic.queryAll("select[name=ActionId]");
    const serviceIdEls = vic.queryAll("select[name=ServiceId]");
    const rules = valueEls.map((valueEl, i) => ({
      value: valueEl.value,
      channel: channelEls[i].value,
      press: pressIdEls[i].value,
      action: actionIdEls[i].value,
      serviceId: serviceIdEls[i].value,
    }));
    m.request({
      method: "POST",
      url: "/radio/rule",
      body: { rules },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        oninit();
      }
    });
  };
  const oninit = () => {
    state.loading = true;
    m.request({
      method: "GET",
      url: "/radio/rule",
    }).then((res) => {
      state.loading = false;
      state.rules = res.rules;
      state.services = res.services;
      state.services.unshift({ id: "", name: "None" });
      state.lastReceived = res.lastReceived;
      m.redraw();
    });
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      return [
        vic.getNav(),
        m("h3", "Radio Rules"),
        m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
        m("div.form", [
          vic.mTable({
            header: ["", "Value", "Channel", "Press", "Action", "Service"],
            rows: state.rules.map((rule, index) => [
              m(
                "button.btn.weak",
                {
                  name: "Remove",
                  value: index,
                  onclick: remove,
                },
                "Remove"
              ),
              m("input[type=text]", {
                name: "Value",
                value: rule.value,
                maxLength: 8,
                style: { width: "60px" },
              }),
              m("input[type=number]", {
                name: "Channel",
                value: rule.channel,
                min: -1,
                max: 100,
              }),
              vic.mSelect(
                "PressId",
                rule.press,
                state.pressOptions.map((x) => ({
                  value: x[0],
                  text: x[1],
                }))
              ),
              vic.mSelect(
                "ActionId",
                rule.action,
                state.actionOptions.map((x) => ({
                  value: x[0],
                  text: x[1],
                }))
              ),
              vic.mSelect(
                "ServiceId",
                rule.serviceId,
                state.services.map((x) => ({ value: x.id, text: x.name }))
              ),
            ]),
          }),
          m("p", [
            m("button.btn", { onclick: add }, "Add+"),
            m("button.btn", { onclick: save }, "Save"),
          ]),
        ]),
      ];
    },
  };
})();

const RadioCommandView = (() => {
  const state = {
    loading: true,
    commands: [{ entry: 1, action: 1, press: 1, serviceId: "" }],
    services: [{ id: "", name: "" }],
    entryActionOptions: [
      ["0-0", "None"],
      ["1-1", "wifi-join"],
      ["1-2", "wifi-mode"],
      ["1-3", "wifi-reset"],
      ["2-1", "app-name"],
      ["2-2", "app-ota"],
      ["3-1", "esp-restart"],
      ["4-1", "boolean-set"],
      ["4-2", "boolean-toggle"],
    ],
    pressOptions: [
      [1, "Click"],
      [2, "Double Click"],
      [3, "Long Press"],
    ],
  };
  const add = () => {
    const entryAction = state.entryActionOptions[0][0].split("-");
    state.commands.push({
      entry: entryAction[0],
      action: entryAction[1],
      press: state.pressOptions[0][0],
      serviceId: "",
    });
    m.redraw();
  };
  const remove = (ev) => {
    if (vic.confirm()) {
      const index = parseInt(ev.target.value, 10);
      state.commands.splice(index, 1);
      m.redraw();
    }
  };
  const save = () => {
    const entryActionEls = vic.queryAll("select[name=EntryAction]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const serviceIdEls = vic.queryAll("select[name=ServiceId]");
    const commands = entryActionEls.map((el, i) => {
      const entryAction = el.value.split("-");
      return {
        entry: entryAction[0],
        action: entryAction[1],
        press: pressIdEls[i].value,
        serviceId: serviceIdEls[i].value,
      };
    });
    m.request({
      method: "POST",
      url: "/radio/command",
      body: { commands },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        oninit();
      }
    });
  };
  const oninit = () => {
    state.loading = true;
    m.request({
      method: "GET",
      url: "/radio/command",
    }).then((res) => {
      state.loading = false;
      state.commands = res.commands;
      state.services = res.services;
      state.services.unshift({ id: "", name: "None" });
      m.redraw();
    });
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      return [
        vic.getNav(),
        m("h3", "Radio Commands"),
        m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
        m("div.form", [
          vic.mTable({
            header: ["", "Entry", "Press", "Service"],
            rows: state.commands.map((command, index) => [
              m(
                "button.btn.weak",
                {
                  name: "Remove",
                  value: index,
                  onclick: remove,
                },
                "Remove"
              ),
              vic.mSelect(
                "EntryAction",
                "" + command.entry + "-" + command.action,
                state.entryActionOptions.map((x) => ({
                  value: x[0],
                  text: x[1],
                }))
              ),
              vic.mSelect(
                "PressId",
                command.press,
                state.pressOptions.map((x) => ({
                  value: x[0],
                  text: x[1],
                }))
              ),
              vic.mSelect(
                "ServiceId",
                command.serviceId,
                state.services.map((x) => ({ value: x.id, text: x.name }))
              ),
            ]),
          }),
          m("p", [
            m("button.btn", { onclick: add }, "Add+"),
            m("button.btn", { onclick: save }, "Save"),
          ]),
        ]),
      ];
    },
  };
})();

const ServicesView = (() => {
  const state = {
    loading: true,
    clientNumber: -1,
    services: [],
  };
  const add = () => {
    m.request({
      method: "POST",
      url: "/service/add",
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        m.route.set(m.buildPathname("/service/:id", { id: res.id }));
      }
    });
  };
  const reset = () => {
    if (vic.confirm()) {
      m.request({
        method: "POST",
        url: "/service/reset",
      }).then((res) => {
        if (res.error) {
          alert(res.error);
        } else {
          m.route.set("/");
        }
      });
    }
  };
  const oninit = () => {
    state.loading = true;
    m.request({
      method: "GET",
      url: "/service/list",
    }).then((res) => {
      state.loading = false;
      state.clientNumber = res.clientNumber;
      state.services = res.services;
      m.redraw();
    });
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      return [
        vic.getNav(),
        m("h3", "Service"),
        m(
          "ul",
          state.services.map((x) =>
            m("li", [
              m(
                m.route.Link,
                {
                  href: m.buildPathname("/service/:id", { id: x.id }),
                },
                x.name
              ),
            ])
          )
        ),
        vic.mTable({
          rows: [["Clients", state.clientNumber]],
        }),
        m("p", [
          m("button.btn", { onclick: add }, "Add+"),
          m("button.btn.weak", { onclick: reset }, "Reset HomeKit"),
        ]),
      ];
    },
  };
})();

const ServiceItemView = (() => {
  const state = {
    loading: true,
    service: {
      id: "",
      name: "",
      type: 1,
      inputPin: -1,
      outputPin: -1,
      inputTrueValue: 0,
      outputTrueValue: 0,
    },
  };
  const save = () => {
    const id = m.route.param("id");
    const service = {
      id,
      name: vic.query("#txtServiceName").value,
      type: vic.query("input[name=ServiceType]:checked").value,
      inputPin: vic.query("input[name=InputPin]").value,
      outputPin: vic.query("input[name=OutputPin]").value,
      inputTrueValue: vic.query("input[name=InputPinTrueValue]:checked").value,
      outputTrueValue: vic.query("input[name=OutputPinTrueValue]:checked")
        .value,
    };
    m.request({
      method: "POST",
      url: "/service?id=" + id,
      body: service,
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        oninit();
      }
    });
  };
  const remove = () => {
    if (vic.confirm()) {
      const id = m.route.param("id");
      m.request({
        method: "DELETE",
        url: "/service?id=" + id,
      }).then((res) => {
        if (res.error) {
          alert(res.error);
        } else {
          m.route.set("/service");
        }
      });
    }
  };
  const oninit = () => {
    state.loading = true;
    const id = m.route.param("id");
    m.request({
      method: "GET",
      url: "/service?id=" + id,
    }).then((res) => {
      state.loading = false;
      state.service = res.service;
      m.redraw();
    });
  };
  const renderPin = (label, name, value, trueValue) => {
    return m("p", [
      m("label", { for: `txt${name}` }, label),
      m("input[type=number]", {
        id: `txt${name}`,
        name,
        value,
        min: -1,
        max: 100,
      }),
      m("span", "Use "),
      m("label", { for: `rdo${name}High` }, [
        m("span", "HIGH"),
        m("input[type=radio]", {
          id: `rdo${name}High`,
          name: `${name}TrueValue`,
          value: 1,
          checked: trueValue === 1,
        }),
      ]),
      m("label", { for: `rdo${name}Low` }, [
        m("span", "LOW"),
        m("input[type=radio]", {
          id: `rdo${name}Low`,
          name: `${name}TrueValue`,
          value: 0,
          checked: trueValue === 0,
        }),
      ]),
      m("span", "as true value"),
    ]);
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      const id = m.route.param("id");
      return [
        vic.getNav(),
        m("h3", `Setting (${state.service.name})`),
        m("p", [
          m(m.route.Link, { href: "/service" }, "< Service"),
          m("span", " | "),
          m(
            m.route.Link,
            { href: m.buildPathname("/service/state/:id", { id }) },
            "State"
          ),
        ]),
        m("div.form", [
          m("p", [
            m("label", { for: "txtServiceName" }, "Name"),
            m("input[type=text]", {
              id: "txtServiceName",
              maxLength: 20,
              value: state.service.name,
            }),
          ]),
          m("fieldset", [
            m("legend", "Service Type"),
            vic.mRadioList(
              "ServiceType",
              [state.service.type],
              [
                {
                  value: 1, // BooleanServiceType = 1
                  text: "Boolean - Service with boolean value such as switcher(on/off), shake sensor(yes/no)",
                },
                {
                  value: 2, // IntegerServiceType = 2
                  text: "Integer - Service with integer value such as temperature, humidness",
                },
              ]
            ),
          ]),
          m("fieldset", [
            m("legend", "IO Pins"),
            renderPin(
              "Input",
              "InputPin",
              state.service.inputPin,
              state.service.inputTrueValue
            ),
            renderPin(
              "Output",
              "OutputPin",
              state.service.outputPin,
              state.service.outputTrueValue
            ),
          ]),
          m("p", [
            m("button.btn", { onclick: save }, "Save"),
            m("button.btn.weak", { onclick: remove }, "Delete"),
          ]),
        ]),
      ];
    },
  };
})();

const ServiceItemStateView = (() => {
  const state = {
    loading: true,
    service: { id: "", name: "", type: 1 },
    value: { boolValue: false, intValue: 0 },
  };
  const save = () => {
    const body = {};
    if (state.service.type === 1) {
      body["boolValue"] = vic.query("input[name=BooleanValue]:checked").value;
    } else if (state.service.type === 2) {
      body["intValue"] = vic.query("#txtIntegerValue").value;
    }
    const id = m.route.param("id");
    m.request({
      method: "POST",
      url: "/service/state?id=" + id,
      body,
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        oninit();
      }
    });
  };
  const oninit = () => {
    state.loading = true;
    const id = m.route.param("id");
    m.request({
      method: "GET",
      url: "/service/state?id=" + id,
    }).then((res) => {
      state.loading = false;
      state.service = res.service;
      state.value = res.value;
      m.redraw();
    });
  };
  const renderBoolean = () => {
    const value = state.value.boolValue;
    return m("fieldset", [
      m("legend", "Boolean Value"),
      vic.mRadioList(
        "BooleanValue",
        [value],
        [
          {
            value: true,
            text: "On/Yes/True",
          },
          {
            value: false,
            text: "Off/No/False",
          },
        ]
      ),
    ]);
  };
  const renderInteger = () => {
    const value = state.value.intValue;
    return m("fieldset", [
      m("legend", "Integer Value"),
      m("p", [
        m("label", { for: "txtIntegerValue" }, "Value"),
        m("input[type=number]", {
          id: "txtIntegerValue",
          value,
        }),
      ]),
    ]);
  };
  return {
    oninit,
    view() {
      if (state.loading) {
        return vic.getLoading();
      }
      const id = m.route.param("id");
      return [
        vic.getNav(),
        m("h3", `State (${state.service.name})`),
        m("p", [
          m(
            m.route.Link,
            { href: m.buildPathname("/service/:id", { id }) },
            `< Setting (${state.service.name})`
          ),
        ]),
        m("div.form", [
          state.service.type === 1 // BooleanServiceType
            ? renderBoolean()
            : state.service.type === 2 // IntegerServiceType
            ? renderInteger()
            : null,
          m("p", [m("button.btn", { onclick: save }, "Save")]),
        ]),
      ];
    },
  };
})();

vic.appendNav((items) =>
  items.concat([
    m("span", " | "),
    vic.navItem("/radio", "Radio"),
    m("span", " | "),
    vic.navItem("/service", "Service"),
  ])
);

vic.appendRoute((config) =>
  Object.assign(config, {
    "/radio": RadioView,
    "/radio/rule": RadioRuleView,
    "/radio/command": RadioCommandView,
    "/service": ServicesView,
    "/service/:id": ServiceItemView,
    "/service/state/:id": ServiceItemStateView,
  })
);
