/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'dart:io';
import 'dart:convert';

/// Path list separator (for PATH env var)
final String s_listSeparator = Platform.isWindows ? ";" : ":";

/// The name of the env var for runtime library path
final String s_libraryPathEnvVariable = Platform.isWindows
    ? "PATH"
    : Platform.isLinux
        ? "LD_LIBRARY_PATH"
        : Platform.isMacOS
            ? "DYLD_LIBRARY_PATH"
            : "";

final String s_qtIncludePath = qtIncludePath();
final String s_qtLibraryPath = qtLibPath();

/**
 * Returns the library filename according to the operating system.
 */
String normalizedLibName(String name) {
  if (Platform.isWindows) return "${name}.dll";
  if (Platform.isMacOS) return "lib${name}.dylib";

  return "lib${name}.so";
}

/**
 * Runs the specified command. Pipes its output to this script's stdout/err.
 */
Future<int> runCommand(String command, List<String> args,
    {String? workingDirectory,
    Map<String, String>? env,
    bool printOnly = false,
    bool silent = false,
    bool useGDB = false,
    bool forwardStdOut = true,
    bool forwardStdErr = true}) async {
  if (workingDirectory != null && !silent)
    print("Running: cd ${workingDirectory}");

  if (useGDB) {
    args.insertAll(0, ["-ex=run", "--args", command]);
    command = "gdb";
  }

  if (!silent) print("Running: ${command} ${args.join(" ")}");
  if (printOnly) return 0;

  Process process = await Process.start(command, args,
          workingDirectory: workingDirectory, environment: env)
      .then((process) {
    if (forwardStdErr) stderr.addStream(process.stderr);
    if (forwardStdOut) stdout.addStream(process.stdout);
    if (useGDB) process.stdin.addStream(stdin);

    return process;
  });

  return await process.exitCode;
}

Future<int> runDartFormat(String folder, {bool printOnly = false}) async {
  if (Platform.isWindows) {
    return runCommand('cmd', ["/C", 'dart', 'format', '--fix', folder],
        printOnly: printOnly);
  } else {
    return runCommand("dart", ['format', '--fix', folder],
        printOnly: printOnly);
  }
}

Future<int> runClangFormat(String folder) async {
  final String clangFormat =
      Platform.environment['DARTAGNAN_CLANG_FORMAT'] ?? "clang-format";

  var dir = Directory(folder);
  final fileEntries = dir.listSync().where((element) =>
      element.path.endsWith('.h') || element.path.endsWith('.cpp'));

  final List<String> files = fileEntries.map((e) => e.path.toString()).toList();
  final List<String> arguments = ["-i"] + files;

  print("Running ${clangFormat}...");
  return runCommand(clangFormat, arguments, silent: true);
}

/**
 * Runs "dart pub get"
 */
Future<int> getPackages(
    {required String workingDirectory, bool printOnly = false}) async {
  if (File("${workingDirectory}/.packages").existsSync()) {
    // No need to get packages
    return 0;
  }

  return runCommand(dartBinary(), ["pub", "get"],
      workingDirectory: workingDirectory, printOnly: printOnly);
}

/**
 * Runs qmake -query
 */
String qmakeQuery(String key) {
  ProcessResult result = Process.runSync('qmake', ['-query']);

  for (String line in result.stdout.toString().split("\n")) {
    if (line.startsWith('${key}:')) {
      line = line.replaceAll('${key}:', '');
      line = line.trim();
      return line;
    }
  }

  return "";
}

String qtIncludePath() {
  return qmakeQuery('QT_INSTALL_HEADERS');
}

String qtLibPath() {
  return qmakeQuery('QT_INSTALL_LIBS');
}

List<String> includeArguments() {
  if (Platform.isMacOS) {
    return [
      "-I${s_qtLibraryPath}/QtCore.framework/Headers",
      "-I${s_qtLibraryPath}/QtGui.framework/Headers",
      "-F${s_qtLibraryPath}",
      "-I..",
      "-Iviews/"
    ];
  } else {
    return [
      "-I${s_qtIncludePath}",
      "-I${s_qtIncludePath}/QtCore",
      "-I${s_qtIncludePath}/QtGui",
      "-I..",
      "-I../3rdparty",
      "-I../fwd_headers",
      "-Iviews/"
    ];
  }
}

enum DartBinaryType {
  Normal, // Usually /usr/bin/dart or so
  ASAN // Something you compiled without tcmalloc, otherwise it's not asan compatible
}

String dartBinary([DartBinaryType type = DartBinaryType.Normal]) {
  switch (type) {
    case DartBinaryType.Normal:
      return "dart";
    case DartBinaryType.ASAN:
      final path = Platform.environment['DART_FOR_ASAN'] ?? "";

      if (path.isEmpty) {
        print(
            "If you want to use ASAN, you'll need to point the DART_FOR_ASAN env variable to your dart binary which you compiled without tcmalloc support");
      }

      return path;
  }
}

class TimeStamp {
  final String filename;
  final String _originalContents;
  final String originalTimeStamp;

  static Future<TimeStamp> create(String filename) async {
    final String timestamp = await getTimeStamp(filename);
    return TimeStamp._ctor(filename, timestamp);
  }

  TimeStamp._ctor(this.filename, this.originalTimeStamp)
      : _originalContents = File(filename).readAsStringSync() {}

  /// @brief Returns whether this file has a different contents now
  bool wasModified() {
    return File(filename).readAsStringSync() != _originalContents;
  }

  static Future<String> getTimeStamp(String filename) async {
    Process process = await Process.start("stat", ['-c', '%y', filename]);

    String result = "";
    await for (var line in process.stdout.transform(utf8.decoder)) {
      result += line;
    }
    await process.exitCode;
    return result.trim();
  }

  Future<bool> restoreTimeStamp() async {
    return await runCommand("touch", ['-d', '${originalTimeStamp}', filename],
            silent: true) ==
        0;
  }
}

class TimeStamps {
  final String path;
  var _timestamps = <TimeStamp>[];

  TimeStamps(this.path) {}

  void create() async {
    final dir = Directory(path);

    dir.list(recursive: true).forEach((entry) async {
      if (entry.path.endsWith('.cpp') || entry.path.endsWith('.h')) {
        _timestamps.add(await TimeStamp.create(entry.path));
      }
    });
  }

  /// @brief Returns the list of files which have a different contents now
  List<TimeStamp> modifiedFiles() {
    return _timestamps.where((t) => t.wasModified()).toList();
  }

  /// @brief Returns the list of files which still have the same contents
  List<TimeStamp> unmodifiedFiles() {
    return _timestamps.where((t) => !t.wasModified()).toList();
  }

  /// @brief restores the timestams of all files which weren't modified
  Future<bool> restoreTimeStamps() async {
    for (final file in unmodifiedFiles()) {
      if (!await file.restoreTimeStamp()) {
        return false;
      }
    }

    return true;
  }
}
