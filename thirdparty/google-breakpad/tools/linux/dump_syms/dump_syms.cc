// Copyright (c) 2011, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <paths.h>
#include <stdio.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "common/linux/dump_symbols.h"

using google_breakpad::WriteSymbolFile;
using google_breakpad::WriteSymbolFileHeader;

int usage(const char* self) {
  fprintf(stderr, "Usage: %s [OPTION] <binary-with-debugging-info> "
          "[directories-for-debug-file]\n\n", self);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -i:         Output module header information only.\n");
  fprintf(stderr, "  -c          Do not generate CFI section\n");
  fprintf(stderr, "  -r          Do not handle inter-compilation "
                                 "unit references\n");
  fprintf(stderr, "  -v          Print all warnings to stderr\n");
  fprintf(stderr, "  -n <name>   Use specified name for name of the object\n");
  fprintf(stderr, "  -o <os>     Use specified name for the "
                                 "operating system\n");
  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2)
    return usage(argv[0]);
  bool header_only = false;
  bool cfi = true;
  bool handle_inter_cu_refs = true;
  bool log_to_stderr = false;
  std::string obj_name;
  const char* obj_os = "Linux";
  int arg_index = 1;
  while (arg_index < argc && strlen(argv[arg_index]) > 0 &&
         argv[arg_index][0] == '-') {
    if (strcmp("-i", argv[arg_index]) == 0) {
      header_only = true;
    } else if (strcmp("-c", argv[arg_index]) == 0) {
      cfi = false;
    } else if (strcmp("-r", argv[arg_index]) == 0) {
      handle_inter_cu_refs = false;
    } else if (strcmp("-v", argv[arg_index]) == 0) {
      log_to_stderr = true;
    } else if (strcmp("-n", argv[arg_index]) == 0) {
      if (arg_index + 1 >= argc) {
        fprintf(stderr, "Missing argument to -n\n");
        return usage(argv[0]);
      }
      obj_name = argv[arg_index + 1];
      ++arg_index;
    } else if (strcmp("-o", argv[arg_index]) == 0) {
      if (arg_index + 1 >= argc) {
        fprintf(stderr, "Missing argument to -o\n");
        return usage(argv[0]);
      }
      obj_os = argv[arg_index + 1];
      ++arg_index;
    } else {
      printf("2.4 %s\n", argv[arg_index]);
      return usage(argv[0]);
    }
    ++arg_index;
  }
  if (arg_index == argc)
    return usage(argv[0]);
  // Save stderr so it can be used below.
  FILE* saved_stderr = fdopen(dup(fileno(stderr)), "w");
  if (!log_to_stderr) {
    if (freopen(_PATH_DEVNULL, "w", stderr)) {
      // If it fails, not a lot we can (or should) do.
      // Add this brace section to silence gcc warnings.
    }
  }
  const char* binary;
  std::vector<string> debug_dirs;
  binary = argv[arg_index];
  for (int debug_dir_index = arg_index + 1;
       debug_dir_index < argc;
       ++debug_dir_index) {
    debug_dirs.push_back(argv[debug_dir_index]);
  }

  if (obj_name.empty())
    obj_name = binary;

  if (header_only) {
    if (!WriteSymbolFileHeader(binary, obj_name, obj_os, std::cout)) {
      fprintf(saved_stderr, "Failed to process file.\n");
      return 1;
    }
  } else {
    SymbolData symbol_data = cfi ? ALL_SYMBOL_DATA : NO_CFI;
    google_breakpad::DumpOptions options(symbol_data, handle_inter_cu_refs);
    if (!WriteSymbolFile(binary, obj_name, obj_os, debug_dirs, options,
                         std::cout)) {
      fprintf(saved_stderr, "Failed to write symbol file.\n");
      return 1;
    }
  }

  return 0;
}
