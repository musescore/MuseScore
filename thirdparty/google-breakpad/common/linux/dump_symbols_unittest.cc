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

// Original author: Ted Mielczarek <ted.mielczarek@gmail.com>

// dump_symbols_unittest.cc:
// Unittests for google_breakpad::DumpSymbols

#include <elf.h>
#include <link.h>
#include <stdio.h>

#include <sstream>
#include <vector>

#include "breakpad_googletest_includes.h"
#include "common/linux/elf_gnu_compat.h"
#include "common/linux/elfutils.h"
#include "common/linux/dump_symbols.h"
#include "common/linux/synth_elf.h"
#include "common/module.h"
#include "common/using_std_string.h"

namespace google_breakpad {

bool ReadSymbolDataInternal(const uint8_t* obj_file,
                            const string& obj_filename,
                            const string& obj_os,
                            const std::vector<string>& debug_dir,
                            const DumpOptions& options,
                            Module** module);

using google_breakpad::synth_elf::ELF;
using google_breakpad::synth_elf::Notes;
using google_breakpad::synth_elf::StringTable;
using google_breakpad::synth_elf::SymbolTable;
using google_breakpad::test_assembler::kLittleEndian;
using google_breakpad::test_assembler::Section;
using std::stringstream;
using std::vector;
using ::testing::Test;
using ::testing::Types;

template<typename ElfClass>
class DumpSymbols : public Test {
 public:
  void GetElfContents(ELF& elf) {
    string contents;
    ASSERT_TRUE(elf.GetContents(&contents));
    ASSERT_LT(0U, contents.size());

    elfdata_v.clear();
    elfdata_v.insert(elfdata_v.begin(), contents.begin(), contents.end());
    elfdata = &elfdata_v[0];
  }

  vector<uint8_t> elfdata_v;
  uint8_t* elfdata;
};

typedef Types<ElfClass32, ElfClass64> ElfClasses;

TYPED_TEST_SUITE(DumpSymbols, ElfClasses);

TYPED_TEST(DumpSymbols, Invalid) {
  Elf32_Ehdr header;
  memset(&header, 0, sizeof(header));
  Module* module;
  DumpOptions options(ALL_SYMBOL_DATA, true);
  EXPECT_FALSE(ReadSymbolDataInternal(reinterpret_cast<uint8_t*>(&header),
                                      "foo",
                                      "Linux",
                                      vector<string>(),
                                      options,
                                      &module));
}

TYPED_TEST(DumpSymbols, SimplePublic) {
  ELF elf(TypeParam::kMachine, TypeParam::kClass, kLittleEndian);
  // Zero out text section for simplicity.
  Section text(kLittleEndian);
  text.Append(4096, 0);
  elf.AddSection(".text", text, SHT_PROGBITS);

  // Add a public symbol.
  StringTable table(kLittleEndian);
  SymbolTable syms(kLittleEndian, TypeParam::kAddrSize, table);
  syms.AddSymbol("superfunc",
                   (typename TypeParam::Addr)0x1000,
                   (typename TypeParam::Addr)0x10,
                 // ELF32_ST_INFO works for 32-or 64-bit.
                 ELF32_ST_INFO(STB_GLOBAL, STT_FUNC),
                 SHN_UNDEF + 1);
  int index = elf.AddSection(".dynstr", table, SHT_STRTAB);
  elf.AddSection(".dynsym", syms,
                 SHT_DYNSYM,          // type
                 SHF_ALLOC,           // flags
                 0,                   // addr
                 index,               // link
                 sizeof(typename TypeParam::Sym));  // entsize

  elf.Finish();
  this->GetElfContents(elf);

  Module* module;
  DumpOptions options(ALL_SYMBOL_DATA, true);
  EXPECT_TRUE(ReadSymbolDataInternal(this->elfdata,
                                     "foo",
                                     "Linux",
                                     vector<string>(),
                                     options,
                                     &module));

  stringstream s;
  module->Write(s, ALL_SYMBOL_DATA);
  const string expected =
    string("MODULE Linux ") + TypeParam::kMachineName
    + " 000000000000000000000000000000000 foo\n"
    "INFO CODE_ID 00000000000000000000000000000000\n"
    "PUBLIC 1000 0 superfunc\n";
  EXPECT_EQ(expected, s.str());
  delete module;
}

TYPED_TEST(DumpSymbols, SimpleBuildID) {
  ELF elf(TypeParam::kMachine, TypeParam::kClass, kLittleEndian);
  // Zero out text section for simplicity.
  Section text(kLittleEndian);
  text.Append(4096, 0);
  elf.AddSection(".text", text, SHT_PROGBITS);

  // Add a Build ID
  const uint8_t kExpectedIdentifierBytes[] =
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13};
  Notes notes(kLittleEndian);
  notes.AddNote(NT_GNU_BUILD_ID, "GNU", kExpectedIdentifierBytes,
                sizeof(kExpectedIdentifierBytes));
  elf.AddSection(".note.gnu.build-id", notes, SHT_NOTE);

  // Add a public symbol.
  StringTable table(kLittleEndian);
  SymbolTable syms(kLittleEndian, TypeParam::kAddrSize, table);
  syms.AddSymbol("superfunc",
                   (typename TypeParam::Addr)0x1000,
                   (typename TypeParam::Addr)0x10,
                 // ELF32_ST_INFO works for 32-or 64-bit.
                 ELF32_ST_INFO(STB_GLOBAL, STT_FUNC),
                 SHN_UNDEF + 1);
  int index = elf.AddSection(".dynstr", table, SHT_STRTAB);
  elf.AddSection(".dynsym", syms,
                 SHT_DYNSYM,          // type
                 SHF_ALLOC,           // flags
                 0,                   // addr
                 index,               // link
                 sizeof(typename TypeParam::Sym));  // entsize

  elf.Finish();
  this->GetElfContents(elf);

  Module* module;
  DumpOptions options(ALL_SYMBOL_DATA, true);
  EXPECT_TRUE(ReadSymbolDataInternal(this->elfdata,
                                     "foo",
                                     "Linux",
                                     vector<string>(),
                                     options,
                                     &module));

  stringstream s;
  module->Write(s, ALL_SYMBOL_DATA);
  const string expected =
    string("MODULE Linux ") + TypeParam::kMachineName
    + " 030201000504070608090A0B0C0D0E0F0 foo\n"
    "INFO CODE_ID 000102030405060708090A0B0C0D0E0F10111213\n"
    "PUBLIC 1000 0 superfunc\n";
  EXPECT_EQ(expected, s.str());
  delete module;
}

}  // namespace google_breakpad
