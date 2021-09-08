const RadioView = {
  state: {
    loading: true,
    inputPin: -1,
    lastReceived: {
      value: "",
      channel: 0,
      timestamp: 1,
      timeSince: "",
    },
  },
  save() {
    const inputPin = vic.query("#txtInputPin").value;
    m.request({
      method: "POST",
      url: "/radio",
      body: { inputPin },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        this.oninit();
      }
    });
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/radio",
    }).then((res) => {
      this.state.loading = false;
      this.state.inputPin = res.inputPin;
      this.state.lastReceived = res.lastReceived;
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
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
      vic.renderTable({
        header: null,
        rows: [
          [
            "Last Received",
            this.state.lastReceived.value
              ? this.state.lastReceived.timeSince + " ago"
              : "-",
          ],
          [
            "Value",
            this.state.lastReceived.value ? this.state.lastReceived.value : "-",
          ],
          [
            "Channel",
            this.state.lastReceived.value
              ? this.state.lastReceived.channel
              : "-",
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
            value: this.state.inputPin,
          }),
        ]),
        m("p", [m("button.btn", { onclick: this.save.bind(this) }, "Save")]),
      ]),
    ];
  },
};

const RadioRuleView = {
  state: {
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
  },
  add() {
    this.state.rules.push({
      value: this.state.lastReceived.value,
      channel: this.state.lastReceived.channel,
      press: this.state.pressOptions[0][0],
      action: this.state.actionOptions[0][0],
      serviceId: "",
    });
    m.redraw();
  },
  remove(ev) {
    if (confirm("Are you sure you want to do it?")) {
      const index = parseInt(ev.target.value, 10);
      this.state.rules.splice(index, 1);
      m.redraw();
    }
  },
  save() {
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
        this.oninit();
      }
    });
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/radio/rule",
    }).then((res) => {
      this.state.loading = false;
      this.state.rules = res.rules;
      this.state.services = res.services;
      this.state.services.unshift({ id: "", name: "None" });
      this.state.lastReceived = res.lastReceived;
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.getLoading();
    }
    return [
      vic.getNav(),
      m("h3", "Radio Rules"),
      m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
      m("div.form", [
        vic.renderTable({
          header: ["", "Value", "Channel", "Press", "Action", "Service"],
          rows: this.state.rules.map((rule, index) => [
            m(
              "button.btn.weak",
              { name: "Remove", value: index, onclick: this.remove.bind(this) },
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
            vic.renderSelect(
              "PressId",
              rule.press,
              this.state.pressOptions.map((x) => ({ value: x[0], text: x[1] }))
            ),
            vic.renderSelect(
              "ActionId",
              rule.action,
              this.state.actionOptions.map((x) => ({ value: x[0], text: x[1] }))
            ),
            vic.renderSelect(
              "ServiceId",
              rule.serviceId,
              this.state.services.map((x) => ({ value: x.id, text: x.name }))
            ),
          ]),
        }),
        m("p", [
          m("button.btn", { onclick: this.add.bind(this) }, "Add+"),
          m("button.btn", { onclick: this.save.bind(this) }, "Save"),
        ]),
      ]),
    ];
  },
};

const RadioCommandView = {
  state: {
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
  },
  add() {
    const entryAction = this.state.entryActionOptions[0][0].split("-");
    this.state.commands.push({
      entry: entryAction[0],
      action: entryAction[1],
      press: this.state.pressOptions[0][0],
      serviceId: "",
    });
    m.redraw();
  },
  remove(ev) {
    if (confirm("Are you sure you want to do it?")) {
      const index = parseInt(ev.target.value, 10);
      this.state.commands.splice(index, 1);
      m.redraw();
    }
  },
  save() {
    const entryActionIdEls = vic.queryAll("select[name=EntryIdActionId]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const serviceIdEls = vic.queryAll("select[name=ServiceId]");
    const commands = entryActionIdEls.map((entryActionIdEl, i) => {
      const entryAction = entryActionIdEl.value.split("-");
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
        this.oninit();
      }
    });
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/radio/command",
    }).then((res) => {
      this.state.loading = false;
      this.state.commands = res.commands;
      this.state.services = res.services;
      this.state.services.unshift({ id: "", name: "None" });
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.getLoading();
    }
    return [
      vic.getNav(),
      m("h3", "Radio Commands"),
      m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
      m("div.form", [
        vic.renderTable({
          header: ["", "Entry", "Press", "Service"],
          rows: this.state.commands.map((command, index) => [
            m(
              "button.btn.weak",
              { name: "Remove", value: index, onclick: this.remove.bind(this) },
              "Remove"
            ),
            vic.renderSelect(
              "EntryIdActionId",
              "" + command.entry + "-" + command.action,
              this.state.entryActionOptions.map((x) => ({
                value: x[0],
                text: x[1],
              }))
            ),
            vic.renderSelect(
              "PressId",
              command.press,
              this.state.pressOptions.map((x) => ({ value: x[0], text: x[1] }))
            ),
            vic.renderSelect(
              "ServiceId",
              command.serviceId,
              this.state.services.map((x) => ({ value: x.id, text: x.name }))
            ),
          ]),
        }),
        m("p", [
          m("button.btn", { onclick: this.add.bind(this) }, "Add+"),
          m("button.btn", { onclick: this.save.bind(this) }, "Save"),
        ]),
      ]),
    ];
  },
};

