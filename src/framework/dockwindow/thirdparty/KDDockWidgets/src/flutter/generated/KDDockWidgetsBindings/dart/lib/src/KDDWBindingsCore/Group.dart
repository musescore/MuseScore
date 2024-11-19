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

class Group extends KDDWBindingsCore.Controller {
  Group.fromCppPointer(var cppPointer, [var needsAutoDelete = false])
      : super.fromCppPointer(cppPointer, needsAutoDelete) {}
  Group.init() : super.init() {}
  factory Group.fromCache(var cppPointer, [needsAutoDelete = false]) {
    if (KDDWBindingsCore.Object.isCached(cppPointer)) {
      var instance =
          KDDWBindingsCore.Object.s_dartInstanceByCppPtr[cppPointer.address];
      if (instance != null) return instance as Group;
    }
    return Group.fromCppPointer(cppPointer, needsAutoDelete);
  }
  String getFinalizerName() {
    return "c_KDDockWidgets__Core__Group_Finalizer";
  } //Group(KDDockWidgets::Core::View * parent)

  Group({required KDDWBindingsCore.View? parent}) : super.init() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__constructor_View')
        .asFunction();
    thisCpp = func(parent == null ? ffi.nullptr : parent.thisCpp);
    KDDWBindingsCore.Object.s_dartInstanceByCppPtr[thisCpp.address] = this;
    registerCallbacks();
  } // actualTitleBar() const
  KDDWBindingsCore.TitleBar actualTitleBar() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__actualTitleBar')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.TitleBar.fromCppPointer(result, false);
  } // addTab(KDDockWidgets::Core::DockWidget * arg__1, KDDockWidgets::InitialOption arg__2)

  addTab(KDDWBindingsCore.DockWidget? arg__1, {required InitialOption arg__2}) {
    final void_Func_voidstar_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__addTab_DockWidget_InitialOption')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp,
        arg__2 == null ? ffi.nullptr : arg__2.thisCpp);
  } // addTab(KDDockWidgets::Core::FloatingWindow * floatingWindow, KDDockWidgets::InitialOption arg__2)

  addTab_2(KDDWBindingsCore.FloatingWindow? floatingWindow,
      {required InitialOption arg__2}) {
    final void_Func_voidstar_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__addTab_FloatingWindow_InitialOption')
        .asFunction();
    func(thisCpp, floatingWindow == null ? ffi.nullptr : floatingWindow.thisCpp,
        arg__2 == null ? ffi.nullptr : arg__2.thisCpp);
  } // addTab(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::InitialOption arg__2)

  addTab_3(KDDWBindingsCore.Group? arg__1, {required InitialOption arg__2}) {
    final void_Func_voidstar_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__addTab_Group_InitialOption')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp,
        arg__2 == null ? ffi.nullptr : arg__2.thisCpp);
  } // allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const

  bool allDockWidgetsHave(int arg__1) {
    final bool_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__allDockWidgetsHave_DockWidgetOption')
        .asFunction();
    return func(thisCpp, arg__1) != 0;
  } // allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const

  bool allDockWidgetsHave_2(int arg__1) {
    final bool_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__allDockWidgetsHave_LayoutSaverOption')
        .asFunction();
    return func(thisCpp, arg__1) != 0;
  } // alwaysShowsTabs() const

  bool alwaysShowsTabs() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__alwaysShowsTabs')
        .asFunction();
    return func(thisCpp) != 0;
  } // anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const

  bool anyDockWidgetsHas(int arg__1) {
    final bool_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__anyDockWidgetsHas_DockWidgetOption')
        .asFunction();
    return func(thisCpp, arg__1) != 0;
  } // anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const

  bool anyDockWidgetsHas_2(int arg__1) {
    final bool_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__anyDockWidgetsHas_LayoutSaverOption')
        .asFunction();
    return func(thisCpp, arg__1) != 0;
  } // anyNonClosable() const

  bool anyNonClosable() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__anyNonClosable')
        .asFunction();
    return func(thisCpp) != 0;
  } // anyNonDockable() const

  bool anyNonDockable() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__anyNonDockable')
        .asFunction();
    return func(thisCpp) != 0;
  } // beingDeletedLater() const

  bool beingDeletedLater() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__beingDeletedLater')
        .asFunction();
    return func(thisCpp) != 0;
  } // biggestDockWidgetMaxSize() const

  Size biggestDockWidgetMaxSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__biggestDockWidgetMaxSize')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  } // containsDockWidget(KDDockWidgets::Core::DockWidget * w) const

  bool containsDockWidget(KDDWBindingsCore.DockWidget? w) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__containsDockWidget_DockWidget')
        .asFunction();
    return func(thisCpp, w == null ? ffi.nullptr : w.thisCpp) != 0;
  } // containsMouse(KDDockWidgets::Point globalPos) const

  bool containsMouse(Point globalPos) {
    final bool_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__containsMouse_Point')
        .asFunction();
    return func(thisCpp, globalPos == null ? ffi.nullptr : globalPos.thisCpp) !=
        0;
  } // createMDIResizeHandler()

  createMDIResizeHandler() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__createMDIResizeHandler')
        .asFunction();
    func(thisCpp);
  } // currentDockWidget() const

  KDDWBindingsCore.DockWidget currentDockWidget() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__currentDockWidget')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // currentIndex() const

  int currentIndex() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__currentIndex')
        .asFunction();
    return func(thisCpp);
  } // currentTabIndex() const

  int currentTabIndex() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__currentTabIndex')
        .asFunction();
    return func(thisCpp);
  }

  static // dbg_numFrames()
      int dbg_numFrames() {
    final int_Func_void func = _dylib
        .lookup<ffi.NativeFunction<int_Func_void_FFI>>(
            'c_static_KDDockWidgets__Core__Group__dbg_numFrames')
        .asFunction();
    return func();
  } // detachTab(KDDockWidgets::Core::DockWidget * arg__1)

  KDDWBindingsCore.FloatingWindow detachTab(
      KDDWBindingsCore.DockWidget? arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__detachTab_DockWidget')
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
    return KDDWBindingsCore.FloatingWindow.fromCppPointer(result, false);
  } // dockWidgetAt(int index) const

  KDDWBindingsCore.DockWidget dockWidgetAt(int index) {
    final voidstar_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__dockWidgetAt_int')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp, index);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // dockWidgetCount() const

  int dockWidgetCount() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__dockWidgetCount')
        .asFunction();
    return func(thisCpp);
  } // dockWidgetsMinSize() const

  Size dockWidgetsMinSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__dockWidgetsMinSize')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  } // dragRect() const

  Rect dragRect() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            cFunctionSymbolName(913))
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Rect.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> dragRect_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance = KDDWBindingsCore
        .Object.s_dartInstanceByCppPtr[thisCpp.address] as Group;
    if (dartInstance == null) {
      print(
          "Dart instance not found for Group::dragRect() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.dragRect();
    return result.thisCpp;
  } // floatingWindow() const

  KDDWBindingsCore.FloatingWindow floatingWindow() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__floatingWindow')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.FloatingWindow.fromCppPointer(result, false);
  } // focusedWidgetChangedCallback()

  focusedWidgetChangedCallback() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            cFunctionSymbolName(915))
        .asFunction();
    func(thisCpp);
  }

  static void focusedWidgetChangedCallback_calledFromC(
      ffi.Pointer<void> thisCpp) {
    var dartInstance = KDDWBindingsCore
        .Object.s_dartInstanceByCppPtr[thisCpp.address] as Group;
    if (dartInstance == null) {
      print(
          "Dart instance not found for Group::focusedWidgetChangedCallback()! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.focusedWidgetChangedCallback();
  }

  static // fromItem(const KDDockWidgets::Core::Item * arg__1)
      KDDWBindingsCore.Group fromItem(Item? arg__1) {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_static_KDDockWidgets__Core__Group__fromItem_Item')
        .asFunction();
    ffi.Pointer<void> result =
        func(arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
    return KDDWBindingsCore.Group.fromCppPointer(result, false);
  } // hasNestedMDIDockWidgets() const

  bool hasNestedMDIDockWidgets() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__hasNestedMDIDockWidgets')
        .asFunction();
    return func(thisCpp) != 0;
  } // hasSingleDockWidget() const

  bool hasSingleDockWidget() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__hasSingleDockWidget')
        .asFunction();
    return func(thisCpp) != 0;
  } // hasTabsVisible() const

  bool hasTabsVisible() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__hasTabsVisible')
        .asFunction();
    return func(thisCpp) != 0;
  } // indexOfDockWidget(const KDDockWidgets::Core::DockWidget * arg__1)

  int indexOfDockWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final int_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__indexOfDockWidget_DockWidget')
        .asFunction();
    return func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // insertDockWidget(KDDockWidgets::Core::DockWidget * arg__1, int index)

  insertDockWidget(KDDWBindingsCore.DockWidget? arg__1, int index) {
    final void_Func_voidstar_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__insertDockWidget_DockWidget_int')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp, index);
  } // insertWidget(KDDockWidgets::Core::DockWidget * arg__1, int index, KDDockWidgets::InitialOption arg__3)

  insertWidget(KDDWBindingsCore.DockWidget? arg__1, int index,
      {required InitialOption arg__3}) {
    final void_Func_voidstar_voidstar_int_voidstar func = _dylib
        .lookup<
                ffi.NativeFunction<
                    void_Func_voidstar_voidstar_ffi_Int32_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__insertWidget_DockWidget_int_InitialOption')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp, index,
        arg__3 == null ? ffi.nullptr : arg__3.thisCpp);
  } // isCentralGroup() const

  bool isCentralGroup() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isCentralGroup')
        .asFunction();
    return func(thisCpp) != 0;
  } // isDockable() const

  bool isDockable() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isDockable')
        .asFunction();
    return func(thisCpp) != 0;
  } // isEmpty() const

  bool isEmpty() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isEmpty')
        .asFunction();
    return func(thisCpp) != 0;
  } // isFloating() const

  bool isFloating() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isFloating')
        .asFunction();
    return func(thisCpp) != 0;
  } // isFocusedChangedCallback()

  isFocusedChangedCallback() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            cFunctionSymbolName(933))
        .asFunction();
    func(thisCpp);
  }

  static void isFocusedChangedCallback_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance = KDDWBindingsCore
        .Object.s_dartInstanceByCppPtr[thisCpp.address] as Group;
    if (dartInstance == null) {
      print(
          "Dart instance not found for Group::isFocusedChangedCallback()! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.isFocusedChangedCallback();
  } // isInFloatingWindow() const

  bool isInFloatingWindow() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isInFloatingWindow')
        .asFunction();
    return func(thisCpp) != 0;
  } // isInMainWindow() const

  bool isInMainWindow() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isInMainWindow')
        .asFunction();
    return func(thisCpp) != 0;
  } // isMDI() const

  bool isMDI() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isMDI')
        .asFunction();
    return func(thisCpp) != 0;
  } // isMDIWrapper() const

  bool isMDIWrapper() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isMDIWrapper')
        .asFunction();
    return func(thisCpp) != 0;
  } // isOverlayed() const

  bool isOverlayed() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isOverlayed')
        .asFunction();
    return func(thisCpp) != 0;
  } // isTheOnlyGroup() const

  bool isTheOnlyGroup() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__isTheOnlyGroup')
        .asFunction();
    return func(thisCpp) != 0;
  } // layoutItem() const

  Item layoutItem() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__layoutItem')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Item.fromCppPointer(result, false);
  } // mainWindow() const

  KDDWBindingsCore.MainWindow mainWindow() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__mainWindow')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.MainWindow.fromCppPointer(result, false);
  } // mdiDockWidgetWrapper() const

  KDDWBindingsCore.DockWidget mdiDockWidgetWrapper() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__mdiDockWidgetWrapper')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DockWidget.fromCppPointer(result, false);
  } // mdiDropAreaWrapper() const

  KDDWBindingsCore.DropArea mdiDropAreaWrapper() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__mdiDropAreaWrapper')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.DropArea.fromCppPointer(result, false);
  } // mdiFrame() const

  KDDWBindingsCore.Group mdiFrame() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__mdiFrame')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.Group.fromCppPointer(result, false);
  } // nonContentsHeight() const

  int nonContentsHeight() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__nonContentsHeight')
        .asFunction();
    return func(thisCpp);
  } // onDockWidgetCountChanged()

  onDockWidgetCountChanged() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__onDockWidgetCountChanged')
        .asFunction();
    func(thisCpp);
  } // onDockWidgetTitleChanged(KDDockWidgets::Core::DockWidget * arg__1)

  onDockWidgetTitleChanged(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__onDockWidgetTitleChanged_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // removeWidget(KDDockWidgets::Core::DockWidget * arg__1)

  removeWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__removeWidget_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // renameTab(int index, const QString & arg__2)

  renameTab(int index, String? arg__2) {
    final void_Func_voidstar_int_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__renameTab_int_QString')
        .asFunction();
    func(thisCpp, index, arg__2?.toNativeUtf8() ?? ffi.nullptr);
  } // restoreToPreviousPosition()

  restoreToPreviousPosition() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__restoreToPreviousPosition')
        .asFunction();
    func(thisCpp);
  } // scheduleDeleteLater()

  scheduleDeleteLater() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__scheduleDeleteLater')
        .asFunction();
    func(thisCpp);
  } // setCurrentDockWidget(KDDockWidgets::Core::DockWidget * arg__1)

  setCurrentDockWidget(KDDWBindingsCore.DockWidget? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__setCurrentDockWidget_DockWidget')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // setCurrentTabIndex(int index)

  setCurrentTabIndex(int index) {
    final void_Func_voidstar_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_FFI>>(
            'c_KDDockWidgets__Core__Group__setCurrentTabIndex_int')
        .asFunction();
    func(thisCpp, index);
  } // setLayout(KDDockWidgets::Core::Layout * arg__1)

  setLayout(KDDWBindingsCore.Layout? arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__setLayout_Layout')
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  } // setLayoutItem(KDDockWidgets::Core::Item * item)

  setLayoutItem(Item? item) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__setLayoutItem_Item')
        .asFunction();
    func(thisCpp, item == null ? ffi.nullptr : item.thisCpp);
  }

  static void setParentView_impl_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void>? parent) {
    var dartInstance = KDDWBindingsCore
        .Object.s_dartInstanceByCppPtr[thisCpp.address] as Group;
    if (dartInstance == null) {
      print(
          "Dart instance not found for Group::setParentView_impl(KDDockWidgets::Core::View * parent)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.setParentView_impl((parent == null || parent.address == 0)
        ? null
        : KDDWBindingsCore.View.fromCppPointer(parent));
  } // stack() const

  KDDWBindingsCore.Stack stack() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__stack')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.Stack.fromCppPointer(result, false);
  } // tabBar() const

  KDDWBindingsCore.TabBar tabBar() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__tabBar')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.TabBar.fromCppPointer(result, false);
  } // title() const

  QString title() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__title')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return QString.fromCppPointer(result, true);
  } // titleBar() const

  KDDWBindingsCore.TitleBar titleBar() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__titleBar')
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return KDDWBindingsCore.TitleBar.fromCppPointer(result, false);
  } // unoverlay()

  unoverlay() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__unoverlay')
        .asFunction();
    func(thisCpp);
  } // updateFloatingActions()

  updateFloatingActions() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__updateFloatingActions')
        .asFunction();
    func(thisCpp);
  } // updateTitleAndIcon()

  updateTitleAndIcon() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__updateTitleAndIcon')
        .asFunction();
    func(thisCpp);
  } // updateTitleBarVisibility()

  updateTitleBarVisibility() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__updateTitleBarVisibility')
        .asFunction();
    func(thisCpp);
  } // userType() const

  int userType() {
    final int_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<int_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__userType')
        .asFunction();
    return func(thisCpp);
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__Core__Group__destructor')
        .asFunction();
    func(thisCpp);
  }

  String cFunctionSymbolName(int methodId) {
    switch (methodId) {
      case 913:
        return "c_KDDockWidgets__Core__Group__dragRect";
      case 915:
        return "c_KDDockWidgets__Core__Group__focusedWidgetChangedCallback";
      case 933:
        return "c_KDDockWidgets__Core__Group__isFocusedChangedCallback";
      case 331:
        return "c_KDDockWidgets__Core__Group__setParentView_impl_View";
    }
    return super.cFunctionSymbolName(methodId);
  }

  static String methodNameFromId(int methodId) {
    switch (methodId) {
      case 913:
        return "dragRect";
      case 915:
        return "focusedWidgetChangedCallback";
      case 933:
        return "isFocusedChangedCallback";
      case 331:
        return "setParentView_impl";
    }
    throw Error();
  }

  void registerCallbacks() {
    assert(thisCpp != null);
    final RegisterMethodIsReimplementedCallback registerCallback = _dylib
        .lookup<ffi.NativeFunction<RegisterMethodIsReimplementedCallback_FFI>>(
            'c_KDDockWidgets__Core__Group__registerVirtualMethodCallback')
        .asFunction();
    final callback913 = ffi.Pointer.fromFunction<voidstar_Func_voidstar_FFI>(
        KDDWBindingsCore.Group.dragRect_calledFromC);
    registerCallback(thisCpp, callback913, 913);
    final callback915 = ffi.Pointer.fromFunction<void_Func_voidstar_FFI>(
        KDDWBindingsCore.Group.focusedWidgetChangedCallback_calledFromC);
    registerCallback(thisCpp, callback915, 915);
    final callback933 = ffi.Pointer.fromFunction<void_Func_voidstar_FFI>(
        KDDWBindingsCore.Group.isFocusedChangedCallback_calledFromC);
    registerCallback(thisCpp, callback933, 933);
    final callback331 =
        ffi.Pointer.fromFunction<void_Func_voidstar_voidstar_FFI>(
            KDDWBindingsCore.Group.setParentView_impl_calledFromC);
    registerCallback(thisCpp, callback331, 331);
  }
}
