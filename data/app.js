document.addEventListener("DOMContentLoaded", () => {
  document.querySelectorAll(".confirm").forEach((element) => {
    element.addEventListener("click", (ev) => {
      if (!confirm("Are you sure you want to do it?")) {
        ev.preventDefault();
      }
    });
  });
});
