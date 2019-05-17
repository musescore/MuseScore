Welcome to the MuseScore plugins documentation!

This page gives a brief overview of the topics related to plugins development.

## Plugin development overview

Here are some points concerning MuseScore plugin development:

#### Plugins are QML components.
MuseScore plugins are coded in
[QML](https://doc.qt.io/qt-5/qmlapplications.html#what-is-qml).
Each plugin is a QML component using which implements some logic inside its
instance of \ref Ms::PluginAPI::PluginAPI "MuseScore" class and are capable to
interact both with MuseScore API and with Qt itself (so MuseScore plugins can
create their own windows, dialogs). Most of plugins are contained within a
single `.qml` file though they may contain more items like resources and
translation files.

#### Debugging plugins
MuseScore provides a simple plugin code editor which allows to develop and
instantly test the developed plugin. This editor can be launched with Plugins →
Plugin Creator menu item or by pressing
<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd>.

Clicking on a New button will create a sample plugin, you can also run your
plugin with Run button and see some debugging output in the console window below
the editing area. In order to print some debugging message to that console use
`console.log()`:
```
var e = newElement(Element.STAFF_TEXT);
console.log("the created element:", e, e.name);
```

Alternatively, you can edit your plugin's code with your favorite text editor
and test it by launching it from MuseScore as long as the plugin is properly
installed. See [this Handbook
page](https://musescore.org/en/handbook/plugins#installation)
for the reference on plugins installation.

## Hello World plugin

Here is an example of a simple plugin which prints "Hello World" to the
debugging console:
```
import MuseScore 3.0

MuseScore {
    menuPath: "Plugins.pluginName"
    description: "Description goes here"
    version: "1.0"
    onRun: {
        console.log("Hello World!");
        Qt.quit();
    }
}
```

Here is what happens here.

- `import MuseScore 3.0` is necessary to use MuseScore API in QML code.
- The `MuseScore { ... }` statement declares an object of `MuseScore` type (\ref
Ms::PluginAPI::PluginAPI is exposed to QML as `MuseScore`). This should be
the root object of any QML plugin for MuseScore.
- Statements like `menuPath: "Plugins.pluginName"` assign properties to that
object, see \ref
Ms::PluginAPI::PluginAPI "the class reference" for the meaning of those
properties. Apart from the properties listed in that page, you can assign any
other properties and/or declare other QML objects too.
- `onRun()` is the function that is invoked when the plugin is executed (via a
menu entry or via a shortcut). This is the entry point of your plugin.
- [`Qt.quit()`](https://doc.qt.io/qt-5/qml-qtqml-qt.html#quit-method) statement
requests termination of the plugin's execution.

MuseScore is shipped with a set of simple plugins which can also be used as
a reference while developing your own plugins.

## Porting MuseScore 2 plugins
This documentation corresponds to the plugins API for MuseScore 3.X version.
To see the information on its difference from MuseScore 2 plugins API as well
as some instructions on adapting MuseScore 2 plugins code to work with
MuseScore 3, please refer to the \ref plugin2to3 page.
