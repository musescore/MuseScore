/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
import '../TypeHelpers.dart';
import '../../Bindings.dart';
import '../../Bindings_KDDWBindingsCore.dart' as KDDWBindingsCore;
import '../../Bindings_KDDWBindingsFlutter.dart' as KDDWBindingsFlutter;
import '../../LibraryLoader.dart';

var _dylib = Library.instance().dylib;

class MainWindow extends KDDWBindingsCore.Controller {
  MainWindow.fromCppPointer(var cppPointer, [var needsAutoDelete = false])
      : super.fromCppPointer(cppPointer, needsAutoDelete) {}
  MainWindow.init() : super.init() {}
  factory MainWindow.fromCache(var cppPointer, [needsAutoDelete = false]) {
    if (KDDWBindingsCore.Object.isCached(cppPointer)) {
      var instance =
          KDDWBindingsCore.Object.s_dartInstanceByCppPtr[cppPointer.address];
      if (instance != null) return instance as MainWindow;
    }
    return MainWindow.fromCppPointer(cppPointer, needsAutoDelete);
  }
  String getFinalizerName() {
    return "c_KDDockWidgets__Core__MainWindow_Finalizer";
  } //MainWindow(KDDockWidgets::Core::View * view, const QString & uniqueName, QFlags<KDDockWidgets::MainWindowOption> options)

  MainWindow(KDDWBindingsCore.View? view, String? uniqueName, int options)
      : super.init() {
    final voidstar_Func_voidstar_voidstar_int func = _dylib
        .lookup<
                ffi
                .NativeFunction<voidstar_Func_voidstar_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__constructor_View_QString_MainWindowOptions')
        .asFunction();
    thisCpp = func(view == null ? ffi.nullptr : view.thisCpp,
        uniqueName?.toNativeUtf8() ?? ffi.nullptr, options);
    KDDWBindingsCore.Object.s_dartInstanceByCppPtr[thisCpp.address] = this;
    registerCallbacks();
  } // addDockWidget(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
  addDockWidget(KDDWBindingsCore.DockWidget? dockWidget, int location,
      {required KDDWBindingsCore.DockWidget? relativeTo,
      required InitialOption initialOption}) {
    final void_Func_voidstar_voidstar_int_voidstar_voidstar func = _dylib
        .lookup<
                ffi.NativeFunction<
                    void_Func_voidstar_voidstar_ffi_Int32_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__addDockWidget_DockWidget_Location_DockWidget_InitialOption')
        .asFunction();
    func(
        thisCpp,
        dockWidget == null ? ffi.nullptr : dockWidget.thisCpp,
        location,
        relativeTo == null ? ffi.nullptr : relativeTo.thisCpp,
        initialOption == null ? ffi.nullptr : initialOption.thisCpp);
  } // addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * dockwidget)

