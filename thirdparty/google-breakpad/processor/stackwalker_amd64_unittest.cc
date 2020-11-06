// Copyright (c) 2010, Google Inc.
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

// Original author: Jim Blandy <jimb@mozilla.com> <jimb@red-bean.com>

// stackwalker_amd64_unittest.cc: Unit tests for StackwalkerAMD64 class.

#include <string.h>
#include <string>
#include <vector>

#include "breakpad_googletest_includes.h"
#include "common/test_assembler.h"
#include "common/using_std_string.h"
#include "google_breakpad/common/minidump_format.h"
#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/call_stack.h"
#include "google_breakpad/processor/code_module.h"
#include "google_breakpad/processor/source_line_resolver_interface.h"
#include "google_breakpad/processor/stack_frame_cpu.h"
#include "processor/stackwalker_unittest_utils.h"
#include "processor/stackwalker_amd64.h"

using google_breakpad::BasicSourceLineResolver;
using google_breakpad::CallStack;
using google_breakpad::CodeModule;
using google_breakpad::StackFrameSymbolizer;
using google_breakpad::StackFrame;
using google_breakpad::StackFrameAMD64;
using google_breakpad::Stackwalker;
using google_breakpad::StackwalkerAMD64;
using google_breakpad::SystemInfo;
using google_breakpad::test_assembler::kLittleEndian;
using google_breakpad::test_assembler::Label;
using google_breakpad::test_assembler::Section;
using std::vector;
using testing::_;
using testing::AnyNumber;
using testing::DoAll;
using testing::Return;
using testing::SetArgumentPointee;
using testing::Test;

class StackwalkerAMD64Fixture {
 public:
  StackwalkerAMD64Fixture()
    : stack_section(kLittleEndian),
      // Give the two modules reasonable standard locations and names
      // for tests to play with.
      module1(0x00007400c0000000ULL, 0x10000, "module1", "version1"),
      module2(0x00007500b0000000ULL, 0x10000, "module2", "version2") {
    // Identify the system as a Linux system.
    system_info.os = "Linux";
    system_info.os_short = "linux";
    system_info.os_version = "Horrendous Hippo";
    system_info.cpu = "x86";
    system_info.cpu_info = "";

    // Put distinctive values in the raw CPU context.
    BrandContext(&raw_context);

    // Create some modules with some stock debugging information.
    modules.Add(&module1);
    modules.Add(&module2);

    // By default, none of the modules have symbol info; call
    // SetModuleSymbols to override this.
    EXPECT_CALL(supplier, GetCStringSymbolData(_, _, _, _, _))
      .WillRepeatedly(Return(MockSymbolSupplier::NOT_FOUND));

    // Avoid GMOCK WARNING "Uninteresting mock function call - returning
    // directly" for FreeSymbolData().
    EXPECT_CALL(supplier, FreeSymbolData(_)).Times(AnyNumber());

    // Reset max_frames_scanned since it's static.
    Stackwalker::set_max_frames_scanned(1024);
  }

  // Set the Breakpad symbol information that supplier should return for
  // MODULE to INFO.
  void SetModuleSymbols(MockCodeModule* module, const string& info) {
    size_t buffer_size;
    char *buffer = supplier.CopySymbolDataAndOwnTheCopy(info, &buffer_size);
    EXPECT_CALL(supplier, GetCStringSymbolData(module, &system_info, _, _, _))
      .WillRepeatedly(DoAll(SetArgumentPointee<3>(buffer),
                            SetArgumentPointee<4>(buffer_size),
                            Return(MockSymbolSupplier::FOUND)));
  }

  // Populate stack_region with the contents of stack_section. Use
  // stack_section.start() as the region's starting address.
  void RegionFromSection() {
    string contents;
    ASSERT_TRUE(stack_section.GetContents(&contents));
    stack_region.Init(stack_section.start().Value(), contents);
  }

