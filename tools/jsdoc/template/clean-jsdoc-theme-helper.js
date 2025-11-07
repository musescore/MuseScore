const has = require('lodash/has');
const klawSync = require('klaw-sync');
const path = require('path');
const fse = require('fs-extra');
const showdown = require('showdown');

const mdToHTMLConverter = new showdown.Converter();

function lsSync(dir, opts = {}) {
    const depth = has(opts, 'depth') ? opts.depth : -1;

    const files = klawSync(dir, {
        depthLimit: depth,
        filter: (f) => !path.basename(f.path).startsWith('.'),
        nodir: true,
    });

    return files.map((f) => f.path);
}

function copyToOutputFolder(filePath, outdir) {
    const resolvedPath = path.resolve(filePath);
    const filename = path.basename(resolvedPath);
    const out = path.join(outdir, filename);

    fse.copyFileSync(resolvedPath, out);
}

function copyToOutputFolderFromArray(filePathArray, outdir) {
    const outputList = [];

    if (Array.isArray(filePathArray)) {
        for (const filePath of filePathArray) {
            if (typeof filePath === 'string') {
                copyToOutputFolder(filePath, outdir);
                outputList.push(path.basename(filePath));
            } else if (typeof filePath === 'object') {
                const { filepath, targets } = filePath;

                copyToOutputFolder(filepath, outdir);
                outputList.push({ filepath: path.basename(filepath), targets });
            }
        }
    }

    return outputList;
}

function buildFooter(themeOpts) {
    var footer = themeOpts.footer;

    return footer;
}

function moduleHeader(themeOpts) {
    var displayModuleHeader = themeOpts.displayModuleHeader || false;

    return displayModuleHeader;
}

function getFavicon(themeOpts) {
    var favicon = themeOpts.favicon || undefined;

    return favicon;
}

// function copy
function createDynamicStyleSheet(themeOpts) {
    var styleClass = themeOpts.create_style || undefined;
    /* prettier-ignore-start */

    return styleClass;
}

function createDynamicsScripts(themeOpts) {
    var scripts = themeOpts.add_scripts || undefined;

    return scripts;
}

function returnPathOfScriptScr(themeOpts) {
    var scriptPath = themeOpts.add_script_path || undefined;

    return scriptPath;
}

function returnPathOfStyleSrc(themeOpts) {
    var stylePath = themeOpts.add_style_path || undefined;

    return stylePath;
}

function includeCss(themeOpts, outdir) {
    var stylePath = themeOpts.include_css || undefined;

    if (stylePath) {
        stylePath = copyToOutputFolderFromArray(stylePath, outdir);
    }

    return stylePath;
}

function resizeable(themeOpts) {
    var resizeOpts = themeOpts.resizeable || {};

    return resizeOpts;
}

function codepen(themeOpts) {
    var codepenOpts = themeOpts.codepen || {};

    return codepenOpts;
}

function includeScript(themeOpts, outdir) {
    var scriptPath = themeOpts.include_js || undefined;

    if (scriptPath) {
        scriptPath = copyToOutputFolderFromArray(scriptPath, outdir);
    }

    return scriptPath;
}

function getMetaTagData(themeOpts) {
    var meta = themeOpts.meta || undefined;

    return meta;
}

function getTheme(themeOpts) {
    var theme = themeOpts.default_theme || 'dark';

    return theme;
}

function getBaseURL(themeOpts) {
    return themeOpts.base_url;
}

function copyStaticFolder(themeOpts, outdir) {
    const staticDir = themeOpts.static_dir || undefined;

    if (staticDir) {
        for (const dir of staticDir) {
            const output = path.join(outdir, dir);

            fse.copySync(dir, output);
        }
    }
}

/**
 * Currently for some reason yields markdown is
 * not processed by jsdoc. So, we are processing it here
 *
 * @param {Array<{type: string, description: string}>} yields
 */
function getProcessedYield(yields) {
    if (!Array.isArray(yields)) return [];

    return yields.map((y) => ({
        ...y,
        description: mdToHTMLConverter.makeHtml(y.description),
    }));
}

module.exports = {
    buildFooter,
    moduleHeader,
    codepen,
    createDynamicStyleSheet,
    createDynamicsScripts,
    getBaseURL,
    getFavicon,
    getMetaTagData,
    getTheme,
    includeCss,
    includeScript,
    resizeable,
    returnPathOfScriptScr,
    returnPathOfStyleSrc,
    copyStaticFolder,
    getProcessedYield,
    lsSync,
};
