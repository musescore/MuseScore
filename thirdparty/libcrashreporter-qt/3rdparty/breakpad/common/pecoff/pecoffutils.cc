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

// pecoffutils.c: Utilities for dealing with PECOFF files
//

#include "common/pecoff/pecoffutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <arpa/inet.h>
#endif

namespace google_breakpad {

bool IsValidPeCoff(const uint8_t* obj_base) {
  // at offset 0x3c, find the offset to PE signature
  const uint32_t* peOffsetPtr = reinterpret_cast<const uint32_t*>(obj_base +
    IMAGE_FILE_HEADER_OFFSET);

  // TODO: ideally we want to check that the offset is less than the size of the
  // mapped file, but we don't have that information at the moment
  //
  // if (*peOffsetPtr > size) return FALSE;

  // check PE signature
  const PeHeader* peHeader = reinterpret_cast<const PeHeader*>(obj_base+*peOffsetPtr);
  if (peHeader->mMagic !=  IMAGE_FILE_MAGIC)
    return false;

  return true;
}

int PeCoffClass(const uint8_t* obj_base) {
  const uint32_t* peOffsetPtr = reinterpret_cast<const uint32_t*>(obj_base +
    IMAGE_FILE_HEADER_OFFSET);
  const PeHeader* peHeader = reinterpret_cast<const PeHeader*>(obj_base+*peOffsetPtr);
  const uint16_t* peOptionalHeader = reinterpret_cast<const uint16_t*>
      (reinterpret_cast<const uint8_t*>(peHeader) + sizeof(PeHeader));

  // We need to read the magic before we know if this a Pe32OptionalHeader or
  // Pe32PlusOptionalHeader, so we don't use those types here.
  return *peOptionalHeader;
}

//
// Header information
//

template<typename PeCoffClassTraits>
const char* PeCoffObjectFileReader<PeCoffClassTraits>::Architecture(
    ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);
  uint16_t arch = peHeader->mMachine;
  switch (arch) {
    case IMAGE_FILE_MACHINE_I386:
      return "x86";
    case IMAGE_FILE_MACHINE_ARM:
      return "arm";
    case IMAGE_FILE_MACHINE_MIPS16:
    case IMAGE_FILE_MACHINE_MIPSFPU:
    case IMAGE_FILE_MACHINE_MIPSFPU16:
    case IMAGE_FILE_MACHINE_WCEMIPSV2:
      return "mips";
    case IMAGE_FILE_MACHINE_POWERPC:
    case IMAGE_FILE_MACHINE_POWERPCFP:
      return "ppc";
    case IMAGE_FILE_MACHINE_AMD64:
      return "x86_64";
    default:
      fprintf(stderr, "unrecognized machine architecture: %d\n",
              peHeader->mMachine);
      return NULL;
  }
}

template<typename PeCoffClassTraits>
bool PeCoffObjectFileReader<PeCoffClassTraits>::Endianness(
    ObjectFileBase obj_base,
    bool* big_endian) {
  // TODO: Not sure what big-endian PECOFF looks like: characteristics flag
  // IMAGE_FILE_BYTES_REVERSED_HI and/or certain machine types are big-endian
  *big_endian = false;
  return true;
}

template<typename PeCoffClassTraits>
typename PeCoffObjectFileReader<PeCoffClassTraits>::Addr
PeCoffObjectFileReader<PeCoffClassTraits>::GetLoadingAddress(
    ObjectFileBase obj_base) {
  const PeOptionalHeader* peOptionalHeader = GetOptionalHeader(obj_base);
  return peOptionalHeader->mImageBase;
}

//
// Section enumeration and location
//

template<typename PeCoffClassTraits>
int PeCoffObjectFileReader<PeCoffClassTraits>::GetNumberOfSections(
    ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);
  return peHeader->mNumberOfSections;
}

template<typename PeCoffClassTraits>
const typename PeCoffObjectFileReader<PeCoffClassTraits>::Section
PeCoffObjectFileReader<PeCoffClassTraits>::FindSectionByIndex(
    ObjectFileBase obj_base, int i) {
  const PeSectionHeader* section_table = GetSectionTable(obj_base);
  return reinterpret_cast<const Section>(&(section_table[i]));
}