  // Fill RAW_CONTEXT with pseudo-random data, for round-trip checking.
  void BrandContext(MDRawContextAMD64 *raw_context) {
    uint8_t x = 173;
    for (size_t i = 0; i < sizeof(*raw_context); i++)
      reinterpret_cast<uint8_t*>(raw_context)[i] = (x += 17);
  }

  SystemInfo system_info;
  MDRawContextAMD64 raw_context;
  Section stack_section;
  MockMemoryRegion stack_region;
  MockCodeModule module1;
  MockCodeModule module2;
  MockCodeModules modules;
  MockSymbolSupplier supplier;
  BasicSourceLineResolver resolver;
  CallStack call_stack;
  const vector<StackFrame*>* frames;
};

class GetContextFrame: public StackwalkerAMD64Fixture, public Test { };

class SanityCheck: public StackwalkerAMD64Fixture, public Test { };

TEST_F(SanityCheck, NoResolver) {
  // There should be no references to the stack in this walk: we don't
  // provide any call frame information, so trying to reconstruct the
  // context frame's caller should fail. So there's no need for us to
  // provide stack contents.
  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = 0x8000000080000000ULL;

  StackFrameSymbolizer frame_symbolizer(NULL, NULL);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  // This should succeed even without a resolver or supplier.
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(1U, modules_without_symbols.size());
  ASSERT_EQ("module1", modules_without_symbols[0]->debug_file());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_GE(1U, frames->size());
  StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(0));
  // Check that the values from the original raw context made it
  // through to the context in the stack frame.
  EXPECT_EQ(0, memcmp(&raw_context, &frame->context, sizeof(raw_context)));
}

TEST_F(GetContextFrame, Simple) {
  // There should be no references to the stack in this walk: we don't
  // provide any call frame information, so trying to reconstruct the
  // context frame's caller should fail. So there's no need for us to
  // provide stack contents.
  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = 0x8000000080000000ULL;

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(1U, modules_without_symbols.size());
  ASSERT_EQ("module1", modules_without_symbols[0]->debug_file());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_GE(1U, frames->size());
  StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(0));
  // Check that the values from the original raw context made it
  // through to the context in the stack frame.
  EXPECT_EQ(0, memcmp(&raw_context, &frame->context, sizeof(raw_context)));
}

// The stackwalker should be able to produce the context frame even
// without stack memory present.
TEST_F(GetContextFrame, NoStackMemory) {
  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = 0x8000000080000000ULL;

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, NULL, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(1U, modules_without_symbols.size());
  ASSERT_EQ("module1", modules_without_symbols[0]->debug_file());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_GE(1U, frames->size());
  StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(0));
  // Check that the values from the original raw context made it
  // through to the context in the stack frame.
  EXPECT_EQ(0, memcmp(&raw_context, &frame->context, sizeof(raw_context)));
}

class GetCallerFrame: public StackwalkerAMD64Fixture, public Test { };

TEST_F(GetCallerFrame, ScanWithoutSymbols) {
  // When the stack walker resorts to scanning the stack,
  // only addresses located within loaded modules are
  // considered valid return addresses.
  // Force scanning through three frames to ensure that the
  // stack pointer is set properly in scan-recovered frames.
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address1 = 0x00007500b0000100ULL;
  uint64_t return_address2 = 0x00007500b0000900ULL;
  Label frame1_sp, frame2_sp, frame1_rbp;
  stack_section
    // frame 0
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // junk that's not
    .D64(0x00007500d0000000ULL)         // a return address

    .D64(return_address1)               // actual return address
    // frame 1
    .Mark(&frame1_sp)
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // more junk
    .D64(0x00007500d0000000ULL)

    .Mark(&frame1_rbp)
    .D64(stack_section.start())         // This is in the right place to be
                                        // a saved rbp, but it's bogus, so
                                        // we shouldn't report it.

    .D64(return_address2)               // actual return address
    // frame 2
    .Mark(&frame2_sp)
    .Append(32, 0);                     // end of stack

  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame1_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(2U, modules_without_symbols.size());
  ASSERT_EQ("module1", modules_without_symbols[0]->debug_file());
  ASSERT_EQ("module2", modules_without_symbols[1]->debug_file());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_EQ(3U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ(0, memcmp(&raw_context, &frame0->context, sizeof(raw_context)));

  StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
  EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame1->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP |
             StackFrameAMD64::CONTEXT_VALID_RBP),
            frame1->context_validity);
  EXPECT_EQ(return_address1, frame1->context.rip);
  EXPECT_EQ(frame1_sp.Value(), frame1->context.rsp);
  EXPECT_EQ(frame1_rbp.Value(), frame1->context.rbp);

  StackFrameAMD64 *frame2 = static_cast<StackFrameAMD64*>(frames->at(2));
  EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame2->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP),
            frame2->context_validity);
  EXPECT_EQ(return_address2, frame2->context.rip);
  EXPECT_EQ(frame2_sp.Value(), frame2->context.rsp);
}

