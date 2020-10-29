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

// pecoffutils.h: Utilities for dealing with PECOFF files
//

#ifndef COMMON_PECOFF_PECOFFUTILS_H__
#define COMMON_PECOFF_PECOFFUTILS_H__

#include "common/pecoff/pecoff.h"
#include "common/pecoff/pecoff_file_id.h"
#include "common/module.h"

namespace google_breakpad {

bool IsValidPeCoff(const uint8_t* obj_file);
int PeCoffClass(const uint8_t* obj_file);

class PeCoffClass32Traits {
 public:
  typedef uint32_t Addr;
  typedef Pe32OptionalHeader PeOptionalHeader;
  static const int kClass = PE32;
  static const size_t kAddrSize = 4;
};

class PeCoffClass64Traits {
 public:
  typedef uint64_t Addr;
  typedef Pe32PlusOptionalHeader PeOptionalHeader;
  static const int kClass = PE32PLUS;
  static const size_t kAddrSize = 8;
};
// Offset isn't part of the traits as although PE32+ uses 64-bit address space,
// it still uses 32-bit RVAs and offsets

template <typename PeCoffClassTraits>
class PeCoffObjectFileReader {
 public:
  typedef const uint8_t* ObjectFileBase;
  typedef const uint8_t* Section;
  typedef uint32_t Offset;
  typedef typename PeCoffClassTraits::Addr Addr;
  static const int kClass = PeCoffClassTraits::kClass;
  static const size_t kAddrSize = PeCoffClassTraits::kAddrSize;

  static bool IsValid(ObjectFileBase obj_file) {
    return IsValidPeCoff(obj_file);
  };

  //
  // Header information
  //

  // Return the breakpad symbol file identifier for the architecture
  static const char* Architecture(ObjectFileBase obj_base);

  // Get the endianness. If it's invalid, return false.
  static bool Endianness(ObjectFileBase obj_base, bool* big_endian);

  // Find the preferred loading address of the binary.
  static Addr GetLoadingAddress(ObjectFileBase obj_base);

  //
  // Section enumeration and location
  //

  static int GetNumberOfSections(ObjectFileBase obj_base);
  static const Section FindSectionByIndex(ObjectFileBase obj_base, int i);
  // Attempt to find a section named |section_name|
  static const Section FindSectionByName(const char* section_name,
                                         ObjectFileBase obj_base);

  //
  // Section information
  //

  // Convert a section into a pointer to the mapped address in the current
  // process.
  static const uint8_t* GetSectionPointer(ObjectFileBase obj_base,
                                          Section section);

  // Get the size of a section
  static Offset GetSectionSize(ObjectFileBase obj_base, Section section);

  // Get relative virtual address (RVA) of a section
  static Offset GetSectionRVA(ObjectFileBase obj_base, Section section);

  // Get name of a section
  static const char* GetSectionName(ObjectFileBase obj_base,Section section);

  // Find any linked section
  static const Section FindLinkedSection(ObjectFileBase obj_base,
                                         Section section) {
    return 0; // PECOFF doesn't have the concept of linked sections
  }

  //
  //
  //

  // Load symbols from the object file's exported symbol table
  static bool ExportedSymbolsToModule(ObjectFileBase obj_base, Module* module);

  // Return the identifier for the file mapped into memory.
  // Return an empty string if the identifier could not be created
  // for the file.
  static string FileIdentifierFromMappedFile(ObjectFileBase obj_base);

  //
  // Helpers for PeCoffFileID
  //

  // Get the build-id
  static bool GetBuildID(ObjectFileBase obj_base,
                         uint8_t identifier[kMDGUIDSize], uint32_t* age);
  // Hash the text section
  static bool HashTextSection(ObjectFileBase obj_base,
                              uint8_t identifier[kMDGUIDSize]);

 private:
  typedef typename PeCoffClassTraits::PeOptionalHeader PeOptionalHeader;

  //
  // Private implementation helper functions
  //
  static const PeHeader* GetHeader(ObjectFileBase obj_base);
  static const PeOptionalHeader* GetOptionalHeader(ObjectFileBase obj_base);
  static const PeSectionHeader* GetSectionTable(ObjectFileBase obj_base);
  static const char* GetStringTable(ObjectFileBase obj_base);
  static const PeDataDirectory* GetDataDirectoryEntry(ObjectFileBase obj_base,
                                                      int entry);
  static const uint8_t* ConvertRVAToPointer(ObjectFileBase obj_base, Offset rva);
};

class PeCoffClass32 : public PeCoffObjectFileReader<PeCoffClass32Traits> { };
class PeCoffClass64 : public PeCoffObjectFileReader<PeCoffClass64Traits> { };

}  // namespace google_breakpad

#endif  // COMMON_PECOFF_PECOFFUTILS_H__
