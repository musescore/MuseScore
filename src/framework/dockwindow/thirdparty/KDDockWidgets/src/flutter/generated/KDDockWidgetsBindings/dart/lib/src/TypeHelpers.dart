/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
// tag=1053

typedef void_Func_voidstar = void Function(ffi.Pointer<void>);
typedef void_Func_voidstar_FFI = ffi.Void Function(ffi.Pointer<void>);
typedef RegisterMethodIsReimplementedCallback = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef RegisterMethodIsReimplementedCallback_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef SignalHandler = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef SignalHandler_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef voidstar_Func_void = ffi.Pointer<void> Function();
typedef voidstar_Func_void_FFI = ffi.Pointer<void> Function();
typedef voidstar_Func_string = ffi.Pointer<void> Function(ffi.Pointer<Utf8>);
typedef voidstar_Func_string_FFI = ffi.Pointer<void> Function(
    ffi.Pointer<Utf8>);
typedef string_Func_voidstar = ffi.Pointer<Utf8> Function(ffi.Pointer<void>);
typedef string_Func_voidstar_FFI = ffi.Pointer<Utf8> Function(
    ffi.Pointer<void>);
typedef bool_Func_voidstar = int Function(ffi.Pointer<void>);
typedef bool_Func_voidstar_FFI = ffi.Int8 Function(ffi.Pointer<void>);
typedef bool_Func_double_double_double = int Function(double, double, double);
typedef bool_Func_ffi_Double_ffi_Double_ffi_Double_FFI = ffi.Int8 Function(
    ffi.Double, ffi.Double, ffi.Double);
typedef void_Func_int = void Function(int);
typedef void_Func_ffi_Int32_FFI = ffi.Void Function(ffi.Int32);
typedef string_Func_void = ffi.Pointer<Utf8> Function();
typedef string_Func_void_FFI = ffi.Pointer<Utf8> Function();
typedef voidstar_Func_voidstar = ffi.Pointer<void> Function(ffi.Pointer<void>);
typedef voidstar_Func_voidstar_FFI = ffi.Pointer<void> Function(
    ffi.Pointer<void>);
typedef voidstar_Func_voidstar_voidstar = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef voidstar_Func_voidstar_voidstar_FFI = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_int_int = void Function(ffi.Pointer<void>, int, int);
typedef void_Func_voidstar_ffi_Int32_ffi_Int32_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Int32, ffi.Int32);
typedef void_Func_voidstar_voidstar = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_bool = void Function(ffi.Pointer<void>, int);
typedef void_Func_voidstar_ffi_Int8_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Int8);
typedef voidstar_Func_int_int = ffi.Pointer<void> Function(int, int);
typedef voidstar_Func_ffi_Int32_ffi_Int32_FFI = ffi.Pointer<void> Function(
    ffi.Int32, ffi.Int32);
typedef int_Func_voidstar = int Function(ffi.Pointer<void>);
typedef int_Func_voidstar_FFI = ffi.Int32 Function(ffi.Pointer<void>);
typedef void_Func_voidstar_int = void Function(ffi.Pointer<void>, int);
typedef void_Func_voidstar_ffi_Int32_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Int32);
typedef voidstar_Func_int_int_int_int = ffi.Pointer<void> Function(
    int, int, int, int);
typedef voidstar_Func_ffi_Int32_ffi_Int32_ffi_Int32_ffi_Int32_FFI
    = ffi.Pointer<void> Function(ffi.Int32, ffi.Int32, ffi.Int32, ffi.Int32);
typedef void_Func_voidstar_int_int_int_int = void Function(
    ffi.Pointer<void>, int, int, int, int);
typedef void_Func_voidstar_ffi_Int32_ffi_Int32_ffi_Int32_ffi_Int32_FFI
    = ffi.Void Function(
        ffi.Pointer<void>, ffi.Int32, ffi.Int32, ffi.Int32, ffi.Int32);
