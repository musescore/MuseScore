# Custom Behaviour

While `KDDockWidget`'s defaults work for most users, it contains countless settings that can be adjusted.<br>
The main place to find tunable behaviour is in `Config.h`.<br>Go over all method's documentation and feel free to use anything not marked
as `@internal`.

The `Config::Flag` is particularly interesting. Example:
```cpp
#include <kddockwidgets/Config.h>

KDDockWidgets::Config::self().setFlags(KDDockWidgets::Config::Flag_AutoHideSupport);
```

Other useful places to modify:
- `KDDockWidgets::DockWidgetOptions` (passed via DockWidget CTOR)
- `KDDockWidgets::MainWindowOptions` (passed via MainWindow CTOR)
- `KDDockWidgets::RestoreOptions` (passed via LayoutSaver CTOR)
- Possibly other enums in `KDDockWidget.h`
- All the API in `core/DockWidget.h`, `qtwidgets/views/DockWidget.h`, `qtquick/views/DockWidget.h`
- Private API
