## Change Log

### In version 4.2.0

1. Add an option to include files list in homepage. Previously we are including it by default. From now the developer has to explicitly ask for it. For more [see](https:/github.com/ankitskvmdam/clean-jsdoc-theme#add-files-list-in-homepage)

### In version 4.0.0

This is the most performant version of `clean-jsdoc-theme`. Check
the [report on lighthouse](https://googlechrome.github.io/lighthouse/viewer/?psiurl=https%3A%2F%2Fankdev.me%2Fclean-jsdoc-theme%2Fv4%2Findex.html&strategy=desktop&category=performance&category=accessibility&category=best-practices&category=seo&category=pwa&utm_source=lh-chrome-ext)
.

### New

1. New dark and light theme. Now you can toggle between dark and light themes.
2. There is an option to change the default font size.
3. New search feature which doesn't cost KBs. [#search](https://github.com/ankitskvmdam/clean-jsdoc-theme#search)
4. Minify all generated files.
5. Added Table of content.

### Removed

1. Search options
2. `theme` options. Now it is `default_theme`
3. `overlay_scrollbar`. We found that you can include overlay scrollbar
   using [`add_script_path`](https://github.com/ankitskvmdam/clean-jsdoc-theme#add-script-paths)
   and [`include_js`](https://github.com/ankitskvmdam/clean-jsdoc-theme#add-custom-script-files) options.
4. `resizeable`. Now there is no option to resize sidebar.

### In version 3.3.2

### Feature

1. Add an option to excludes inherited symbols. (#96)[https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/96]

### In version 3.3.0

### Feature

1. Now there is an option to order the navbar section.

### Bug Fixes

1. Fix: Line height of empty line is render as 0 on Mozilla firefox.
2. Fix: Unnecessary errors with empty examples (#92)[https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/92]
3. Fix: Disabled source still produces 'details' block (
   #90)[https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/90]
4. Fix: Tutorials that share their name with a class will have that classes methods dropdown (
   #80)[https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/89]

### Options Removed

1. `menuLocation`: `menuLocation` theme_opts is removed. Now `sections` can be used to order the entire navbar section

### In version 3.2.8

### Bug Fixes

1. Fix: Search condition statement. (#81)[https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/81]
2. Fix: missing styles/heading.css. (#80)[https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/80]

### Feature

1. Feat: add menuLocation option to adjust the place to append extra links. (
   #78)[https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/78]

### In version 3.2.7

### Bug Fix

1. Fix: quotes issue for codepen options.
2. Fix: source is not printing if source is the only key. (
   #71)[https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/71]
3. Fix (css): font size of return type. (#70)[https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/70]

### In version 3.2.6

### Feature

1. Add an option to open code in codepen.

### Bug Fix

1. Fix the css of example
   caption. [View changes.](https://github.com/ankitskvmdam/clean-jsdoc-theme/commit/1cba9400a6d9ae2991eb5b32282e7572510656c6)
2. Fix the overflow css of code
   section. [View changes.](https://github.com/ankitskvmdam/clean-jsdoc-theme/commit/1cba9400a6d9ae2991eb5b32282e7572510656c6)
3. Fix the css of code block in dark theme.

### In version 3.2.4

### Bug Fix

1. When the codebase is large then search didn't
   work. [#68](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/68)

### In version 3.2.3

### Bug Fix

1. On mobile screen navbar, main content and footer are not as expected after applying resize.

### In version 3.2.1

### Feature

1. Now there is an option to make navbar resizeable.

### In version 3.2.0

### Bug Fix

1. When passing HTML as title, an NPM Error occurs. In this release that error is fixed.

### Others

1. Remove filter support.

### In version 3.1.2

### New

1. Change ham animation.

### Others

1. Update readme file.

### In version 3.1.0

### Bug fixes

1. fix unclosed <div> in method.tmpl [#63](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/63)

### New

1. Add brand-new light and dark theme.
2. Add collapsible main section in navbar.

### Refactor

1. Refactor css and js to improve performance.

### In version 3.0.10

### Bug Fixes

1. Previously, we are enforcing below templates:

```js
// jsdoc config file
// ...other options.
  templates: {
    cleverLinks: true,
    monospaceLinks: false,
    default: {
        outputSourceFiles: false,
    }
  },
```

In this release we removed this template rule.

### In version 3.0.9

#### Bug Fixes

1. On Mobile screen the ham icon is not visible. That is solved in this release.
2. Fix: Tooltip copied text color.
3. Fix: When copied code, `JAVASCRIPT\nCopied!` also got attached with it.

### In version 3.0.8

#### Feature

1. Add support for `@see` documentation link. [#59](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/59)

## In version 3.0.7

### Feature

1. Now left panel classes and modules can be
   collapse. [Feature Request #57](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/57)

## In version 3.0.6

### Bug Fix

1. Fix body overflow.
2. Fix css for table.

## In version 3.0.4

### New

1. Update `search`. Now instead of boolean it accepts an object. This object is used to configure
   search. [fuse.js options](https://fusejs.io/api/options.html)

## In version 3.0.2

### Bug Fix

1. Simplify the meta/script/link
   declaration [Pull Request: [#56](https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/56)] [Thanks to [GMartigny](https://github.com/GMartigny)]

## In version 3.0.0

### New

1. Add an option to include css files.
2. Add an option to add js.
3. Add an option to include static folder.
4. OverlayScrollbar are now supported by default. If you don't want to use it pass an option to disable it.

### Breaking changes

1. `add_script_path` previously you have to pass an array of string, but now you have to pass an array of object where
   keys are the attributes of the script tag
2. `add_style_path` previously you have to pass an array of string, but now you have to pass an array of object where
   keys are the attributes of the link tag
3. `meta` previously you have to pass an array of string, but now you have to pass an array of object where keys are the
   attributes of the meta tag

## In version 2.2.15

### New

1. Scrollable code area

## In version 2.2.14

### Bug Fix

1. Malformed HTML when parsing 'default' JSDoc
   tags [issue: [#48](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/48)]

## In version 2.2.13

### New

1. Make the # before members and methods a clickable
   anchor. [pull request: [#44](https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/44)] [Thanks to [GMartigny](https://github.com/GMartigny)]

### Other

1. Change jsdoc into a
   peerDependency [pull request: [#45](https://github.com/ankitskvmdam/clean-jsdoc-theme/pull/45)][Thanks to [GMartigny](https://github.com/GMartigny)]

## In version 2.2.12

### New

1. Add dark theme.

### Bug fix

1. Fix typescript-eslint camelCase rule
   issue [issue: [#37](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/37)]
2. Fix ordered list style [issue: [#40](https://github.com/ankitskvmdam/clean-jsdoc-theme/issues/40)]
3. Fix code overflow issue.
