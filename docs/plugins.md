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
create their own windows, dialogs). Most of the plugins are contained within a
single `.qml` file though they may contain more items like resources and
translation files.

#### Debugging plugins
MuseScore provides a simple plugin code editor which allows developing and
instantly testing the developed plugin. This editor can be launched with Plugins →
Plugin Creator menu item or by pressing
<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd>.

Clicking on a New button will create a sample plugin, you can also run your
plugin with the Run button and see some debugging output in the console window below
the editing area. In order to print some debugging messages to that console use
`console.log()`:
```
var e = newElement(Element.STAFF_TEXT);
console.log("the created element:", e, e.name);
```

Alternatively, you can edit your plugin's code with your favourite text editor
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

## Documentation from Earlier Versions
Although it is not up to date, documentation for creating plugins for earlier versions is available and may be useful.
https://musescore.org/en/handbook/developers-handbook/plugin-development


## Plugins Installed By Default
MuseScore is shipped with a set of simple plugins which can also be used as
a reference while developing your own plugins.

### ABC Import
This plugin imports ABC text from a file or the clipboard. Internet connection is required, because it uses an external web-service for the conversion, which uses abc2xml and gets send the ABC data, returns MusicXML and imports that into MuseScore.

### Color Notes
This demo plugin colors notes in the selected range (or the entire score), depending on their pitch. It colors the note head of all notes in all staves and voices according to the Boomwhackers convention. Each pitch has a different color. C and C♯ have different color. C♯ and D♭ have the same color.
To color all the notes in black, just run that plugin again (on the same selection). You could also use the 'Remove Notes Color' plugin for this.

### Create Score
This demo plugin creates a new score. It creates a new piano score with 4 quarters C D E F. It's a good start to learn how to make a new score and add notes from a plugin.

### helloQml
This demo plugin shows some basic tasks.

### Note Names
This plugin names notes in the selected range or the entire score. It displays the names of the notes (as a staff text) as per MuseScore's language settings, for voices 1 and 3 above the staff, for voices 2 and 4 below the staff, and for chords in a comma-separated list, starting with the top note.

### Note Names -Interactive
This plugin names notes as per your language setting.

### Panel
This demo plugin creates a GUI panel.

### random
Creates a random score.

### random2
Creates a random score too.

### run
This demo plugin runs an external command. Probably this will only work on Linux.

### scorelist
This test plugin iterates through the score list.

### View
Demo plugin to demonstrate the use of a ScoreView

### Walk
This test plugin walks through all elements in a score


## Porting MuseScore 2 plugins
This documentation corresponds to the plugins API for MuseScore 3.X version.
To see the information on its difference from MuseScore 2 plugins API as well
as some instructions on adapting MuseScore 2 plugins code to work with
MuseScore 3, please refer to the \ref plugin2to3 page.

## Internationalization
For the questions related to making a plugin translatable to different languages
please refer to https://github.com/musescore/MuseScore/blob/master/doc/i18n.md.
