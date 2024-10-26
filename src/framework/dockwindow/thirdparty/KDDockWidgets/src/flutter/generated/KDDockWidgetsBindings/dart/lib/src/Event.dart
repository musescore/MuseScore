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

class Event_Type {
  static const MouseButtonPress = 0;
  static const MouseButtonDblClick = 1;
  static const MouseButtonRelease = 2;
  static const MouseMove = 3;
  static const NonClientAreaMouseButtonPress = 4;
  static const NonClientAreaMouseButtonRelease = 5;
  static const NonClientAreaMouseMove = 6;
  static const NonClientAreaMouseButtonDblClick = 7;
  static const HoverEnter = 8;
  static const HoverLeave = 9;
  static const HoverMove = 10;
  static const DragEnter = 11;
  static const DragLeave = 12;
  static const DragMove = 13;
  static const Drop = 14;
  static const Move = 15;
  static const FocusIn = 16;
  static const WindowActivate = 17;
  static const LayoutRequest = 18;
  static const Close = 19;
}

class Event implements ffi.Finalizable {
  static var s_dartInstanceByCppPtr = Map<int, Event>();
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

  factory Event.fromCache(var cppPointer, [needsAutoDelete = false]) {
    return (s_dartInstanceByCppPtr[cppPointer.address] ??
        Event.fromCppPointer(cppPointer, needsAutoDelete)) as Event;
  }
  Event.fromCppPointer(var cppPointer, [this._needsAutoDelete = false]) {
    thisCpp = cppPointer;
  }
  Event.init() {}
  String getFinalizerName() {
    return "c_KDDockWidgets__Event_Finalizer";
  }

  bool get m_accepted {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event___get_m_accepted')
        .asFunction();
    return func(thisCpp) != 0;
  }

  set m_accepted(bool m_accepted_) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Event___set_m_accepted_bool')
        .asFunction();
    func(thisCpp, m_accepted_ ? 1 : 0);
  }

  bool get m_spontaneous {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event___get_m_spontaneous')
        .asFunction();
    return func(thisCpp) != 0;
  }

  set m_spontaneous(bool m_spontaneous_) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Event___set_m_spontaneous_bool')
        .asFunction();
    func(thisCpp, m_spontaneous_ ? 1 : 0);
  } //Event(KDDockWidgets::Event::Type type)

  Event(int type) {
    final voidstar_Func_int func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Event__constructor_Type')
        .asFunction();
    thisCpp = func(type);
    Event.s_dartInstanceByCppPtr[thisCpp.address] = this;
  } // accept()
  accept() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__accept')
        .asFunction();
    func(thisCpp);
  } // ignore()

  ignore() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__ignore')
        .asFunction();
    func(thisCpp);
  } // isAccepted() const

  bool isAccepted() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__isAccepted')
        .asFunction();
    return func(thisCpp) != 0;
  } // spontaneous() const

  bool spontaneous() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__spontaneous')
        .asFunction();
    return func(thisCpp) != 0;
  } // type() const

  int type() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__type')
        .asFunction();
    return func(thisCpp);
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Event__destructor')
        .asFunction();
    func(thisCpp);
  }
}