TEST_F(GetCallerFrame, ScanWithFunctionSymbols) {
  // During stack scanning, if a potential return address
  // is located within a loaded module that has symbols,
  // it is only considered a valid return address if it
  // lies within a function's bounds.
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address = 0x00007500b0000110ULL;
  Label frame1_sp, frame1_rbp;

  stack_section
    // frame 0
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // junk that's not
    .D64(0x00007500b0000000ULL)         // a return address

    .D64(0x00007400c0001000ULL)         // a couple of plausible addresses
    .D64(0x00007500b000aaaaULL)         // that are not within functions

    .D64(return_address)                // actual return address
    // frame 1
    .Mark(&frame1_sp)
    .Append(32, 0)                      // end of stack
    .Mark(&frame1_rbp);
  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame1_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  SetModuleSymbols(&module1,
                   // The youngest frame's function.
                   "FUNC 100 400 10 platypus\n");
  SetModuleSymbols(&module2,
                   // The calling frame's function.
                   "FUNC 100 400 10 echidna\n");

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(0U, modules_without_symbols.size());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_EQ(2U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ("platypus", frame0->function_name);
  EXPECT_EQ(0x00007400c0000100ULL, frame0->function_base);

  StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
  EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame1->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP |
             StackFrameAMD64::CONTEXT_VALID_RBP),
            frame1->context_validity);
  EXPECT_EQ(return_address, frame1->context.rip);
  EXPECT_EQ(frame1_sp.Value(), frame1->context.rsp);
  EXPECT_EQ(frame1_rbp.Value(), frame1->context.rbp);
  EXPECT_EQ("echidna", frame1->function_name);
  EXPECT_EQ(0x00007500b0000100ULL, frame1->function_base);
}

