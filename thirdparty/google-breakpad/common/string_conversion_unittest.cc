// Copyright (c) 2019, Google Inc.
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

// string_conversion_unittest.cc: Unit tests for google_breakpad::UTF* helpers.

#include <string>
#include <vector>

#include "breakpad_googletest_includes.h"
#include "common/string_conversion.h"

using google_breakpad::UTF8ToUTF16;
using google_breakpad::UTF8ToUTF16Char;
using google_breakpad::UTF16ToUTF8;
using std::vector;

TEST(StringConversionTest, UTF8ToUTF16) {
  const char in[] = "aßc";
  vector<uint16_t> out;
  vector<uint16_t> exp{'a', 0xdf, 'c', 0};
  UTF8ToUTF16(in, &out);
  EXPECT_EQ(4u, out.size());
  EXPECT_EQ(exp, out);
}

TEST(StringConversionTest, UTF8ToUTF16Char) {
  const char in[] = "a";
  uint16_t out[3] = {0xff, 0xff, 0xff};
  EXPECT_EQ(1, UTF8ToUTF16Char(in, 1, out));
  EXPECT_EQ('a', out[0]);
  EXPECT_EQ(0, out[1]);
  EXPECT_EQ(0xff, out[2]);
}

TEST(StringConversionTest, UTF16ToUTF8) {
  vector<uint16_t> in{'a', 0xdf, 'c', 0};
  EXPECT_EQ("aßc", UTF16ToUTF8(in, false));
}
