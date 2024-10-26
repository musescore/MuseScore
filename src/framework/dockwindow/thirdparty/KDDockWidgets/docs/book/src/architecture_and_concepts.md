# Architecture and Concepts

`KDDockWidgets` is divided into `Core` and one `frontend` for each supported `GUI toolkit`.<br>
The `Core` is pure `C++` with no dependency on `Qt`. Business logic and state often lives here, and classes are
namespaced with `KDDockWidgets::Core::`.<br>
We have two complete frontends, `qtwidgets` and `qtquick` and a third one called `flutter`, which is lacking some futures still. Their code is namespaced with `KDDockWidgets::QtWidgets`, `KDDockWidgets::QtQuick` and `KDDockWidgets::Flutter` respectively.

## Views and Controllers

`View` is a fancy word that meant `QWidget`. Since we introduced support for non-QtWidget toolkits we needed to drop the QWidget nomenculature. For QtQuick, views are `QQuickItem`s, and for Flutter, they are `StatefullWidget`.

While views are something graphical, which depends on the rendering technology you're using, `Controllers` on the other hand are gui-agnostic, they live in `Core`, and are reused by all frontends.

Non-exhaustive list of controllers and views:
- TitleBar
- TabBar
- Stack
- Group
- DockWidget
- MainWindow
- FloatingWindow
- DropArea

For each of the above there's a View and a Controller. For example, for TitleBar, there's `Core::TitleBar`, implemented in `src/core/TitleBar.cpp`, which is rendered by `QtWidgets::TitleBar` (or `QtQuick::TitleBar` or `Flutter::TitleBar`), implemented in `/src/qtwidgets/views/TitleBar.cpp` and so on.

Porting to another GUI toolkit involves reimplementing all views.<br>
Here's a brief description of each.

### Guest View

This is the view that the user (library user) wants to dock. It has some custom content that
is only relevant for the application. From KDDW's perspective we don't care what's inside,
we'll just help dock it.

### DockWidget

The DockWidget is a visual container for the Guest, in other words, its visual parent.
Visually, DockWidget and Guest might be indistinguishable, except for some margin added by
DockWidget. One reason to have this extra indirection is that it allows us to have a common API
to deal with the Guest. Since guest is provided by the application developer it doesn't have any interface.

### TitleBar

A TitleBar is the area that has the dock widget title and the float and close buttons.
Usually KDDW won't use native OS title bars but draw its own.

### TabBar

DockWidgets can be grouped together in tabs. A tab bar is just a group of tabs.
In Qt this is something like QTabBar.

### Stack

A stack is closely related to TabBar. A stack is a group of dock widgets where only one is visible
at a time, the visible one is controlled by the current tab.  In Qt this would be QTabWidget.

### Group

The Group is a container that ties the previous concepts all together.
It's composed of 1 or more tabbed DockWidgets, a TitleBar, a Stack and a TabBar.

## Layout

Represents a docking layout. Currently 2 are supported, the traditional nested docking with
resizable splitters (this is the default), implemented by MultiSplitter. And a MDI layout, where
the dock widgets can be arbitrary positioned and even overlap inside an area.

The layouts deal in Frame. You add Frame objects to a layout.

## Separator

A visual separator between two widgets, which allows the user to resize dock widgets with mouse.

## FloatingWindow

When a dock widget isn't embedded into a window it's said to be floating. It's its own
top-level native window. This class ties all the previous together. It contains one layout, which
contains multiple groups.

## MainWindow

Not much different from FloatingWindow technically, but users will often add status bar, tool bar
and menu bars to a main window, while FloatingWindow is just an utility window (Qt::Tool).
MainWindow also has support for a SideBar.

## SideBar

A side bar is a place in the MainWindow where you can "minimize" dock widgets.
It's also called the auto-hide future. When you send a dock widget to the sidebar it will close
but show a button in the sidebar, if you press it, it will show the dock widget as an overlay.


## Platform, Screen, Window, ViewFactory
Not everything is a View/Controller. There's a bunch of other abstractions that need to be implemented when creating a new frontend.

While Qt has `QGuiApplication`, `QScreen`, `QWindow` we have similar abstractions but in a more toolkit agnostic way. See all the pure virtual methods in Core::Platform, Core::Screen, Core::Window and View::Factory and implement them.

## Layouting engine

This is like the core of the core, it lives in src/core/layouting. The code there doesn't know anything about docking
or dnd. It implements our layouting, which is a recursively nested layout. Imagine a layout composed nested  QVBoxLayout and QHBoxLayout supporting any depth. min/max/preferred sizes are supported and recursively propagated up.
See `src/core/layouting/examples` for how to use just this layouting engine without any docking. This is pure C++ and doesn't depend on Qt.
