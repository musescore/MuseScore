/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

// A script that runs the tests but first ensures the bundle is built

import 'dart:io';
import 'src/flutter/utils.dart';

bool isAOT = false;
bool isASAN = false;
bool isLSAN = false;
bool useGDB = false;
bool ubsanPrintStacks = false;

String kddwSourceDir() {
  return Platform.script.path.replaceAll("run_flutter_tests.dart", "");
}

Future<int> runTests(String? singleTestName, List<String> singleTestArgs,
    String buildDir) async {
  if (!await Directory(buildDir).exists()) {
    final presetName = buildDir
        .replaceAll(kddwSourceDir(), "")
        .replaceAll("build-", "")
        .replaceAll("/", "");
    print("ERROR: Could not find $buildDir.\n"
        "Be sure to build the preset: $presetName");
    return -1;
  }

  if (singleTestName != null &&
      !await File("$buildDir/bin/$singleTestName").exists()) {
    print("ERROR: Could not find executable $buildDir/bin/$singleTestName\n");
    return -1;
  }

  /// Make sure the C++ is built:
  int result = await runCommand("ninja", [], workingDirectory: buildDir);
  if (result != 0) {
    print("Failed to build C++");
    return result;
  }

  /// Make sure the flutter bundle is built:
  final flutterEmbedderDir = "${kddwSourceDir()}/tests/flutter_tests_embedder/";
  result = await runCommand(
      "flutter", ["build", "bundle", "--suppress-analytics"],
      workingDirectory: flutterEmbedderDir);

  if (result != 0) {
    print("Failed to build bundle");
    return result;
  }

  if (isAOT) {
    result = await runCommand(
        "flutter", ["build", "linux", "--release", "--suppress-analytics"],
        workingDirectory: flutterEmbedderDir);

    if (result != 0) {
      print("Failed to build AOT snapshot (libapp.so)");
      return result;
    }
  }

  final String aotValue = isAOT ? "1" : "0";
  final String lsanValue = isLSAN ? "1" : "0";
  final String asanOptions = "detect_leaks=$lsanValue";
  final String ubsanOptions = ubsanPrintStacks ? "print_stacktrace=1" : "";
  final String gensnapshotOptions = "--no-strip";
  final String libraryPath = "$buildDir/lib/";

  print("export KDDW_FLUTTER_TESTS_USE_AOT=$aotValue");
  print("export ASAN_OPTIONS=$asanOptions");
  if (ubsanPrintStacks) print("export UBSAN_OPTIONS=$ubsanOptions");
  print("export DARTAGNAN_BINDINGSLIB_PATH=$libraryPath");
  print("\n");

  final env = {
    "KDDW_FLUTTER_TESTS_USE_AOT": aotValue,
    "ASAN_OPTIONS": asanOptions,
    "EXTRA_GEN_SNAPSHOT_OPTIONS": gensnapshotOptions,
    if (ubsanPrintStacks) "UBSAN_OPTIONS": ubsanOptions,
    "DARTAGNAN_BINDINGSLIB_PATH": libraryPath
  };

  /// Now we can run the tests:

  if (singleTestName == null) {
    // Run everything:
    return await runCommand("ctest", ["-j5"],
        workingDirectory: buildDir, env: env);
  } else {
    // Run a single test:
    final String executableName = useGDB ? "gdb" : "bin/$singleTestName";
    List<String> args = [
      // "--disable-service-auth-codes",
      // "--verbose-logging",
      // "--start-paused"
      if (useGDB) ...[
        "-ex=run",
        "--args",
        "bin/$singleTestName",
        ...singleTestArgs
      ] else
        ...singleTestArgs
    ];
    return await runCommand(executableName, args,
        workingDirectory: buildDir, env: env);
  }
}

void printUsage() {
  print(
      "Usage: dart run_flutter_tests.dart [--aot] [--asan] [--lsan] [--ubsan-stacktraces]");
  print("Or specify a single test to run:");
  print(
      "dart run_flutter_tests.dart [--aot] [--asan] [--lsan] [--ubsan-stacktraces] [--gdb] <test_name> [args]");
  print("Use asan_symbolize.py if --lsan isn't symbolizing libapp.so");
}

String calculateBuildDir() {
  String result;

  if (isAOT) {
    result = "build-dev-flutter-aot";
  } else {
    result = "build-dev-flutter";
  }

  if (isASAN) result += "-asan";

  print("Using $result");
  return "${kddwSourceDir()}${result}/";
}

Future<void> main(List<String> args) async {
  final _args = List<String>.from(args);

  isLSAN = _args.remove("--lsan");
  isASAN = _args.remove("--asan") || isLSAN;
  isAOT =
      _args.remove("--aot") || isLSAN; // LSAN requires AOT for simbolization
  useGDB = _args.remove("--gdb");
  ubsanPrintStacks = _args.remove("--ubsan-stacktraces");
  final bool isHelp = _args.remove("--help") || _args.remove("-h");

  if (isHelp) {
    printUsage();
    exit(0);
  }

  if (ubsanPrintStacks && !isASAN) {
    print("ERROR: --ubsan-stacktraces requires --asan");
    exit(1);
  }

  final String? singleTestName = _args.isEmpty ? null : _args.first;
  List<String> singleTestArgs = _args.isEmpty ? [] : _args.sublist(1);

  final result =
      await runTests(singleTestName, singleTestArgs, calculateBuildDir());
  final bool isSuccess = result == 0;
  if (isSuccess)
    print("SUCCESS!");
  else
    print("ERROR!");

  if (isLSAN && !isAOT && !isSuccess) {
    print("\nConsider using LSAN with AOT so you can get symbolized traces");
  }
  exit(result);
}
