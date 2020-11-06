// Copyright (c) 2009, Google Inc.
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

// linux_ptrace_dumper_unittest.cc:
// Unit tests for google_breakpad::LinuxPtraceDumper.
//
// This file was renamed from linux_dumper_unittest.cc and modified due
// to LinuxDumper being splitted into two classes.

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "breakpad_googletest_includes.h"
#include "client/linux/minidump_writer/linux_ptrace_dumper.h"
#include "client/linux/minidump_writer/minidump_writer_unittest_utils.h"
#include "common/linux/eintr_wrapper.h"
#include "common/linux/file_id.h"
#include "common/linux/ignore_ret.h"
#include "common/linux/safe_readlink.h"
#include "common/memory_allocator.h"
#include "common/using_std_string.h"

#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif

using namespace google_breakpad;

namespace {

pid_t SetupChildProcess(int number_of_threads) {
  char kNumberOfThreadsArgument[2];
  sprintf(kNumberOfThreadsArgument, "%d", number_of_threads);

  int fds[2];
  EXPECT_NE(-1, pipe(fds));

  pid_t child_pid = fork();
  if (child_pid == 0) {
    // In child process.
    close(fds[0]);

    string helper_path(GetHelperBinary());
    if (helper_path.empty()) {
      fprintf(stderr, "Couldn't find helper binary\n");
      _exit(1);
    }

    // Pass the pipe fd and the number of threads as arguments.
    char pipe_fd_string[8];
    sprintf(pipe_fd_string, "%d", fds[1]);
    execl(helper_path.c_str(),
          "linux_dumper_unittest_helper",
          pipe_fd_string,
          kNumberOfThreadsArgument,
          NULL);
    // Kill if we get here.
    printf("Errno from exec: %d", errno);
    std::string err_str = "Exec of  " + helper_path + " failed";
    perror(err_str.c_str());
    _exit(1);
  }
  close(fds[1]);

  // Wait for all child threads to indicate that they have started
  for (int threads = 0; threads < number_of_threads; threads++) {
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fds[0];
    pfd.events = POLLIN | POLLERR;

    const int r = HANDLE_EINTR(poll(&pfd, 1, 1000));
    EXPECT_EQ(1, r);
    EXPECT_TRUE(pfd.revents & POLLIN);
    uint8_t junk;
    EXPECT_EQ(read(fds[0], &junk, sizeof(junk)),
              static_cast<ssize_t>(sizeof(junk)));
  }
  close(fds[0]);

  // There is a race here because we may stop a child thread before
  // it is actually running the busy loop. Empirically this sleep
  // is sufficient to avoid the race.
  usleep(100000);
  return child_pid;
}

typedef wasteful_vector<uint8_t> id_vector;
typedef testing::Test LinuxPtraceDumperTest;

/* Fixture for running tests in a child process. */
class LinuxPtraceDumperChildTest : public testing::Test {
 protected:
  virtual void SetUp() {
    child_pid_ = fork();
#ifndef __ANDROID__
    prctl(PR_SET_PTRACER, child_pid_);
#endif
  }

  /* Gtest is calling TestBody from this class, which sets up a child
   * process in which the RealTestBody virtual member is called.
   * As such, TestBody is not supposed to be overridden in derived classes.
   */
  virtual void TestBody() /* final */ {
    if (child_pid_ == 0) {
      // child process
      RealTestBody();
      _exit(HasFatalFailure() ? kFatalFailure :
            (HasNonfatalFailure() ? kNonFatalFailure : 0));
    }

    ASSERT_TRUE(child_pid_ > 0);
    int status;
    waitpid(child_pid_, &status, 0);
    if (WEXITSTATUS(status) == kFatalFailure) {
      GTEST_FATAL_FAILURE_("Test failed in child process");
    } else if (WEXITSTATUS(status) == kNonFatalFailure) {
      GTEST_NONFATAL_FAILURE_("Test failed in child process");
    }
  }

  /* Gtest defines TestBody functions through its macros, but classes
   * derived from this one need to define RealTestBody instead.
   * This is achieved by defining a TestBody macro further below.
   */
  virtual void RealTestBody() = 0;

