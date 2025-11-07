const uglify = require('uglify-js');
const csso = require('csso');
const path = require('path');
const fs = require('fs');

const pwd = path.join(__dirname);
const scriptsPath = path.join(pwd, 'static', 'scripts');
const cssPath = path.join(pwd, 'static', 'styles');

const uglifyFiles = [
    {
        src: path.join(scriptsPath, 'core.js'),
        dest: path.join(scriptsPath, 'core.min.js'),
    },
    {
        src: path.join(scriptsPath, 'search.js'),
        dest: path.join(scriptsPath, 'search.min.js'),
    },
    {
        src: path.join(scriptsPath, 'third-party', 'hljs-original.js'),
        dest: path.join(scriptsPath, 'third-party', 'hljs.js'),
    },
    {
        src: path.join(scriptsPath, 'third-party', 'hljs-line-num-original.js'),
        dest: path.join(scriptsPath, 'third-party', 'hljs-line-num.js'),
    },
    {
        src: path.join(scriptsPath, 'third-party', 'tocbot.js'),
        dest: path.join(scriptsPath, 'third-party', 'tocbot.min.js'),
    },
];

const cssFilePaths = [
    path.join(cssPath, 'clean-jsdoc-theme-base.css'),
    path.join(cssPath, 'clean-jsdoc-theme-dark.css'),
    path.join(cssPath, 'clean-jsdoc-theme-light.css'),
];

let css = '';

for (const p of cssFilePaths) {
    css += fs.readFileSync(p, 'utf-8');
}

const cssWithScrollbar =
    css +
    fs.readFileSync(path.join(cssPath, 'clean-jsdoc-theme-scrollbar.css'));

const minifiedCss = csso.minify(css, { restructure: false });
const minifiedCssWithScrollbar = csso.minify(cssWithScrollbar, {
    restructure: false,
});

fs.writeFileSync(
    path.join(cssPath, 'clean-jsdoc-theme.min.css'),
    minifiedCssWithScrollbar.css
);

fs.writeFileSync(
    path.join(cssPath, 'clean-jsdoc-theme-without-scrollbar.min.css'),
    minifiedCss.css
);

for (const f of uglifyFiles) {
    const data = fs.readFileSync(f.src, 'utf-8');

    const out = uglify.minify(data, {
        compress: true,
    });

    fs.writeFileSync(f.dest, out.code);
}
