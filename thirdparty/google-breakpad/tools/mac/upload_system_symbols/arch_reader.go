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

package main

import (
	"debug/macho"
)

/*
#include "arch_constants.h"
*/
import "C"

// getArchStringFromHeader takes a MachO FileHeader and returns a string that
// represents the CPU type and subtype.
// This function is a Go version of src/common/mac/arch_utilities.cc:BreakpadGetArchInfoFromCpuType().
func getArchStringFromHeader(header macho.FileHeader) string {
	// TODO(rsesek): As of 10.9.4, OS X doesn't list these in /usr/include/mach/machine.h.
	if header.Cpu == C.kCPU_TYPE_ARM64 && header.SubCpu == C.kCPU_SUBTYPE_ARM64_ALL {
		return "arm64"
	}
	if header.Cpu == C.kCPU_TYPE_ARM64 && header.SubCpu == C.kCPU_SUBTYPE_ARM64_E {
		return "arm64e"
	}
	if header.Cpu == C.kCPU_TYPE_ARM && header.SubCpu == C.kCPU_SUBTYPE_ARM_V7S {
		return "armv7s"
	}

	cstr := C.GetNXArchInfoName(C.cpu_type_t(header.Cpu), C.cpu_subtype_t(header.SubCpu))
	if cstr == nil {
		return ""
	}
	return C.GoString(cstr)
}

const (
	MachODylib    macho.Type = C.kMachHeaderFtypeDylib
	MachOBundle              = C.kMachHeaderFtypeBundle
	MachOExe                 = C.kMachHeaderFtypeExe
	MachODylinker            = C.kMachHeaderFtypeDylinker
)
