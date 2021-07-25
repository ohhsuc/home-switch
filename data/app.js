(function (win) {
  const _init = [];
  window.vic = (fn) => {
    _init.push(fn);
  };
  const run = () => {
    const fn = _init.shift();
    if (typeof fn === "function") {
      Promise.resolve(fn())
        .then(run)
        .catch((ex) => console.error(ex));
    }
  };
  win.addEventListener("DOMContentLoaded", run);
})(window);

Object.assign(vic, {
  arr2opts: (items) => items.map((x) => ({ value: x[0], text: x[1] })),
});

vic(() => {
  document.querySelectorAll(".confirm").forEach((element) => {
    element.addEventListener("click", (ev) => {
      if (!confirm("Are you sure you want to do it?")) {
        ev.preventDefault();
      }
    });
  });
});

vic(() => {
  return fetch("/tmpl/htm")
    .then((res) => res.text())
    .then((tmpl) => {
      document.querySelector("#html-tmpl").innerHTML = tmpl;
    });
});
