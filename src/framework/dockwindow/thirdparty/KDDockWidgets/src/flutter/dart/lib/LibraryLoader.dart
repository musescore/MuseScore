/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'dart:io';
import 'package:KDDockWidgetsBindings/LibraryLoader.dart';

String bindingsLibraryName() {
  String name = "kddockwidgets";
  if (Platform.isWindows) {
    return "${name}.dll";
  } else if (Platform.isMacOS) {
    return "lib${name}.dylib";
  } else {
    return "lib${name}.so";
  }
}

void initLibraryLoader() {
  Library.libraryGetPathFunc = () {
    final bindingsPath =
        Platform.environment["DARTAGNAN_BINDINGSLIB_PATH"] ?? "";
    final libraryName = bindingsLibraryName();

    if (bindingsPath.isEmpty) {
      print(
          "Please set DARTAGNAN_BINDINGSLIB_PATH env variable pointing to the location of ${libraryName}");
      return libraryName;
    }

    return "${bindingsPath}/${libraryName}";
  };
}
