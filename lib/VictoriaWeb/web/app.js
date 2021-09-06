(function (win) {
  const _init = [];
  const vic = (fn) => {
    _init.push(fn);
  };
  vic._last = [];
  const run = () => {
    const fn = _init.shift() || vic._last.shift();
    if (typeof fn === "function") {
      Promise.resolve(fn())
        .then(run)
        .catch((ex) => console.error(ex));
    }
  };
  win.vic = vic;
  win.addEventListener("DOMContentLoaded", run);
})(window);

vic.query = (selector) => document.querySelector(selector);
vic.queryAll = (selector) => Array.from(document.querySelectorAll(selector));

vic.mainNavigation = m("p.nav", [
  m(m.route.Link, { href: "/" }, "Home"),
  m("span", " | "),
  m(m.route.Link, { href: "/wifi" }, "Wifi"),
  m("span", " | "),
  m(m.route.Link, { href: "/fs" }, "File System"),
  m("span", " | "),
  m(m.route.Link, { href: "/ota" }, "OTA"),
  m("span", " | "),
  m(m.route.Link, { href: "/reset" }, "Reset"),
]);

vic.loadingComponent = [vic.mainNavigation, m("div.loading", "Loading....")];

vic.appendNav = (nav) => {
  nav.forEach((m) => vic.mainNavigation.children.push(m));
};

vic.appendRoute = (config) => {
  vic._routeConfig = config;
};

vic.renderSelect = (name, value, options) => {
  return m(
    "select",
    { name },
    options.map((x) =>
      m("option", { value: x.value, selected: value === x.value }, x.text)
    )
  );
};

vic.renderTable = (data) => {
  let rows = [];
  if (data.header && data.header.length > 0) {
    rows.push(
      m(
        "tr",
        data.header.map((c) => m("th.lt", [c]))
      )
    );
  }
  if (data.rows && data.rows.length > 0) {
    rows = rows.concat(
      data.rows.map((row) =>
        m(
          "tr",
          row.map((cell) => m("td", [cell]))
        )
      )
    );
  }
  return m("table", rows);
};

function renderSelectionList(type, name, values, list) {
  const items = list.map((item) =>
    m("li", [
      m("input", {
        type,
        name,
        id: `rdo${name}${item.value}`,
        value: item.value,
        checked: values.includes(item.value),
      }),
      m("label", { for: `rdo${name}${item.value}` }, item.text),
    ])
  );
  return m("ul", items);
}

vic.renderRadioList = (name, values, list) =>
  renderSelectionList("radio", name, values, list);

vic.renderCheckboxList = (name, values, list) =>
  renderSelectionList("checkbox", name, values, list);

const HomeView = {
  state: {
    loading: true,
    rows: [],
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/home",
    }).then((res) => {
      this.state.loading = false;
      this.state.rows = [
        ["Running", res.running],
        ["Wifi Mode", res.wifiMode],
        ["Joined", res.joined ? res.joined : "-"],
        [
          "STA Address",
          res.staAddress
            ? m("a", { href: `http://${res.staAddress}` }, res.staAddress)
            : "-",
        ],
        ["STA MAC Address", res.staMacAddress],
        [
          "AP Address",
          res.apAddress
            ? m("a", { href: `http://${res.apAddress}` }, res.apAddress)
            : "-",
        ],
        ["AP MAC Address", res.apMacAddress],
        ["Firmware Version", res.firmwareVersion],
      ];
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", "Home"),
      m("p", [
        vic.renderTable({
          header: null,
          rows: this.state.rows,
        }),
      ]),
    ];
  },
};

const FileSystemView = {
  state: {
    loading: true,
    infos: [],
    files: [],
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/fs",
    }).then((res) => {
      this.state.loading = false;
      this.state.infos = [
        ["Total Bytes", res.totalBytes],
        ["Used Bytes", res.usedBytes],
        ["Max Path Length", res.maxPathLength],
        ["Max Open Files", res.maxOpenFiles],
        ["Block Size", res.blockSize],
        ["Page Size", res.pageSize],
      ];
      this.state.files = res.files;
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", "File System"),
      m("p", [
        vic.renderTable({
          header: ["Storage", ""],
          rows: this.state.infos,
        }),
      ]),
      m("p", [
        vic.renderTable({
          header: ["File", "Bytes"],
          rows: Object.keys(this.state.files).map((path) => [
            m(
              m.route.Link,
              {
                href: m.buildPathname("/file/:path", { path }),
              },
              path
            ),
            this.state.files[path],
          ]),
        }),
      ]),
    ];
  },
};

