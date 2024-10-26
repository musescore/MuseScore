# Custom Styling

`KDDockWidgets` allows you to derive from several internal widgets (now called `views`, more QWidget agnostic term) and provide your own
`paintEvent` and sizings (or .qml files for QtQuick).

You can derive any of these components:

- `MainWindow`
- `DockWidget`
- `FloatingWindow`, the window containing one or more dock widgets
- `TitleBar`
- `Group`, a group of tabbed dock widgets with a titlebar
- `TabBar`, similar concept to QTabBar
- `Stack`, similar concept to QTabWidget.
- `Separator`, allows resizing dock widgets inside a layout
- `RubberBand`, indicates the drop location when dragging
- `SideBar`, the sidebar when using the "auto hide feature"
- `ClassicDropIndicatorOverlay`, the drop indicators
- `SegmentedDropIndicatorOverlay`, the drop indicators in segmented mode

After deriving one or more of the above, create a custom `ViewFactory.h` which returns your derived instances.
Then call `KDDockWidgets::Config::self().setViewFactory(new MyCustomWidgetFactory());`.

See `examples/dockwidgets/MyViewFactory.h` for QtWidgets, or `examples/qtquick/customtitlebar/` for QtQuick.

## CSS

`Qt StyleSheets` are not, and will not, be supported. See the comments in
`examples/dockwidgets/MyTitleBar_CSS.h` for why. You can however use some minimal
CSS, as shown in that example, just don't report bugs about it.
