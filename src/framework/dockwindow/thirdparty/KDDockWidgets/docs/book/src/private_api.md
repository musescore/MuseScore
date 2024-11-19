# Public and Private API

`KDDockWidget` guarantees source-compatibility for the `public API`, only.<br>
Users seeking more advanced usage are welcome to use the `private API`, which can in theory break source-compat between releases.
In practice however, the `private API` won't be changing much. Expect the `public API` to have much better documentation though.

## Public API

The API is mostly `MainWindow` and `DockWidget` and a few other classes. Here's a list of headers you can
`#include`:
```
<kddockwidgets/LayoutSaver.h>
<kddockwidgets/Config.h>
<kddockwidgets/KDDockWidgets.h>

<kddockwidgets/qtwidgets/views/DockWidget.h>
<kddockwidgets/qtwidgets/views/MainWindow.h>
<kddockwidgets/qtwidgets/views/MainWindowMDI.h>

<kddockwidgets/qtquick/views/DockWidget.h>
<kddockwidgets/qtquick/views/MainWindow.h>
<kddockwidgets/qtquick/views/MainWindowMDI.h>
```

## Private API

Everything else.
