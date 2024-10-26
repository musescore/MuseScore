# Porting from v1 to v2

To make the framework toolkit agnostic to support `QtQuick` and even non-Qt toolkits
such as `Flutter` some API changes were required.
<br>
Each component was split into 2, one part lives in `core/` and is gui agnostic,
 while the other part lives in a frontend specific folder, such as `qtwidgets/` (or `qtquick/`).

For example, TitleBar was split into `KDDockWidgets::Core::TitleBar` and
`KDDockWidgets::QtWidgets::TitleBar`.  The former we call it a `Controller`,
while the latter is the `View`. In the QtWidgets case, a view is a `QWidget`,
while in QtQuick it's a QQuickItem.

The main public API didn't change much, but if you're using private API heavily
to achieve advanced customizations then take note of these renamings:

(Assume `::QtQuick::` namespace instead of `::QtWidgets::` if you're on QtQuick).

- `Frame` was renamed to `Group` as it represents a tab group
- `TabWidget` was renamed to `Stack`
- `KDDockWidgets::DefaultWidgetFactory` was renamed to `KDDockWidgets::QtWidgets::ViewFactory`
- The `*Widget` and `*Quick` classes dropped the suffix and are namespaced instead:
  - `KDDockWidgets::TitleBarWidget` -> `KDDockWidgets::QtWidgets::TitleBar`
  - `KDDockWidgets::TitleBarQuick` -> `KDDockWidgets::QtQuick::TitleBar`
  - Same for DropArea, FloatingWindow, TabBar, Group, Stack, MainWindow, DockWidget
- You can include the views for example with `#include <kddockwidgets/qtwidgets/views/TitleBar.h>`.
  This is only required if you're customizing the default visuals.
- `KDDockWidgets::DefaultWidgetFactory::s_dropIndicatorType` is now `KDDockWidgets::Core::ViewFactory::s_dropIndicatorType`

If some API is missing in the View, try looking in the controller, so, for example,
look in `KDDockWidgets::Core::TitleBar` instead of `KDDockWidgets::QtWidgets::TitleBar`.

## Headers

Views are in `kddockwidgets/FRONTEND/views/`. Replace FRONTEND with `qtwidgets` or `qtquick`.

So for example, if you had:

```c++
#include <kddockwidgets/private/widgets/TitleBarWidget_p.h>
#include <kddockwidgets/private/TabWidget_p.h>
#include <kddockwidgets/private/widgets/TabBarWidget_p.h>
```

You should now have:

```c++
#include <kddockwidgets/qtwidgets/views/TitleBar.h>
#include <kddockwidgets/qtwidgets/Stack.h>
#include <kddockwidgets/qtwidgets/views/TabBar.h>
```
