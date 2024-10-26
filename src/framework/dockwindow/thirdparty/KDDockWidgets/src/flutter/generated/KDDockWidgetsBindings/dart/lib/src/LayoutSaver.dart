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
final _finalizerFunc = (String name) {
  return _dylib
      .lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer)>>(name);
};

Map<String, ffi.NativeFinalizer> _finalizers = {};

class LayoutSaver implements ffi.Finalizable {
  static var s_dartInstanceByCppPtr = Map<int, LayoutSaver>();
  var _thisCpp = null;
  bool _needsAutoDelete = true;
  get thisCpp => _thisCpp;
  set thisCpp(var ptr) {
    _thisCpp = ptr;
    ffi.Pointer<ffi.Void> ptrvoid = ptr.cast<ffi.Void>();
    if (_needsAutoDelete) {
      final String finalizerName = getFinalizerName();
      if (!_finalizers.keys.contains(runtimeType)) {
        _finalizers[finalizerName] =
            ffi.NativeFinalizer(_finalizerFunc(finalizerName).cast());
      }
      _finalizers[finalizerName]!.attach(this, ptrvoid);
    }
  }

  static bool isCached(var cppPointer) {
    return s_dartInstanceByCppPtr.containsKey(cppPointer.address);
  }

  factory LayoutSaver.fromCache(var cppPointer, [needsAutoDelete = false]) {
    return (s_dartInstanceByCppPtr[cppPointer.address] ??
        LayoutSaver.fromCppPointer(cppPointer, needsAutoDelete)) as LayoutSaver;
  }
  LayoutSaver.fromCppPointer(var cppPointer, [this._needsAutoDelete = false]) {
    thisCpp = cppPointer;
  }
  LayoutSaver.init() {}
  String getFinalizerName() {
    return "c_KDDockWidgets__LayoutSaver_Finalizer";
  } //LayoutSaver()

  LayoutSaver() {
    final voidstar_Func_void func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_void_FFI>>(
            'c_KDDockWidgets__LayoutSaver__constructor')
        .asFunction();
    thisCpp = func();
    LayoutSaver.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } // restoreFromFile(const QString & jsonFilename)
  bool restoreFromFile(String? jsonFilename) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__LayoutSaver__restoreFromFile_QString')
        .asFunction();
    return func(thisCpp, jsonFilename?.toNativeUtf8() ?? ffi.nullptr) != 0;
  }

  static // restoreInProgress()
      bool restoreInProgress() {
    final bool_Func_void func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_void_FFI>>(
            'c_static_KDDockWidgets__LayoutSaver__restoreInProgress')
        .asFunction();
    return func() != 0;
  } // saveToFile(const QString & jsonFilename)

  bool saveToFile(String? jsonFilename) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__LayoutSaver__saveToFile_QString')
        .asFunction();
    return func(thisCpp, jsonFilename?.toNativeUtf8() ?? ffi.nullptr) != 0;
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__LayoutSaver__destructor')
        .asFunction();
    func(thisCpp);
  }
}
