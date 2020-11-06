// Copyright (c) 2018, Google Inc.
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

#include "processor/convert_old_arm64_context.h"

#include <string.h>

namespace google_breakpad {

void ConvertOldARM64Context(const MDRawContextARM64_Old& old,
                            MDRawContextARM64* context) {
  context->context_flags = MD_CONTEXT_ARM64;
  if (old.context_flags & MD_CONTEXT_ARM64_INTEGER_OLD) {
    context->context_flags |=
        MD_CONTEXT_ARM64_INTEGER | MD_CONTEXT_ARM64_CONTROL;
  }
  if (old.context_flags & MD_CONTEXT_ARM64_FLOATING_POINT_OLD) {
    context->context_flags |= MD_CONTEXT_ARM64_FLOATING_POINT;
  }

  context->cpsr = old.cpsr;

  static_assert(sizeof(old.iregs) == sizeof(context->iregs),
                "iregs size mismatch");
  memcpy(context->iregs, old.iregs, sizeof(context->iregs));

  static_assert(sizeof(old.float_save.regs) == sizeof(context->float_save.regs),
                "float_save.regs size mismatch");
  memcpy(context->float_save.regs,
         old.float_save.regs,
         sizeof(context->float_save.regs));
  context->float_save.fpcr = old.float_save.fpcr;
  context->float_save.fpsr = old.float_save.fpsr;

  memset(context->bcr, 0, sizeof(context->bcr));
  memset(context->bvr, 0, sizeof(context->bvr));
  memset(context->wcr, 0, sizeof(context->wcr));
  memset(context->wvr, 0, sizeof(context->wvr));
}

}  // namespace google_breakpad
