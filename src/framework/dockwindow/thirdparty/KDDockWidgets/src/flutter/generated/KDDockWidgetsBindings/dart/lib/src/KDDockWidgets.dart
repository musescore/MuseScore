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

class KDDockWidgets_Location {
  static const Location_None = 0;
  static const Location_OnLeft = 1;
  static const Location_OnTop = 2;
  static const Location_OnRight = 3;
  static const Location_OnBottom = 4;
}

class KDDockWidgets_MainWindowOption {
  static const MainWindowOption_None = 0;
  static const MainWindowOption_HasCentralFrame = 1;
  static const MainWindowOption_MDI = 2;
  static const MainWindowOption_HasCentralWidget = 5;
  static const MainWindowOption_QDockWidgets = 8;
  static const MainWindowOption_ManualInit = 16;
}

class KDDockWidgets_DockWidgetOption {
  static const DockWidgetOption_None = 0;
  static const DockWidgetOption_NotClosable = 1;
  static const DockWidgetOption_NotDockable = 2;
  static const DockWidgetOption_DeleteOnClose = 4;
  static const DockWidgetOption_MDINestable = 8;
}

class KDDockWidgets_LayoutSaverOption {
  static const None = 0;
  static const Skip = 1;
  static const CheckForPreviousRestore = 2;
}

class KDDockWidgets_IconPlace {
  static const TitleBar = 1;
  static const TabBar = 2;
  static const ToggleAction = 4;
  static const All = 7;
}

class KDDockWidgets_FrontendType {
  static const QtWidgets = 1;
  static const QtQuick = 2;
  static const Flutter = 3;
}

class KDDockWidgets_DefaultSizeMode {
  static const ItemSize = 0;
  static const Fair = 1;
  static const FairButFloor = 2;
  static const NoDefaultSizeMode = 3;
}

class KDDockWidgets_AddingOption {
  static const AddingOption_None = 0;
  static const AddingOption_StartHidden = 1;
}

class KDDockWidgets_InitialVisibilityOption {
  static const StartVisible = 0;
  static const StartHidden = 1;
  static const PreserveCurrentTab = 2;
}

class KDDockWidgets_DropLocation {
  static const DropLocation_None = 0;
  static const DropLocation_Left = 1;
  static const DropLocation_Top = 2;
  static const DropLocation_Right = 4;
  static const DropLocation_Bottom = 8;
  static const DropLocation_Center = 16;
  static const DropLocation_OutterLeft = 32;
  static const DropLocation_OutterTop = 64;
  static const DropLocation_OutterRight = 128;
  static const DropLocation_OutterBottom = 256;
  static const DropLocation_Inner = 15;
  static const DropLocation_Outter = 480;
  static const DropLocation_Horizontal = 165;
  static const DropLocation_Vertical = 330;
} // fuzzyCompare(double a, double b, double epsilon)

bool fuzzyCompare(double a, double b, {double epsilon = 0.0001}) {
  final bool_Func_double_double_double func = _dylib
      .lookup<
              ffi
              .NativeFunction<bool_Func_ffi_Double_ffi_Double_ffi_Double_FFI>>(
          'c_static_KDDockWidgets__fuzzyCompare_double_double_double')
      .asFunction();
  return func(a, b, epsilon) != 0;
} // initFrontend(KDDockWidgets::FrontendType arg__1)

initFrontend(int arg__1) {
  final void_Func_int func = _dylib
      .lookup<ffi.NativeFunction<void_Func_ffi_Int32_FFI>>(
          'c_static_KDDockWidgets__initFrontend_FrontendType')
      .asFunction();
  func(arg__1);
} // spdlogLoggerName()

String spdlogLoggerName() {
  final string_Func_void func = _dylib
      .lookup<ffi.NativeFunction<string_Func_void_FFI>>(
          'c_static_KDDockWidgets__spdlogLoggerName')
      .asFunction();
  ffi.Pointer<Utf8> result = func();
  return result.toDartString();
}
