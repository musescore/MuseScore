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
//
// pecoff_file_id.cc: Return a unique identifier for a file
//

#include "common/pecoff/pecoff_file_id.h"

#include <string.h>
#include "common/pecoff/pecoffutils.h"

namespace google_breakpad {

// Attempt to locate a CodeView build-id section in a PECOFF binary
// and copy as many bytes of it as will fit into |identifier|.
static bool FindPeCoffBuildID(const uint8_t* mapped_base,
                              uint8_t identifier[kMDGUIDSize],
                              uint32_t *age) {
  int peclass = PeCoffClass(mapped_base);
  if (peclass == PE32)
    return PeCoffClass32::GetBuildID(mapped_base, identifier, age);
  if (peclass == PE32PLUS)
    return PeCoffClass64::GetBuildID(mapped_base, identifier, age);

  return false;
}

// Attempt to locate the .text section of a binary and generate
// a simple hash by XORing the first page worth of bytes into |identifier|.
static bool HashPeCoffTextSection(const uint8_t* mapped_base,
                                  uint8_t identifier[kMDGUIDSize]) {
  int peclass = PeCoffClass(mapped_base);
  if (peclass == PE32)
    return PeCoffClass32::HashTextSection(mapped_base, identifier);
  if (peclass == PE32PLUS)
    return PeCoffClass64::HashTextSection(mapped_base, identifier);

  return false;
}

bool PeCoffFileID::PeCoffFileIdentifierFromMappedFile(const void* base,
                                                      uint8_t identifier[kMDGUIDSize],
                                                      uint32_t *age) {
  *age = 0;

  // Look for a build id first.
  if (FindPeCoffBuildID(reinterpret_cast<const uint8_t *>(base), identifier,
                        age))
    return true;

#if 1
  // XXX: Fallback to a default debug_identifier.
  memset(identifier, 0, kMDGUIDSize);
  *age = 0;
  return true;
#else
  // Fall back on hashing the first page of the text section.
  // (This is of questionable value as the Windows Minidump writer doesn't have
  // this feature)
  return HashPeCoffTextSection(reinterpret_cast<const uint8_t *>(base),
                               identifier);
#endif
}

}  // namespace google_breakpad
