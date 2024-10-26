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

class Config implements ffi.Finalizable {
  static var s_dartInstanceByCppPtr = Map<int, Config>();
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

  factory Config.fromCache(var cppPointer, [needsAutoDelete = false]) {
    return (s_dartInstanceByCppPtr[cppPointer.address] ??
        Config.fromCppPointer(cppPointer, needsAutoDelete)) as Config;
  }
  Config.fromCppPointer(var cppPointer, [this._needsAutoDelete = false]) {
    thisCpp = cppPointer;
  }
  Config.init() {}
  String getFinalizerName() {
    return "c_KDDockWidgets__Config_Finalizer";
  } // absoluteWidgetMaxSize() const

  Size absoluteWidgetMaxSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__absoluteWidgetMaxSize')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  } // absoluteWidgetMinSize() const

  Size absoluteWidgetMinSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__absoluteWidgetMinSize')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  } // draggedWindowOpacity() const

  double draggedWindowOpacity() {
    final double_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<double_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__draggedWindowOpacity')
        .asFunction();
    return func(thisCpp);
  } // dropIndicatorsInhibited() const

  bool dropIndicatorsInhibited() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__dropIndicatorsInhibited')
        .asFunction();
    return func(thisCpp) != 0;
  } // layoutSaverUsesStrictMode() const

  bool layoutSaverUsesStrictMode() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__layoutSaverUsesStrictMode')
        .asFunction();
    return func(thisCpp) != 0;
  } // layoutSpacing() const

  int layoutSpacing() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__layoutSpacing')
        .asFunction();
    return func(thisCpp);
  } // mdiPopupThreshold() const

  int mdiPopupThreshold() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__mdiPopupThreshold')
        .asFunction();
    return func(thisCpp);
  } // onlyProgrammaticDrag() const

  bool onlyProgrammaticDrag() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__onlyProgrammaticDrag')
        .asFunction();
    return func(thisCpp) != 0;
  } // printDebug()

  printDebug() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__printDebug')
        .asFunction();
    func(thisCpp);
  }

  static // self()
      Config self() {
    final voidstar_Func_void func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_void_FFI>>(
            'c_static_KDDockWidgets__Config__self')
        .asFunction();
    ffi.Pointer<void> result = func();
    return Config.fromCppPointer(result, false);
  } // separatorThickness() const

  int separatorThickness() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__separatorThickness')
        .asFunction();
    return func(thisCpp);
  } // setAbsoluteWidgetMaxSize(KDDockWidgets::Size size)

  setAbsoluteWidgetMaxSize(Size size) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Config__setAbsoluteWidgetMaxSize_Size')
        .asFunction();
    func(thisCpp, size == null ? ffi.nullptr : size.thisCpp);
  } // setAbsoluteWidgetMinSize(KDDockWidgets::Size size)

  setAbsoluteWidgetMinSize(Size size) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Config__setAbsoluteWidgetMinSize_Size')
        .asFunction();
    func(thisCpp, size == null ? ffi.nullptr : size.thisCpp);
  } // setDraggedWindowOpacity(double opacity)

  setDraggedWindowOpacity(double opacity) {
    final void_Func_voidstar_double func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Double_FFI>>(
            'c_KDDockWidgets__Config__setDraggedWindowOpacity_double')
        .asFunction();
    func(thisCpp, opacity);
  } // setDropIndicatorsInhibited(bool inhibit) const

  setDropIndicatorsInhibited(bool inhibit) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Config__setDropIndicatorsInhibited_bool')
        .asFunction();
    func(thisCpp, inhibit ? 1 : 0);
  } // setLayoutSaverStrictMode(bool arg__1)

  setLayoutSaverStrictMode(bool arg__1) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Config__setLayoutSaverStrictMode_bool')
        .asFunction();
    func(thisCpp, arg__1 ? 1 : 0);
  } // setLayoutSpacing(int arg__1)

  setLayoutSpacing(int arg__1) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Config__setLayoutSpacing_int')
        .asFunction();
    func(thisCpp, arg__1);
  } // setMDIPopupThreshold(int arg__1)

  setMDIPopupThreshold(int arg__1) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Config__setMDIPopupThreshold_int')
        .asFunction();
    func(thisCpp, arg__1);
  } // setOnlyProgrammaticDrag(bool arg__1)

  setOnlyProgrammaticDrag(bool arg__1) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Config__setOnlyProgrammaticDrag_bool')
        .asFunction();
    func(thisCpp, arg__1 ? 1 : 0);
  } // setSeparatorThickness(int value)

  setSeparatorThickness(int value) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Config__setSeparatorThickness_int')
        .asFunction();
    func(thisCpp, value);
  } // setStartDragDistance(int arg__1)

  setStartDragDistance(int arg__1) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Config__setStartDragDistance_int')
        .asFunction();
    func(thisCpp, arg__1);
  } // setTransparencyOnlyOverDropIndicator(bool only)

  setTransparencyOnlyOverDropIndicator(bool only) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Config__setTransparencyOnlyOverDropIndicator_bool')
        .asFunction();
    func(thisCpp, only ? 1 : 0);
  } // setViewFactory(KDDockWidgets::Core::ViewFactory * arg__1)

  setViewFactory(KDDWBindingsCore.ViewFactory? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Config__setViewFactory_ViewFactory')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // startDragDistance() const

  int startDragDistance() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__startDragDistance')
        .asFunction();
    return func(thisCpp);
  } // transparencyOnlyOverDropIndicator() const

  bool transparencyOnlyOverDropIndicator() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__transparencyOnlyOverDropIndicator')
        .asFunction();
    return func(thisCpp) != 0;
  } // viewFactory() const

  KDDWBindingsCore.ViewFactory viewFactory() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__viewFactory')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.ViewFactory.fromCppPointer(result, false);
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Config__destructor')
        .asFunction();
    func(thisCpp);
  }
}
