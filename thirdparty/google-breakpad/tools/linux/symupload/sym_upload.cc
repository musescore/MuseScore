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

// symupload.cc: Upload a symbol file to a HTTP server.  The upload is sent as
// a multipart/form-data POST request with the following parameters:
//  code_file: the basename of the module, e.g. "app"
//  debug_file: the basename of the debugging file, e.g. "app"
//  debug_identifier: the debug file's identifier, usually consisting of
//                    the guid and age embedded in the pdb, e.g.
//                    "11111111BBBB3333DDDD555555555555F"
//  version: the file version of the module, e.g. "1.2.3.4"
//  os: the operating system that the module was built for
//  cpu: the CPU that the module was built for
//  symbol_file: the contents of the breakpad-format symbol file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locale>

#include "common/linux/symbol_upload.h"

using google_breakpad::sym_upload::UploadProtocol;
using google_breakpad::sym_upload::Options;

static void StrToUpper(std::string* str) {
  if (str == nullptr) {
    fprintf(stderr, "nullptr passed to StrToUpper.\n");
    exit(1);
  }
  for (size_t i = 0; i < str->length(); i++) {
    (*str)[i] = std::toupper((*str)[i], std::locale::classic());
  }
}

//=============================================================================
static void
Usage(int argc, const char *argv[]) {
  fprintf(stderr, "Submit symbol information.\n");
  fprintf(stderr, "Usage: %s [options...] <symbol-file> <upload-URL>\n",
      argv[0]);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "<symbol-file> should be created by using the dump_syms"
      "tool.\n");
  fprintf(stderr, "<upload-URL> is the destination for the upload\n");
  fprintf(stderr, "-p:\t <protocol> One of ['sym-upload-v1',"
    " 'sym-upload-v2'], defaults to 'sym-upload-v1'.\n");
  fprintf(stderr, "-v:\t Version information (e.g., 1.2.3.4)\n");
  fprintf(stderr, "-x:\t <host[:port]> Use HTTP proxy on given port\n");
  fprintf(stderr, "-u:\t <user[:password]> Set proxy user and password\n");
  fprintf(stderr, "-h:\t Usage\n");
  fprintf(stderr, "-?:\t Usage\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "These options only work with 'sym-upload-v2' protocol:\n");
  fprintf(stderr, "-k:\t <API-key> A secret used to authenticate with the"
      " API.\n");
  fprintf(stderr, "-f:\t Force symbol upload if already exists.\n");
  fprintf(stderr, "-t:\t <symbol-type> Explicitly set symbol upload type ("
      "default is 'breakpad').\n"
      "\t One of ['breakpad', 'elf', 'pe', 'macho', 'debug_only', 'dwp', "
      "'dsym', 'pdb'].\n"
      "\t Note: When this flag is set to anything other than 'breakpad', then "
      "the '-c' and '-i' flags must also be set.\n");
  fprintf(stderr, "-c:\t <code-file> Explicitly set 'code_file' for symbol "
      "upload (basename of executable).\n");
  fprintf(stderr, "-i:\t <debug-id> Explicitly set 'debug_id' for symbol "
      "upload (typically build ID of executable).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "  With 'sym-upload-v1':\n");
  fprintf(stderr, "    %s path/to/symbol_file http://myuploadserver\n",
      argv[0]);
  fprintf(stderr, "  With 'sym-upload-v2':\n");
  fprintf(stderr, "    [Defaulting to symbol type 'BREAKPAD']\n");
  fprintf(stderr, "    %s -p sym-upload-v2 -k mysecret123! "
      "path/to/symbol_file http://myuploadserver\n", argv[0]);
  fprintf(stderr, "    [Explicitly set symbol type to 'elf']\n");
  fprintf(stderr, "    %s -p sym-upload-v2 -k mysecret123! -t elf "
      "-c app -i 11111111BBBB3333DDDD555555555555F "
      "path/to/symbol_file http://myuploadserver\n", argv[0]);
}

//=============================================================================
static void
SetupOptions(int argc, const char *argv[], Options *options) {
  extern int optind, optopt;
  int ch;
  constexpr char flag_pattern[] = "u:v:x:p:k:t:c:i:hf?";

  while ((ch = getopt(argc, (char * const*)argv, flag_pattern)) != -1) {
    switch (ch) {
      case 'h':
      case '?':
        Usage(argc, argv);
        // ch might be '?' because getopt found an error while parsing args (as
        // opposed to finding "-?" as an arg), in which case optopt is set to
        // the bad arg value, so return an error code if optopt is set,
        // otherwise exit cleanly.
        exit(optopt == 0 ? 0 : 1);
        break;
      case 'u':
        options->proxy_user_pwd = optarg;
        break;
      case 'v':
        options->version = optarg;
        break;
      case 'x':
        options->proxy = optarg;
        break;
      case 'p':
        if (strcmp(optarg, "sym-upload-v2") == 0) {
          options->upload_protocol = UploadProtocol::SYM_UPLOAD_V2;
        } else if (strcmp(optarg, "sym-upload-v1") == 0) {
          options->upload_protocol = UploadProtocol::SYM_UPLOAD_V1;
        } else {
          fprintf(stderr, "Invalid protocol '%s'\n", optarg);
          Usage(argc, argv);
          exit(1);
        }
        break;
      case 'k':
        options->api_key = optarg;
        break;
      case 't': {
        // This is really an enum, so treat as upper-case for consistency with
        // enum naming convention on server-side.
        options->type = optarg;
        StrToUpper(&(options->type));
        break;
      }
      case 'c':
        options->code_file = optarg;
        break;
      case 'i':
        options->debug_id = optarg;
        break;
      case 'f':
        options->force = true;
        break;

      default:
        fprintf(stderr, "Invalid option '%c'\n", ch);
        Usage(argc, argv);
        exit(1);
        break;
    }
  }

  if ((argc - optind) != 2) {
    fprintf(stderr, "%s: Missing symbols file and/or upload-URL\n", argv[0]);
    Usage(argc, argv);
    exit(1);
  }

  bool is_breakpad_upload = options->type.empty() ||
      options->type == google_breakpad::sym_upload::kBreakpadSymbolType;
  bool has_code_file = !options->code_file.empty();
  bool has_debug_id = !options->debug_id.empty();
  if (is_breakpad_upload && (has_code_file || has_debug_id)) {
    fprintf(stderr, "\n");
    fprintf(stderr, "%s: -c and -i should only be specified for non-breakpad "
        "symbol upload types.\n", argv[0]);
    fprintf(stderr, "\n");
    Usage(argc, argv);
    exit(1);
  }
  if (!is_breakpad_upload && (!has_code_file || !has_debug_id)) {
    fprintf(stderr, "\n");
    fprintf(stderr, "%s: -c and -i must be specified for non-breakpad "
        "symbol upload types.\n", argv[0]);
    fprintf(stderr, "\n");
    Usage(argc, argv);
    exit(1);
  }

  options->symbolsPath = argv[optind];
  options->uploadURLStr = argv[optind + 1];
}

//=============================================================================
int main(int argc, const char* argv[]) {
  Options options;
  SetupOptions(argc, argv, &options);
  google_breakpad::sym_upload::Start(&options);
  return options.success ? 0 : 1;
}
