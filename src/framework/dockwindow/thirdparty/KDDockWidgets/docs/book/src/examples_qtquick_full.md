# full example

This example tries to show most of `KDDockWidget`'s functionality in a single example.<br>
It is not meant to be copy-pasted entirely. Instead, copy only the functionalities you need.

Note: This example is not as complete as the [QtWidgets counter-part](examples_qtwidgets_full.md), it's missing many options
that are supported by our QtQuick implementation but just not showed in the example.

View the source at [GitHub](https://github.com/KDAB/KDDockWidgets/blob/2.0/examples/qtquick/dockwidgets/).

Run `./bin/qtquick_dockwidgets --help` to see the available options:

```bash
KDDockWidgets example application

Options:
  -h, --help                Displays help on commandline options.
  --help-all                Displays help including Qt specific options.
  -t                        Hide titlebars when tabs are visible
  -z                        Show tabs even if there's only one
  -b                        Floating dockWidgets have maximize/restore buttons
                            instead of float/dock button
  -k                        Floating dockWidgets have a minimize button.
                            Implies not being an utility window (~Qt::Tool)
  --no-qttool               (internal) Don't use Qt::Tool
  --no-parent-for-floating  (internal) FloatingWindows won't have a parent
  --no-drop-indicators      (internal) Don't use any drop indicators
```
