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
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"reflect"
	"unsafe"
)

/*
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <string.h>

#include "arch_constants.h"
*/
import "C"

var (
	ErrNotMachO        = errors.New("GetMachOImageInfo: file is not a supported Mach-O image")
	ErrUnsupportedArch = errors.New("GetMachOImageInfo: unknown architecture detected")
)

const (
	ArchI386   = "i386"
	ArchX86_64 = "x86_64"
)

type MachOType int

const (
	MachODylib  MachOType = C.kMachHeaderFtypeDylib
	MachOBundle           = C.kMachHeaderFtypeBundle
	MachOExe              = C.kMachHeaderFtypeExe
)

type ImageInfo struct {
	Type MachOType
	Arch string
}

// GetMachOImageInfo will read the file at filepath and determine if it is
// Mach-O. If it is, it will return a slice of ImageInfo that describe the
// images in the file (may be more than one if it is a fat image).
func GetMachOImageInfo(filepath string) ([]ImageInfo, error) {
	f, err := os.Open(filepath)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	// Read the magic number to determine the type of file this is.
	var magic uint32
	err = binary.Read(f, binary.LittleEndian, &magic)
	if err != nil {
		return nil, err
	}

	// Rewind the file since the magic number is a field in the header
	// structs.
	f.Seek(0, os.SEEK_SET)

	switch magic {
	case C.kMachHeaderMagic32:
		return readThinHeader(f, C.kMachHeaderMagic32)
	case C.kMachHeaderMagic64:
		return readThinHeader(f, C.kMachHeaderMagic64)
	case C.kMachHeaderCigamFat: // Fat header is big-endian but was read in little.
		return readFatHeader(f)
	}

	return nil, ErrNotMachO
}

func readThinHeader(f *os.File, expectedMagic uint32) ([]ImageInfo, error) {
	var (
		magic, filetype uint32
		cpu             C.cpu_type_t
		err             error
	)

	if expectedMagic == C.kMachHeaderMagic32 {
		magic, cpu, filetype, err = readThin32Header(f)
	} else if expectedMagic == C.kMachHeaderMagic64 {
		magic, cpu, filetype, err = readThin64Header(f)
	} else {
		panic(fmt.Sprintf("Unexpected magic %#x", magic))
	}
	if err != nil {
		return nil, err
	}

	if magic != expectedMagic {
		return nil, fmt.Errorf("readThinHeader: unexpected magic number %#x", magic)
	}

	arch := cpuTypeToArch(cpu)
	if arch == "" {
		return nil, ErrUnsupportedArch
	}
	return []ImageInfo{{MachOType(filetype), arch}}, nil
}

func readThin32Header(f *os.File) (uint32, C.cpu_type_t, uint32, error) {
	var machHeader C.struct_mach_header
	err := readStruct(f, binary.LittleEndian, unsafe.Pointer(&machHeader), C.struct_mach_header{})
	if err != nil {
		return 0, 0, 0, err
	}
	return uint32(machHeader.magic), machHeader.cputype, uint32(machHeader.filetype), nil
}

func readThin64Header(f *os.File) (uint32, C.cpu_type_t, uint32, error) {
	var machHeader C.struct_mach_header_64
	err := readStruct(f, binary.LittleEndian, unsafe.Pointer(&machHeader), C.struct_mach_header_64{})
	if err != nil {
		return 0, 0, 0, err
	}
	return uint32(machHeader.magic), machHeader.cputype, uint32(machHeader.filetype), nil
}

func readFatHeader(f *os.File) ([]ImageInfo, error) {
	var fatHeader C.struct_fat_header
	err := readStruct(f, binary.BigEndian, unsafe.Pointer(&fatHeader), C.struct_fat_header{})
	if err != nil {
		return nil, err
	}

	if fatHeader.magic != C.kMachHeaderMagicFat {
		return nil, fmt.Errorf("readFatHeader: unexpected magic number %#x", fatHeader.magic)
	}

	// Read the fat_arch headers.
	headers := make([]C.struct_fat_arch, fatHeader.nfat_arch)
	for i := 0; i < int(fatHeader.nfat_arch); i++ {
		var fatArch C.struct_fat_arch
		err = readStruct(f, binary.BigEndian, unsafe.Pointer(&fatArch), C.struct_fat_arch{})
		if err != nil {
			return nil, fmt.Errorf("readFatHeader: %v", err)
		}
		headers[i] = fatArch
	}

	seenArches := make(map[string]int)

	// Now go to each arch in the fat image and read its mach header.
	infos := make([]ImageInfo, 0, len(headers))
	for _, header := range headers {
		f.Seek(int64(header.offset), os.SEEK_SET)

		var thinarch []ImageInfo
		var expectedArch string
		switch header.cputype {
		case C.kCPUType_i386:
			thinarch, err = readThinHeader(f, C.kMachHeaderMagic32)
			expectedArch = ArchI386
		case C.kCPUType_x86_64:
			thinarch, err = readThinHeader(f, C.kMachHeaderMagic64)
			expectedArch = ArchX86_64
		default:
			err = ErrUnsupportedArch
		}

		if err != nil {
			return nil, err
		}
		if thinarch[0].Arch != expectedArch {
			return nil, fmt.Errorf("readFatHeader: expected arch %d, got %d", thinarch[0].Arch, expectedArch)
		}

		infos = append(infos, thinarch[0])
		seenArches[thinarch[0].Arch]++
	}

	for arch, count := range seenArches {
		if count != 1 {
			return nil, fmt.Errorf("readFatHeader: duplicate arch %s detected", arch)
		}
	}

	return infos, nil
}

// TODO(rsesek): Support more arches.
func cpuTypeToArch(cpu C.cpu_type_t) string {
	switch cpu {
	case C.kCPUType_i386:
		return ArchI386
	case C.kCPUType_x86_64:
		return ArchX86_64
	default:
		return ""
	}
}

// readStruct is a incomplete version of binary.Read that uses unsafe pointers
// to set values in unexported fields. From |f|, this will read the fields of
// the |destType| template instance, in the specified byte |order|, and place
// the resulting memory into |dest|.
func readStruct(f *os.File, order binary.ByteOrder, dest unsafe.Pointer, destType interface{}) error {
	rv := reflect.ValueOf(destType)
	rt := rv.Type()
	destPtr := uintptr(dest)

	for i := 0; i < rv.NumField(); i++ {
		field := rv.Field(i)
		fieldType := rt.Field(i)

		var vp unsafe.Pointer
		var err error

		switch field.Kind() {
		case reflect.Int32:
			var v int32
			vp = unsafe.Pointer(&v)
			err = binary.Read(f, order, &v)
		case reflect.Uint32:
			var v uint32
			vp = unsafe.Pointer(&v)
			err = binary.Read(f, order, &v)
		default:
			err = fmt.Errorf("readStruct: unsupported type %v", fieldType)
		}

		if err != nil {
			return err
		}

		memcpy(destPtr+fieldType.Offset, vp, fieldType.Type.Size())
	}
	return nil
}

func memcpy(dest uintptr, value unsafe.Pointer, size uintptr) {
	C.memcpy(unsafe.Pointer(dest), value, C.size_t(size))
}
