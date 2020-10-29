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
	"testing"
)

func TestFat(t *testing.T) {
	tests := []struct {
		file   string
		mht    MachOType
		arches []string
	}{
		{"testdata/libarchtest32.dylib", MachODylib, []string{ArchI386}},
		{"testdata/libarchtest64.dylib", MachODylib, []string{ArchX86_64}},
		{"testdata/libarchtest.dylib", MachODylib, []string{ArchI386, ArchX86_64}},
		{"testdata/archtest32.exe", MachOExe, []string{ArchI386}},
		{"testdata/archtest64.exe", MachOExe, []string{ArchX86_64}},
		{"testdata/archtest.exe", MachOExe, []string{ArchI386, ArchX86_64}},
	}

	for _, e := range tests {
		imageinfo, err := GetMachOImageInfo(e.file)
		if err != nil {
			t.Errorf("Unexpected error: %v", err)
		}

		expected := make(map[string]bool)
		for _, arch := range e.arches {
			expected[arch] = false
		}

		if len(imageinfo) != len(e.arches) {
			t.Errorf("Wrong number of arches, got %d, expected %d", len(imageinfo), len(e.arches))
		}

		for _, ii := range imageinfo {
			if ii.Type != e.mht {
				t.Errorf("Wrong MachOType got %d, expected %d", ii.Type, e.mht)
			}
			if o, k := expected[ii.Arch]; o || !k {
				t.Errorf("Unexpected architecture %q", ii.Arch)
			}
			expected[ii.Arch] = true
		}

		for k, v := range expected {
			if !v {
				t.Errorf("Did not get expected architecture %s", k)
			}
		}
	}
}
