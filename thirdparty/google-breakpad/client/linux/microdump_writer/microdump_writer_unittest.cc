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

#include <ctype.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#include <sstream>
#include <string>

#include "breakpad_googletest_includes.h"
#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/microdump_extra_info.h"
#include "client/linux/microdump_writer/microdump_writer.h"
#include "common/linux/breakpad_getcontext.h"
#include "common/linux/eintr_wrapper.h"
#include "common/linux/ignore_ret.h"
#include "common/scoped_ptr.h"
#include "common/tests/auto_tempdir.h"
#include "common/using_std_string.h"

using namespace google_breakpad;

extern "C" {
extern char __executable_start;
extern char __etext;
}

namespace {

typedef testing::Test MicrodumpWriterTest;

MicrodumpExtraInfo MakeMicrodumpExtraInfo(
    const char* build_fingerprint,
    const char* product_info,
    const char* gpu_fingerprint) {
  MicrodumpExtraInfo info;
  info.build_fingerprint = build_fingerprint;
  info.product_info = product_info;
  info.gpu_fingerprint = gpu_fingerprint;
  info.process_type = "Browser";
  return info;
}

bool ContainsMicrodump(const std::string& buf) {
  return std::string::npos != buf.find("-----BEGIN BREAKPAD MICRODUMP-----") &&
         std::string::npos != buf.find("-----END BREAKPAD MICRODUMP-----");
}

const char kIdentifiableString[] = "_IDENTIFIABLE_";
const uintptr_t kCrashAddress = 0xdeaddeadu;

void CrashAndGetMicrodump(const MappingList& mappings,
                          const MicrodumpExtraInfo& microdump_extra_info,
                          std::string* microdump,
                          bool skip_dump_if_principal_mapping_not_referenced = false,
                          uintptr_t address_within_principal_mapping = 0,
                          bool sanitize_stack = false) {
  int fds[2];
  ASSERT_NE(-1, pipe(fds));

  AutoTempDir temp_dir;
  string stderr_file = temp_dir.path() + "/stderr.log";
  int err_fd = open(stderr_file.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  ASSERT_NE(-1, err_fd);

  char identifiable_string[sizeof(kIdentifiableString)];

  // This string should not appear in the resulting microdump if it
  // has been sanitized.
  strcpy(identifiable_string, kIdentifiableString);
  // Force the strcpy to not be optimized away.
  IGNORE_RET(write(STDOUT_FILENO, identifiable_string, 0));

  const pid_t child = fork();
  if (child == 0) {
    close(fds[1]);
    char b;
    IGNORE_RET(HANDLE_EINTR(read(fds[0], &b, sizeof(b))));
    close(fds[0]);
    syscall(__NR_exit);
  }
  close(fds[0]);

  ExceptionHandler::CrashContext context;
  memset(&context, 0, sizeof(context));
  // Pretend the current context is the child context (which is
  // approximately right) so that we have a valid stack pointer, and
  // can fetch child stack data via ptrace.
  getcontext(&context.context);
  // Set a non-zero tid to avoid tripping asserts.
  context.tid = child;
  context.siginfo.si_signo = MD_EXCEPTION_CODE_LIN_DUMP_REQUESTED;
  context.siginfo.si_addr = reinterpret_cast<void*>(kCrashAddress);

  // Redirect temporarily stderr to the stderr.log file.
  int save_err = dup(STDERR_FILENO);
  ASSERT_NE(-1, save_err);
  ASSERT_NE(-1, dup2(err_fd, STDERR_FILENO));

  ASSERT_TRUE(WriteMicrodump(child, &context, sizeof(context), mappings,
                             skip_dump_if_principal_mapping_not_referenced,
                             address_within_principal_mapping, sanitize_stack,
                             microdump_extra_info));

  // Revert stderr back to the console.
  dup2(save_err, STDERR_FILENO);
  close(save_err);

  // Read back the stderr file and check for the microdump marker.
  fsync(err_fd);
  lseek(err_fd, 0, SEEK_SET);

  microdump->clear();
  char buf[1024];

  while (true) {
    int bytes_read = IGNORE_EINTR(read(err_fd, buf, 1024));
    if (bytes_read <= 0) break;
    microdump->append(buf, buf + bytes_read);
  }
  close(err_fd);
  close(fds[1]);
}

void ExtractMicrodumpStackContents(const string& microdump_content,
                                   string* result) {
  std::istringstream iss(microdump_content);
  result->clear();
  for (string line; std::getline(iss, line);) {
    if (line.find("S ") == 0) {
      std::istringstream stack_data(line);
      std::string key;
      std::string addr;
      std::string data;
      stack_data >> key >> addr >> data;
      EXPECT_TRUE((data.size() & 1u) == 0u);
      result->reserve(result->size() + data.size() / 2);
      for (size_t i = 0; i < data.size(); i += 2) {
        std::string byte = data.substr(i, 2);
        result->push_back(static_cast<char>(strtoul(byte.c_str(), NULL, 16)));
      }
    }
  }
}

void CheckMicrodumpContents(const string& microdump_content,
                            const MicrodumpExtraInfo& expected_info) {
  std::istringstream iss(microdump_content);
  bool did_find_os_info = false;
  bool did_find_product_info = false;
  bool did_find_process_type = false;
  bool did_find_crash_reason = false;
  bool did_find_gpu_info = false;
  for (string line; std::getline(iss, line);) {
    if (line.find("O ") == 0) {
      std::istringstream os_info_tokens(line);
      string token;
      os_info_tokens.ignore(2); // Ignore the "O " preamble.
      // Check the OS descriptor char (L=Linux, A=Android).
      os_info_tokens >> token;
      ASSERT_TRUE(token == "L" || token == "A");

      os_info_tokens >> token; // HW architecture.
      os_info_tokens >> token; // Number of cpus.
      for (size_t i = 0; i < token.size(); ++i)
        ASSERT_TRUE(isxdigit(token[i]));
      os_info_tokens >> token; // SW architecture.

      // Check that the build fingerprint is in the right place.
      os_info_tokens >> token;
      ASSERT_FALSE(os_info_tokens.fail());
      if (expected_info.build_fingerprint)
        ASSERT_EQ(expected_info.build_fingerprint, token);
      did_find_os_info = true;
    } else if (line.find("P ") == 0) {
      if (expected_info.process_type)
        ASSERT_EQ(string("P ") + expected_info.process_type, line);
      did_find_process_type = true;
    } else if (line.find("R ") == 0) {
      std::istringstream crash_reason_tokens(line);
      string token;
      unsigned crash_reason;
      string crash_reason_str;
      uintptr_t crash_address;
      crash_reason_tokens.ignore(2); // Ignore the "R " preamble.
      crash_reason_tokens >> std::hex >> crash_reason >> crash_reason_str >>
          crash_address;
      ASSERT_FALSE(crash_reason_tokens.fail());
      ASSERT_EQ(MD_EXCEPTION_CODE_LIN_DUMP_REQUESTED, crash_reason);
      ASSERT_EQ("DUMP_REQUESTED", crash_reason_str);
      ASSERT_EQ(kCrashAddress, crash_address);
      did_find_crash_reason = true;
    } else if (line.find("V ") == 0) {
      if (expected_info.product_info)
        ASSERT_EQ(string("V ") + expected_info.product_info, line);
      did_find_product_info = true;
    } else if (line.find("G ") == 0) {
      if (expected_info.gpu_fingerprint)
        ASSERT_EQ(string("G ") + expected_info.gpu_fingerprint, line);
      did_find_gpu_info = true;
    }
  }
  ASSERT_TRUE(did_find_os_info);
  ASSERT_TRUE(did_find_product_info);
  ASSERT_TRUE(did_find_process_type);
  ASSERT_TRUE(did_find_crash_reason);
  ASSERT_TRUE(did_find_gpu_info);
}

bool MicrodumpStackContains(const string& microdump_content,
                            const string& expected_content) {
  string result;
  ExtractMicrodumpStackContents(microdump_content, &result);
  return result.find(kIdentifiableString) != string::npos;
}

void CheckMicrodumpContents(const string& microdump_content,
                            const string& expected_fingerprint,
                            const string& expected_product_info,
                            const string& expected_gpu_fingerprint) {
  CheckMicrodumpContents(
      microdump_content,
      MakeMicrodumpExtraInfo(expected_fingerprint.c_str(),
                             expected_product_info.c_str(),
                             expected_gpu_fingerprint.c_str()));
}

TEST(MicrodumpWriterTest, BasicWithMappings) {
  // Push some extra mapping to check the MappingList logic.
  const uint32_t memory_size = sysconf(_SC_PAGESIZE);
  const char* kMemoryName = "libfoo.so";
  const uint8_t kModuleGUID[sizeof(MDGUID)] = {
     0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
     0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
  };

  MappingInfo info;
  info.start_addr = memory_size;
  info.size = memory_size;
  info.offset = 42;
  strcpy(info.name, kMemoryName);

  MappingList mappings;
  MappingEntry mapping;
  mapping.first = info;
  memcpy(mapping.second, kModuleGUID, sizeof(MDGUID));
  mappings.push_back(mapping);

  std::string buf;
  CrashAndGetMicrodump(mappings, MicrodumpExtraInfo(), &buf);
  ASSERT_TRUE(ContainsMicrodump(buf));

#ifdef __LP64__
  ASSERT_NE(std::string::npos,
            buf.find("M 0000000000001000 000000000000002A 0000000000001000 "
                     "33221100554477668899AABBCCDDEEFF0 libfoo.so"));
#else
  ASSERT_NE(std::string::npos,
            buf.find("M 00001000 0000002A 00001000 "
                     "33221100554477668899AABBCCDDEEFF0 libfoo.so"));
#endif

  // In absence of a product info in the minidump, the writer should just write
  // an unknown marker.
  ASSERT_NE(std::string::npos, buf.find("V UNKNOWN:0.0.0.0"));
}

// Ensure that no output occurs if the interest region is set, but
// doesn't overlap anything on the stack.
TEST(MicrodumpWriterTest, NoOutputIfUninteresting) {
  const char kProductInfo[] = "MockProduct:42.0.2311.99";
  const char kBuildFingerprint[] =
      "aosp/occam/mako:5.1.1/LMY47W/12345678:userdegbug/dev-keys";
  const char kGPUFingerprint[] =
      "Qualcomm;Adreno (TM) 330;OpenGL ES 3.0 V@104.0 AU@  (GIT@Id3510ff6dc)";
  const MicrodumpExtraInfo kMicrodumpExtraInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, kGPUFingerprint));

