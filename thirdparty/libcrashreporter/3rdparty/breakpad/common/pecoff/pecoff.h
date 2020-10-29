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

// pecoff.h: PECOFF file format
//

#ifndef COMMON_PECOFF_PECOFF_H__
#define COMMON_PECOFF_PECOFF_H__

#include <stdint.h>

#define IMAGE_FILE_HEADER_OFFSET             0x3c

#define IMAGE_FILE_MAGIC                     0x00004550 // "PE\0\0"

#define IMAGE_FILE_MACHINE_UNKNOWN           0x0000
#define IMAGE_FILE_MACHINE_ALPHA             0x0184
#define IMAGE_FILE_MACHINE_ALPHA64           0x0284
#define IMAGE_FILE_MACHINE_AM33              0x01d3
#define IMAGE_FILE_MACHINE_AMD64             0x8664
#define IMAGE_FILE_MACHINE_ARM               0x01c0
#define IMAGE_FILE_MACHINE_ARMV7             0x01c4
#define IMAGE_FILE_MACHINE_CEE               0xc0ee
#define IMAGE_FILE_MACHINE_CEF               0x0cef
#define IMAGE_FILE_MACHINE_EBC               0x0ebc
#define IMAGE_FILE_MACHINE_I386              0x014c
#define IMAGE_FILE_MACHINE_IA64              0x0200
#define IMAGE_FILE_MACHINE_M32R              0x9041
#define IMAGE_FILE_MACHINE_M68K              0x0268
#define IMAGE_FILE_MACHINE_MIPS16            0x0266
#define IMAGE_FILE_MACHINE_MIPSFPU           0x0366
#define IMAGE_FILE_MACHINE_MIPSFPU16         0x0466
#define IMAGE_FILE_MACHINE_POWERPC           0x01f0
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define IMAGE_FILE_MACHINE_R10000            0x0168
#define IMAGE_FILE_MACHINE_R3000             0x0162
#define IMAGE_FILE_MACHINE_R4000             0x0166
#define IMAGE_FILE_MACHINE_SH3               0x01a2
#define IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define IMAGE_FILE_MACHINE_SH3E              0x01a4
#define IMAGE_FILE_MACHINE_SH4               0x01a6
#define IMAGE_FILE_MACHINE_SH5               0x01a8
#define IMAGE_FILE_MACHINE_THUMB             0x01c2
#define IMAGE_FILE_MACHINE_TRICORE           0x0520
#define IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169
#define IMAGE_FILE_MACHINE_AMD64             0x8664

struct PeHeader {
  uint32_t mMagic;   // IMAGE_FILE_MAGIC
  uint16_t mMachine; // IMAGE_FILE_MACHINE_* values
  uint16_t mNumberOfSections;
  uint32_t mTimeDateStamp;
  uint32_t mPointerToSymbolTable;
  uint32_t mNumberOfSymbols;
  uint16_t mSizeOfOptionalHeader;
  uint16_t mCharacteristics;
};

enum PeMagic {
  PE32     = 0x010b, // 32 bit
  PE32PLUS = 0x020b, // 64 bit address space, 2GB image size limit
};

struct PeDataDirectory {
  uint32_t mVirtualAddress;
  uint32_t mSize;
};

struct Pe32OptionalHeader {
  uint16_t mMagic;   // PeMagic
  uint8_t  mMajorLinkerVersion;
  uint8_t  mMinorLinkerVersion;
  uint32_t mSizeOfCode;
  uint32_t mSizeOfInitializedData;
  uint32_t mSizeOfUninitializedData;
  uint32_t mAddressOfEntryPoint;
  uint32_t mBaseOfCode;
  uint32_t mBaseOfData;
  uint32_t mImageBase;
  uint32_t mSectionAlignment;
  uint32_t mFileAlignment;
  uint16_t mMajorOperatingSystemVersion;
  uint16_t mMinorOperatingSystemVersion;
  uint16_t mMajorImageVersion;
  uint16_t mMinorImageVersion;
  uint16_t mMajorSubsystemVersion;
  uint16_t mMinorSubsystemVersion;
  uint32_t mWin32VersionValue;
  uint32_t mSizeOfImage;
  uint32_t mSizeOfHeaders;
  uint32_t mCheckSum;
  uint16_t mSubsystem;
  uint16_t mDllCharacteristics;
  uint32_t mSizeOfStackReserve;
  uint32_t mSizeOfStackCommit;
  uint32_t mSizeOfHeapReserve;
  uint32_t mSizeOfHeapCommit;
  uint32_t mLoaderFlags;
  uint32_t mNumberOfRvaAndSizes;
  PeDataDirectory mDataDirectory[0];
};