  id_vector make_vector() {
    return id_vector(&allocator, kDefaultBuildIdSize);
  }

 private:
  static const int kFatalFailure = 1;
  static const int kNonFatalFailure = 2;

  pid_t child_pid_;
  PageAllocator allocator;
};

}  // namespace

/* Replace TestBody declarations within TEST*() with RealTestBody
 * declarations */
#define TestBody RealTestBody

TEST_F(LinuxPtraceDumperChildTest, Setup) {
  LinuxPtraceDumper dumper(getppid());
}

TEST_F(LinuxPtraceDumperChildTest, FindMappings) {
  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());

  ASSERT_TRUE(dumper.FindMapping(reinterpret_cast<void*>(getpid)));
  ASSERT_TRUE(dumper.FindMapping(reinterpret_cast<void*>(printf)));
  ASSERT_FALSE(dumper.FindMapping(NULL));
}

TEST_F(LinuxPtraceDumperChildTest, ThreadList) {
  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());

  ASSERT_GE(dumper.threads().size(), (size_t)1);
  bool found = false;
  for (size_t i = 0; i < dumper.threads().size(); ++i) {
    if (dumper.threads()[i] == getppid()) {
      ASSERT_FALSE(found);
      found = true;
    }
  }
  ASSERT_TRUE(found);
}

// Helper stack class to close a file descriptor and unmap
// a mmap'ed mapping.
class StackHelper {
 public:
  StackHelper()
    : fd_(-1), mapping_(NULL), size_(0) {}
  ~StackHelper() {
    if (size_)
      munmap(mapping_, size_);
    if (fd_ >= 0)
      close(fd_);
  }
  void Init(int fd, char* mapping, size_t size) {
    fd_ = fd;
    mapping_ = mapping;
    size_ = size;
  }

  char* mapping() const { return mapping_; }
  size_t size() const { return size_; }

 private:
  int fd_;
  char* mapping_;
  size_t size_;
};

class LinuxPtraceDumperMappingsTest : public LinuxPtraceDumperChildTest {
 protected:
  virtual void SetUp();

  string helper_path_;
  size_t page_size_;
  StackHelper helper_;
};

void LinuxPtraceDumperMappingsTest::SetUp() {
  helper_path_ = GetHelperBinary();
  if (helper_path_.empty()) {
    FAIL() << "Couldn't find helper binary";
    _exit(1);
  }

  // mmap two segments out of the helper binary, one
  // enclosed in the other, but with different protections.
  page_size_ = sysconf(_SC_PAGESIZE);
  const size_t kMappingSize = 3 * page_size_;
  int fd = open(helper_path_.c_str(), O_RDONLY);
  ASSERT_NE(-1, fd) << "Failed to open file: " << helper_path_
                    << ", Error: " << strerror(errno);
  char* mapping =
    reinterpret_cast<char*>(mmap(NULL,
                                 kMappingSize,
                                 PROT_READ,
                                 MAP_SHARED,
                                 fd,
                                 0));
  ASSERT_TRUE(mapping);

  // Ensure that things get cleaned up.
  helper_.Init(fd, mapping, kMappingSize);

  // Carve a page out of the first mapping with different permissions.
  char* inside_mapping =  reinterpret_cast<char*>(
      mmap(mapping + 2 * page_size_,
           page_size_,
           PROT_NONE,
           MAP_SHARED | MAP_FIXED,
           fd,
           // Map a different offset just to
           // better test real-world conditions.
           page_size_));
  ASSERT_TRUE(inside_mapping);

  LinuxPtraceDumperChildTest::SetUp();
}

TEST_F(LinuxPtraceDumperMappingsTest, MergedMappings) {
  // Now check that LinuxPtraceDumper interpreted the mappings properly.
  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());
  int mapping_count = 0;
  for (unsigned i = 0; i < dumper.mappings().size(); ++i) {
    const MappingInfo& mapping = *dumper.mappings()[i];
    if (strcmp(mapping.name, this->helper_path_.c_str()) == 0) {
      // This mapping should encompass the entire original mapped
      // range.
      EXPECT_EQ(reinterpret_cast<uintptr_t>(this->helper_.mapping()),
                mapping.start_addr);
      EXPECT_EQ(this->helper_.size(), mapping.size);
      EXPECT_EQ(0U, mapping.offset);
      mapping_count++;
    }
  }
  EXPECT_EQ(1, mapping_count);
}

