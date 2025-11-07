const SECTION_TYPE = {
    Classes: 'Classes',
    Modules: 'Modules',
    Externals: 'Externals',
    Events: 'Events',
    Namespaces: 'Namespaces',
    Mixins: 'Mixins',
    Tutorials: 'Tutorials',
    Interfaces: 'Interfaces',
    Global: 'Global',
};

const defaultSections = [
    SECTION_TYPE.Modules,
    SECTION_TYPE.Classes,
    SECTION_TYPE.Externals,
    SECTION_TYPE.Events,
    SECTION_TYPE.Namespaces,
    SECTION_TYPE.Mixins,
    SECTION_TYPE.Tutorials,
    SECTION_TYPE.Interfaces,
    SECTION_TYPE.Global,
];

const HTML_MINIFY_OPTIONS = {
    collapseWhitespace: true,
    removeComments: true,
    html5: true,
    minifyJS: true,
    minifyCSS: true,
};

module.exports = {
    HTML_MINIFY_OPTIONS,
    SECTION_TYPE,
    defaultSections
}