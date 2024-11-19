# full example

This example tries to show most of `KDDockWidget`'s functionality in a single example.<br>
It is not meant to be copy-pasted entirely. Instead, copy only the functionalities you need.

View the source in [GitHub](https://github.com/KDAB/KDDockWidgets/blob/2.0/examples/dockwidgets/main.cpp).

If you just want to get started quickly, see the [minimal](examples_qtwidgets_minimal.md) instead.

Run `./bin/qtwidgets_dockwidgets --help` to see the available options:

```bash
Usage: ./bin/qtwidgets_dockwidgets [options] savedlayout
KDDockWidgets example application

Options:
  -h, --help                                   Displays help on commandline
                                               options.
  --help-all                                   Displays help including Qt
                                               specific options.
  -p                                           Shows how to style framework
                                               internals via ViewFactory
  -r                                           Support re-ordering tabs with
                                               mouse
  -t                                           Hide titlebars when tabs are
                                               visible
  -q                                           Don't hide title bars if
                                               floating, even if
                                               Flag_HideTitleBarWhenTabsVisible
                                               is specified.
  -z                                           Show tabs even if there's only
                                               one
  -l                                           Use lazy resize
  -m                                           Shows two multiple main windows
  -i                                           Only usable with -m. Make the
                                               two main windows incompatible
                                               with each other. (Illustrates
                                               (MainWindowBase::setAffinityName)
  -c                                           Tabs have a close button
  -n                                           DockWidget #0 will be
                                               non-closable
  -s                                           Don't restore main window
                                               geometry, restore dock widgets in
                                               relative sizes
  -x                                           Double clicking a title bar will
                                               maximize a floating window
  -d                                           DockWidget #9 will be
                                               non-dockable
  -b                                           Floating dockWidgets have
                                               maximize/restore buttons instead
                                               of float/dock button
  -k                                           Floating dockWidgets have a
                                               minimize button. Implies not
                                               being an utility window
                                               (~Qt::Tool)
  -y                                           Use segmented indicators instead
                                               of classical
  -u                                           FloatingWindows will be normal
                                               windows instead of utility
                                               windows
  -o                                           FloatingWindows will have
                                               Qt::WindowStaysOnTopHint. Implies
                                               not being an utility window (try
                                               it with -u too)
  -g                                           Make dock #8 have a max-size of
                                               200x200.
  -w                                           Enables auto-hide/minimization
                                               to side-bar support
  --close-only-current-tab                     The title bar's close button
                                               will only close the current tab
                                               instead of all. Illustrates using
                                               Config::Flag_CloseOnlyCurrentTab
  --dont-close-widget-before-restore           DockWidgets 6, 7 and 8 won't be
                                               closed before a restore.
                                               Illustrates
                                               LayoutSaverOption::Skip
  --block-close-event                          DockWidget #0 will block close
                                               events
  --programmatic-drag                          Shows how to start a drag
                                               programmatically (advanced usage)
  --show-buttons-in-tabbar-if-titlebar-hidden  If we're not using title bars
                                               we'll still show the close and
                                               float button in the tab bar
  --central-widget                             The main window will have a
                                               non-detachable central widget
  --allow-switch-tabs-via-menu                 Allow switching tabs via context
                                               menu in tabs area
  --hide-certain-docking-indicators            Illustrates usage of
                                               Config::setDropIndicatorAllowedFu
                                               nc()
  --ctrl-toggles-drop-indicators               Ctrl key toggles drop indicators
  -f                                           Persistent central group
  --no-qttool                                  (internal) Don't use Qt::Tool
  --no-parent-for-floating                     (internal) FloatingWindows won't
                                               have a parent
  --native-title-bar                           (internal) FloatingWindows a
                                               native title bar
  --no-drop-indicators                         (internal) Don't use any drop
                                               indicators

Arguments:
  savedlayout                                  loads the specified json file at
                                               startup
```