  std::string buf;
  MappingList no_mappings;

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfo, &buf, true, 0);
  ASSERT_FALSE(ContainsMicrodump(buf));
}

// Ensure that stack content does not contain an identifiable string if the
// stack is sanitized.
TEST(MicrodumpWriterTest, StringRemovedBySanitization) {
  const char kProductInfo[] = "MockProduct:42.0.2311.99";
  const char kBuildFingerprint[] =
      "aosp/occam/mako:5.1.1/LMY47W/12345678:userdegbug/dev-keys";
  const char kGPUFingerprint[] =
      "Qualcomm;Adreno (TM) 330;OpenGL ES 3.0 V@104.0 AU@  (GIT@Id3510ff6dc)";

  const MicrodumpExtraInfo kMicrodumpExtraInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, kGPUFingerprint));

  std::string buf;
  MappingList no_mappings;

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfo, &buf, false, 0u, true);
  ASSERT_TRUE(ContainsMicrodump(buf));
  ASSERT_FALSE(MicrodumpStackContains(buf, kIdentifiableString));
}

// Ensure that stack content does contain an identifiable string if the
// stack is not sanitized.
TEST(MicrodumpWriterTest, StringPresentIfNotSanitized) {
  const char kProductInfo[] = "MockProduct:42.0.2311.99";
  const char kBuildFingerprint[] =
      "aosp/occam/mako:5.1.1/LMY47W/12345678:userdegbug/dev-keys";
  const char kGPUFingerprint[] =
      "Qualcomm;Adreno (TM) 330;OpenGL ES 3.0 V@104.0 AU@  (GIT@Id3510ff6dc)";

  const MicrodumpExtraInfo kMicrodumpExtraInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, kGPUFingerprint));

  std::string buf;
  MappingList no_mappings;

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfo, &buf, false, 0u, false);
  ASSERT_TRUE(ContainsMicrodump(buf));
  ASSERT_TRUE(MicrodumpStackContains(buf, kIdentifiableString));
}

