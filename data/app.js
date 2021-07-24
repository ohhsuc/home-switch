(function () {
  const _init = [];
  window.Vic = (fn) => {
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
  window.addEventListener("DOMContentLoaded", run);
})();

Vic(() => {
  document.querySelectorAll(".confirm").forEach((element) => {
    element.addEventListener("click", (ev) => {
      if (!confirm("Are you sure you want to do it?")) {
        ev.preventDefault();
      }
    });
  });
});

Vic(() => {
  return fetch("/tmpl/htm")
    .then((res) => res.text())
    .then((tmpl) => {
      document.querySelector("#html-tmpl").innerHTML = tmpl;
    });
});