// StackwalkerAMD64::GetCallerByFramePointerRecovery should never return an
// instruction pointer of 0 because IP of 0 is an end of stack marker and the
// stack walk may be terminated prematurely.  Instead it should return NULL
// so that the stack walking code can proceed to stack scanning.
TEST_F(GetCallerFrame, GetCallerByFramePointerRecovery) {
  MockCodeModule user32_dll(0x00007ff9cb8a0000ULL, 0x14E000, "user32.dll",
                            "version1");
  SetModuleSymbols(&user32_dll,  // user32.dll
                   "PUBLIC fa60 0 DispatchMessageWorker\n"
                   "PUBLIC fee0 0 UserCallWinProcCheckWow\n"
                   "PUBLIC 1cdb0 0 _fnHkINLPMSG\n"
                   "STACK CFI INIT fa60 340 .cfa: $rsp .ra: .cfa 8 - ^\n"
                   "STACK CFI fa60 .cfa: $rsp 128 +\n"
                   "STACK CFI INIT fee0 49f .cfa: $rsp .ra: .cfa 8 - ^\n"
                   "STACK CFI fee0 .cfa: $rsp 240 +\n"
                   "STACK CFI INIT 1cdb0 9f .cfa: $rsp .ra: .cfa 8 - ^\n"
                   "STACK CFI 1cdb0 .cfa: $rsp 80 +\n");

  // Create some modules with some stock debugging information.
  MockCodeModules local_modules;
  local_modules.Add(&user32_dll);

  Label frame0_rsp;
  Label frame0_rbp;
  Label frame1_rsp;
  Label frame2_rsp;

  stack_section.start() = 0x00000099abf0f238ULL;
  stack_section
    .Mark(&frame0_rsp)
    .D64(0x00007ff9cb8b00dcULL)
    .Mark(&frame1_rsp)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000001ULL)
    .D64(0x00000099abf0f308ULL)
    .D64(0x00007ff9cb8bce3aULL)  // Stack residue from execution of
                                 // user32!_fnHkINLPMSG+0x8a
    .D64(0x000000000000c2e0ULL)
    .D64(0x00000099abf0f328ULL)
    .D64(0x0000000100000001ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x00007ff9ccad53e4ULL)
    .D64(0x0000000000000048ULL)
    .D64(0x0000000000000001ULL)
    .D64(0x00000099abf0f5e0ULL)
    .D64(0x00000099b61f7388ULL)
    .D64(0x0000000000000030ULL)
    .D64(0xffffff66540f0a1fULL)
    .D64(0xffffff6649e08c77ULL)
    .D64(0x00007ff9cb8affb4ULL)  // Return address in
                                 // user32!UserCallWinProcCheckWow+0xd4
    .D64(0x0000000000000000ULL)
    .D64(0x00000099abf0f368ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x0000000000000000ULL)
    .D64(0x00000099a8150fd8ULL)
    .D64(0x00000099abf0f3e8ULL)
    .D64(0x00007ff9cb8afc07ULL)  // Return address in
                                 // user32!DispatchMessageWorker+0x1a7
    .Mark(&frame2_rsp)
    .Append(256, 0)
    .Mark(&frame0_rbp)           // The following are expected by
                                 // GetCallerByFramePointerRecovery.
    .D64(0xfffffffffffffffeULL)  // %caller_rbp = *(%callee_rbp)
    .D64(0x0000000000000000ULL)  // %caller_rip = *(%callee_rbp + 8)
    .D64(0x00000099a3e31040ULL)  // %caller_rsp = *(%callee_rbp + 16)
    .Append(256, 0);

  RegionFromSection();
  raw_context.rip = 0x00000099a8150fd8ULL;  // IP in context frame is guarbage
  raw_context.rsp = frame0_rsp.Value();
  raw_context.rbp = frame0_rbp.Value();

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region,
                          &local_modules, &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(0U, modules_without_symbols.size());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();

  ASSERT_EQ(3U, frames->size());

  {  // To avoid reusing locals by mistake
    StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(0));
    EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame->trust);
    ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame->context_validity);
    EXPECT_EQ("", frame->function_name);
    EXPECT_EQ(0x00000099a8150fd8ULL, frame->instruction);
    EXPECT_EQ(0x00000099a8150fd8ULL, frame->context.rip);
    EXPECT_EQ(frame0_rsp.Value(), frame->context.rsp);
    EXPECT_EQ(frame0_rbp.Value(), frame->context.rbp);
  }

  {  // To avoid reusing locals by mistake
    StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(1));
    EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame->trust);
    ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
               StackFrameAMD64::CONTEXT_VALID_RSP |
               StackFrameAMD64::CONTEXT_VALID_RBP),
              frame->context_validity);
    EXPECT_EQ("UserCallWinProcCheckWow", frame->function_name);
    EXPECT_EQ(140710838468828ULL, frame->instruction + 1);
    EXPECT_EQ(140710838468828ULL, frame->context.rip);
    EXPECT_EQ(frame1_rsp.Value(), frame->context.rsp);
    EXPECT_EQ(&user32_dll, frame->module);
  }

  {  // To avoid reusing locals by mistake
    StackFrameAMD64 *frame = static_cast<StackFrameAMD64*>(frames->at(2));
    EXPECT_EQ(StackFrame::FRAME_TRUST_CFI, frame->trust);
    ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
               StackFrameAMD64::CONTEXT_VALID_RSP |
               StackFrameAMD64::CONTEXT_VALID_RBP),
              frame->context_validity);
    EXPECT_EQ("DispatchMessageWorker", frame->function_name);
    EXPECT_EQ(140710838467591ULL, frame->instruction + 1);
    EXPECT_EQ(140710838467591ULL, frame->context.rip);
    EXPECT_EQ(frame2_rsp.Value(), frame->context.rsp);
    EXPECT_EQ(&user32_dll, frame->module);
  }
}