const ServicesView = {
  state: {
    loading: true,
    services: [],
  },
  add() {
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
  },
  reset() {
    if (confirm("Are you sure you want to do it?")) {
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
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/service/list",
    }).then((res) => {
      this.state.loading = false;
      this.state.services = res.services;
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.getLoading();
    }
    return [
      vic.getNav(),
      m("h3", "Service"),
      m(
        "ul",
        this.state.services.map((x) =>
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
      m("p", [
        m("button.btn", { onclick: this.add.bind(this) }, "Add+"),
        m(
          "button.btn.weak",
          { onclick: this.reset.bind(this) },
          "Reset Accessory"
        ),
      ]),
    ];
  },
};

const ServiceItemView = {
  state: {
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
  },
  save() {
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
        this.oninit();
      }
    });
  },
  remove() {
    if (confirm("Are you sure you want to do it?")) {
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
  },
  oninit() {
    this.state.loading = true;
    const id = m.route.param("id");
    m.request({
      method: "GET",
      url: "/service?id=" + id,
    }).then((res) => {
      this.state.loading = false;
      this.state.service = res.service;
      m.redraw();
    });
  },
  renderPin(label, name, value, trueValue) {
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
  },
  view() {
    if (this.state.loading) {
      return vic.getLoading();
    }
    const id = m.route.param("id");
    return [
      vic.getNav(),
      m("h3", `Setting (${this.state.service.name})`),
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
            value: this.state.service.name,
          }),
        ]),
        m("fieldset", [
          m("legend", "Service Type"),
          vic.renderRadioList(
            "ServiceType",
            [this.state.service.type],
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
          this.renderPin(
            "Input",
            "InputPin",
            this.state.service.inputPin,
            this.state.service.inputTrueValue
          ),
          this.renderPin(
            "Output",
            "OutputPin",
            this.state.service.outputPin,
            this.state.service.outputTrueValue
          ),
        ]),
        m("p", [
          m("button.btn", { onclick: this.save.bind(this) }, "Save"),
          m("button.btn.weak", { onclick: this.remove.bind(this) }, "Delete"),
        ]),
      ]),
    ];
  },
};

const ServiceItemStateView = {
  state: {
    loading: true,
    service: { id: "", name: "", type: 1 },
    value: { boolValue: false, intValue: 0 },
  },
  save() {
    const body = {};
    if (this.state.service.type === 1) {
      body["boolValue"] = vic.query("input[name=BooleanValue]:checked").value;
    } else if (this.state.service.type === 2) {
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
        this.oninit();
      }
    });
  },
  oninit() {
    this.state.loading = true;
    const id = m.route.param("id");
    m.request({
      method: "GET",
      url: "/service/state?id=" + id,
    }).then((res) => {
      this.state.loading = false;
      this.state.service = res.service;
      this.state.value = res.value;
      m.redraw();
    });
  },
  renderBoolean() {
    const value = this.state.value.boolValue;
    return m("fieldset", [
      m("legend", "Boolean Value"),
      vic.renderRadioList(
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
  },
  renderInteger() {
    const value = this.state.value.intValue;
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
  },
  view() {
    if (this.state.loading) {
      return vic.getLoading();
    }
    const id = m.route.param("id");
    return [
      vic.getNav(),
      m("h3", `State (${this.state.service.name})`),
      m("p", [
        m(
          m.route.Link,
          { href: m.buildPathname("/service/:id", { id }) },
          `< Setting (${this.state.service.name})`
        ),
      ]),
      m("div.form", [
        this.state.service.type === 1 // BooleanServiceType
          ? this.renderBoolean()
          : this.state.service.type === 2 // IntegerServiceType
          ? this.renderInteger()
          : null,
        m("p", [m("button.btn", { onclick: this.save.bind(this) }, "Save")]),
      ]),
    ];
  },
};

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
