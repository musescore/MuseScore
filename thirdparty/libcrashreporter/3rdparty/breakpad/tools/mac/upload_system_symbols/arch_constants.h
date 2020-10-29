/* Copyright 2014, Google Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
 * Neither the name of Google Inc. nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <mach-o/fat.h>
#include <mach-o/loader.h>

// Go/Cgo does not support #define constants, so turn them into symbols
// that are reachable from Go.

const cpu_type_t kCPUType_i386 = CPU_TYPE_I386;
const cpu_type_t kCPUType_x86_64 = CPU_TYPE_X86_64;

const uint32_t kMachHeaderMagic32 = MH_MAGIC;
const uint32_t kMachHeaderMagic64 = MH_MAGIC_64;
const uint32_t kMachHeaderMagicFat = FAT_MAGIC;
const uint32_t kMachHeaderCigamFat = FAT_CIGAM;

const uint32_t kMachHeaderFtypeDylib = MH_DYLIB;
const uint32_t kMachHeaderFtypeBundle = MH_BUNDLE;
const uint32_t kMachHeaderFtypeExe = MH_EXECUTE;