// Don't use frame pointer recovery if %rbp is not 8-byte aligned, which
// indicates that it's not being used as a frame pointer.
TEST_F(GetCallerFrame, FramePointerNotAligned) {
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address1 = 0x00007500b0000100ULL;
  Label frame0_rbp, not_frame1_rbp, frame1_sp;
  stack_section
    // frame 0
    .Align(8, 0)
    .Append(2, 0)                       // mis-align the frame pointer
    .Mark(&frame0_rbp)
    .D64(not_frame1_rbp)                // not the previous frame pointer
    .D64(0x00007500b0000a00ULL)         // plausible but wrong return address
    .Align(8, 0)
    .D64(return_address1)               // return address
    // frame 1
    .Mark(&frame1_sp)
    .Mark(&not_frame1_rbp)
    .Append(32, 0);                     // end of stack


  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame0_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  frames = call_stack.frames();
  ASSERT_EQ(2U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ(0, memcmp(&raw_context, &frame0->context, sizeof(raw_context)));

  StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
  EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame1->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP),
            frame1->context_validity);
  EXPECT_EQ(return_address1, frame1->context.rip);
  EXPECT_EQ(frame1_sp.Value(), frame1->context.rsp);
}

// Don't use frame pointer recovery if the recovered %rip is not
// a canonical x86-64 address.
TEST_F(GetCallerFrame, NonCanonicalInstructionPointerFromFramePointer) {
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address1 = 0x00007500b0000100ULL;
  Label frame0_rbp, frame1_sp, not_frame1_bp;
  stack_section
    // frame 0
    .Align(8, 0)
    .Mark(&frame0_rbp)
    .D64(not_frame1_bp)                 // some junk on the stack
    .D64(0xDADADADADADADADA)            // not the return address
    .D64(return_address1)               // return address
    // frame 1
    .Mark(&frame1_sp)
    .Append(16, 0)
    .Mark(&not_frame1_bp)
    .Append(32, 0);                     // end of stack


  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame0_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  frames = call_stack.frames();
  ASSERT_EQ(2U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ(0, memcmp(&raw_context, &frame0->context, sizeof(raw_context)));

  StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
  EXPECT_EQ(StackFrame::FRAME_TRUST_SCAN, frame1->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP),
            frame1->context_validity);
  EXPECT_EQ(return_address1, frame1->context.rip);
  EXPECT_EQ(frame1_sp.Value(), frame1->context.rsp);
}

