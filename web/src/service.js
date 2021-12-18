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
    vic.navItem("/service", "Service"),
  ])
);

vic.appendRoute((config) =>
  Object.assign(config, {
    "/service": ServicesView,
    "/service/:id": ServiceItemView,
    "/service/state/:id": ServiceItemStateView,
  })
);
