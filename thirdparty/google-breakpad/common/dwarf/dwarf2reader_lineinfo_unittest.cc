// Copyright (c) 2020, Google Inc.
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

// Original author: Sterling Augustine <saugustine@google.com>

// dwarf2reader_lineinfo_unittest.cc: Unit tests for dwarf2reader::LineInfo

#include <stdint.h>
#include <stdlib.h>

#include <string>
#include <vector>

#include "breakpad_googletest_includes.h"
#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2reader.h"
#include "google_breakpad/common/breakpad_types.h"

using std::vector;
using testing::InSequence;
using testing::Return;
using testing::Sequence;
using testing::Test;
using testing::_;

using namespace dwarf2reader;

namespace {

const uint8_t dwarf5_line_program[] = {
  0x40, 0x0, 0x0, 0x0,  // unit_length (end - begin)
  // begin
  0x05, 0x0,  // version
  0x8,  // address_size
  0x0,  // segment_selector_size
  0x26, 0x0, 0x0, 0x0, // header_length (end_header_end - begin_header)
  // begin_header:
  0x1,  // minimum_instruction_length
  0x1,  // maximum_operations_per_instruction
  0x1,  // default_is_stmt
  0xfb, // line_base
  0xe,  // line_range
  0xd,  // opcode_base and lengths
  0x0, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x1,
  0x1,  // directory entry format count
  DW_LNCT_path, DW_FORM_strp,
  0x1,  // directories count
  0x1, 0x0, 0x0, 0x0, // offset into .debug_line_str
  0x2,  // file_name_entry_format_count
  DW_LNCT_directory_index, DW_FORM_data1,
  DW_LNCT_path, DW_FORM_line_strp,
  0x1,  // filename count
  0x0,  // directory index
  0x1, 0x0, 0x0, 0x0, // offset into .debug_str
  // end_header
  DW_LNS_set_file, 0x0,
  //  set address to 0x0
  0x0, 0x9, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  // Advance Address by 0 and line by 3
  0x15,
  // Advance PC by 1
  0x2, 0x1,
  0x0,
  DW_LNE_end_sequence,
  DW_LNE_end_sequence,
  // end
};

const uint8_t dwarf4_line_program[] = {
  0x37, 0x0, 0x0, 0x0,  // unit_length (end - begin)
  // begin
  0x04, 0x0,  // version
  0x1d, 0x0, 0x0, 0x0, // header_length (end_header - begin_header)
  // begin_header:
  0x1,  // minimum_instruction_length
  0x1,  // maximum_operations_per_instruction
  0x1,  // default_is_stmt
  0xfb, // line_base
  0xe,  // line_range
  0xd,  // opcode_base and lengths
  0x0, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x1,
  '/', 'a', '\0',  // directory entry 1 (zeroth entry implied)
  '\0', // end of directory table
  'b', '/', 'c', '\0',  // file entry 1 (zeroth entry implied)
  0, // file 1 directory
  0, // file 1 modification time
  0, // file 1 length
  '\0', // end of file table
  // end_header
  DW_LNS_set_file, 0x0,
  //  set address to 0x0
  0x0, 0x9, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  // Advance Address by 0 and line by 3
  0x15,
  // Advance PC by 1
  0x2, 0x1,
  0x0,
  DW_LNE_end_sequence,
  DW_LNE_end_sequence,
  // end
};

class MockLineInfoHandler: public LineInfoHandler {
 public:
  MOCK_METHOD(void, DefineDir, (const string&, uint32_t dir_num), (override));
  MOCK_METHOD(void, DefineFile, (const string& name, int32_t file_num,
                                 uint32_t dir_num, uint64_t mod_time,
                                 uint64_t length), (override));
  MOCK_METHOD(void, AddLine, (uint64_t address, uint64_t length,
                              uint32_t file_num, uint32_t line_num,
                              uint32_t column_num), (override));
};

const uint8_t string_section[] = {'x', '/', 'a', '\0'};
const uint8_t line_string_section[] = {'x', 'b', '/', 'c', '\0' };

struct LineProgram: public Test {
  MockLineInfoHandler handler_;
};

TEST_F(LineProgram, ReadLinesDwarf5) {
  ByteReader byte_reader(ENDIANNESS_LITTLE);
  // LineTables don't specify the offset size like Compilation Units do.
  byte_reader.SetOffsetSize(4);
  LineInfo line_reader(dwarf5_line_program,
                       sizeof(dwarf5_line_program),
                       &byte_reader,
                       string_section,
                       sizeof(string_section),
                       line_string_section,
                       sizeof(line_string_section),
                       &handler_);
  EXPECT_CALL(handler_, DefineDir("/a", 0)).Times(1);
  EXPECT_CALL(handler_, DefineFile("b/c", 0, 0, 0, 0)).Times(1);
  EXPECT_CALL(handler_, AddLine(0, 1, 0, 4, 0)).Times(1);
  EXPECT_EQ(line_reader.Start(), sizeof(dwarf5_line_program));
}

TEST_F(LineProgram, ReadLinesDwarf4) {
  ByteReader byte_reader(ENDIANNESS_LITTLE);
  // LineTables don't specify the offset size like Compilation Units do.
  byte_reader.SetOffsetSize(4);
  // dwarf4 line info headers don't encode the address size.
  byte_reader.SetAddressSize(8);
  LineInfo line_reader(dwarf4_line_program,
                       sizeof(dwarf5_line_program),
                       &byte_reader,
                       // dwarf4 line tables can't access the string sections
                       // so pass values likely to make assertions fail if
                       // the code uses them improperly.
                       nullptr, 0, nullptr, 0,
                       &handler_);
  EXPECT_CALL(handler_, DefineDir("", 0)).Times(1);
  EXPECT_CALL(handler_, DefineDir("/a", 1)).Times(1);
  EXPECT_CALL(handler_, DefineFile("", 0, 0, 0, 0)).Times(1);
  EXPECT_CALL(handler_, DefineFile("b/c", 1, 0, 0, 0)).Times(1);
  EXPECT_CALL(handler_, AddLine(0, 1, 0, 4, 0)).Times(1);
  EXPECT_EQ(line_reader.Start(), sizeof(dwarf4_line_program));
}

}  // anonymous namespace
