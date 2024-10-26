# QtWidgets

`QtWidgets` is the original KDDW frontend, hence it's the most feature complete.<br>
Although this is still the recommended frontend, you can also chose `QtQuick` and open a bug report for any missing feature.<br>

By default a KDDW build will support both `QtWidgets` and `QtQuick`.
If you only need `QtWidgets` then you should configure KDDW with:
```bash
-DKDDockWidgets_FRONTENDS="qtwidgets"
```

Simply to reduce binary size.

If you do have both QtWidgets and QtQuick support built, then don't forget to:
```cpp
KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
```