struct Pe32PlusOptionalHeader {
  uint16_t mMagic;   // PeMagic
  uint8_t  mMajorLinkerVersion;
  uint8_t  mMinorLinkerVersion;
  uint32_t mSizeOfCode;
  uint32_t mSizeOfInitializedData;
  uint32_t mSizeOfUninitializedData;
  uint32_t mAddressOfEntryPoint;
  uint32_t mBaseOfCode;
  uint64_t mImageBase;
  uint32_t mSectionAlignment;
  uint32_t mFileAlignment;
  uint16_t mMajorOperatingSystemVersion;
  uint16_t mMinorOperatingSystemVersion;
  uint16_t mMajorImageVersion;
  uint16_t mMinorImageVersion;
  uint16_t mMajorSubsystemVersion;
  uint16_t mMinorSubsystemVersion;
  uint32_t mWin32VersionValue;
  uint32_t mSizeOfImage;
  uint32_t mSizeOfHeaders;
  uint32_t mCheckSum;
  uint16_t mSubsystem;
  uint16_t mDllCharacteristics;
  uint64_t mSizeOfStackReserve;
  uint64_t mSizeOfStackCommit;
  uint64_t mSizeOfHeapReserve;
  uint64_t mSizeOfHeapCommit;
  uint32_t mLoaderFlags;
  uint32_t mNumberOfRvaAndSizes;
  PeDataDirectory mDataDirectory[0];
};

#define PE_EXPORT_TABLE                 0
#define PE_IMPORT_TABLE                 1
#define PE_RESOURCE_TABLE               2
#define PE_EXCEPTION_TABLE              3
#define PE_CERTIFICATE_TABLE            4
#define PE_BASE_RELOCATION_TABLE        5
#define PE_DEBUG_DATA                   6
#define PE_ARCHITECTURE                 7
#define PE_GLOBAL_PTR                   8
#define PE_TLS_TABLE                    9
#define PE_LOAD_CONFIG_TABLE            10
#define PE_BOUND_IMPORT_TABLE           11
#define PE_IMPORT_ADDRESS_TABLE         12
#define PE_DELAY_IMPORT_DESCRIPTOR      13
#define PE_CLR_RUNTIME_HEADER           14

struct PeDebugDirectory {
  uint32_t mCharacteristics;
  uint32_t mTimeDateStamp;
  uint16_t mMajorVersion;
  uint16_t mMinorVersion;
  uint32_t mType;
  uint32_t mSizeOfData;
  uint32_t mAddressOfRawData;
  uint32_t mPointerToRawData;
};

#define IMAGE_DEBUG_TYPE_UNKNOWN          0
#define IMAGE_DEBUG_TYPE_COFF             1
#define IMAGE_DEBUG_TYPE_CODEVIEW         2
#define IMAGE_DEBUG_TYPE_FPO              3
#define IMAGE_DEBUG_TYPE_MISC             4
#define IMAGE_DEBUG_TYPE_EXCEPTION        5
#define IMAGE_DEBUG_TYPE_FIXUP            6
#define IMAGE_DEBUG_TYPE_OMAP_TO_SRC      7
#define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC    8
#define IMAGE_DEBUG_TYPE_BORLAND          9
#define IMAGE_DEBUG_TYPE_RESERVED10       10
#define IMAGE_DEBUG_TYPE_CLSID            11

struct CvInfoPbd70
{
  uint32_t mCvSignature;
  uint8_t  mSignature[16];
  uint32_t mAge;
  uint8_t  mPdbFileName[];
};

#define CODEVIEW_PDB70_CVSIGNATURE 0x53445352 // "RSDS"
#define CODEVIEW_PDB20_CVSIGNATURE 0x3031424e // "NB10"
#define CODEVIEW_CV50_CVSIGNATURE  0x3131424e // "NB11"
#define CODEVIEW_CV41_CVSIGNATURE  0x3930424e // “NB09"

struct PeSectionHeader {
  char  mName[8];
  union {
    uint32_t mPhysicalAddress;
    uint32_t mVirtualSize;
  } ;
  uint32_t mVirtualAddress;
  uint32_t mSizeOfRawData;
  uint32_t mPointerToRawData;
  uint32_t mPointerToRelocations;
  uint32_t mPointerToLinenumbers;
  uint16_t mNumberOfRelocations;
  uint16_t mNumberOfLinenumbers;
  uint32_t mCharacteristics;
};

struct __attribute__ ((__packed__))  PeSymbol
{
  union {
    char   mName[8];  // Symbol Name
    struct {
      uint32_t mFirst4Bytes;
      uint32_t mSecond4Bytes;
    };
  };

  uint32_t mValue;    // Value of Symbol
  uint16_t mScNum;    // Section Number
  uint16_t mType;     // Symbol Type
  uint8_t  mSClass;   // Storage Class
  uint8_t  mNumAux;   // Auxiliary Count
};

struct PeExportTable {
  uint32_t mFlags;
  uint32_t mTimeDateStamp;
  uint16_t mMajorVersion;
  uint16_t mMinorVErsion;
  uint32_t mNameRVA;
  uint32_t mOrdinalBase;
  uint32_t mAddressTableEntries;
  uint32_t mNumberofNamePointers;
  uint32_t mExportAddressTableRVA;
  uint32_t mNamePointerRVA;
  uint32_t mOrdinalTableRVA;
};

#endif// COMMON_PECOFF_PECOFF_H__
