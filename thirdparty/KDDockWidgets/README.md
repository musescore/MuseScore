KDDockWidgets
=============
`KDDockWidgets` is a Qt dock widget library written by KDAB, suitable for replacing
`QDockWidget` and implementing advanced functionalities missing in Qt.

Although `KDDockWidgets` is ready to be used out of the box, it can also be seen
as a framework to allow building very tailored custom docking systems. It tries
to expose every internal widget and every knob for the app developer to tune.


Motivation
==========
Throughout the years KDAB contributed and funded bug fixes and features to `QDockWidget`.
Sadly, this was very painful. Each bug fix or feature took many days of implementation,
and an equal number of days just to fix dozens of regressions.

`QDockWidget` mixes GUI code with logic with state, making it very hard
to move forward with new features. Furthermore, our customers were getting more
creative with their requests, so it was clear we needed a better docking framework.

You will find more information in these places:

 * [our official home page](https://www.kdab.com/development-resources/qt-tools/kddockwidgets)
 * [online detailed browsable API reference](https://docs.kdab.com/kddockwidgets)
 * [our example programs](examples/)

We also have an [in browser demo](https://demos.kdab.com/wasm/kddockwidgets/dockwidgets.html). Note
however that this demo isn't fully featured, as it's running on Qt for WebAssembly.

Features
========
- Provide advanced docking that QDockWidget doesn't support
  - Native window resize on Windows (allowing for Aero-snap even with custom title bar decorations)
  - Arrow drop indicators for great drop precision
  - Allow for totally different, user provided, drop indicator types
  - Nesting dock widgets in a floating window and docking that group back to main window
  - Docking to any main window, not only to the parent main window
  - Docking to center of main window, or simply removing the concept of "central widget"
  - Main window supporting detachable tabs in center widget
  - Detaching arbitrary tabs from a tab bar into a dock area
  - Supporting more than 1 main window
  - Support for affinities, making some dock widgets only dockable on certain main windows
  - Allow to hide TitleBar and just show tabs. Allows dragging via the tab bar.
  - Exposing inner helper widgets so the user can customize them or provide his own
    - Customize tab widgets
    - Customize title bars
    - Customize window frames
    - Custom widget separators
- Cross-platform (macOS, Linux, Windows, WebAssembly, Wayland, X11/XCB, EGLFS are working)
  See README-Wayland.md and README-WASM.md for platform specific information.
- Layouting engine honouring min/max size constraints and some size policies
- PySide2 bindings
- Clean codebase
  - Not mixing GUI with state with logic with animations
  - Great test coverage, even the GUI and DnD operations are tested. 200 tests currently.
  - Fuzzer for doing random testing and finding bugs
- Lazy separator resize
- Reordering tabs with mouse
- Partial layout save/restore, affecting only a chosen sub-set
- Double click on title bar to maximize
- Double click on separator to distribute equally
- Show close button on tabs
- Allow to make a dock widget non-closable and/or non-dockable
- Optional minimize and maximize button on the title bar
- FloatingWindows can be utility windows or full native

Screen capture
==============
![Screen capture](./screencap.gif?raw=true "The docking system in action")


Roadmap
========
  - QtQuick support

Trying out the examples
=======================
A full demo that showcases most of the features lives in [examples/dockwidgets](examples/dockwidgets).

A simpler example can be found in [examples/minimal](examples/minimal),
which might be more indicated to learn the API, as it's less overwhelming than the full demo.

Open a terminal capable of building Qt5 applications.
Make sure you have cmake, ninja, compiler, Qt, etc in PATH.

Adapt the instructions to suit your cmake generator and operating system.
Build and install the KDDockWidgets framework:

```
$ cmake -G Ninja -DCMAKE_INSTALL_PREFIX=/path/where/to/install ../path/to/kddockwidgets
$ cmake --build .
$ cmake --build . --target install
```

Now build and run the example:

```
$ cd path/to/kddockwidgets/examples/dockwidgets/
$ cmake -G Ninja -DCMAKE_PREFIX_PATH=/path/where/to/install
$ cmake --build .
$ ./kddockwidgets_example
```

The installation directory defaults to `c:\KDAB\KDDockWidgets-<version>` on Windows
and `/usr/local/KDAB/KDDockWidgets-<version>` on non-Windows.

You can change the installation location by passing the option `-DCMAKE_INSTALL_PREFIX=/install/path` to cmake.

Using
=====
From your CMake project, add

```
    find_package(KDDockWidgets CONFIG)
```

and link to the imported target `KDAB::kddockwidgets`.
That's all you need to do (the imported target also brings in the include directories)

You may also need to modify the `CMAKE_MODULE_PATH` environment variable depending
on where you installed KDDockWidgets.


Python Bindings
================
Make sure you have PySide2, shiboken2 and shiboken2-generator installed.
As this time, you cannot get shiboken2-generator because the wheels are not on PyPi.
To use the wheels do this:

```
% pip3 install \
    --index-url=http://download.qt.io/official_releases/QtForPython/ \
    --trusted-host download.qt.io \
    shiboken2 pyside2 shiboken2_generator
```

For more info visit https://doc.qt.io/qtforpython/shiboken2/gettingstarted.html

Once QtForPython is installed you are ready to generate the PySide2 bindings
for KDDockWidgets.

Next pass `-DKDDockWidgets_PYTHON_BINDINGS=ON` to CMake, followed by the
make command.

The bindings will be installed to the passed `-DCMAKE_INSTALL_PREFIX`, which
might require setting the `PYTHONPATH` env variable to point to that path when
running applications.  Alternatively, configure the bindings install location
by passing `-DKDDockWidgets_PYTHON_BINDINGS_INSTALL_PREFIX=/usr/lib/python3.8/site-packages`
to CMake (adjust to the python path on your system).

To run the KDDW python example

```
$ export PYTHONPATH=/kddw/install/path # Only if needed
$ cd python/examples/
$ rcc -g python -o rc_assets.py ../../examples/dockwidgets/resources_example.qrc
$ python3 main.py
```

Versioning
==========

New features go to master while the stable branch only accepts non-intrusive bug fixes.

We'll try to remain source-compatible across versions. API will get a deprecation
notice before being removed in the next version. Note that this source-compatibility
effort is only for the public API. Private API (headers ending in _p.h) might change so you
shouldn't depend on them. Private API is only exposed so more advanced users can
override, for example `paintEvent()`, and not so they can change internal business logic.

We don't promise or test binary compatibility. It's advised that you recompile
your application whenever updating KDDW.


Supported Qt versions and toolchains
=====================================
KDDockWidgets requires Qt >= 5.9 (or >=5.12 if Python bindings are enabled).
The QtQuick support will require Qt >= 5.15.
Qt 6 is supported.

Regarding compilers, whatever toolchain is able to build Qt 5.9 should also be
fine. Note however that MSVC 2013 isn't supported anymore due to compiler crashes.


Licensing
=========
KDDockWidgets is (C) 2018-2021, Klarälvdalens Datakonsult AB, and is licensed according to
the terms of the [GPL 2.0](LICENSES/GPL-2.0-only.txt) or [GPL 3.0](LICENSES/GPL-3.0-only.txt).

Contact KDAB at <info@kdab.com> to inquire about commercial licensing.


Get Involved
============
Please submit your issue reports to our GitHub space at
https://github.com/KDAB/KDDockWidgets


When reporting bugs please make it easy for the maintainer to reproduce it. Use `examples/minimal/` or
`examples/dockwidgets/` for reproducing the problem. If you did modifications to the example in order to
reproduce then please attach the *patch* and not a picture of your changes. You can get a patch by doing
`git diff > repro.diff` at the repo root.

Also state which KDDW sha1, branch or version you're using, and which operating system.


KDAB will happily accept external contributions; however, **all** contributions require a
signed [Copyright Assignment Agreement](docs/KDDockWidgets-CopyrightAssignmentForm.pdf).

This is needed so we can continue to dual-license it.

Contact info@kdab.com for more information.

Thanks to our [contributors](CONTRIBUTORS.txt).

About KDAB
==========
KDDockWidgets is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit https://www.kdab.com to meet the people who write code like this.

Stay up-to-date with KDAB product announcements:

* [KDAB Newsletter](https://news.kdab.com)
* [KDAB Blogs](https://www.kdab.com/category/blogs)
* [KDAB on Twitter](https://twitter.com/KDABQt)
