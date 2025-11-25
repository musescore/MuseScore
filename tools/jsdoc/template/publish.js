const _ = require('lodash');
const env = require('jsdoc/env');
const fs = require('fs-extra');
const helper = require('jsdoc/util/templateHelper');
const logger = require('jsdoc/util/logger');
const path = require('jsdoc/path');
const { taffy } = require('@jsdoc/salty');
const template = require('jsdoc/template');
const htmlMinify = require('html-minifier-terser');

const {
    buildFooter,
    codepen,
    createDynamicStyleSheet,
    createDynamicsScripts,
    getBaseURL,
    getFavicon,
    getMetaTagData,
    getTheme,
    includeCss,
    includeScript,
    moduleHeader,
    resizeable,
    returnPathOfScriptScr,
    returnPathOfStyleSrc,
    copyStaticFolder,
    getProcessedYield,
    lsSync,
} = require('./clean-jsdoc-theme-helper');

const {
    HTML_MINIFY_OPTIONS,
    SECTION_TYPE,
    defaultSections,
} = require('./clean-jsdoc-theme-defaults');

const htmlsafe = helper.htmlsafe;
const linkto = helper.linkto;
const resolveAuthorLinks = helper.resolveAuthorLinks;
const hasOwnProp = Object.prototype.hasOwnProperty;

const themeOpts = (env && env.opts && env.opts.theme_opts) || {};

let data;
let view;
/**
 * @type {Array<{title: string, link: string, description: string}>}
 */
const searchList = [];
const hasSearch =
    themeOpts.search === undefined ? true : Boolean(themeOpts.search);

// eslint-disable-next-line no-restricted-globals
let outdir = path.resolve(path.normalize(env.opts.destination));

function mkdirSync(filepath) {
    return fs.mkdirSync(filepath, { recursive: true });
}

function sourceToDestination(parentDir, sourcePath, destDir) {
    const relativeSource = path.relative(parentDir, sourcePath);

    return path.resolve(path.join(destDir, relativeSource));
}

function find(spec) {
    return helper.find(data, spec);
}

function tutoriallink(tutorial) {
    return helper.toTutorial(tutorial, null, {
        tag: 'em',
        classname: 'disabled',
        prefix: 'Tutorial: ',
    });
}

function getAncestorLinks(doclet) {
    return helper.getAncestorLinks(data, doclet);
}