template<typename PeCoffClassTraits>
const typename PeCoffObjectFileReader<PeCoffClassTraits>::Section
PeCoffObjectFileReader<PeCoffClassTraits>::FindSectionByName(
    const char* section_name, ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);
  const PeSectionHeader* section_table = GetSectionTable(obj_base);
  const char* string_table = GetStringTable(obj_base);
  uint32_t string_table_length = *(reinterpret_cast<const uint32_t*>(string_table));

  for (int s = 0; s < peHeader->mNumberOfSections; s++) {
    const char* name = section_table[s].mName;

    // look up long section names in string table
    if (name[0] == '/') {
      unsigned int offset = ::strtoul(section_table[s].mName+1, NULL, 10);

      if (offset > string_table_length)
        fprintf(stderr, "section name offset %d exceeds string table length",
                offset);
      else
        name = string_table + offset;
    }

    if (::strcmp(section_name, name) == 0) {
      return reinterpret_cast<const Section>(&(section_table[s]));
    }
  }

  // nothing found
  return NULL;
}

//
// Section information
//

template<typename PeCoffClassTraits>
const uint8_t*
PeCoffObjectFileReader<PeCoffClassTraits>::GetSectionPointer(
    ObjectFileBase obj_base, Section section) {
  return reinterpret_cast<const uint8_t*>(obj_base) + reinterpret_cast<const PeSectionHeader*>(section)->mPointerToRawData;
}

template<typename PeCoffClassTraits>
typename PeCoffObjectFileReader<PeCoffClassTraits>::Offset
PeCoffObjectFileReader<PeCoffClassTraits>::GetSectionSize(
    ObjectFileBase obj_base, Section section) {

  // There are mSizeOfRawData bytes of data for the section in the mapped image
  // file.  Return mVirtualSize if it's smaller.
  // This doesn't handle the case where mVirtualSize is larger and the section
  // should be zero padded, because we have nowhere to do that.
  if ((reinterpret_cast<const PeSectionHeader*>(section)->mVirtualSize) <
      (reinterpret_cast<const PeSectionHeader*>(section)->mSizeOfRawData))
    return reinterpret_cast<const PeSectionHeader*>(section)->mVirtualSize;

  return reinterpret_cast<const PeSectionHeader*>(section)->mSizeOfRawData;
}

template<typename PeCoffClassTraits>
typename PeCoffObjectFileReader<PeCoffClassTraits>::Offset
PeCoffObjectFileReader<PeCoffClassTraits>::GetSectionRVA(
    ObjectFileBase obj_base, Section section) {
  return reinterpret_cast<const PeSectionHeader*>(section)->mVirtualAddress;
}

template<typename PeCoffClassTraits>
const char* PeCoffObjectFileReader<PeCoffClassTraits>::GetSectionName(
    ObjectFileBase obj_base,Section section) {
    const char* string_table = GetStringTable(obj_base);
    uint32_t string_table_length = *(reinterpret_cast<const uint32_t*>(string_table));
    const char* name = reinterpret_cast<const PeSectionHeader*>(section)->mName;

    // look up long section names in string table
    if (name[0] == '/') {
      unsigned int offset = ::strtoul(name+1, NULL, 10);

      if (offset > string_table_length)
        fprintf(stderr, "section name offset %d exceeds string table length",
                offset);
      else
        name = string_table + offset;
    }

    return name;
}

//
//
//

template<class PeCoffClassTraits>
bool PeCoffObjectFileReader<PeCoffClassTraits>::ExportedSymbolsToModule(
    ObjectFileBase obj_base, Module* module) {
  // locate the export table, if present
  const PeDataDirectory* data_directory_export_entry = GetDataDirectoryEntry(obj_base, PE_EXPORT_TABLE);
  if (data_directory_export_entry && data_directory_export_entry->mSize != 0) {
    const PeExportTable* export_table = reinterpret_cast<const PeExportTable*>(ConvertRVAToPointer(obj_base, data_directory_export_entry->mVirtualAddress));
    const uint32_t* eat = reinterpret_cast<const uint32_t*>(ConvertRVAToPointer(obj_base, export_table->mExportAddressTableRVA));
    const uint32_t* enpt = reinterpret_cast<const uint32_t*>(ConvertRVAToPointer(obj_base, export_table->mNamePointerRVA));
    const uint16_t* eot = reinterpret_cast<const uint16_t*>(ConvertRVAToPointer(obj_base, export_table->mOrdinalTableRVA));

    // process the export name pointer table
    for (uint32_t i = 0; i < export_table->mNumberofNamePointers; i++) {
      // look up the name for the export
      uint32_t export_name_rva = enpt[i];
      if (export_name_rva == 0)
        continue;
      const char* export_name = reinterpret_cast<const char*>(ConvertRVAToPointer(obj_base, export_name_rva));

      // find the corresponding export address table entry
      uint16_t export_ordinal = eot[i];
      if ((export_ordinal < export_table->mOrdinalBase) ||
          (export_ordinal >= (export_table->mOrdinalBase + export_table->mAddressTableEntries))) {
        fprintf(stderr, "exported ordinal %d out of range for EAT!\n", export_ordinal);
        continue;
      }

      uint32_t eat_index = export_ordinal - export_table->mOrdinalBase;
      uint32_t export_rva = eat[eat_index];

      // if the export's address lies inside the export table, it's a forwarded
      // export, which we can ignore
      if ((export_rva >= data_directory_export_entry->mVirtualAddress) &&
          (export_rva < (data_directory_export_entry->mVirtualAddress + data_directory_export_entry->mSize)))
        continue;

      Module::Extern* ext = new Module::Extern;
      ext->name = export_name;
      ext->address = export_rva + GetLoadingAddress(obj_base);
      module->AddExtern(ext);
    }

    return true;
  }

  // report if a COFF symbol table exists, but we don't use it (yet)
  // According to the PECOFF spec. COFF debugging information is deprecated.
  // We don't know of any tools which produce that and don't produce DWARF or
  // MS CodeView debug information.
  const PeHeader* peHeader = GetHeader(obj_base);
  if (peHeader->mPointerToSymbolTable) {
    fprintf(stderr, "COFF debug symbols present but are not implemented\n");
  }

  return false;
}