// Test that set_max_frames_scanned prevents using stack scanning
// to find caller frames.
TEST_F(GetCallerFrame, ScanningNotAllowed) {
  // When the stack walker resorts to scanning the stack,
  // only addresses located within loaded modules are
  // considered valid return addresses.
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address1 = 0x00007500b0000100ULL;
  uint64_t return_address2 = 0x00007500b0000900ULL;
  Label frame1_sp, frame2_sp, frame1_rbp;
  stack_section
    // frame 0
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // junk that's not
    .D64(0x00007500d0000000ULL)         // a return address

    .D64(return_address1)               // actual return address
    // frame 1
    .Mark(&frame1_sp)
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // more junk
    .D64(0x00007500d0000000ULL)

    .Mark(&frame1_rbp)
    .D64(stack_section.start())         // This is in the right place to be
                                        // a saved rbp, but it's bogus, so
                                        // we shouldn't report it.

    .D64(return_address2)               // actual return address
    // frame 2
    .Mark(&frame2_sp)
    .Append(32, 0);                     // end of stack

  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame1_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  Stackwalker::set_max_frames_scanned(0);

  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(1U, modules_without_symbols.size());
  ASSERT_EQ("module1", modules_without_symbols[0]->debug_file());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_EQ(1U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ(0, memcmp(&raw_context, &frame0->context, sizeof(raw_context)));
}

TEST_F(GetCallerFrame, CallerPushedRBP) {
  // Functions typically push their %rbp upon entry and set %rbp pointing
  // there.  If stackwalking finds a plausible address for the next frame's
  // %rbp directly below the return address, assume that it is indeed the
  // next frame's %rbp.
  stack_section.start() = 0x8000000080000000ULL;
  uint64_t return_address = 0x00007500b0000110ULL;
  Label frame0_rbp, frame1_sp, frame1_rbp;

  stack_section
    // frame 0
    .Append(16, 0)                      // space

    .D64(0x00007400b0000000ULL)         // junk that's not
    .D64(0x00007500b0000000ULL)         // a return address

    .D64(0x00007400c0001000ULL)         // a couple of plausible addresses
    .D64(0x00007500b000aaaaULL)         // that are not within functions

    .Mark(&frame0_rbp)
    .D64(frame1_rbp)                    // caller-pushed %rbp
    .D64(return_address)                // actual return address
    // frame 1
    .Mark(&frame1_sp)
    .Append(32, 0)                      // body of frame1
    .Mark(&frame1_rbp)                  // end of stack
    .D64(0);
  RegionFromSection();

  raw_context.rip = 0x00007400c0000200ULL;
  raw_context.rbp = frame0_rbp.Value();
  raw_context.rsp = stack_section.start().Value();

  SetModuleSymbols(&module1,
                   // The youngest frame's function.
                   "FUNC 100 400 10 sasquatch\n");
  SetModuleSymbols(&module2,
                   // The calling frame's function.
                   "FUNC 100 400 10 yeti\n");

  StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
  StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                          &frame_symbolizer);
  vector<const CodeModule*> modules_without_symbols;
  vector<const CodeModule*> modules_with_corrupt_symbols;
  ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                          &modules_with_corrupt_symbols));
  ASSERT_EQ(0U, modules_without_symbols.size());
  ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
  frames = call_stack.frames();
  ASSERT_EQ(2U, frames->size());

  StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
  EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
  ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
  EXPECT_EQ(frame0_rbp.Value(), frame0->context.rbp);
  EXPECT_EQ("sasquatch", frame0->function_name);
  EXPECT_EQ(0x00007400c0000100ULL, frame0->function_base);

  StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
  EXPECT_EQ(StackFrame::FRAME_TRUST_FP, frame1->trust);
  ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
             StackFrameAMD64::CONTEXT_VALID_RSP |
             StackFrameAMD64::CONTEXT_VALID_RBP),
            frame1->context_validity);
  EXPECT_EQ(return_address, frame1->context.rip);
  EXPECT_EQ(frame1_sp.Value(), frame1->context.rsp);
  EXPECT_EQ(frame1_rbp.Value(), frame1->context.rbp);
  EXPECT_EQ("yeti", frame1->function_name);
  EXPECT_EQ(0x00007500b0000100ULL, frame1->function_base);
}