function hashToLink(doclet, hash, dependencies) {
    let url;

    if (!/^(#.+)/.test(hash)) {
        return hash;
    }

    url = helper.createLink(doclet, dependencies);
    url = url.replace(/(#.+|$)/, hash);

    return `<a href="${url}">${hash}</a>`;
}

function needsSignature({ kind, type, meta }) {
    let needsSig = false;

    // function and class definitions always get a signature
    if (kind === 'function' || kind === 'class') {
        needsSig = true;
    }

    // typedefs that contain functions get a signature, too
    else if (kind === 'typedef' && type && type.names && type.names.length) {
        for (let i = 0, l = type.names.length; i < l; i++) {
            if (type.names[i].toLowerCase() === 'function') {
                needsSig = true;
                break;
            }
        }
    }

    // and namespaces that are functions get a signature (but finding them is a
    // bit messy)
    else if (
        kind === 'namespace' &&
        meta &&
        meta.code &&
        meta.code.type &&
        meta.code.type.match(/[Ff]unction/)
    ) {
        needsSig = true;
    }

    return needsSig;
}

function getSignatureAttributes({ optional, nullable }) {
    const attributes = [];

    if (optional) {
        attributes.push('opt');
    }

    if (nullable === true) {
        attributes.push('nullable');
    } else if (nullable === false) {
        attributes.push('non-null');
    }

    return attributes;
}

function updateItemName(item) {
    const attributes = getSignatureAttributes(item);
    let itemName = item.name || '';

    if (item.variable) {
        itemName = '&hellip;' + itemName;
    }

    if (attributes && attributes.length) {
        itemName = `${itemName}<span class="signature-attributes">${attributes.join(
            ', '
        )}</span>`;
    }

    return itemName;
}

function addParamAttributes(params) {
    return params
        .filter(({ name }) => name && !name.includes('.'))
        .map(updateItemName);
}

function buildItemTypeStrings(item) {
    const types = [];

    if (item && item.type && item.type.names) {
        item.type.names.forEach(function (name) {
            types.push(linkto(name, htmlsafe(name)));
        });
    }

    return types;
}

function buildAttribsString(attribs) {
    let attribsString = '';

    if (attribs && attribs.length) {
        attribsString = htmlsafe(`(${attribs.join(', ')}) `);
    }

    return attribsString;
}

function addNonParamAttributes(items) {
    let types = [];

    items.forEach(function (item) {
        types = types.concat(buildItemTypeStrings(item));
    });

    return types;
}

function addSignatureParams(f) {
    const params = f.params ? addParamAttributes(f.params) : [];

    f.signature = `${f.signature || ''}(${params.join(', ')})`;
}

function addSignatureReturns(f) {
    const attribs = [];
    let attribsString = '';
    let returnTypes = [];
    let returnTypesString = '';
    const source = f.yields || f.returns;

    // jam all the return-type attributes into an array. this could create odd results (for example,
    // if there are both nullable and non-nullable return types), but let's assume that most people
    // who use multiple @return tags aren't using Closure Compiler type annotations, and vice-versa.
    if (source) {
        source.forEach((item) => {
            helper.getAttribs(item).forEach((attrib) => {
                if (!attribs.includes(attrib)) {
                    attribs.push(attrib);
                }
            });
        });

        attribsString = buildAttribsString(attribs);
    }

    if (source) {
        returnTypes = addNonParamAttributes(source);
    }
    if (returnTypes.length) {
        returnTypesString = ` &rarr; ${attribsString}{${returnTypes.join(
            '|'
        )}}`;
    }

    let signatureOutput = '';

    if (f.signature) {
        signatureOutput =
            '<span class="signature">' + (f.signature || '') + '</span>';
    }
    if (returnTypesString) {
        signatureOutput +=
            '<span class="type-signature">' + returnTypesString + '</span>';
    }

    f.signature = signatureOutput;
}

function addSignatureTypes(f) {
    const types = f.type ? buildItemTypeStrings(f) : [];

    f.signature =
        `${f.signature || ''}<span class="type-signature">` +
        `${types.length ? ` :${types.join('|')}` : ''}</span>`;
}

function addAttribs(f) {
    const attribs = helper.getAttribs(f);
    const attribsString = buildAttribsString(attribs);

    f.attribs = `<span class="type-signature">${attribsString}</span>`;
}

function shortenPaths(files, commonPrefix) {
    Object.keys(files).forEach(function (file) {
        files[file].shortened = files[file].resolved
            .replace(commonPrefix, '')
            // always use forward slashes
            .replace(/\\/g, '/');
    });

    return files;
}

function getPathFromDoclet({ meta }) {
    if (!meta) {
        return null;
    }

    return meta.path && meta.path !== 'null'
        ? path.join(meta.path, meta.filename)
        : meta.filename;
}

function createPrettyAnchor(elementType, ancestor, name, href) {
    return `<${elementType} ${href ? `href="${href}"` : ''} class="has-anchor">
        <span class="ancestors">
            ${ancestor}~
        </span>
        ${name}
    </${elementType}>`;
}

function prefixModuleToItemAnchor(item) {
    let { anchor } = item;

    let anchorLink = anchor.split('href="')[1].split('"')[0];
    let cleanLink = anchorLink.replace(/\.html$/, '');

    let prettyAnchor;

    cleanLink.replace(
        /module-([^-]+)(?:-|\.)(.*)/,
        (_match, modulename, methodname) => {
            prettyAnchor = createPrettyAnchor(
                'a',
                modulename,
                methodname,
                anchorLink
            );
        }
    );

    return prettyAnchor || anchor;
}

async function generate(title, docs, filename, resolveLinks) {
    let docData;
    let html;
    let outpath;

    docData = {
        env: env,
        title: title,
        docs: docs,
        filename,
    };

    outpath = path.join(outdir, filename);
    html = view.render('container.tmpl', docData);

    if (resolveLinks !== false) {
        html = helper.resolveLinks(html); // turn {@link foo} into <a href="foodoc.html">foo</a>
    }

    const minifiedHtml = await htmlMinify.minify(html, HTML_MINIFY_OPTIONS);

    fs.writeFileSync(outpath, minifiedHtml, 'utf8');
}

function generateSourceFiles(sourceFiles, encoding = 'utf8') {
    Object.keys(sourceFiles).forEach(function (file) {
        let source;
        // links are keyed to the shortened path in each doclet's `meta.shortpath` property
        const sourceOutFile = helper.getUniqueFilename(
            sourceFiles[file].shortened
        );

        helper.registerLink(sourceFiles[file].shortened, sourceOutFile);

        try {
            source = {
                kind: 'source',
                title: sourceOutFile.replace('.html', ''),
                code: helper.htmlsafe(
                    fs.readFileSync(sourceFiles[file].resolved, encoding)
                ),
            };
        } catch (e) {
            logger.error(
                'Error while generating source file %s: %s',
                file,
                e.message
            );
        }

        generate(
            `Source: ${sourceFiles[file].shortened}`,
            [source],
            sourceOutFile,
            false
        );
    });
}

/**
 * Look for classes or functions with the same name as modules (which indicates that the module
 * exports only that class or function), then attach the classes or functions to the `module`
 * property of the appropriate module doclets. The name of each class or function is also updated
 * for display purposes. This function mutates the original arrays.
 *
 * @private
 * @param {Array.<module:jsdoc/doclet.Doclet>} doclets - The array of classes and functions to
 * check.
 * @param {Array.<module:jsdoc/doclet.Doclet>} modules - The array of module doclets to search.
 */
function attachModuleSymbols(doclets, modules) {
    const symbols = {};

    // build a lookup table
    doclets.forEach((symbol) => {
        symbols[symbol.longname] = symbols[symbol.longname] || [];
        symbols[symbol.longname].push(symbol);
    });

    modules.forEach((module) => {
        if (symbols[module.longname]) {
            module.modules = symbols[module.longname]
                // Only show symbols that have a description. Make an exception for classes, because
                // we want to show the constructor-signature heading no matter what.
                .filter(
                    ({ description, kind }) => description || kind === 'class'
                )
                .map((symbol) => {
                    symbol = _.cloneDeep(symbol);

                    if (symbol.kind === 'class' || symbol.kind === 'function') {
                        symbol.name = `${symbol.name.replace(
                            'module:',
                            '(require("'
                        )}"))`;
                    }

                    return symbol;
                });
        }
    });
}

function buildSidebarMembers({
    items,
    itemHeading,
    itemsSeen,
    linktoFn,
    sectionName,
}) {
    const navProps = {
        name: itemHeading,
        items: [],
        id: `sidebar-${itemHeading.toLowerCase()}`,
    };

    if (items.length) {
        items.forEach(function (item) {
            const currentItem = {
                name: item.name,
                anchor: item.longname
                    ? linktoFn(item.longname, item.name)
                    : linktoFn('', item.name),
                children: [],
            };

            const methods =
                sectionName === SECTION_TYPE.Tutorials ||
                sectionName === SECTION_TYPE.Global
                    ? []
                    : find({
                          kind: 'function',
                          memberof: item.longname,
                          inherited: {
                              '!is': Boolean(themeOpts.exclude_inherited),
                          },
                      });

            if (!hasOwnProp.call(itemsSeen, item.longname)) {
                currentItem.anchor = linktoFn(
                    item.longname,
                    item.name.replace(/^module:/, '')
                );

                if (methods.length) {
                    methods.forEach(function (method) {
                        const itemChild = {
                            name: method.longName,
                            link: linktoFn(method.longname, method.name),
                        };

                        currentItem.children.push(itemChild);
                    });
                }
                itemsSeen[item.longname] = true;
            }

            navProps.items.push(currentItem);
        });
    }

    return navProps;
}

function buildSearchListForData() {
    data().each((item) => {
        if (item.kind !== 'package' && !item.inherited) {
            searchList.push({
                title: item.longname,
                link: linkto(item.longname, item.name),
                description: item.description,
            });
        }
    });
}

function linktoTutorial(longName, name) {
    return tutoriallink(name);
}

function linktoExternal(longName, name) {
    return linkto(longName, name.replace(/(^"|"$)/g, ''));
}

/**
 * This function is added by clean-jsdoc-theme devs
 * This function is added by clean-jsdoc-theme devs
 * This function is added by clean-jsdoc-theme devs
 *
 */
function buildNavbar() {
    return {
        menu: themeOpts.menu || undefined,
        search: hasSearch,
    };
}

/**
 * This function is added by clean-jsdoc-theme devs
 * This function is added by clean-jsdoc-theme devs
 * This function is added by clean-jsdoc-theme devs
 *
 * @param {object} members The members that will be used to create the sidebar.
 * @param {array<object>} members.classes
 * @param {array<object>} members.externals
 * @param {array<object>} members.globals
 * @param {array<object>} members.mixins
 * @param {array<object>} members.modules
 * @param {array<object>} members.namespaces
 * @param {array<object>} members.tutorials
 * @param {array<object>} members.events
 * @param {array<object>} members.interfaces
 * @return {string} The HTML for the navigation sidebar.
 */
function buildSidebar(members) {
    const title = themeOpts.title;

    const isHTML = RegExp.prototype.test.bind(/(<([^>]+)>)/i);

    const nav = {
        sections: [],
    };

    if (!isHTML(title)) {
        nav.title = {
            title,
            isHTML: false,
        };
    } else {
        nav.title = {
            title,
            isHTML: true,
        };
    }

    const seen = {};
    const seenTutorials = {};
    const seenGlobal = {};

    const sectionsOrder = themeOpts.sections || defaultSections;

    const sections = {
        [SECTION_TYPE.Modules]: buildSidebarMembers({
            itemHeading: 'Modules',
            items: members.modules,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Modules,
        }),

        [SECTION_TYPE.Classes]: buildSidebarMembers({
            itemHeading: 'Classes',
            items: members.classes,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Classes,
        }),

        [SECTION_TYPE.Externals]: buildSidebarMembers({
            itemHeading: 'Externals',
            items: members.externals,
            itemsSeen: seen,
            linktoFn: linktoExternal,
            sectionName: SECTION_TYPE.Externals,
        }),

        [SECTION_TYPE.Events]: buildSidebarMembers({
            itemHeading: 'Events',
            items: members.events,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Events,
        }),

        [SECTION_TYPE.Namespaces]: buildSidebarMembers({
            itemHeading: 'Namespaces',
            items: members.namespaces,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Namespaces,
        }),

        [SECTION_TYPE.Mixins]: buildSidebarMembers({
            itemHeading: 'Mixins',
            items: members.mixins,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Mixins,
        }),

        [SECTION_TYPE.Tutorials]: buildSidebarMembers({
            itemHeading: 'Tutorials',
            items: members.tutorials,
            itemsSeen: seenTutorials,
            linktoFn: linktoTutorial,
            sectionName: SECTION_TYPE.Tutorials,
        }),

        [SECTION_TYPE.Interfaces]: buildSidebarMembers({
            itemHeading: 'Interfaces',
            items: members.interfaces,
            itemsSeen: seen,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Interfaces,
        }),

        [SECTION_TYPE.Global]: buildSidebarMembers({
            itemHeading: 'Global',
            items: members.globals,
            itemsSeen: seenGlobal,
            linktoFn: linkto,
            sectionName: SECTION_TYPE.Global,
        }),
    };

    sectionsOrder.forEach((section) => {
        if (SECTION_TYPE[section] !== undefined) {
            nav.sections.push(sections[section]);
        } else {
            const errorMsg = `While building nav. Section name: ${section} is not recognized.
            Accepted sections are: ${defaultSections.join(', ')}
            `;

            throw new Error(errorMsg);
        }
    });

    return nav;
}

/**
    @param {TAFFY} taffyData See <http://taffydb.com/>.
    @param {object} opts
    @param {Tutorial} tutorials
 */
exports.publish = async function (taffyData, opts, tutorials) {
    let classes;
    let conf;
    let externals;
    let files;
    let fromDir;
    let globalUrl;
    let indexUrl;
    let interfaces;
    let members;
    let mixins;
    let modules;
    let namespaces;
    let outputSourceFiles;
    let packageInfo;
    let packages;
    const sourceFilePaths = [];
    let sourceFiles = {};
    let staticFileFilter;
    let staticFilePaths;
    let staticFiles;
    let staticFileScanner;
    let templatePath;

    data = taffyData;

    // MUFIX 
    data().each(function(doclet) {

        // Removed (static) from view
        if (doclet.scope === 'static') {
            doclet.scope = undefined;
        }

        // Added `api.` prefix
        if (doclet.kind === "namespace") {
            doclet.name = "api."+doclet.name
        }
    });

    conf = env.conf.templates || {};
    conf.default = conf.default || {};

    templatePath = path.normalize(opts.template);
    view = new template.Template(path.join(templatePath, 'tmpl'));

    // claim some special filenames in advance, so the All-Powerful Overseer of Filename Uniqueness
    // doesn't try to hand them out later
    indexUrl = helper.getUniqueFilename('index');
    // don't call registerLink() on this one! 'index' is also a valid longname

    globalUrl = helper.getUniqueFilename('global');
    helper.registerLink('global', globalUrl);

    // set up templating
    view.layout = conf.default.layoutFile
        ? path.resolve(conf.default.layoutFile)
        : 'layout.tmpl';

    // set up tutorials for helper
    helper.setTutorials(tutorials);

    data = helper.prune(data);

    // eslint-disable-next-line no-extra-boolean-cast, no-implicit-coercion
    if (themeOpts.sort !== false) {
        data.sort('longname, version, since');
    }

    helper.addEventListeners(data);

    data().each((doclet) => {
        let sourcePath;

        doclet.attribs = '';

        if (doclet.examples) {
            doclet.examples = doclet.examples.map((example) => {
                let caption;
                let code;

                if (
                    example.match(
                        /^\s*<caption>([\s\S]+?)<\/caption>(\s*[\n\r])([\s\S]+)$/i
                    )
                ) {
                    caption = RegExp.$1;
                    code = RegExp.$3;
                }

                return {
                    caption: caption || '',
                    code: code || example,
                };
            });
        }

        if (doclet.see) {
            doclet.see.forEach(function (seeItem, i) {
                doclet.see[i] = hashToLink(doclet, seeItem);
            });
        }

        // build a list of source files
        if (doclet.meta) {
            sourcePath = getPathFromDoclet(doclet);
            sourceFiles[sourcePath] = {
                resolved: sourcePath,
                shortened: null,
            };
            if (sourceFilePaths.indexOf(sourcePath) === -1) {
                sourceFilePaths.push(sourcePath);
            }
        }

        // added by clean-jsdoc-theme-dev.
        // to process yields.
        if (doclet.yields) {
            doclet.yields = getProcessedYield(doclet.yields);
        }
    });

    // update outdir if necessary, then create outdir
    packageInfo = (find({ kind: 'package' }) || [])[0];
    if (packageInfo && packageInfo.name) {
        outdir = path.join(outdir, packageInfo.name, packageInfo.version || '');
    }
    mkdirSync(outdir);

    // copy external static folders
    copyStaticFolder(themeOpts, outdir);

    // copy the template's static files to outdir
    fromDir = path.join(templatePath, 'static');
    staticFiles = lsSync(fromDir);

    staticFiles.forEach((fileName) => {
        const toPath = sourceToDestination(fromDir, fileName, outdir);

        mkdirSync(path.dirname(toPath));
        fs.copyFileSync(fileName, toPath);
    });

    // copy user-specified static files to outdir
    if (conf.default.staticFiles) {
        // The canonical property name is `include`. We accept `paths` for backwards compatibility
        // with a bug in JSDoc 3.2.x.
        staticFilePaths =
            conf.default.staticFiles.include ||
            conf.default.staticFiles.paths ||
            [];
        staticFileFilter = new (require('jsdoc/src/filter').Filter)(
            conf.default.staticFiles
        );
        staticFileScanner = new (require('jsdoc/src/scanner').Scanner)();

        staticFilePaths.forEach((filePath) => {
            filePath = path.resolve(env.pwd, filePath);
            const extraStaticFiles = staticFileScanner.scan(
                [filePath],
                10,
                staticFileFilter
            );

            extraStaticFiles.forEach((fileName) => {
                const toPath = sourceToDestination(filePath, fileName, outdir);

                mkdirSync(path.dirname(toPath));
                fs.copyFileSync(fileName, toPath);
            });
        });
    }

    if (sourceFilePaths.length) {
        sourceFiles = shortenPaths(
            sourceFiles,
            path.commonPrefix(sourceFilePaths)
        );
    }

    data().each(function (doclet) {
        let docletPath;
        const url = helper.createLink(doclet);

        helper.registerLink(doclet.longname, url);

        // add a shortened version of the full path
        if (doclet.meta) {
            docletPath = getPathFromDoclet(doclet);
            docletPath = sourceFiles[docletPath].shortened;
            if (docletPath) {
                doclet.meta.shortpath = docletPath;
            }
        }
    });

    data().each(function (doclet) {
        const url = helper.longnameToUrl[doclet.longname];

        if (url.indexOf('#') > -1) {
            doclet.id = helper.longnameToUrl[doclet.longname].split(/#/).pop();
        } else {
            doclet.id = doclet.name;
        }

        if (needsSignature(doclet)) {
            addSignatureParams(doclet);
            addSignatureReturns(doclet);
            addAttribs(doclet);
        }
    });

    // do this after the urls have all been generated
    data().each((doclet) => {
        doclet.ancestors = getAncestorLinks(doclet);

        if (doclet.kind === 'member') {
            addSignatureTypes(doclet);
            addAttribs(doclet);
        }

        if (doclet.kind === 'constant') {
            addSignatureTypes(doclet);
            addAttribs(doclet);
            doclet.kind = 'member';
        }
    });

    members = helper.getMembers(data);
    members.tutorials = tutorials.children;

    // output pretty-printed source files by default
    outputSourceFiles = Boolean(
        conf.default && conf.default.outputSourceFiles !== false
    );

    // add template helpers
    view.find = find;
    view.linkto = linkto;
    view.resolveAuthorLinks = resolveAuthorLinks;
    view.tutoriallink = tutoriallink;
    view.htmlsafe = htmlsafe;
    view.outputSourceFiles = outputSourceFiles;
    view.footer = buildFooter(themeOpts);
    view.displayModuleHeader = moduleHeader(themeOpts);
    view.favicon = getFavicon(themeOpts);
    view.dynamicStyle = createDynamicStyleSheet(themeOpts);
    view.dynamicStyleSrc = returnPathOfStyleSrc(themeOpts);
    view.dynamicScript = createDynamicsScripts(themeOpts);
    view.dynamicScriptSrc = returnPathOfScriptScr(themeOpts);
    view.includeScript = includeScript(themeOpts, outdir);
    view.includeCss = includeCss(themeOpts, outdir);
    view.meta = getMetaTagData(themeOpts);
    view.theme = getTheme(themeOpts);
    // once for all
    view.sidebar = buildSidebar(members);
    view.navbar = buildNavbar(themeOpts);
    view.resizeable = resizeable(themeOpts);
    view.codepen = codepen(themeOpts);
    view.excludeInherited = Boolean(themeOpts.exclude_inherited);
    view.baseURL = getBaseURL(themeOpts);
    view.shouldRemoveScrollbarStyle = Boolean(
        themeOpts.shouldRemoveScrollbarStyle
    );
    attachModuleSymbols(
        find({ longname: { left: 'module:' } }),
        members.modules
    );

    if (themeOpts.prefixModuleToSidebarItems_experimental) {
        view.sidebar.sections.forEach((section, i) => {
            view.sidebar.sections[i].items = section.items.map((item) => {
                item.anchor = prefixModuleToItemAnchor(item);

                return item;
            });
        });
    }

    // generate the pretty-printed source files first so other pages can link to them
    if (outputSourceFiles) {
        generateSourceFiles(sourceFiles, opts.encoding);
    }

    if (members.globals.length) {
        await generate('Global', [{ kind: 'globalobj' }], globalUrl);
    }

    // index page displays information from package.json and lists files
    files = find({ kind: 'file' });
    packages = find({ kind: 'package' });
    // added by clean-jsdoc-theme-devs
    const homepageTitle = themeOpts.homepageTitle || 'Home';
    const includeFilesListInHomepage =
        themeOpts.includeFilesListInHomepage || false;

    await generate(
        homepageTitle,
        packages
            .concat([
                {
                    kind: 'mainpage',
                    readme: opts.readme,
                    longname: opts.mainpagetitle
                        ? opts.mainpagetitle
                        : 'Main Page',
                },
            ])
            .concat(includeFilesListInHomepage ? files : []),
        indexUrl
    );

    // set up the lists that we'll use to generate pages
    classes = taffy(members.classes);
    modules = taffy(members.modules);
    namespaces = taffy(members.namespaces);
    mixins = taffy(members.mixins);
    externals = taffy(members.externals);
    interfaces = taffy(members.interfaces);

    Object.keys(helper.longnameToUrl).forEach(async function (longname) {
        const myClasses = helper.find(classes, { longname: longname });
        const myExternals = helper.find(externals, { longname: longname });
        const myInterfaces = helper.find(interfaces, { longname: longname });
        const myMixins = helper.find(mixins, { longname: longname });
        const myModules = helper.find(modules, { longname: longname });
        const myNamespaces = helper.find(namespaces, { longname: longname });

        if (myModules.length) {
            await generate(
                `Module: ${myModules[0].name}`,
                myModules,
                helper.longnameToUrl[longname]
            );
        }

        if (myClasses.length) {
            await generate(
                `Class: ${myClasses[0].name}`,
                myClasses,
                helper.longnameToUrl[longname]
            );
        }

        if (myNamespaces.length) {
            await generate(
                `Namespace: ${myNamespaces[0].name}`,
                myNamespaces,
                helper.longnameToUrl[longname]
            );
        }

        if (myMixins.length) {
            await generate(
                `Mixin: ${myMixins[0].name}`,
                myMixins,
                helper.longnameToUrl[longname]
            );
        }

        if (myExternals.length) {
            await generate(
                `External: ${myExternals[0].name}`,
                myExternals,
                helper.longnameToUrl[longname]
            );
        }

        if (myInterfaces.length) {
            await generate(
                `Interface: ${myInterfaces[0].name}`,
                myInterfaces,
                helper.longnameToUrl[longname]
            );
        }
    });

    // TODO: move the tutorial functions to templateHelper.js
    async function generateTutorial(title, tutorial, filename) {
        const tutorialData = {
            title: title,
            header: tutorial.title,
            content: tutorial.parse(),
            children: tutorial.children,
            filename,
        };

        const tutorialPath = path.join(outdir, filename);
        let html = view.render('tutorial.tmpl', tutorialData);

        // yes, you can use {@link} in tutorials too!
        html = helper.resolveLinks(html); // turn {@link foo} into <a href="foodoc.html">foo</a>

        const minifiedHTML = await htmlMinify.minify(html, HTML_MINIFY_OPTIONS);

        fs.writeFileSync(tutorialPath, minifiedHTML, 'utf8');

        // added by clean-jsdoc-theme-devs
        // adding support for tutorial
        if (!hasSearch) return;

        try {
            const baseName = path.basename(tutorialPath);
            let body = /<body.*?>([\s\S]*)<\/body>/.exec(tutorialData.content);
            let description = '';

            if (!Array.isArray(body)) {
                body = /<article.*?>([\s\S]*)<\/article>/.exec(
                    tutorialData.content
                );
            }

            if (Array.isArray(body) && typeof body[1] === 'string') {
                description = body[1]
                    // Replacing all html tags
                    .replace(/(<([^>]+)>)/g, '')
                    // Replacing all kind of line breaks
                    .replace(/(\r\n|\n|\r)/gm, ' ')
                    // Replacing all multi spaces with single space
                    .replace(/\s+/gm, ' ')
                    // Taking only first 100 characters
                    .substring(0, 100);
            }

            if (typeof baseName === 'string' && baseName) {
                searchList.push({
                    title: tutorialData.header,
                    link: `<a href="${baseName}">${baseName}</a>`,
                    description,
                });
            }
        } catch (error) {
            console.error(
                'There was some error while creating search array for tutorial.'
            );
            console.error(error);
        }
    }

    // tutorials can have only one parent so there is no risk for loops
    function saveChildren({ children }) {
        children.forEach(function (child) {
            generateTutorial(
                `Tutorial: ${child.title}`,
                child,
                helper.tutorialToUrl(child.name)
            );
            saveChildren(child);
        });
    }

    saveChildren(tutorials);

    // added by clean-jsdoc-theme-devs
    // output search file if search
    if (hasSearch) {
        buildSearchListForData();
        mkdirSync(path.join(outdir, 'data'));
        fs.writeFileSync(
            path.join(outdir, 'data', 'search.json'),
            JSON.stringify({
                list: searchList,
            })
        );
    }
};