template<class PeCoffClassTraits>
string
PeCoffObjectFileReader<PeCoffClassTraits>::FileIdentifierFromMappedFile(
    ObjectFileBase obj_file) {
  uint8_t identifier[kMDGUIDSize];
  uint32_t age;

  if (!PeCoffFileID::PeCoffFileIdentifierFromMappedFile(obj_file, identifier, &age))
    return "";

  // Endian-ness swap to match dump processor expectation.
  uint8_t identifier_swapped[kMDGUIDSize];
  memcpy(identifier_swapped, identifier, kMDGUIDSize);
  uint32_t* data1 = reinterpret_cast<uint32_t*>(identifier_swapped);
  *data1 = htonl(*data1);
  uint16_t* data2 = reinterpret_cast<uint16_t*>(identifier_swapped + 4);
  *data2 = htons(*data2);
  uint16_t* data3 = reinterpret_cast<uint16_t*>(identifier_swapped + 6);
  *data3 = htons(*data3);

  // Format the file identifier in IDENTIFIER as a UUID with the
  // dashes removed.
  char identifier_str[40];
  int buffer_idx = 0;
  for (int idx = 0; idx < kMDGUIDSize; ++idx) {
    int hi = (identifier_swapped[idx] >> 4) & 0x0F;
    int lo = (identifier_swapped[idx]) & 0x0F;

    identifier_str[buffer_idx++] = (hi >= 10) ? 'A' + hi - 10 : '0' + hi;
    identifier_str[buffer_idx++] = (lo >= 10) ? 'A' + lo - 10 : '0' + lo;
  }
  identifier_str[buffer_idx] = 0;
  string id = identifier_str;

  // Append age
  char age_string[9];
  snprintf(age_string, sizeof(age_string) / sizeof(age_string[0]), "%X", age);
  id += age_string;

  return id;
}

//
// Helper functions for PeCoffFileID
//

template<typename PeCoffClassTraits>
bool PeCoffObjectFileReader<PeCoffClassTraits>::GetBuildID(
    ObjectFileBase obj_base,
    uint8_t identifier[kMDGUIDSize],
    uint32_t* age) {
  // locate the debug directory, if present
  const PeDataDirectory* data_directory_debug_entry = GetDataDirectoryEntry(obj_base, PE_DEBUG_DATA);
  if (!data_directory_debug_entry)
    return false;

  uint32_t debug_directory_size = data_directory_debug_entry->mSize;
  if (debug_directory_size == 0)
    return false;

  const PeDebugDirectory* debug_directory = reinterpret_cast<const PeDebugDirectory*>(ConvertRVAToPointer(obj_base, data_directory_debug_entry->mVirtualAddress));
  if (debug_directory == NULL) {
    fprintf(stderr, "No section containing the debug directory VMA could be found\n");
    return false;
  }

  // search the debug directory for a codeview entry
  for (int i = 0; i < debug_directory_size/sizeof(PeDebugDirectory); i++) {
    if (debug_directory[i].mType == IMAGE_DEBUG_TYPE_CODEVIEW) {
      // interpret the codeview record to get build-id
      const CvInfoPbd70* codeview_record = reinterpret_cast<const CvInfoPbd70*>
          (obj_base + debug_directory[i].mPointerToRawData);
      if ((codeview_record->mCvSignature) == CODEVIEW_PDB70_CVSIGNATURE) {
        memcpy(identifier, codeview_record->mSignature, kMDGUIDSize);
        *age =  codeview_record->mAge;
        return true;
      } else {
        fprintf(stderr, "Unhandled codeview signature %x\n",
                codeview_record->mCvSignature);
      }
    }
  }

  fprintf(stderr, "No codeview entry in debug directory\n");
  return false;
}

