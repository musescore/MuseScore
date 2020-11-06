// Copyright (c) 2014 Google Inc.
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

// microdump_stackwalk.cc: Process a microdump with MicrodumpProcessor, printing
// the results, including stack traces.

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

#include "common/path_helper.h"
#include "common/scoped_ptr.h"
#include "common/using_std_string.h"
#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/microdump.h"
#include "google_breakpad/processor/microdump_processor.h"
#include "google_breakpad/processor/process_state.h"
#include "google_breakpad/processor/stack_frame_symbolizer.h"
#include "processor/logging.h"
#include "processor/simple_symbol_supplier.h"
#include "processor/stackwalk_common.h"


namespace {

struct Options {
  bool machine_readable;
  bool output_stack_contents;

  string microdump_file;
  std::vector<string> symbol_paths;
};

using google_breakpad::BasicSourceLineResolver;
using google_breakpad::Microdump;
using google_breakpad::MicrodumpProcessor;
using google_breakpad::ProcessResult;
using google_breakpad::ProcessState;
using google_breakpad::scoped_ptr;
using google_breakpad::SimpleSymbolSupplier;
using google_breakpad::StackFrameSymbolizer;

// Processes |options.microdump_file| using
// MicrodumpProcessor. |options.symbol_path|, if non-empty, is the
// base directory of a symbol storage area, laid out in the format
// required by SimpleSymbolSupplier.  If such a storage area is
// specified, it is made available for use by the MicrodumpProcessor.
//
// Returns the value of MicrodumpProcessor::Process. If processing succeeds,
// prints identifying OS and CPU information from the microdump, crash
// information and call stacks for the crashing thread.
// All information is printed to stdout.
int PrintMicrodumpProcess(const Options& options) {
  std::ifstream file_stream(options.microdump_file);
  std::vector<char> bytes;
  file_stream.seekg(0, std::ios_base::end);
  bytes.resize(file_stream.tellg());
  if (bytes.empty()) {
    BPLOG(ERROR) << "Microdump is empty.";
    return 1;
  }
  file_stream.seekg(0, std::ios_base::beg);
  file_stream.read(&bytes[0], bytes.size());
  string microdump_content(&bytes[0], bytes.size());

  scoped_ptr<SimpleSymbolSupplier> symbol_supplier;
  if (!options.symbol_paths.empty()) {
    symbol_supplier.reset(new SimpleSymbolSupplier(options.symbol_paths));
  }

  BasicSourceLineResolver resolver;
  StackFrameSymbolizer frame_symbolizer(symbol_supplier.get(), &resolver);
  ProcessState process_state;
  MicrodumpProcessor microdump_processor(&frame_symbolizer);
  Microdump microdump(microdump_content);
  ProcessResult res = microdump_processor.Process(&microdump,
                                                  &process_state);

  if (res == google_breakpad::PROCESS_OK) {
    if (options.machine_readable) {
      PrintProcessStateMachineReadable(process_state);
    } else {
      PrintProcessState(process_state, options.output_stack_contents, &resolver);
    }
    return 0;
  }

  BPLOG(ERROR) << "MicrodumpProcessor::Process failed (code = " << res << ")";
  return 1;
}

}  // namespace

static void Usage(int argc, const char *argv[], bool error) {
  fprintf(error ? stderr : stdout,
          "Usage: %s [options] <microdump-file> [symbol-path ...]\n"
          "\n"
          "Output a stack trace for the provided microdump\n"
          "\n"
          "Options:\n"
          "\n"
          "  -m         Output in machine-readable format\n"
          "  -s         Output stack contents\n",
          google_breakpad::BaseName(argv[0]).c_str());
}

static void SetupOptions(int argc, const char *argv[], Options* options) {
  int ch;

  options->machine_readable = false;
  options->output_stack_contents = false;

  while ((ch = getopt(argc, (char * const*)argv, "hms")) != -1) {
    switch (ch) {
      case 'h':
        Usage(argc, argv, false);
        exit(0);
        break;

      case 'm':
        options->machine_readable = true;
        break;
      case 's':
        options->output_stack_contents = true;
        break;

      case '?':
        Usage(argc, argv, true);
        exit(1);
        break;
    }
  }

  if ((argc - optind) == 0) {
    fprintf(stderr, "%s: Missing microdump file\n", argv[0]);
    Usage(argc, argv, true);
    exit(1);
  }

  options->microdump_file = argv[optind];

  for (int argi = optind + 1; argi < argc; ++argi)
    options->symbol_paths.push_back(argv[argi]);
}

int main(int argc, const char* argv[]) {
  Options options;
  SetupOptions(argc, argv, &options);

  return PrintMicrodumpProcess(options);
}