// Ensure that output occurs if the interest region is set, and
// does overlap something on the stack.
TEST(MicrodumpWriterTest, OutputIfInteresting) {
  const char kProductInfo[] = "MockProduct:42.0.2311.99";
  const char kBuildFingerprint[] =
      "aosp/occam/mako:5.1.1/LMY47W/12345678:userdegbug/dev-keys";
  const char kGPUFingerprint[] =
      "Qualcomm;Adreno (TM) 330;OpenGL ES 3.0 V@104.0 AU@  (GIT@Id3510ff6dc)";

  const MicrodumpExtraInfo kMicrodumpExtraInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, kGPUFingerprint));

  std::string buf;
  MappingList no_mappings;

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfo, &buf, true,
                       reinterpret_cast<uintptr_t>(CrashAndGetMicrodump));
  ASSERT_TRUE(ContainsMicrodump(buf));
}

// Ensure that the product info and build fingerprint metadata show up in the
// final microdump if present.
TEST(MicrodumpWriterTest, BuildFingerprintAndProductInfo) {
  const char kProductInfo[] = "MockProduct:42.0.2311.99";
  const char kBuildFingerprint[] =
      "aosp/occam/mako:5.1.1/LMY47W/12345678:userdegbug/dev-keys";
  const char kGPUFingerprint[] =
      "Qualcomm;Adreno (TM) 330;OpenGL ES 3.0 V@104.0 AU@  (GIT@Id3510ff6dc)";
  const MicrodumpExtraInfo kMicrodumpExtraInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, kGPUFingerprint));
  std::string buf;
  MappingList no_mappings;

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfo, &buf);
  ASSERT_TRUE(ContainsMicrodump(buf));
  CheckMicrodumpContents(buf, kMicrodumpExtraInfo);
}

