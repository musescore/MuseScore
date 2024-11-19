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

class DockRegistry extends KDDWBindingsCore.Object {
  DockRegistry.fromCppPointer(var cppPointer, [var needsAutoDelete = false])
      : super.fromCppPointer(cppPointer, needsAutoDelete) {}
  DockRegistry.init() : super.init() {}
  factory DockRegistry.fromCache(var cppPointer, [needsAutoDelete = false]) {
    if (KDDWBindingsCore.Object.isCached(cppPointer)) {
      var instance =
          KDDWBindingsCore.Object.s_dartInstanceByCppPtr[cppPointer.address];
      if (instance != null) return instance as DockRegistry;
    }
    return DockRegistry.fromCppPointer(cppPointer, needsAutoDelete);
  }
  String getFinalizerName() {
    return "c_KDDockWidgets__DockRegistry_Finalizer";
  } // clear()

  clear() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__clear')
        .asFunction();
    func(thisCpp);
  } // containsDockWidget(const QString & uniqueName) const

  bool containsDockWidget(String? uniqueName) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__containsDockWidget_QString')
        .asFunction();
    return func(thisCpp, uniqueName?.toNativeUtf8() ?? ffi.nullptr) != 0;
  } // containsMainWindow(const QString & uniqueName) const

  bool containsMainWindow(String? uniqueName) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__containsMainWindow_QString')
        .asFunction();
    return func(thisCpp, uniqueName?.toNativeUtf8() ?? ffi.nullptr) != 0;
  } // dockByName(const QString & arg__1) const

  KDDWBindingsCore.DockWidget dockByName(String? arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__dockByName_QString')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1?.toNativeUtf8() ?? ffi.nullptr);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // ensureAllFloatingWidgetsAreMorphed()

  ensureAllFloatingWidgetsAreMorphed() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__ensureAllFloatingWidgetsAreMorphed')
        .asFunction();
    func(thisCpp);
  } // focusedDockWidget() const

  KDDWBindingsCore.DockWidget focusedDockWidget() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__focusedDockWidget')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // groupInMDIResize() const

  KDDWBindingsCore.Group groupInMDIResize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__groupInMDIResize')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.Group.fromCppPointer(result, false);
  } // hasFloatingWindows() const

  bool hasFloatingWindows() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__hasFloatingWindows')
        .asFunction();
    return func(thisCpp) != 0;
  } // isEmpty(bool excludeBeingDeleted) const

  bool isEmpty({bool excludeBeingDeleted = false}) {
    final bool_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__DockRegistry__isEmpty_bool')
        .asFunction();
    return func(thisCpp, excludeBeingDeleted ? 1 : 0) != 0;
  } // isSane() const

  bool isSane() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__isSane')
        .asFunction();
    return func(thisCpp) != 0;
  } // itemIsInMainWindow(const KDDockWidgets::Core::Item * arg__1) const

  bool itemIsInMainWindow(Item? arg__1) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__itemIsInMainWindow_Item')
        .asFunction();
    return func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp) != 0;
  } // layoutForItem(const KDDockWidgets::Core::Item * arg__1) const

  KDDWBindingsCore.Layout layoutForItem(Item? arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__layoutForItem_Item')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
    return KDDWBindingsCore.Layout.fromCppPointer(result, false);
  } // mainWindowByName(const QString & arg__1) const

  KDDWBindingsCore.MainWindow mainWindowByName(String? arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__mainWindowByName_QString')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1?.toNativeUtf8() ?? ffi.nullptr);
    return KDDWBindingsCore.MainWindow.fromCppPointer(result, false);
  } // maybeDelete()

  maybeDelete() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__maybeDelete')
        .asFunction();
    func(thisCpp);
  } // registerDockWidget(KDDockWidgets::Core::DockWidget * arg__1)

  registerDockWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerDockWidget_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // registerFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)

  registerFloatingWindow(KDDWBindingsCore.FloatingWindow? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerFloatingWindow_FloatingWindow')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // registerGroup(KDDockWidgets::Core::Group * arg__1)

  registerGroup(KDDWBindingsCore.Group? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerGroup_Group')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // registerLayoutSaver()

  registerLayoutSaver() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerLayoutSaver')
        .asFunction();
    func(thisCpp);
  } // registerMainWindow(KDDockWidgets::Core::MainWindow * arg__1)

  registerMainWindow(KDDWBindingsCore.MainWindow? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerMainWindow_MainWindow')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  }

  static // self()
      DockRegistry self() {
    final voidstar_Func_void func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_void_FFI>>(
            'c_static_KDDockWidgets__DockRegistry__self')
        .asFunction();
    ffi.Pointer<void> result = func();
    return DockRegistry.fromCppPointer(result, false);
  } // setFocusedDockWidget(KDDockWidgets::Core::DockWidget * arg__1)

  setFocusedDockWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__setFocusedDockWidget_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * arg__1) const

  KDDWBindingsCore.SideBar sideBarForDockWidget(
      KDDWBindingsCore.DockWidget? arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__sideBarForDockWidget_DockWidget')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
    return KDDWBindingsCore.SideBar.fromCppPointer(result, false);
  } // unregisterDockWidget(KDDockWidgets::Core::DockWidget * arg__1)

  unregisterDockWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__unregisterDockWidget_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)

  unregisterFloatingWindow(KDDWBindingsCore.FloatingWindow? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__unregisterFloatingWindow_FloatingWindow')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // unregisterGroup(KDDockWidgets::Core::Group * arg__1)

  unregisterGroup(KDDWBindingsCore.Group? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__unregisterGroup_Group')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // unregisterLayoutSaver()

  unregisterLayoutSaver() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__unregisterLayoutSaver')
        .asFunction();
    func(thisCpp);
  } // unregisterMainWindow(KDDockWidgets::Core::MainWindow * arg__1)

  unregisterMainWindow(KDDWBindingsCore.MainWindow? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__unregisterMainWindow_MainWindow')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__DockRegistry__destructor')
        .asFunction();
    func(thisCpp);
  }

  String cFunctionSymbolName(int methodId) {
    switch (methodId) {}
    return super.cFunctionSymbolName(methodId);
  }

  static String methodNameFromId(int methodId) {
    switch (methodId) {}
    throw Error();
  }

  void registerCallbacks() {
    assert(thisCpp != null);
    final RegisterMethodIsReimplementedCallback registerCallback = _dylib
        .lookup<ffi.NativeFunction<RegisterMethodIsReimplementedCallback_FFI>>(
            'c_KDDockWidgets__DockRegistry__registerVirtualMethodCallback')
        .asFunction();
  }
}
