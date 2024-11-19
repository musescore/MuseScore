# 3rdparty

Here we list which 3rdparty software used and possibly shipped by KDDW and their respective licenses.

## Runtime dependencies

### nlohmann JSON

JSON c++ library.<br>
[code](src/3rdparty/nlohmann/nlohmann/json.hpp)<br>
[MIT licensed](LICENSES/MIT.txt)

### KDBindings

Non-Qt signal/slot implementation.

[code](src/3rdparty/kdbindings/)<br>
[MIT licensed](LICENSES/MIT.txt)

### KDStlContainerAdaptor

Only used for a Flutter build.
Replacement for QVector.

[code](src/3rdparty/kdtoolbox/KDStlContainerAdaptor.h)<br>
[MIT licensed](LICENSES/MIT.txt)

## Build-time / CI tests 3rdparty

These dependencies are only used during build or other CI purposes like running tests.

### ECM

CMake build helpers.

[code](cmake/ECM/modules/)<br>
[BSD licensed](LICENSES/BSD-3-Clause.txt)

### doxygen-awesome.css

Template to generate documentation.

[code](docs/api/doxygen-awesome.css)<br>
[MIT licensed](LICENSES/MIT.txt)

### QCoro

co-routine Qt implementation, only used by Flutter unit-tests.

[code](src/3rdparty/qcoro/)<br>
[MIT licensed](LICENSES/MIT.txt)

### Flutter embedder and generated files

Only used for a flutter build and only during tests.

[code](tests/flutter_tests_embedder) and [code](examples/flutter)<br>
[BSD licensed](LICENSES/BSD-3-Clause.txt)
