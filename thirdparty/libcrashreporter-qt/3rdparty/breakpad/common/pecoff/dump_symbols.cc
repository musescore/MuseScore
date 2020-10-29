// Copyright (c) 2011 Google Inc.
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

#include "common/pecoff/dump_symbols.h"

#include "common/pecoff/pecoffutils.h"
#include "common/pecoff/dump_symbols-inl.h"

namespace google_breakpad {

// Not explicitly exported, but not static so it can be used in unit tests.
bool ReadSymbolDataInternal(const uint8_t* obj_file,
                            const string& obj_filename,
                            const std::vector<string>& debug_dirs,
                            const DumpOptions& options,
                            Module** module) {
  if (!IsValidPeCoff(obj_file)) {
    fprintf(stderr, "Not a valid PE/COFF file: %s\n", obj_filename.c_str());
    return false;
  }

  int peclass = PeCoffClass(obj_file);
  if (peclass == PE32) {
    return ReadSymbolDataFromObjectFile<PeCoffClass32>(
        reinterpret_cast<const PeCoffClass32::ObjectFileBase>(obj_file), obj_filename, debug_dirs,
        options, module);
  }
  if (peclass == PE32PLUS) {
    return ReadSymbolDataFromObjectFile<PeCoffClass64>(
        reinterpret_cast<const PeCoffClass64::ObjectFileBase>(obj_file), obj_filename, debug_dirs,
        options, module);
  }

  return false;
}

bool WriteSymbolFile(const string &obj_file,
                     const std::vector<string>& debug_dirs,
                     const DumpOptions& options,
                     std::ostream &sym_stream) {
  Module* module;
  if (!ReadSymbolData(obj_file, debug_dirs, options, &module))
    return false;

  bool result = module->Write(sym_stream, options.symbol_data);
  delete module;
  return result;
}

bool ReadSymbolData(const string& obj_file,
                    const std::vector<string>& debug_dirs,
                    const DumpOptions& options,
                    Module** module) {
  MmapWrapper map_wrapper;
  const void* pe_header = NULL;

  if (!LoadFile(obj_file, &map_wrapper, &pe_header))
    return false;

  return ReadSymbolDataInternal(reinterpret_cast<const uint8_t*>(pe_header),
                                obj_file, debug_dirs, options, module);
}

}  // namespace google_breakpad
