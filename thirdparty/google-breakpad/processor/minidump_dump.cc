// Copyright (c) 2006, Google Inc.
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

// minidump_dump.cc: Print the contents of a minidump file in somewhat
// readable text.
//
// Author: Mark Mentovai

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common/scoped_ptr.h"
#include "google_breakpad/processor/minidump.h"
#include "processor/logging.h"

namespace {

using google_breakpad::Minidump;
using google_breakpad::MinidumpThreadList;
using google_breakpad::MinidumpModuleList;
using google_breakpad::MinidumpMemoryInfoList;
using google_breakpad::MinidumpMemoryList;
using google_breakpad::MinidumpException;
using google_breakpad::MinidumpAssertion;
using google_breakpad::MinidumpSystemInfo;
using google_breakpad::MinidumpMiscInfo;
using google_breakpad::MinidumpBreakpadInfo;
using google_breakpad::MinidumpCrashpadInfo;

struct Options {
  Options()
      : minidumpPath(), hexdump(false), hexdump_width(16) {}

  string minidumpPath;
  bool hexdump;
  unsigned int hexdump_width;
};

static void DumpRawStream(Minidump *minidump,
                          uint32_t stream_type,
                          const char *stream_name,
                          int *errors) {
  uint32_t length = 0;
  if (!minidump->SeekToStreamType(stream_type, &length)) {
    return;
  }

  printf("Stream %s:\n", stream_name);

  if (length == 0) {
    printf("\n");
    return;
  }
  std::vector<char> contents(length);
  if (!minidump->ReadBytes(&contents[0], length)) {
    ++*errors;
    BPLOG(ERROR) << "minidump.ReadBytes failed";
    return;
  }
  size_t current_offset = 0;
  while (current_offset < length) {
    size_t remaining = length - current_offset;
    // Printf requires an int and direct casting from size_t results
    // in compatibility warnings.
    uint32_t int_remaining = remaining;
    printf("%.*s", int_remaining, &contents[current_offset]);
    char *next_null = reinterpret_cast<char*>(
        memchr(&contents[current_offset], 0, remaining));
    if (next_null == NULL)
      break;
    printf("\\0\n");
    size_t null_offset = next_null - &contents[0];
    current_offset = null_offset + 1;
  }
  printf("\n\n");
}

static bool PrintMinidumpDump(const Options& options) {
  Minidump minidump(options.minidumpPath,
                    options.hexdump);
  if (!minidump.Read()) {
    BPLOG(ERROR) << "minidump.Read() failed";
    return false;
  }
  minidump.Print();

  int errors = 0;

  MinidumpThreadList *thread_list = minidump.GetThreadList();
  if (!thread_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetThreadList() failed";
  } else {
    thread_list->Print();
  }

  // It's useful to be able to see the full list of modules here even if it
  // would cause minidump_stackwalk to fail.
  MinidumpModuleList::set_max_modules(UINT32_MAX);
  MinidumpModuleList *module_list = minidump.GetModuleList();
  if (!module_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetModuleList() failed";
  } else {
    module_list->Print();
  }

  MinidumpMemoryList *memory_list = minidump.GetMemoryList();
  if (!memory_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetMemoryList() failed";
  } else {
    memory_list->Print();
  }

  MinidumpException *exception = minidump.GetException();
  if (!exception) {
    BPLOG(INFO) << "minidump.GetException() failed";
  } else {
    exception->Print();
  }

  MinidumpAssertion *assertion = minidump.GetAssertion();
  if (!assertion) {
    BPLOG(INFO) << "minidump.GetAssertion() failed";
  } else {
    assertion->Print();
  }

  MinidumpSystemInfo *system_info = minidump.GetSystemInfo();
  if (!system_info) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetSystemInfo() failed";
  } else {
    system_info->Print();
  }

  MinidumpMiscInfo *misc_info = minidump.GetMiscInfo();
  if (!misc_info) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetMiscInfo() failed";
  } else {
    misc_info->Print();
  }

  MinidumpBreakpadInfo *breakpad_info = minidump.GetBreakpadInfo();
  if (!breakpad_info) {
    // Breakpad info is optional, so don't treat this as an error.
    BPLOG(INFO) << "minidump.GetBreakpadInfo() failed";
  } else {
    breakpad_info->Print();
  }

  MinidumpMemoryInfoList *memory_info_list = minidump.GetMemoryInfoList();
  if (!memory_info_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetMemoryInfoList() failed";
  } else {
    memory_info_list->Print();
  }

  MinidumpCrashpadInfo *crashpad_info = minidump.GetCrashpadInfo();
  if (crashpad_info) {
    // Crashpad info is optional, so don't treat absence as an error.
    crashpad_info->Print();
  }

  DumpRawStream(&minidump,
                MD_LINUX_CMD_LINE,
                "MD_LINUX_CMD_LINE",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_ENVIRON,
                "MD_LINUX_ENVIRON",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_LSB_RELEASE,
                "MD_LINUX_LSB_RELEASE",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_PROC_STATUS,
                "MD_LINUX_PROC_STATUS",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_CPU_INFO,
                "MD_LINUX_CPU_INFO",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_MAPS,
                "MD_LINUX_MAPS",
                &errors);

  return errors == 0;
}

//=============================================================================
static void
Usage(int argc, char *argv[], bool error) {
  FILE *fp = error ? stderr : stdout;

  fprintf(fp,
          "Usage: %s [options...] <minidump>\n"
          "Dump data in a minidump.\n"
          "\n"
          "Options:\n"
          "  <minidump> should be a minidump.\n"
          "  -x:\t Display memory in a hexdump like format\n"
          "  -h:\t Usage\n",
          argv[0]);
}

//=============================================================================
static void
SetupOptions(int argc, char *argv[], Options *options) {
  int ch;

  while ((ch = getopt(argc, (char * const*)argv, "xh")) != -1) {
    switch (ch) {
      case 'x':
        options->hexdump = true;
        break;
      case 'h':
        Usage(argc, argv, false);
        exit(0);

      default:
        Usage(argc, argv, true);
        exit(1);
        break;
    }
  }

  if ((argc - optind) != 1) {
    fprintf(stderr, "%s: Missing minidump file\n", argv[0]);
    exit(1);
  }

  options->minidumpPath = argv[optind];
}

}  // namespace

int main(int argc, char *argv[]) {
  Options options;
  BPLOG_INIT(&argc, &argv);
  SetupOptions(argc, argv, &options);
  return PrintMinidumpDump(options) ? 0 : 1;
}
