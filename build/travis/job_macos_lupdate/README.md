
### Dependencies

The compilation relies on the availability of a zip archive containing Qt.
In order to reduce the size of the download, debug symbols and libraries are
not zipped.


This file is created with the following commands:

```
export QT_VERSION=5.10.0
cd Qt/$QT_VERSION/clang_64
zip -r qt5_mac.zip * -x "*_debug.*" "*_debug" "*.dSYM" "**.dSYM**"
```