struct CFIFixture: public StackwalkerAMD64Fixture {
  CFIFixture() {
    // Provide a bunch of STACK CFI records; we'll walk to the caller
    // from every point in this series, expecting to find the same set
    // of register values.
    SetModuleSymbols(&module1,
                     // The youngest frame's function.
                     "FUNC 4000 1000 10 enchiridion\n"
                     // Initially, just a return address.
                     "STACK CFI INIT 4000 100 .cfa: $rsp 8 + .ra: .cfa 8 - ^\n"
                     // Push %rbx.
                     "STACK CFI 4001 .cfa: $rsp 16 + $rbx: .cfa 16 - ^\n"
                     // Save %r12 in %rbx.  Weird, but permitted.
                     "STACK CFI 4002 $r12: $rbx\n"
                     // Allocate frame space, and save %r13.
                     "STACK CFI 4003 .cfa: $rsp 40 + $r13: .cfa 32 - ^\n"
                     // Put the return address in %r13.
                     "STACK CFI 4005 .ra: $r13\n"
                     // Save %rbp, and use it as a frame pointer.
                     "STACK CFI 4006 .cfa: $rbp 16 + $rbp: .cfa 24 - ^\n"

                     // The calling function.
                     "FUNC 5000 1000 10 epictetus\n"
                     // Mark it as end of stack.
                     "STACK CFI INIT 5000 1000 .cfa: $rsp .ra 0\n");

    // Provide some distinctive values for the caller's registers.
    expected.rsp = 0x8000000080000000ULL;
    expected.rip = 0x00007400c0005510ULL;
    expected.rbp = 0x68995b1de4700266ULL;
    expected.rbx = 0x5a5beeb38de23be8ULL;
    expected.r12 = 0xed1b02e8cc0fc79cULL;
    expected.r13 = 0x1d20ad8acacbe930ULL;
    expected.r14 = 0xe94cffc2f7adaa28ULL;
    expected.r15 = 0xb638d17d8da413b5ULL;

    // By default, registers are unchanged.
    raw_context = expected;
  }

  // Walk the stack, using stack_section as the contents of the stack
  // and raw_context as the current register values. (Set
  // raw_context.rsp to the stack's starting address.) Expect two
  // stack frames; in the older frame, expect the callee-saves
  // registers to have values matching those in 'expected'.
  void CheckWalk() {
    RegionFromSection();
    raw_context.rsp = stack_section.start().Value();

    StackFrameSymbolizer frame_symbolizer(&supplier, &resolver);
    StackwalkerAMD64 walker(&system_info, &raw_context, &stack_region, &modules,
                            &frame_symbolizer);
    vector<const CodeModule*> modules_without_symbols;
    vector<const CodeModule*> modules_with_corrupt_symbols;
    ASSERT_TRUE(walker.Walk(&call_stack, &modules_without_symbols,
                            &modules_with_corrupt_symbols));
    ASSERT_EQ(0U, modules_without_symbols.size());
    ASSERT_EQ(0U, modules_with_corrupt_symbols.size());
    frames = call_stack.frames();
    ASSERT_EQ(2U, frames->size());

    StackFrameAMD64 *frame0 = static_cast<StackFrameAMD64*>(frames->at(0));
    EXPECT_EQ(StackFrame::FRAME_TRUST_CONTEXT, frame0->trust);
    ASSERT_EQ(StackFrameAMD64::CONTEXT_VALID_ALL, frame0->context_validity);
    EXPECT_EQ("enchiridion", frame0->function_name);
    EXPECT_EQ(0x00007400c0004000ULL, frame0->function_base);

    StackFrameAMD64 *frame1 = static_cast<StackFrameAMD64*>(frames->at(1));
    EXPECT_EQ(StackFrame::FRAME_TRUST_CFI, frame1->trust);
    ASSERT_EQ((StackFrameAMD64::CONTEXT_VALID_RIP |
               StackFrameAMD64::CONTEXT_VALID_RSP |
               StackFrameAMD64::CONTEXT_VALID_RBP |
               StackFrameAMD64::CONTEXT_VALID_RBX |
               StackFrameAMD64::CONTEXT_VALID_R12 |
               StackFrameAMD64::CONTEXT_VALID_R13 |
               StackFrameAMD64::CONTEXT_VALID_R14 |
               StackFrameAMD64::CONTEXT_VALID_R15),
              frame1->context_validity);
    EXPECT_EQ(expected.rip, frame1->context.rip);
    EXPECT_EQ(expected.rsp, frame1->context.rsp);
    EXPECT_EQ(expected.rbp, frame1->context.rbp);
    EXPECT_EQ(expected.rbx, frame1->context.rbx);
    EXPECT_EQ(expected.r12, frame1->context.r12);
    EXPECT_EQ(expected.r13, frame1->context.r13);
    EXPECT_EQ(expected.r14, frame1->context.r14);
    EXPECT_EQ(expected.r15, frame1->context.r15);
    EXPECT_EQ("epictetus", frame1->function_name);
  }

