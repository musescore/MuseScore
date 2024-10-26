/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/// This helper script generates the Dart bindings for KDDockWidgets

import 'dart:io';
import 'utils.dart' as Utils;
import 'package:path/path.dart';

final String s_sourcePath = dirname(Platform.script.toFilePath());

Future<int> runCommand(String command, List<String> args,
    {String? workingDirectory = null, Map<String, String>? env = null}) async {
  print("Running: ${command} ${args.join(" ")}");

  Process process = await Process.start(command, args,
          workingDirectory: workingDirectory, environment: env)
      .then((process) {
    stderr.addStream(process.stderr);
    stdout.addStream(process.stdout);

    return process;
  });

  return await process.exitCode;
}

String generatedDir() {
  return s_sourcePath + "/generated";
}

String generatedFFICpp() {
  return generatedDir() + "/" + "KDDockWidgetsBindings/dart/ffi";
}

List<String> qtIncludes() {
  return Utils.includeArguments();
}

void main(List<String> args_) async {
  int res = await runCommand(
      "dartagnan",
      ["--output-directory=${generatedDir()}", "--license-file=license-file-template"] +
          qtIncludes() +
          ["bindings_global.h", "typesystem.xml"]);

  if (res != 0) {
    print("Error running dartagnan!");
    exit(res);
  }

  /// If dartagnan already ran, then the tests don't need to
  final bool runCodeFormat =
      !Platform.environment.containsKey('DARTAGNAN_RUNS_CODE_FORMAT');

  if (runCodeFormat) {
    res = await Utils.runDartFormat(generatedDir());
    if (res != 0) {
      print("Error running dartfmt!");
      exit(res);
    }

    res = await Utils.runClangFormat(generatedFFICpp());
    if (res != 0) {
      print("Error running clang-format!");
      exit(res);
    }
  }
}