TEST(MicrodumpWriterTest, NoProductInfo) {
  const char kBuildFingerprint[] = "foobar";
  const char kGPUFingerprint[] = "bazqux";
  std::string buf;
  MappingList no_mappings;

  const MicrodumpExtraInfo kMicrodumpExtraInfoNoProductInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, NULL, kGPUFingerprint));

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfoNoProductInfo, &buf);
  ASSERT_TRUE(ContainsMicrodump(buf));
  CheckMicrodumpContents(buf, kBuildFingerprint, "UNKNOWN:0.0.0.0",
                         kGPUFingerprint);
}

TEST(MicrodumpWriterTest, NoGPUInfo) {
  const char kProductInfo[] = "bazqux";
  const char kBuildFingerprint[] = "foobar";
  std::string buf;
  MappingList no_mappings;

  const MicrodumpExtraInfo kMicrodumpExtraInfoNoGPUInfo(
      MakeMicrodumpExtraInfo(kBuildFingerprint, kProductInfo, NULL));

  CrashAndGetMicrodump(no_mappings, kMicrodumpExtraInfoNoGPUInfo, &buf);
  ASSERT_TRUE(ContainsMicrodump(buf));
  CheckMicrodumpContents(buf, kBuildFingerprint, kProductInfo, "UNKNOWN");
}
}  // namespace