template<typename PeCoffClassTraits>
bool PeCoffObjectFileReader<PeCoffClassTraits>::HashTextSection(
    ObjectFileBase obj_base,
    uint8_t identifier[kMDGUIDSize]) {
  Section text_section;
  Offset text_size;

  if (!(text_section = FindSectionByName(".text", obj_base)) ||
      ((text_size = GetSectionSize(obj_base, text_section)) == 0))
    return false;

  memset(identifier, 0, kMDGUIDSize);
  const uint8_t* ptr = GetSectionPointer(obj_base, text_section);
  const uint8_t* ptr_end = ptr + std::min(text_size, 4096U);
  while (ptr < ptr_end) {
    for (unsigned i = 0; i < kMDGUIDSize; i++)
      identifier[i] ^= ptr[i];
    ptr += kMDGUIDSize;
  }
  return true;
}

//
// Private implementation helper functions
//

template<typename PeCoffClassTraits>
const PeHeader* PeCoffObjectFileReader<PeCoffClassTraits>::GetHeader(
    ObjectFileBase obj_base) {
  const uint32_t* peOffsetPtr = reinterpret_cast<const uint32_t*>(obj_base +
    IMAGE_FILE_HEADER_OFFSET);
  const PeHeader* peHeader = reinterpret_cast<const PeHeader*>(obj_base+*peOffsetPtr);
  return peHeader;
}

template<typename PeCoffClassTraits>
const typename PeCoffObjectFileReader<PeCoffClassTraits>::PeOptionalHeader*
PeCoffObjectFileReader<PeCoffClassTraits>::GetOptionalHeader(
    ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);
  PeOptionalHeader* peOptionalHeader = (PeOptionalHeader*) ((uint32_t*)peHeader + 6);
  return peOptionalHeader;
}

template<typename PeCoffClassTraits>
const PeSectionHeader*
PeCoffObjectFileReader<PeCoffClassTraits>::GetSectionTable(
    ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);
  const PeOptionalHeader* peOptionalHeader = GetOptionalHeader(obj_base);

  // section table immediately follows optional header
  const PeSectionHeader* section_table = reinterpret_cast<const PeSectionHeader*>
      (reinterpret_cast<const uint8_t*>(peOptionalHeader) + peHeader->mSizeOfOptionalHeader);
  return section_table;
}

template<typename PeCoffClassTraits>
const char* PeCoffObjectFileReader<PeCoffClassTraits>::GetStringTable(
    ObjectFileBase obj_base) {
  const PeHeader* peHeader = GetHeader(obj_base);

  // string table immediately follows symbol table
  uint32_t string_table_offset = peHeader->mPointerToSymbolTable + peHeader->mNumberOfSymbols*sizeof(PeSymbol);
  const char* string_table = reinterpret_cast<const char*>(obj_base) + string_table_offset;
  return string_table;
}

template<class PeCoffClassTraits>
const PeDataDirectory*
PeCoffObjectFileReader<PeCoffClassTraits>::GetDataDirectoryEntry(
    ObjectFileBase obj_base, int entry) {
  const PeOptionalHeader* peOptionalHeader = GetOptionalHeader(obj_base);

  // the data directory immediately follows the optional header
  const PeDataDirectory* data_directory = reinterpret_cast<const PeDataDirectory*>(&peOptionalHeader->mDataDirectory[0]);
  uint32_t data_directory_size = peOptionalHeader->mNumberOfRvaAndSizes;

  // locate the required directory entry, if present
  if (data_directory_size < entry)
    return NULL;

  return &data_directory[entry];
}

template<typename PeCoffClassTraits>
const uint8_t*
PeCoffObjectFileReader<PeCoffClassTraits>::ConvertRVAToPointer(
    ObjectFileBase obj_base,
    Offset rva) {
  // find which section contains the rva to compute it's mapped address
  const PeSectionHeader* section_table = GetSectionTable(obj_base);
  for (int s = 0; s < GetNumberOfSections(obj_base); s++) {
    const PeSectionHeader* section = &(section_table[s]);

    if ((rva >= section->mVirtualAddress) &&
        (rva < (section->mVirtualAddress + section->mSizeOfRawData)))
    {
      uint32_t offset = rva - section->mVirtualAddress;
      const uint8_t* pointer = GetSectionPointer(obj_base, (Section)section) + offset;
      return pointer;
    }
  }

  fprintf(stderr, "No section could be found containing RVA %x\n", rva);
  return NULL;
}

// instantiation of templated classes
template class PeCoffObjectFileReader<PeCoffClass32Traits>;
template class PeCoffObjectFileReader<PeCoffClass64Traits>;

}
