# QtQuick

KDDockWidgets for QtQuick requires `Qt >= 6.2.1`.

Qt 5.15 will probably work, but it's not built or tested by KDAB CI, we
advise users to move to `Qt 6` as soon as possible.

By default a KDDW build will support both `QtWidgets` and `QtQuick`.
If you only need `QtQuick` then you should configure KDDW with:
```bash
-DKDDockWidgets_FRONTENDS="qtquick"
```

If you do have both QtWidgets and QtQuick support built, then don't forget to:
```cpp
KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtQuick);
```

## Troubleshooting

- `QtGraphicalEffects` is not supported, as it's buggy when moving between different QWindows.
  See for example QTBUG-94943, KDDockWidgets issue #213. Also search the Qt bug tracker
  for "QQuickItem: Cannot use same item on different windows at the same time"

- Very rarely, in some Nvidia/X11 setups, floating/docking has noticeable lag (like 1 second)
  This could be solved by going to Nvidia's settings and making sure all monitors have
  the same refresh rate and disabling "Allow Flipping". It's not known why this solves it. Might also
  be a bug in Qt.

- `"EGLFS: OpenGL windows cannot be mixed with others"` QtQuick on EGLFS does not support having more
  than 1 window. This is a known QtQuick limitation. The QtWidgets stack worksaround this by compositing
  all windows into a single native window.

- `module "QtQuick.Controls" is not installed`

Set the `QML_IMPORT_PATH` env var pointing to your Qt qml plugin dir or check the
Qt documentation on how to deploy QtQuick applications.

```bash
# Replace with your actual path
export QML_IMPORT_PATH=/home/user/Qt/5.15.2/gcc_64/qml
```
