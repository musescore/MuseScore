This directory contains CMake helpers and Find modules.

The latter are used to search for dependencies **without requiring pkg-config**.
If you want to use the config file install by a package instead of its find
module, set `CMAKE_FIND_PACKAGE_PREFER_CONFIG` to `ON`

Refer to [this file](../README.cmake.md) for more details about the Find modules.
