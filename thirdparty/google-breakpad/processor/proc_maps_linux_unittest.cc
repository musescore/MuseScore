// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "breakpad_googletest_includes.h"
#include "common/using_std_string.h"
#include "google_breakpad/processor/proc_maps_linux.h"

namespace {

TEST(ProcMapsTest, Empty) {
  std::vector<google_breakpad::MappedMemoryRegion> regions;
  EXPECT_TRUE(ParseProcMaps("", &regions));
  EXPECT_EQ(0u, regions.size());
}

TEST(ProcMapsTest, NoSpaces) {
  static const char kNoSpaces[] =
      "00400000-0040b000 r-xp 00002200 fc:00 794418 /bin/cat\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kNoSpaces, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0x00400000u, regions[0].start);
  EXPECT_EQ(0x0040b000u, regions[0].end);
  EXPECT_EQ(0x00002200u, regions[0].offset);
  EXPECT_EQ("/bin/cat", regions[0].path);
}

TEST(ProcMapsTest, Spaces) {
  static const char kSpaces[] =
      "00400000-0040b000 r-xp 00002200 fc:00 794418 /bin/space cat\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kSpaces, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0x00400000u, regions[0].start);
  EXPECT_EQ(0x0040b000u, regions[0].end);
  EXPECT_EQ(0x00002200u, regions[0].offset);
  EXPECT_EQ("/bin/space cat", regions[0].path);
}

TEST(ProcMapsTest, NoNewline) {
  static const char kNoSpaces[] =
      "00400000-0040b000 r-xp 00002200 fc:00 794418 /bin/cat";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_FALSE(ParseProcMaps(kNoSpaces, &regions));
}

TEST(ProcMapsTest, NoPath) {
  static const char kNoPath[] =
      "00400000-0040b000 rw-p 00000000 00:00 0 \n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kNoPath, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0x00400000u, regions[0].start);
  EXPECT_EQ(0x0040b000u, regions[0].end);
  EXPECT_EQ(0x00000000u, regions[0].offset);
  EXPECT_EQ("", regions[0].path);
}

TEST(ProcMapsTest, Heap) {
  static const char kHeap[] =
      "022ac000-022cd000 rw-p 00000000 00:00 0 [heap]\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kHeap, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0x022ac000u, regions[0].start);
  EXPECT_EQ(0x022cd000u, regions[0].end);
  EXPECT_EQ(0x00000000u, regions[0].offset);
  EXPECT_EQ("[heap]", regions[0].path);
}

#if defined(ARCH_CPU_32_BITS)
TEST(ProcMapsTest, Stack32) {
  static const char kStack[] =
      "beb04000-beb25000 rw-p 00000000 00:00 0 [stack]\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kStack, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0xbeb04000u, regions[0].start);
  EXPECT_EQ(0xbeb25000u, regions[0].end);
  EXPECT_EQ(0x00000000u, regions[0].offset);
  EXPECT_EQ("[stack]", regions[0].path);
}
#elif defined(ARCH_CPU_64_BITS)
TEST(ProcMapsTest, Stack64) {
  static const char kStack[] =
      "7fff69c5b000-7fff69c7d000 rw-p 00000000 00:00 0 [stack]\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kStack, &regions));
  ASSERT_EQ(1u, regions.size());

  EXPECT_EQ(0x7fff69c5b000u, regions[0].start);
  EXPECT_EQ(0x7fff69c7d000u, regions[0].end);
  EXPECT_EQ(0x00000000u, regions[0].offset);
  EXPECT_EQ("[stack]", regions[0].path);
}
#endif

TEST(ProcMapsTest, Multiple) {
  static const char kMultiple[] =
      "00400000-0040b000 r-xp 00000000 fc:00 794418 /bin/cat\n"
      "0060a000-0060b000 r--p 0000a000 fc:00 794418 /bin/cat\n"
      "0060b000-0060c000 rw-p 0000b000 fc:00 794418 /bin/cat\n";

  std::vector<google_breakpad::MappedMemoryRegion> regions;
  ASSERT_TRUE(ParseProcMaps(kMultiple, &regions));
  ASSERT_EQ(3u, regions.size());

  EXPECT_EQ(0x00400000u, regions[0].start);
  EXPECT_EQ(0x0040b000u, regions[0].end);
  EXPECT_EQ(0x00000000u, regions[0].offset);
  EXPECT_EQ("/bin/cat", regions[0].path);

  EXPECT_EQ(0x0060a000u, regions[1].start);
  EXPECT_EQ(0x0060b000u, regions[1].end);
  EXPECT_EQ(0x0000a000u, regions[1].offset);
  EXPECT_EQ("/bin/cat", regions[1].path);

  EXPECT_EQ(0x0060b000u, regions[2].start);
  EXPECT_EQ(0x0060c000u, regions[2].end);
  EXPECT_EQ(0x0000b000u, regions[2].offset);
  EXPECT_EQ("/bin/cat", regions[2].path);
}

TEST(ProcMapsTest, Permissions) {
  static struct {
    const char* input;
    uint8_t permissions;
  } kTestCases[] = {
    {"00400000-0040b000 ---s 00000000 fc:00 794418 /bin/cat\n", 0},
    {"00400000-0040b000 ---S 00000000 fc:00 794418 /bin/cat\n", 0},
    {"00400000-0040b000 r--s 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::READ},
    {"00400000-0040b000 -w-s 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::WRITE},
    {"00400000-0040b000 --xs 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::EXECUTE},
    {"00400000-0040b000 rwxs 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::READ
        | google_breakpad::MappedMemoryRegion::WRITE
        | google_breakpad::MappedMemoryRegion::EXECUTE},
    {"00400000-0040b000 ---p 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::PRIVATE},
    {"00400000-0040b000 r--p 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::READ
        | google_breakpad::MappedMemoryRegion::PRIVATE},
    {"00400000-0040b000 -w-p 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::WRITE
        | google_breakpad::MappedMemoryRegion::PRIVATE},
    {"00400000-0040b000 --xp 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::EXECUTE
        | google_breakpad::MappedMemoryRegion::PRIVATE},
    {"00400000-0040b000 rwxp 00000000 fc:00 794418 /bin/cat\n",
      google_breakpad::MappedMemoryRegion::READ
        | google_breakpad::MappedMemoryRegion::WRITE
        | google_breakpad::MappedMemoryRegion::EXECUTE
        | google_breakpad::MappedMemoryRegion::PRIVATE},
  };

  for (size_t i = 0; i < sizeof(kTestCases) / sizeof(kTestCases[0]); ++i) {
    std::vector<google_breakpad::MappedMemoryRegion> regions;
    EXPECT_TRUE(ParseProcMaps(kTestCases[i].input, &regions));
    EXPECT_EQ(1u, regions.size());
    if (regions.empty())
      continue;
    EXPECT_EQ(kTestCases[i].permissions, regions[0].permissions);
  }
}

TEST(ProcMapsTest, MissingFields) {
  static const char* kTestCases[] = {
    "00400000\n",                               // Missing end + beyond.
    "00400000-0040b000\n",                      // Missing perms + beyond.
    "00400000-0040b000 r-xp\n",                 // Missing offset + beyond.
    "00400000-0040b000 r-xp 00000000\n",        // Missing device + beyond.
    "00400000-0040b000 r-xp 00000000 fc:00\n",  // Missing inode + beyond.
    "00400000-0040b000 00000000 fc:00 794418 /bin/cat\n",  // Missing perms.
    "00400000-0040b000 r-xp fc:00 794418 /bin/cat\n",      // Missing offset.
    "00400000-0040b000 r-xp 00000000 fc:00 /bin/cat\n",    // Missing inode.
    "00400000 r-xp 00000000 fc:00 794418 /bin/cat\n",      // Missing end.
    "-0040b000 r-xp 00000000 fc:00 794418 /bin/cat\n",     // Missing start.
    "00400000-0040b000 r-xp 00000000 794418 /bin/cat\n",   // Missing device.
  };

  for (size_t i = 0; i < sizeof(kTestCases) / sizeof(kTestCases[0]); ++i) {
    std::vector<google_breakpad::MappedMemoryRegion> regions;
    EXPECT_FALSE(ParseProcMaps(kTestCases[i], &regions));
  }
}

TEST(ProcMapsTest, InvalidInput) {
  static const char* kTestCases[] = {
    "thisisal-0040b000 rwxp 00000000 fc:00 794418 /bin/cat\n",
    "0040000d-linvalid rwxp 00000000 fc:00 794418 /bin/cat\n",
    "00400000-0040b000 inpu 00000000 fc:00 794418 /bin/cat\n",
    "00400000-0040b000 rwxp tforproc fc:00 794418 /bin/cat\n",
    "00400000-0040b000 rwxp 00000000 ma:ps 794418 /bin/cat\n",
    "00400000-0040b000 rwxp 00000000 fc:00 parse! /bin/cat\n",
  };

  for (size_t i = 0; i < sizeof(kTestCases) / sizeof(kTestCases[0]); ++i) {
    std::vector<google_breakpad::MappedMemoryRegion> regions;
    EXPECT_FALSE(ParseProcMaps(kTestCases[i], &regions));
  }
}

TEST(ProcMapsTest, ParseProcMapsEmptyString) {
  std::vector<google_breakpad::MappedMemoryRegion> regions;
  EXPECT_TRUE(ParseProcMaps("", &regions));
  EXPECT_EQ(0ULL, regions.size());
}

// Testing a couple of remotely possible weird things in the input:
// - Line ending with \r\n or \n\r.
// - File name contains quotes.
// - File name has whitespaces.
TEST(ProcMapsTest, ParseProcMapsWeirdCorrectInput) {
  std::vector<google_breakpad::MappedMemoryRegion> regions;
  const string kContents =
    "00400000-0040b000 r-xp 00000000 fc:00 2106562 "
      "               /bin/cat\r\n"
    "7f53b7dad000-7f53b7f62000 r-xp 00000000 fc:00 263011 "
      "       /lib/x86_64-linux-gnu/libc-2.15.so\n\r"
    "7f53b816d000-7f53b818f000 r-xp 00000000 fc:00 264284 "
      "        /lib/x86_64-linux-gnu/ld-2.15.so\n"
    "7fff9c7ff000-7fff9c800000 r-xp 00000000 00:00 0 "
      "               \"vd so\"\n"
    "ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0 "
      "               [vsys call]\n";
  EXPECT_TRUE(ParseProcMaps(kContents, &regions));
  EXPECT_EQ(5ULL, regions.size());
  EXPECT_EQ("/bin/cat", regions[0].path);
  EXPECT_EQ("/lib/x86_64-linux-gnu/libc-2.15.so", regions[1].path);
  EXPECT_EQ("/lib/x86_64-linux-gnu/ld-2.15.so", regions[2].path);
  EXPECT_EQ("\"vd so\"", regions[3].path);
  EXPECT_EQ("[vsys call]", regions[4].path);
}

}  // namespace
