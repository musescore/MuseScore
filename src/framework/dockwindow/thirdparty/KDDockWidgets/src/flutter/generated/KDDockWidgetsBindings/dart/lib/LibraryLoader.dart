/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
import 'dart:ffi' as ffi;
import 'dart:io' show Platform;

String bindingsLibraryName(String name) {
  if (Platform.isWindows) return "${name}.dll";
  if (Platform.isMacOS) return "lib${name}.dylib";
  return "lib${name}.so";
}

typedef LibraryGetPathFunc = String Function();

class Library {
  var _dylib;
  static Library? _library;
  static LibraryGetPathFunc? libraryGetPathFunc;

  ffi.DynamicLibrary get dylib {
    return _dylib;
  }

  factory Library.instance() {
    // Singleton impl.
    if (_library == null) _library = Library._();
    return _library!;
  }

  Library._() {
    if (libraryGetPathFunc == null) {
      // DYLD_LIBRARY_PATH doesn't work by default on newer macOS. Instead
      // introduce our own env variable for the same use case
      var bindingsPath =
          Platform.environment["DARTAGNAN_BINDINGSLIB_PATH"] ?? "";

      var libraryPath = bindingsLibraryName("kddockwidgets-qt6");
      if (!bindingsPath.isEmpty) {
        libraryPath = bindingsPath + "/" + libraryPath;
      }

      _dylib = ffi.DynamicLibrary.open(libraryPath);
    } else {
      _dylib = ffi.DynamicLibrary.open(libraryGetPathFunc!());
    }
  }
}