typedef voidstar_Func_voidstar_int_int_int_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int, int, int, int);
typedef voidstar_Func_voidstar_ffi_Int32_ffi_Int32_ffi_Int32_ffi_Int32_FFI
    = ffi.Pointer<void> Function(
        ffi.Pointer<void>, ffi.Int32, ffi.Int32, ffi.Int32, ffi.Int32);
typedef bool_Func_voidstar_voidstar = int Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef bool_Func_void = int Function();
typedef bool_Func_void_FFI = ffi.Int8 Function();
typedef voidstar_Func_int = ffi.Pointer<void> Function(int);
typedef voidstar_Func_ffi_Int32_FFI = ffi.Pointer<void> Function(ffi.Int32);
typedef voidstar_Func_int_voidstar = ffi.Pointer<void> Function(
    int, ffi.Pointer<void>);
typedef voidstar_Func_ffi_Int32_voidstar_FFI = ffi.Pointer<void> Function(
    ffi.Int32, ffi.Pointer<void>);
typedef voidstar_Func_voidstar_voidstar_voidstar_int_voidstar_int
    = ffi.Pointer<void> Function(ffi.Pointer<void>, ffi.Pointer<void>,
        ffi.Pointer<void>, int, ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_voidstar_voidstar_ffi_Int32_voidstar_ffi_Int32_FFI
    = ffi.Pointer<void> Function(ffi.Pointer<void>, ffi.Pointer<void>,
        ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>, ffi.Int32);
typedef voidstar_Func_voidstar_voidstar_voidstar = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef voidstar_Func_voidstar_voidstar_voidstar_FFI = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_int_voidstar = void Function(
    ffi.Pointer<void>, int, ffi.Pointer<void>);
typedef void_Func_voidstar_ffi_Int32_voidstar_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>);
typedef int_Func_voidstar_voidstar = int Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef int_Func_voidstar_voidstar_FFI = ffi.Int32 Function(
    ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_voidstar = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_voidstar_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_int_bool = void Function(
    ffi.Pointer<void>, int, int);
typedef void_Func_voidstar_ffi_Int32_ffi_Int8_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Int32, ffi.Int8);
typedef bool_Func_voidstar_bool = int Function(ffi.Pointer<void>, int);
typedef bool_Func_voidstar_ffi_Int8_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Int8);
typedef voidstar_Func_voidstar_voidstar_int_int_int = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Pointer<void>, int, int, int);
typedef voidstar_Func_voidstar_voidstar_ffi_Int32_ffi_Int32_ffi_Int32_FFI
    = ffi.Pointer<void> Function(
        ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32, ffi.Int32, ffi.Int32);
typedef voidstar_Func_voidstar_voidstar_voidstar_int = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_voidstar_voidstar_ffi_Int32_FFI
    = ffi.Pointer<void> Function(
        ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef int_Func_void = int Function();
typedef int_Func_void_FFI = ffi.Int32 Function();
typedef void_Func_bool = void Function(int);
typedef void_Func_ffi_Int8_FFI = ffi.Void Function(ffi.Int8);
typedef voidstar_Func_voidstar_int_bool = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int, int);
typedef voidstar_Func_voidstar_ffi_Int32_ffi_Int8_FFI = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Int32, ffi.Int8);
typedef voidstar_Func_voidstar_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_ffi_Int32_FFI = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Int32);
typedef bool_Func_voidstar_int_int = int Function(ffi.Pointer<void>, int, int);
typedef bool_Func_voidstar_ffi_Int32_ffi_Int32_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Int32, ffi.Int32);
typedef void_Func_voidstar_double = void Function(ffi.Pointer<void>, double);
typedef void_Func_voidstar_ffi_Double_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Double);
typedef voidstar_Func_voidstar_int_voidstar_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int, ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_ffi_Int32_voidstar_ffi_Int32_FFI
    = ffi.Pointer<void> Function(
        ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>, ffi.Int32);
