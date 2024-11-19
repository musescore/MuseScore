# WebAssembly

KDDockWidgets works with WebAssembly with the following known limitations:

- Depending on the browser, glitches, slowness, or lack of transparency while dragging windows might happen.
This is specially true on Linux on browsers with 3D acceleration disabled.
Please file a bug with Qt or your distro as it's out of scope for KDDW to fix.

- Qt 5 WASM probably works, but is unsupported and untested.

- KDDW QtQuick is untested on WASM

## Demo

A demo is available at <https://demos.kdab.com/wasm/kddockwidgets/dockwidgets.html>.

## Building KDDW

This can be done by following the generic instructions at <https://doc.qt.io/qt-6/wasm.html>.

For a Linux system, it's something like this (adjust paths):
```
$ source ~/emsdk/emsdk_env.sh
$ em++ --version # Needs to be 3.1.37 for Qt 6.6.0
$ cd KDDockWidgets/
$ ~/Qt/6.6.0/wasm_multithread/bin/qt-cmake --preset=wasm-release
$ cd build-wasm-release/
$ ninja kddockwidgets
```


## Builds tips for your own app

- Use `qt_add_executable` instead of `add_executable`, otherwise the `*.html` file won't be generated.
- Link to `libkddockwidgets-qt6.a`
- As the build is static, don't forget to initialize KDDW's resources:

```cpp
#ifdef QT_STATIC
    Q_INIT_RESOURCE(kddockwidgets_resources);
#endif
```