const FileItemView = {
  state: {
    loading: true,
    file: { name: "", size: 0, content: "" },
  },
  request(method, body) {
    const path = m.route.param("path");
    return m.request({
      method,
      url: "/file",
      params: { path },
      body,
    });
  },
  save() {
    this.request("POST", {
      content: vic.query("textarea").value,
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
      this.request("DELETE").then((res) => {
        if (res.error) {
          alert(res.error);
          return;
        }
        m.route.set("/fs");
      });
    }
  },
  oninit() {
    this.state.loading = true;
    this.request("GET").then((res) => {
      this.state.file = res;
      this.state.loading = false;
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", `${this.state.file.name} (${this.state.file.size} bytes)`),
      m("p", [m(m.route.Link, { href: "/fs" }, "< Files")]),
      m("div.form", [
        m("p", [
          m("textarea", { cols: 50, rows: 10 }, this.state.file.content),
        ]),
        m("p", [
          m("button.btn", { onclick: this.save.bind(this) }, "Save"),
          m("button.btnWeak", { onclick: this.remove.bind(this) }, "Delete"),
        ]),
      ]),
    ];
  },
};

const WifiView = {
  state: {
    loading: true,
    connected: null,
    founds: [{ ssid: "", rssi: 10 }],
    password: "",
  },
  scan() {
    this.state.password = vic.query("#txtPassword").value;
    this.oninit();
  },
  join() {
    const ssidEl = vic.query("input[type=radio]:checked");
    if (!ssidEl) {
      alert("Please select wifi to join");
      return;
    }
    const passEl = vic.query("#txtPassword");
    m.request({
      method: "POST",
      url: "/wifi/join",
      body: { ssid: ssidEl.value, password: passEl.value },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else if (res.ip) {
        alert("Success! obtain ip: " + res.ip);
      }
      m.redraw();
    });
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/wifi",
    }).then((res) => {
      this.state.loading = false;
      Object.assign(this.state, res);
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", "Join WiFi"),
      m("div.form", [
        vic.renderRadioList(
          "ssid",
          [this.state.connected],
          this.state.founds.map((x) => ({
            value: x.ssid,
            text: `${x.ssid} (${-x.rssi}%)`,
          }))
        ),
        m("p", [
          m("label", { for: "txtPassword" }, "Password"),
          m("input[type=text]", {
            id: "txtPassword",
            maxLength: 32,
            value: this.state.password,
          }),
        ]),
        m("p", [
          m("button.btn", { onclick: this.scan.bind(this) }, "Scan"),
          m("button.btn", { onclick: this.join.bind(this) }, "Join"),
        ]),
      ]),
    ];
  },
};

const OtaView = {
  state: {
    loading: true,
    version: null,
    newVersion: null,
    rows: [],
  },
  fire() {
    const version = "";
    const otaType = "";
    m.request({
      method: "POST",
      url: "/ota/fire",
      body: { version, otaType },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        m.redraw();
      }
    });
  },
  oninit() {
    this.state.loading = true;
    m.request({
      method: "GET",
      url: "/ota",
    }).then((res) => {
      this.state.loading = false;
      this.state.version = res.otaVersion;
      this.state.newVersion = res.otaNewVersion;
      this.state.rows = [
        ["Chip ID", res.chipId],
        ["Chip Size", res.chipSize],
        ["Sketch Size", res.sketchSize],
        ["Sketch Free Space", res.sketchFreeSpace],
        ["Sketch MD5", res.sketchMD5],
        ["SDK Version", res.sdkVersion],
      ];
      m.redraw();
    });
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", "OTA"),
      m("p", [m("a", { href: "/update" }, "Update - Over Web")]),
      m("div.form", [
        m("p", [
          vic.renderTable({
            header: ["ESP", ""],
            rows: this.state.rows,
          }),
        ]),
        m("p", "Remote Latest: " + this.state.newVersion),
        m("p", "Local Firmware: " + this.state.version),
        vic.renderRadioList(
          "OtaType",
          ["sketch"],
          [
            { value: "all", text: "All" },
            { value: "sketch", text: "Sketch" },
            { value: "fs", text: "File System" },
          ]
        ),
        m("p", [
          m(
            "button.btn",
            { onclick: this.fire.bind(this) },
            "Load + Burn " + this.state.newVersion
          ),
        ]),
      ]),
    ];
  },
};

const ResetView = {
  state: {
    loading: true,
  },
  reset() {
    const values = vic
      .queryAll("input[name=Reset]:checked")
      .map((x) => x.value)
      .join(",");
    m.request({
      method: "POST",
      url: "/reset",
      body: { values },
    }).then((res) => {
      if (res.error) {
        alert(res.error);
      } else {
        m.redraw();
      }
    });
  },
  oninit() {
    this.state.loading = false;
  },
  view() {
    if (this.state.loading) {
      return vic.loadingComponent;
    }
    return [
      vic.mainNavigation,
      m("h3", "Reset"),
      m("div.form", [
        vic.renderCheckboxList(
          "Reset",
          [],
          [
            { value: "EspRestart", text: "ESP Restart" },
            { value: "EspReset", text: "ESP Reset" },
            { value: "EspEraseCfg", text: "ESP Erase Config" },
            { value: "WifiReset", text: "Reset Wifi" },
          ]
        ),
        m("p", [m("button.btn", { onclick: this.reset.bind(this) }, "Submit")]),
      ]),
    ];
  },
};

const NotfoundView = {
  view() {
    return "Oops... Notfound";
  },
};

vic(() => {
  const root = vic.query("div.main");
  m.route(
    root,
    "/",
    Object.assign(
      {
        "/": HomeView,
        "/ota": OtaView,
        "/fs": FileSystemView,
        "/file/:path": FileItemView,
        "/wifi": WifiView,
        "/reset": ResetView,
        "/404": NotfoundView,
      },
      vic._routeConfig || {}
    )
  );
});