TEST_F(LinuxPtraceDumperChildTest, BuildProcPath) {
  const pid_t pid = getppid();
  LinuxPtraceDumper dumper(pid);

  char maps_path[NAME_MAX] = "";
  char maps_path_expected[NAME_MAX];
  snprintf(maps_path_expected, sizeof(maps_path_expected),
           "/proc/%d/maps", pid);
  EXPECT_TRUE(dumper.BuildProcPath(maps_path, pid, "maps"));
  EXPECT_STREQ(maps_path_expected, maps_path);

  EXPECT_FALSE(dumper.BuildProcPath(NULL, pid, "maps"));
  EXPECT_FALSE(dumper.BuildProcPath(maps_path, 0, "maps"));
  EXPECT_FALSE(dumper.BuildProcPath(maps_path, pid, ""));
  EXPECT_FALSE(dumper.BuildProcPath(maps_path, pid, NULL));

  char long_node[NAME_MAX];
  size_t long_node_len = NAME_MAX - strlen("/proc/123") - 1;
  memset(long_node, 'a', long_node_len);
  long_node[long_node_len] = '\0';
  EXPECT_FALSE(dumper.BuildProcPath(maps_path, 123, long_node));
}

#if !defined(__ARM_EABI__) && !defined(__mips__)
// Ensure that the linux-gate VDSO is included in the mapping list.
TEST_F(LinuxPtraceDumperChildTest, MappingsIncludeLinuxGate) {
  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());

  void* linux_gate_loc =
    reinterpret_cast<void*>(dumper.auxv()[AT_SYSINFO_EHDR]);
  ASSERT_TRUE(linux_gate_loc);
  bool found_linux_gate = false;

  const wasteful_vector<MappingInfo*> mappings = dumper.mappings();
  const MappingInfo* mapping;
  for (unsigned i = 0; i < mappings.size(); ++i) {
    mapping = mappings[i];
    if (!strcmp(mapping->name, kLinuxGateLibraryName)) {
      found_linux_gate = true;
      break;
    }
  }
  EXPECT_TRUE(found_linux_gate);
  EXPECT_EQ(linux_gate_loc, reinterpret_cast<void*>(mapping->start_addr));
  EXPECT_EQ(0, memcmp(linux_gate_loc, ELFMAG, SELFMAG));
}

// Ensure that the linux-gate VDSO can generate a non-zeroed File ID.
TEST_F(LinuxPtraceDumperChildTest, LinuxGateMappingID) {
  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());

  bool found_linux_gate = false;
  const wasteful_vector<MappingInfo*> mappings = dumper.mappings();
  unsigned index = 0;
  for (unsigned i = 0; i < mappings.size(); ++i) {
    if (!strcmp(mappings[i]->name, kLinuxGateLibraryName)) {
      found_linux_gate = true;
      index = i;
      break;
    }
  }
  ASSERT_TRUE(found_linux_gate);

  // Need to suspend the child so ptrace actually works.
  ASSERT_TRUE(dumper.ThreadsSuspend());
  id_vector identifier(make_vector());
  ASSERT_TRUE(dumper.ElfFileIdentifierForMapping(*mappings[index],
                                                 true,
                                                 index,
                                                 identifier));

  id_vector empty_identifier(make_vector());
  empty_identifier.resize(kDefaultBuildIdSize, 0);
  EXPECT_NE(empty_identifier, identifier);
  EXPECT_TRUE(dumper.ThreadsResume());
}
#endif