  // The values we expect to find for the caller's registers.
  MDRawContextAMD64 expected;
};

class CFI: public CFIFixture, public Test { };

TEST_F(CFI, At4000) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x00007400c0005510ULL) // return address
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004000ULL;
  CheckWalk();
}

TEST_F(CFI, At4001) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0x00007400c0005510ULL) // return address
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004001ULL;
  raw_context.rbx = 0xbe0487d2f9eafe29ULL; // callee's (distinct) %rbx value
  CheckWalk();
}

TEST_F(CFI, At4002) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0x00007400c0005510ULL) // return address
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004002ULL;
  raw_context.rbx = 0xed1b02e8cc0fc79cULL; // saved %r12
  raw_context.r12 = 0xb0118de918a4bceaULL; // callee's (distinct) %r12 value
  CheckWalk();
}

TEST_F(CFI, At4003) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x0e023828dffd4d81ULL) // garbage
    .D64(0x1d20ad8acacbe930ULL) // saved %r13
    .D64(0x319e68b49e3ace0fULL) // garbage
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0x00007400c0005510ULL) // return address
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004003ULL;
  raw_context.rbx = 0xed1b02e8cc0fc79cULL; // saved %r12
  raw_context.r12 = 0x89d04fa804c87a43ULL; // callee's (distinct) %r12
  raw_context.r13 = 0x5118e02cbdb24b03ULL; // callee's (distinct) %r13
  CheckWalk();
}

// The results here should be the same as those at module offset 0x4003.
TEST_F(CFI, At4004) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x0e023828dffd4d81ULL) // garbage
    .D64(0x1d20ad8acacbe930ULL) // saved %r13
    .D64(0x319e68b49e3ace0fULL) // garbage
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0x00007400c0005510ULL) // return address
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004004ULL;
  raw_context.rbx = 0xed1b02e8cc0fc79cULL; // saved %r12
  raw_context.r12 = 0x89d04fa804c87a43ULL; // callee's (distinct) %r12
  raw_context.r13 = 0x5118e02cbdb24b03ULL; // callee's (distinct) %r13
  CheckWalk();
}

TEST_F(CFI, At4005) {
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x4b516dd035745953ULL) // garbage
    .D64(0x1d20ad8acacbe930ULL) // saved %r13
    .D64(0xa6d445e16ae3d872ULL) // garbage
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0xaa95fa054aedfbaeULL) // garbage
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004005ULL;
  raw_context.rbx = 0xed1b02e8cc0fc79cULL; // saved %r12
  raw_context.r12 = 0x46b1b8868891b34aULL; // callee's %r12
  raw_context.r13 = 0x00007400c0005510ULL; // return address
  CheckWalk();
}

TEST_F(CFI, At4006) {
  Label frame0_rbp;
  Label frame1_rsp = expected.rsp;
  stack_section
    .D64(0x043c6dfceb91aa34ULL) // garbage
    .D64(0x1d20ad8acacbe930ULL) // saved %r13
    .D64(0x68995b1de4700266ULL) // saved %rbp
    .Mark(&frame0_rbp)          // frame pointer points here
    .D64(0x5a5beeb38de23be8ULL) // saved %rbx
    .D64(0xf015ee516ad89eabULL) // garbage
    .Mark(&frame1_rsp);         // This effectively sets stack_section.start().
  raw_context.rip = 0x00007400c0004006ULL;
  raw_context.rbp = frame0_rbp.Value();
  raw_context.rbx = 0xed1b02e8cc0fc79cULL; // saved %r12
  raw_context.r12 = 0x26e007b341acfebdULL; // callee's %r12
  raw_context.r13 = 0x00007400c0005510ULL; // return address
  CheckWalk();
}
