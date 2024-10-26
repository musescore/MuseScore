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
final _finalizerFunc = (String name) {
  return _dylib
      .lookup<ffi.NativeFunction<ffi.Void Function(ffi.Pointer)>>(name);
};

Map<String, ffi.NativeFinalizer> _finalizers = {};

class Window implements ffi.Finalizable {
  static var s_dartInstanceByCppPtr = Map<int, Window>();
  var _thisCpp = null;
  bool _needsAutoDelete = false;
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

  factory Window.fromCache(var cppPointer, [needsAutoDelete = false]) {
    return (s_dartInstanceByCppPtr[cppPointer.address] ??
        Window.fromCppPointer(cppPointer, needsAutoDelete)) as Window;
  }
  Window.fromCppPointer(var cppPointer, [this._needsAutoDelete = false]) {
    thisCpp = cppPointer;
  }
  Window.init() {}
  String getFinalizerName() {
    return "c_KDDockWidgets__flutter__Window_Finalizer";
  } // destroy()

  destroy() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            cFunctionSymbolName(16))
        .asFunction();
    func(thisCpp);
  }

  static void destroy_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::destroy()! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.destroy();
  } // frameGeometry() const

  Rect frameGeometry() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            cFunctionSymbolName(17))
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Rect.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> frameGeometry_calledFromC(
      ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::frameGeometry() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.frameGeometry();
    return result.thisCpp;
  } // fromNativePixels(KDDockWidgets::Point arg__1) const

  Point fromNativePixels(Point arg__1) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            cFunctionSymbolName(18))
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
    return Point.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> fromNativePixels_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void> arg__1) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::fromNativePixels(KDDockWidgets::Point arg__1) const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.fromNativePixels(Point.fromCppPointer(arg__1));
    return result.thisCpp;
  } // geometry() const

  Rect geometry() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            cFunctionSymbolName(19))
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Rect.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> geometry_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::geometry() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.geometry();
    return result.thisCpp;
  } // isActive() const

  bool isActive() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            cFunctionSymbolName(20))
        .asFunction();
    return func(thisCpp) != 0;
  }

  static int isActive_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::isActive() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.isActive();
    return result ? 1 : 0;
  } // isFullScreen() const

  bool isFullScreen() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            cFunctionSymbolName(21))
        .asFunction();
    return func(thisCpp) != 0;
  }

  static int isFullScreen_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::isFullScreen() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.isFullScreen();
    return result ? 1 : 0;
  } // isVisible() const

  bool isVisible() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            cFunctionSymbolName(22))
        .asFunction();
    return func(thisCpp) != 0;
  }

  static int isVisible_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::isVisible() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.isVisible();
    return result ? 1 : 0;
  } // mapFromGlobal(KDDockWidgets::Point globalPos) const

  Point mapFromGlobal(Point globalPos) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            cFunctionSymbolName(23))
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, globalPos == null ? ffi.nullptr : globalPos.thisCpp);
    return Point.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> mapFromGlobal_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void> globalPos) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::mapFromGlobal(KDDockWidgets::Point globalPos) const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.mapFromGlobal(Point.fromCppPointer(globalPos));
    return result.thisCpp;
  } // mapToGlobal(KDDockWidgets::Point localPos) const

  Point mapToGlobal(Point localPos) {
    final voidstar_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_voidstar_FFI>>(
            cFunctionSymbolName(24))
        .asFunction();
    ffi.Pointer<void> result =
        func(thisCpp, localPos == null ? ffi.nullptr : localPos.thisCpp);
    return Point.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> mapToGlobal_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void> localPos) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::mapToGlobal(KDDockWidgets::Point localPos) const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.mapToGlobal(Point.fromCppPointer(localPos));
    return result.thisCpp;
  } // maxSize() const

  Size maxSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            cFunctionSymbolName(25))
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> maxSize_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::maxSize() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.maxSize();
    return result.thisCpp;
  } // minSize() const

  Size minSize() {
    final voidstar_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<voidstar_Func_voidstar_FFI>>(
            cFunctionSymbolName(26))
        .asFunction();
    ffi.Pointer<void> result = func(thisCpp);
    return Size.fromCppPointer(result, true);
  }

  static ffi.Pointer<void> minSize_calledFromC(ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::minSize() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.minSize();
    return result.thisCpp;
  } // resize(int width, int height)

  resize(int width, int height) {
    final void_Func_voidstar_int_int func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int32_ffi_Int32_FFI>>(
            cFunctionSymbolName(27))
        .asFunction();
    func(thisCpp, width, height);
  }

  static void resize_calledFromC(
      ffi.Pointer<void> thisCpp, int width, int height) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::resize(int width, int height)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.resize(width, height);
  } // setFramePosition(KDDockWidgets::Point targetPos)

  setFramePosition(Point targetPos) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            cFunctionSymbolName(28))
        .asFunction();
    func(thisCpp, targetPos == null ? ffi.nullptr : targetPos.thisCpp);
  }

  static void setFramePosition_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void> targetPos) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::setFramePosition(KDDockWidgets::Point targetPos)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.setFramePosition(Point.fromCppPointer(targetPos));
  } // setGeometry(KDDockWidgets::Rect arg__1)

  setGeometry(Rect arg__1) {
    final void_Func_voidstar_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_voidstar_FFI>>(
            cFunctionSymbolName(29))
        .asFunction();
    func(thisCpp, arg__1 == null ? ffi.nullptr : arg__1.thisCpp);
  }

  static void setGeometry_calledFromC(
      ffi.Pointer<void> thisCpp, ffi.Pointer<void> arg__1) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::setGeometry(KDDockWidgets::Rect arg__1)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.setGeometry(Rect.fromCppPointer(arg__1));
  } // setVisible(bool arg__1)

  setVisible(bool arg__1) {
    final void_Func_voidstar_bool func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_ffi_Int8_FFI>>(
            cFunctionSymbolName(30))
        .asFunction();
    func(thisCpp, arg__1 ? 1 : 0);
  }

  static void setVisible_calledFromC(ffi.Pointer<void> thisCpp, int arg__1) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::setVisible(bool arg__1)! (${thisCpp.address})");
      throw Error();
    }
    dartInstance.setVisible(arg__1 != 0);
  } // supportsHonouringLayoutMinSize() const

  bool supportsHonouringLayoutMinSize() {
    final bool_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<bool_Func_voidstar_FFI>>(
            cFunctionSymbolName(31))
        .asFunction();
    return func(thisCpp) != 0;
  }

  static int supportsHonouringLayoutMinSize_calledFromC(
      ffi.Pointer<void> thisCpp) {
    var dartInstance =
        KDDWBindingsFlutter.Window.s_dartInstanceByCppPtr[thisCpp.address];
    if (dartInstance == null) {
      print(
          "Dart instance not found for Window::supportsHonouringLayoutMinSize() const! (${thisCpp.address})");
      throw Error();
    }
    final result = dartInstance.supportsHonouringLayoutMinSize();
    return result ? 1 : 0;
  }

  void release() {
    final void_Func_voidstar func = _dylib
        .lookup<ffi.NativeFunction<void_Func_voidstar_FFI>>(
            'c_KDDockWidgets__flutter__Window__destructor')
        .asFunction();
    func(thisCpp);
  }

  String cFunctionSymbolName(int methodId) {
    switch (methodId) {
      case 16:
        return "c_KDDockWidgets__flutter__Window__destroy";
      case 17:
        return "c_KDDockWidgets__flutter__Window__frameGeometry";
      case 18:
        return "c_KDDockWidgets__flutter__Window__fromNativePixels_Point";
      case 19:
        return "c_KDDockWidgets__flutter__Window__geometry";
      case 20:
        return "c_KDDockWidgets__flutter__Window__isActive";
      case 21:
        return "c_KDDockWidgets__flutter__Window__isFullScreen";
      case 22:
        return "c_KDDockWidgets__flutter__Window__isVisible";
      case 23:
        return "c_KDDockWidgets__flutter__Window__mapFromGlobal_Point";
      case 24:
        return "c_KDDockWidgets__flutter__Window__mapToGlobal_Point";
      case 25:
        return "c_KDDockWidgets__flutter__Window__maxSize";
      case 26:
        return "c_KDDockWidgets__flutter__Window__minSize";
      case 27:
        return "c_KDDockWidgets__flutter__Window__resize_int_int";
      case 28:
        return "c_KDDockWidgets__flutter__Window__setFramePosition_Point";
      case 29:
        return "c_KDDockWidgets__flutter__Window__setGeometry_Rect";
      case 30:
        return "c_KDDockWidgets__flutter__Window__setVisible_bool";
      case 31:
        return "c_KDDockWidgets__flutter__Window__supportsHonouringLayoutMinSize";
    }
    return "";
  }

  static String methodNameFromId(int methodId) {
    switch (methodId) {
      case 16:
        return "destroy";
      case 17:
        return "frameGeometry";
      case 18:
        return "fromNativePixels";
      case 19:
        return "geometry";
      case 20:
        return "isActive";
      case 21:
        return "isFullScreen";
      case 22:
        return "isVisible";
      case 23:
        return "mapFromGlobal";
      case 24:
        return "mapToGlobal";
      case 25:
        return "maxSize";
      case 26:
        return "minSize";
      case 27:
        return "resize";
      case 28:
        return "setFramePosition";
      case 29:
        return "setGeometry";
      case 30:
        return "setVisible";
      case 31:
        return "supportsHonouringLayoutMinSize";
    }
    throw Error();
  }

  void registerCallbacks() {
    assert(thisCpp != null);
    final RegisterMethodIsReimplementedCallback registerCallback = _dylib
        .lookup<ffi.NativeFunction<RegisterMethodIsReimplementedCallback_FFI>>(
            'c_KDDockWidgets__flutter__Window__registerVirtualMethodCallback')
        .asFunction();
    final callback16 = ffi.Pointer.fromFunction<void_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.destroy_calledFromC);
    registerCallback(thisCpp, callback16, 16);
    final callback17 = ffi.Pointer.fromFunction<voidstar_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.frameGeometry_calledFromC);
    registerCallback(thisCpp, callback17, 17);
    final callback18 =
        ffi.Pointer.fromFunction<voidstar_Func_voidstar_voidstar_FFI>(
            KDDWBindingsFlutter.Window.fromNativePixels_calledFromC);
    registerCallback(thisCpp, callback18, 18);
    final callback19 = ffi.Pointer.fromFunction<voidstar_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.geometry_calledFromC);
    registerCallback(thisCpp, callback19, 19);
    const callbackExcept20 = 0;
    final callback20 = ffi.Pointer.fromFunction<bool_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.isActive_calledFromC, callbackExcept20);
    registerCallback(thisCpp, callback20, 20);
    const callbackExcept21 = 0;
    final callback21 = ffi.Pointer.fromFunction<bool_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.isFullScreen_calledFromC, callbackExcept21);
    registerCallback(thisCpp, callback21, 21);
    const callbackExcept22 = 0;
    final callback22 = ffi.Pointer.fromFunction<bool_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.isVisible_calledFromC, callbackExcept22);
    registerCallback(thisCpp, callback22, 22);
    final callback23 =
        ffi.Pointer.fromFunction<voidstar_Func_voidstar_voidstar_FFI>(
            KDDWBindingsFlutter.Window.mapFromGlobal_calledFromC);
    registerCallback(thisCpp, callback23, 23);
    final callback24 =
        ffi.Pointer.fromFunction<voidstar_Func_voidstar_voidstar_FFI>(
            KDDWBindingsFlutter.Window.mapToGlobal_calledFromC);
    registerCallback(thisCpp, callback24, 24);
    final callback25 = ffi.Pointer.fromFunction<voidstar_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.maxSize_calledFromC);
    registerCallback(thisCpp, callback25, 25);
    final callback26 = ffi.Pointer.fromFunction<voidstar_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.minSize_calledFromC);
    registerCallback(thisCpp, callback26, 26);
    final callback27 =
        ffi.Pointer.fromFunction<void_Func_voidstar_ffi_Int32_ffi_Int32_FFI>(
            KDDWBindingsFlutter.Window.resize_calledFromC);
    registerCallback(thisCpp, callback27, 27);
    final callback28 =
        ffi.Pointer.fromFunction<void_Func_voidstar_voidstar_FFI>(
            KDDWBindingsFlutter.Window.setFramePosition_calledFromC);
    registerCallback(thisCpp, callback28, 28);
    final callback29 =
        ffi.Pointer.fromFunction<void_Func_voidstar_voidstar_FFI>(
            KDDWBindingsFlutter.Window.setGeometry_calledFromC);
    registerCallback(thisCpp, callback29, 29);
    final callback30 =
        ffi.Pointer.fromFunction<void_Func_voidstar_ffi_Int8_FFI>(
            KDDWBindingsFlutter.Window.setVisible_calledFromC);
    registerCallback(thisCpp, callback30, 30);
    const callbackExcept31 = 0;
    final callback31 = ffi.Pointer.fromFunction<bool_Func_voidstar_FFI>(
        KDDWBindingsFlutter.Window.supportsHonouringLayoutMinSize_calledFromC,
        callbackExcept31);
    registerCallback(thisCpp, callback31, 31);
  }
}