typedef void_Func_voidstar_int_voidstar_voidstar_bool = void Function(
    ffi.Pointer<void>, int, ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef void_Func_voidstar_ffi_Int32_voidstar_voidstar_ffi_Int8_FFI
    = ffi.Void Function(ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>,
        ffi.Pointer<void>, ffi.Int8);
typedef voidstar_Func_voidstar_int_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int, int);
typedef voidstar_Func_voidstar_ffi_Int32_ffi_Int32_FFI = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Int32, ffi.Int32);
typedef bool_Func_voidstar_voidstar_voidstar = int Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_voidstar_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_int = int Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef bool_Func_voidstar_voidstar_ffi_Int32_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef voidstar_Func_voidstar_voidstar_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_voidstar_ffi_Int32_FFI = ffi.Pointer<void>
    Function(ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef void_Func_voidstar_voidstar_int_voidstar_voidstar = void Function(
    ffi.Pointer<void>,
    ffi.Pointer<void>,
    int,
    ffi.Pointer<void>,
    ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_ffi_Int32_voidstar_voidstar_FFI
    = ffi.Void Function(ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32,
        ffi.Pointer<void>, ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_int_voidstar = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int, ffi.Pointer<void>);
typedef void_Func_voidstar_voidstar_ffi_Int32_voidstar_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>);
typedef voidstar_Func_voidstar_bool = ffi.Pointer<void> Function(
    ffi.Pointer<void>, int);
typedef voidstar_Func_voidstar_ffi_Int8_FFI = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Int8);
typedef void_Func_voidstar_voidstar_voidstar_int = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef void_Func_voidstar_voidstar_voidstar_ffi_Int32_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef bool_Func_voidstar_voidstar_int_voidstar = int Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int, ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_ffi_Int32_voidstar_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32, ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_int_voidstar_voidstar = int Function(
    ffi.Pointer<void>,
    ffi.Pointer<void>,
    int,
    ffi.Pointer<void>,
    ffi.Pointer<void>);
typedef bool_Func_voidstar_voidstar_ffi_Int32_voidstar_voidstar_FFI
    = ffi.Int8 Function(ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32,
        ffi.Pointer<void>, ffi.Pointer<void>);
typedef bool_Func_voidstar_int = int Function(ffi.Pointer<void>, int);
typedef bool_Func_voidstar_ffi_Int32_FFI = ffi.Int8 Function(
    ffi.Pointer<void>, ffi.Int32);
typedef void_Func_voidstar_voidstar_int = void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int);
typedef void_Func_voidstar_voidstar_ffi_Int32_FFI = ffi.Void Function(
    ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32);
typedef int_Func_int = int Function(int);
typedef int_Func_ffi_Int32_FFI = ffi.Int32 Function(ffi.Int32);
typedef voidstar_Func_voidstar_voidstar_int_int = ffi.Pointer<void> Function(
    ffi.Pointer<void>, ffi.Pointer<void>, int, int);
typedef voidstar_Func_voidstar_voidstar_ffi_Int32_ffi_Int32_FFI
    = ffi.Pointer<void> Function(
        ffi.Pointer<void>, ffi.Pointer<void>, ffi.Int32, ffi.Int32);
typedef bool_Func_voidstar_int_int_voidstar_int = int Function(
    ffi.Pointer<void>, int, int, ffi.Pointer<void>, int);
typedef bool_Func_voidstar_ffi_Int32_ffi_Int32_voidstar_ffi_Int32_FFI
    = ffi.Int8 Function(
        ffi.Pointer<void>, ffi.Int32, ffi.Int32, ffi.Pointer<void>, ffi.Int32);
typedef double_Func_voidstar = double Function(ffi.Pointer<void>);
typedef double_Func_voidstar_FFI = ffi.Double Function(ffi.Pointer<void>);
