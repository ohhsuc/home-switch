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

Object.assign(vic, {
  last: (fn) => vic._last.push(fn),
  arr2opts: (items) => items.map((x) => ({ value: x[0], text: x[1] })),
});

vic(() => {
  return fetch("/tmpl/htm")
    .then((res) => res.text())
    .then((tmpl) => {
      document.querySelector("#html-tmpl").innerHTML = tmpl;
    });
});

vic.last(() => {
  document.querySelectorAll(".confirm").forEach((element) => {
    element.addEventListener("click", (ev) => {
      if (!confirm("Are you sure you want to do it?")) {
        ev.preventDefault();
      }
    });
  });
});
