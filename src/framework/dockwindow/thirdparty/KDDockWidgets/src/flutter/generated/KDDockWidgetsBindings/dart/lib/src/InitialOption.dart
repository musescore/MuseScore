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

class InitialOption implements ffi.Finalizable {
  static var s_dartInstanceByCppPtr = Map<int, InitialOption>();
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

  factory InitialOption.fromCache(var cppPointer, [needsAutoDelete = false]) {
    return (s_dartInstanceByCppPtr[cppPointer.address] ??
            InitialOption.fromCppPointer(cppPointer, needsAutoDelete))
        as InitialOption;
  }
  InitialOption.fromCppPointer(var cppPointer,
      [this._needsAutoDelete = false]) {
    thisCpp = cppPointer;
  }
  InitialOption.init() {}
  String getFinalizerName() {
    return "c_KDDockWidgets__InitialOption_Finalizer";
  } //InitialOption()

  InitialOption() {
    final voidstar_Func_void func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_void_FFI>>(
            'c_KDDockWidgets__InitialOption__constructor')
        .asFunction();
    thisCpp = func();
    InitialOption.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } //InitialOption(KDDockWidgets::DefaultSizeMode mode)
  InitialOption.ctor2(int mode) {
    final voidstar_Func_int func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_ffi_Int32_FFI>>(
            'c_KDDockWidgets__InitialOption__constructor_DefaultSizeMode')
        .asFunction();
    thisCpp = func(mode);
    InitialOption.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } //InitialOption(KDDockWidgets::InitialVisibilityOption v)
  InitialOption.ctor3(int v) {
    final voidstar_Func_int func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_ffi_Int32_FFI>>(
            'c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption')
        .asFunction();
    thisCpp = func(v);
    InitialOption.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } //InitialOption(KDDockWidgets::InitialVisibilityOption v, KDDockWidgets::Size size)
  InitialOption.ctor4(int v, Size size) {
    final voidstar_Func_int_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_ffi_Int32_voidstar_FFI>>(
            'c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption_Size')
        .asFunction();
    thisCpp = func(v, size == null ? ffi.nullptr : size.thisCpp);
    InitialOption.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } //InitialOption(KDDockWidgets::Size size)
  InitialOption.ctor5(Size size) {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__InitialOption__constructor_Size')
        .asFunction();
    thisCpp = func(size == null ? ffi.nullptr : size.thisCpp);
    InitialOption.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } // preservesCurrentTab() const
  bool preservesCurrentTab() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__InitialOption__preservesCurrentTab')
        .asFunction();
    return func(thisCpp) != 0;
  } // startsHidden() const

  bool startsHidden() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__InitialOption__startsHidden')
        .asFunction();
    return func(thisCpp) != 0;
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__InitialOption__destructor')
        .asFunction();
    func(thisCpp);
  }
}
