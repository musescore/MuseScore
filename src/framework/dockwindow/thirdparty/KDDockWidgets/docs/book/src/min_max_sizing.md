# min/max sizing

## QtWidgets

KDDW will honour min/max constraints set on the guest widget.<br>
You can run the example `qtwidgets_dockwidgets -g` to see the max-size in action.<br>
Setting the constraints directly on `DockWidget` is not advised.

## QtQuick

Minimum sizes are supported but not maximum sizes.<br>
There's no public API to set the minimum sizes, but you can set a special property called `kddockwidgets_min_size`
on your guest item.<br>
For an example, run the `qtquick_dockwidgets` sample executable, then go to "File" menu, and chose `New widget with min-size`.

Note that while KDDW floating windows and docked widgets will honour the min size, the main window itself won't, as that's an item completely controlled by the user. Maybe we could expose the main window's KDDW layout min/max size, then users could use that to calculate the main window's min/max.