TEST_F(LinuxPtraceDumperChildTest, FileIDsMatch) {
  // Calculate the File ID of our binary using both
  // FileID::ElfFileIdentifier and LinuxDumper::ElfFileIdentifierForMapping
  // and ensure that we get the same result from both.
  char exe_name[PATH_MAX];
  ASSERT_TRUE(SafeReadLink("/proc/self/exe", exe_name));

  LinuxPtraceDumper dumper(getppid());
  ASSERT_TRUE(dumper.Init());
  const wasteful_vector<MappingInfo*> mappings = dumper.mappings();
  bool found_exe = false;
  unsigned i;
  for (i = 0; i < mappings.size(); ++i) {
    const MappingInfo* mapping = mappings[i];
    if (!strcmp(mapping->name, exe_name)) {
      found_exe = true;
      break;
    }
  }
  ASSERT_TRUE(found_exe);

  id_vector identifier1(make_vector());
  id_vector identifier2(make_vector());
  EXPECT_TRUE(dumper.ElfFileIdentifierForMapping(*mappings[i], true, i,
                                                 identifier1));
  FileID fileid(exe_name);
  EXPECT_TRUE(fileid.ElfFileIdentifier(identifier2));

  string identifier_string1 =
      FileID::ConvertIdentifierToUUIDString(identifier1);
  string identifier_string2 =
      FileID::ConvertIdentifierToUUIDString(identifier2);
  EXPECT_EQ(identifier_string1, identifier_string2);
}

/* Get back to normal behavior of TEST*() macros wrt TestBody. */
#undef TestBody

TEST(LinuxPtraceDumperTest, VerifyStackReadWithMultipleThreads) {
  static const size_t kNumberOfThreadsInHelperProgram = 5;

  pid_t child_pid = SetupChildProcess(kNumberOfThreadsInHelperProgram);
  ASSERT_NE(child_pid, -1);

  // Children are ready now.
  LinuxPtraceDumper dumper(child_pid);
  ASSERT_TRUE(dumper.Init());
#if defined(THREAD_SANITIZER)
  EXPECT_GE(dumper.threads().size(), (size_t)kNumberOfThreadsInHelperProgram);
#else
  EXPECT_EQ(dumper.threads().size(), (size_t)kNumberOfThreadsInHelperProgram);
#endif
  EXPECT_TRUE(dumper.ThreadsSuspend());

  ThreadInfo one_thread;
  size_t matching_threads = 0;
  for (size_t i = 0; i < dumper.threads().size(); ++i) {
    EXPECT_TRUE(dumper.GetThreadInfoByIndex(i, &one_thread));
    const void* stack;
    size_t stack_len;
    EXPECT_TRUE(dumper.GetStackInfo(&stack, &stack_len,
        one_thread.stack_pointer));
    // In the helper program, we stored a pointer to the thread id in a
    // specific register. Check that we can recover its value.
#if defined(__ARM_EABI__)
    pid_t* process_tid_location = (pid_t*)(one_thread.regs.uregs[3]);
#elif defined(__aarch64__)
    pid_t* process_tid_location = (pid_t*)(one_thread.regs.regs[3]);
#elif defined(__i386)
    pid_t* process_tid_location = (pid_t*)(one_thread.regs.ecx);
#elif defined(__x86_64)
    pid_t* process_tid_location = (pid_t*)(one_thread.regs.rcx);
#elif defined(__mips__)
    pid_t* process_tid_location =
        reinterpret_cast<pid_t*>(one_thread.mcontext.gregs[1]);
#else
#error This test has not been ported to this platform.
#endif
    pid_t one_thread_id;
    dumper.CopyFromProcess(&one_thread_id,
                           dumper.threads()[i],
                           process_tid_location,
                           4);
    matching_threads += (dumper.threads()[i] == one_thread_id) ? 1 : 0;
  }
  EXPECT_EQ(matching_threads, kNumberOfThreadsInHelperProgram);
  EXPECT_TRUE(dumper.ThreadsResume());
  kill(child_pid, SIGKILL);

  // Reap child
  int status;
  ASSERT_NE(-1, HANDLE_EINTR(waitpid(child_pid, &status, 0)));
  ASSERT_TRUE(WIFSIGNALED(status));
  ASSERT_EQ(SIGKILL, WTERMSIG(status));
}

