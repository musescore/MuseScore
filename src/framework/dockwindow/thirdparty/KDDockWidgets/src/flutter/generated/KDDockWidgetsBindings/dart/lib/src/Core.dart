/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
import 'TypeHelpers.dart';
import '../Bindings.dart';
import '../Bindings_KDDWBindingsCore.dart' as KDDWBindingsCore;
import '../Bindings_KDDWBindingsFlutter.dart' as KDDWBindingsFlutter;
import '../LibraryLoader.dart';

var _dylib = Library.instance().dylib;

class Core_ViewType {
  static const FIRST = 1;
  static const None = 0;
  static const Frame = 1;
  static const Group = 1;
  static const TitleBar = 2;
  static const TabBar = 4;
  static const Stack = 8;
  static const FloatingWindow = 16;
  static const Separator = 32;
  static const DockWidget = 64;
  static const LayoutItem = 256;
  static const SideBar = 512;
  static const MainWindow = 1024;
  static const ViewWrapper = 2048;
  static const DropArea = 4096;
  static const MDILayout = 8192;
  static const RubberBand = 16384;
  static const DropAreaIndicatorOverlay = 32768;
  static const LAST = 16384;
}