  addDockWidgetAsTab(KDDWBindingsCore.DockWidget? dockwidget) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__addDockWidgetAsTab_DockWidget')
        .asFunction();
    func(thisCpp, dockwidget == null ? ffi.nullptr : dockwidget.thisCpp);
  } // addDockWidgetToSide(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::InitialOption initialOption)

  addDockWidgetToSide(KDDWBindingsCore.DockWidget? dockWidget, int location,
      {required InitialOption initialOption}) {
    final void_Func_voidstar_voidstar_int_voidstar func = _dylib
        .lookup<
                ffi.NativeFunction<
                    void_Func_voidstar_voidstar_ffi_Int32_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__addDockWidgetToSide_DockWidget_Location_InitialOption')
        .asFunction();
    func(thisCpp, dockWidget == null ? ffi.nullptr : dockWidget.thisCpp,
        location, initialOption == null ? ffi.nullptr : initialOption.thisCpp);
  } // anySideBarIsVisible() const

  bool anySideBarIsVisible() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__anySideBarIsVisible')
        .asFunction();
    return func(thisCpp) != 0;
  } // centerWidgetMargins() const

  Margins centerWidgetMargins() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__centerWidgetMargins')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Margins.fromCppPointer(result, true);
  } // centralAreaGeometry() const

  Rect centralAreaGeometry() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__centralAreaGeometry')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Rect.fromCppPointer(result, true);
  } // clearSideBarOverlay(bool deleteGroup)

  clearSideBarOverlay({bool deleteGroup = true}) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__clearSideBarOverlay_bool')
        .asFunction();
    func(thisCpp, deleteGroup ? 1 : 0);
  } // closeDockWidgets(bool force)

  bool closeDockWidgets({bool force = false}) {
    final bool_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int8_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__closeDockWidgets_bool')
        .asFunction();
    return func(thisCpp, force ? 1 : 0) != 0;
  } // dropArea() const

  KDDWBindingsCore.DropArea dropArea() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__dropArea')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DropArea.fromCppPointer(result, false);
  } // init(const QString & name)

  init(String? name) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__init_QString')
        .asFunction();
    func(thisCpp, name?.toNativeUtf8() ?? ffi.nullptr);
  } // isMDI() const

  bool isMDI() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__isMDI')
        .asFunction();
    return func(thisCpp) != 0;
  } // layout() const

  KDDWBindingsCore.Layout layout() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__layout')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.Layout.fromCppPointer(result, false);
  } // layoutEqually()

  layoutEqually() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__layoutEqually')
        .asFunction();
    func(thisCpp);
  } // layoutParentContainerEqually(KDDockWidgets::Core::DockWidget * dockWidget)

  layoutParentContainerEqually(KDDWBindingsCore.DockWidget? dockWidget) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__layoutParentContainerEqually_DockWidget')
        .asFunction();
    func(thisCpp, dockWidget == null ? ffi.nullptr : dockWidget.thisCpp);
  } // moveToSideBar(KDDockWidgets::Core::DockWidget * dw)

  moveToSideBar(KDDWBindingsCore.DockWidget? dw) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__moveToSideBar_DockWidget')
        .asFunction();
    func(thisCpp, dw == null ? ffi.nullptr : dw.thisCpp);
  } // multiSplitter() const

  KDDWBindingsCore.DropArea multiSplitter() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__multiSplitter')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DropArea.fromCppPointer(result, false);
  } // options() const

  int options() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__options')
        .asFunction();
    return func(thisCpp);
  } // overlayMargin() const

  int overlayMargin() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__overlayMargin')
        .asFunction();
    return func(thisCpp);
  } // overlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)

  overlayOnSideBar(KDDWBindingsCore.DockWidget? dw) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__overlayOnSideBar_DockWidget')
        .asFunction();
    func(thisCpp, dw == null ? ffi.nullptr : dw.thisCpp);
  } // overlayedDockWidget() const

  KDDWBindingsCore.DockWidget overlayedDockWidget() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__overlayedDockWidget')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // restoreFromSideBar(KDDockWidgets::Core::DockWidget * dw)

  restoreFromSideBar(KDDWBindingsCore.DockWidget? dw) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__restoreFromSideBar_DockWidget')
        .asFunction();
    func(thisCpp, dw == null ? ffi.nullptr : dw.thisCpp);
  } // setContentsMargins(int l, int t, int r, int b)

  setContentsMargins(int l, int t, int r, int b) {
    final void_Func_voidstar_int_int_int_int func = _dylib
        .lookup<
                ffi.NativeFunction<
                    void_Func_voidstar_ffi_Int32_ffi_Int32_ffi_Int32_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__setContentsMargins_int_int_int_int')
        .asFunction();
    func(thisCpp, l, t, r, b);
  } // setOverlayMargin(int margin)

  setOverlayMargin(int margin) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__setOverlayMargin_int')
        .asFunction();
    func(thisCpp, margin);
  }

  static void setParentView_impl_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void>? parent) {
    var dartInstance = KDDWBindingsCore
        .Object.s_dartInstanceByCppPtr[thisCpp.address] as MainWindow;
    if (dartInstance == null) {
      print(
          "Dart instance not found for MainWindow::setParentView_impl(KDDockWidgets::Core::View * parent)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.setParentView_impl((parent == null || parent.address == 0)
        ? null
        : KDDWBindingsCore.View.fromCppPointer(parent));
  } // setUniqueName(const QString & uniqueName)

  setUniqueName(String? uniqueName) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__setUniqueName_QString')
        .asFunction();
    func(thisCpp, uniqueName?.toNativeUtf8() ?? ffi.nullptr);
  } // sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * dw) const

  KDDWBindingsCore.SideBar sideBarForDockWidget(
      KDDWBindingsCore.DockWidget? dw) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__sideBarForDockWidget_DockWidget')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, dw == null ? ffi.nullptr : dw.thisCpp);
    return KDDWBindingsCore.SideBar.fromCppPointer(result, false);
  } // toggleOverlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)

  toggleOverlayOnSideBar(KDDWBindingsCore.DockWidget? dw) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__toggleOverlayOnSideBar_DockWidget')
        .asFunction();
    func(thisCpp, dw == null ? ffi.nullptr : dw.thisCpp);
  } // uniqueName() const

  QString uniqueName() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__uniqueName')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return QString.fromCppPointer(result, true);
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__destructor')
        .asFunction();
    func(thisCpp);
  }

  String cFunctionSymbolName(int methodId) {
    switch (methodId) {
      case 331:
        return "c_KDDockWidgets__Core__MainWindow__setParentView_impl_View";
    }
    return super.cFunctionSymbolName(methodId);
  }

  static String methodNameFromId(int methodId) {
    switch (methodId) {
      case 331:
        return "setParentView_impl";
    }
    throw Error();
  }

  void registerCallbacks() {
    assert(thisCpp != null);
    final RegisterMethodIsReimplementedCallback registerCallback = _dylib
        .lookup<ffi.NativeFunction<RegisterMethodIsReimplementedCallback_FFI>>(
            'c_KDDockWidgets__Core__MainWindow__registerVirtualMethodCallback')
        .asFunction();
    final callback331 =
        ffi.Pointer.fromFunction<void_Func_voidstar_voidstar_FFI>(
            KDDWBindingsCore.Controller.setParentView_impl_calledFromC);
    registerCallback(thisCpp, callback331, 331);
  }
}
