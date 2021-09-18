const cp = require("child_process");
const path = require("path");
const fse = require("fs-extra");
const del = require("del");
const gulp = require("gulp");
const uglify = require("gulp-uglify");
const cleanCSS = require("gulp-clean-css");
const rename = require("gulp-rename");

const resource = {
  css: ["src/*.css"],
  js: ["src/*.js"],
  copy: ["src/fav.*"],
};

const paths = {
  dist: "../data/web",
};

function clean() {
  return del([paths.dist], { force: true });
}

function copy() {
  return gulp.src(resource.copy).pipe(gulp.dest(paths.dist));
}

function styles() {
  return gulp
    .src(resource.css)
    .pipe(cleanCSS())
    .pipe(
      rename({
        suffix: ".min",
      })
    )
    .pipe(gulp.dest(paths.dist));
}

function scripts() {
  return gulp
    .src(resource.js)
    .pipe(uglify())
    .pipe(
      rename({
        suffix: ".min",
      })
    )
    .pipe(gulp.dest(paths.dist));
}

function watch() {
  gulp.watch(resource.css, styles);
  gulp.watch(resource.js, scripts);
}

const build = gulp.series(
  clean,
  buildDeps,
  gulp.parallel(copy, styles, scripts)
);

function buildDeps() {
  const rootDir = path.resolve(__dirname, "../");
  const libDeps = path.resolve(__dirname, "../.pio/libdeps");
  const envNames = fse.readdirSync(libDeps);
  for (const envName of envNames) {
    const libDir = path.resolve(libDeps, envName, "home-esp8266/data");
    if (fse.existsSync(libDir)) {
      fse.copySync(libDir, path.resolve(rootDir, "data"), {
        overwrite: true,
      });
    }
  }
  return Promise.resolve();
}

// export gulp tasks
exports.styles = styles;
exports.scripts = scripts;
exports.watch = watch;
exports.build = build;
exports.default = build;