TEST_F(LinuxPtraceDumperTest, SanitizeStackCopy) {
  static const size_t kNumberOfThreadsInHelperProgram = 1;

  pid_t child_pid = SetupChildProcess(kNumberOfThreadsInHelperProgram);
  ASSERT_NE(child_pid, -1);

  LinuxPtraceDumper dumper(child_pid);
  ASSERT_TRUE(dumper.Init());
  EXPECT_TRUE(dumper.ThreadsSuspend());

  ThreadInfo thread_info;
  EXPECT_TRUE(dumper.GetThreadInfoByIndex(0, &thread_info));

  const uintptr_t defaced =
#if defined(__LP64__)
      0x0defaced0defaced;
#else
      0x0defaced;
#endif

  uintptr_t simulated_stack[2];

  // Pointers into the stack shouldn't be sanitized.
  memset(simulated_stack, 0xff, sizeof(simulated_stack));
  simulated_stack[1] = thread_info.stack_pointer;
  dumper.SanitizeStackCopy(reinterpret_cast<uint8_t*>(&simulated_stack),
                           sizeof(simulated_stack), thread_info.stack_pointer,
                           sizeof(uintptr_t));
  ASSERT_NE(simulated_stack[1], defaced);

  // Memory prior to the stack pointer should be cleared.
  ASSERT_EQ(simulated_stack[0], 0u);

  // Small integers should not be sanitized.
  for (int i = -4096; i <= 4096; ++i) {
    memset(simulated_stack, 0, sizeof(simulated_stack));
    simulated_stack[0] = static_cast<uintptr_t>(i);
    dumper.SanitizeStackCopy(reinterpret_cast<uint8_t*>(&simulated_stack),
                             sizeof(simulated_stack), thread_info.stack_pointer,
                             0u);
    ASSERT_NE(simulated_stack[0], defaced);
  }

  // The instruction pointer definitely should point into an executable mapping.
  const MappingInfo* mapping_info = dumper.FindMappingNoBias(
      reinterpret_cast<uintptr_t>(thread_info.GetInstructionPointer()));
  ASSERT_NE(mapping_info, nullptr);
  ASSERT_TRUE(mapping_info->exec);

  // Pointers to code shouldn't be sanitized.
  memset(simulated_stack, 0, sizeof(simulated_stack));
  simulated_stack[1] = thread_info.GetInstructionPointer();
  dumper.SanitizeStackCopy(reinterpret_cast<uint8_t*>(&simulated_stack),
                           sizeof(simulated_stack), thread_info.stack_pointer,
                           0u);
  ASSERT_NE(simulated_stack[0], defaced);

  // String fragments should be sanitized.
  memcpy(simulated_stack, "abcdefghijklmnop", sizeof(simulated_stack));
  dumper.SanitizeStackCopy(reinterpret_cast<uint8_t*>(&simulated_stack),
                           sizeof(simulated_stack), thread_info.stack_pointer,
                           0u);
  ASSERT_EQ(simulated_stack[0], defaced);
  ASSERT_EQ(simulated_stack[1], defaced);

  // Heap pointers should be sanititzed.
#if defined(__ARM_EABI__)
  uintptr_t heap_addr = thread_info.regs.uregs[3];
#elif defined(__aarch64__)
  uintptr_t heap_addr = thread_info.regs.regs[3];
#elif defined(__i386)
  uintptr_t heap_addr = thread_info.regs.ecx;
#elif defined(__x86_64)
  uintptr_t heap_addr = thread_info.regs.rcx;
#elif defined(__mips__)
  uintptr_t heap_addr = thread_info.mcontext.gregs[1];
#else
#error This test has not been ported to this platform.
#endif
  memset(simulated_stack, 0, sizeof(simulated_stack));
  simulated_stack[0] = heap_addr;
  dumper.SanitizeStackCopy(reinterpret_cast<uint8_t*>(&simulated_stack),
                           sizeof(simulated_stack), thread_info.stack_pointer,
                           0u);
  ASSERT_EQ(simulated_stack[0], defaced);

  EXPECT_TRUE(dumper.ThreadsResume());
  kill(child_pid, SIGKILL);

  // Reap child.
  int status;
  ASSERT_NE(-1, HANDLE_EINTR(waitpid(child_pid, &status, 0)));
  ASSERT_TRUE(WIFSIGNALED(status));
  ASSERT_EQ(SIGKILL, WTERMSIG(status));
}
