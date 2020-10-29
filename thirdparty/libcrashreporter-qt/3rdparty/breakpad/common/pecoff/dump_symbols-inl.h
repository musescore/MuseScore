// Copyright (c) 2011 Google Inc.
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

// dump_symbols-inl.h: implement google_breakpad::WriteSymbolFile:
// Find all the debugging info in a file and dump it as a Breakpad symbol file.

#ifndef COMMON_PECOFF_DUMP_SYMBOLS_INL_H
#define COMMON_PECOFF_DUMP_SYMBOLS_INL_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "common/dwarf/bytereader-inl.h"
#include "common/dwarf/dwarf2diehandler.h"
#include "common/dwarf_cfi_to_module.h"
#include "common/dwarf_cu_to_module.h"
#include "common/dwarf_line_to_module.h"
#include "common/module.h"
#include "common/scoped_ptr.h"
#ifndef NO_STABS_SUPPORT
#include "common/stabs_reader.h"
#include "common/stabs_to_module.h"
#endif
#include "common/using_std_string.h"

// This namespace contains helper functions.
namespace {

using google_breakpad::DumpOptions;
using google_breakpad::DwarfCFIToModule;
using google_breakpad::DwarfCUToModule;
using google_breakpad::DwarfLineToModule;
using google_breakpad::Module;
#ifndef NO_STABS_SUPPORT
using google_breakpad::StabsToModule;
#endif
using google_breakpad::scoped_ptr;

//
// FDWrapper
//
// Wrapper class to make sure opened file is closed.
//
class FDWrapper {
 public:
  explicit FDWrapper(int fd) :
    fd_(fd) {}
  ~FDWrapper() {
    if (fd_ != -1)
      close(fd_);
  }
  int get() {
    return fd_;
  }
  int release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
  }
 private:
  int fd_;
};

//
// MmapWrapper
//
// Wrapper class to make sure mapped regions are unmapped.
//
class MmapWrapper {
 public:
  MmapWrapper() : is_set_(false) {}
  ~MmapWrapper() {
    if (is_set_ && base_ != NULL) {
      assert(size_ > 0);
#ifndef _WIN32
      munmap(base_, size_);
#else
      UnmapViewOfFile(base_);
      CloseHandle(hMap_);
#endif
    }
  }
  void *set(int obj_fd, size_t mapped_size) {
#ifndef _WIN32
    void *mapped_address = mmap(NULL, mapped_size,
          PROT_READ | PROT_WRITE, MAP_PRIVATE, obj_fd, 0);
    if (mapped_address == MAP_FAILED)
      return NULL;
#else
    HANDLE h = (HANDLE)_get_osfhandle(obj_fd);
    hMap_ = CreateFileMapping(h, NULL, PAGE_READONLY,0, 0, NULL);
    // XXX: should also use SEC_IMAGE_NO_EXECUTE on Windows 6.2 or later
    if (!hMap_) {
      return NULL;
    }
    void *mapped_address = MapViewOfFile(hMap_, FILE_MAP_READ, 0, 0, 0);
    if (!mapped_address) {
      CloseHandle(hMap_);
      return NULL;
    }
#endif
    is_set_ = true;
    base_ = mapped_address;
    size_ = mapped_size;
    return mapped_address;
  }
  void release() {
    assert(is_set_);
    is_set_ = false;
    base_ = NULL;
    size_ = 0;
  }

 private:
  bool is_set_;
  void *base_;
  size_t size_;
#ifdef _WIN32
  HANDLE hMap_;
#endif
};

#ifndef NO_STABS_SUPPORT
template<typename ObjectFileReader>
bool LoadStabs(const typename ObjectFileReader::ObjectFileBase header,
               const typename ObjectFileReader::Section stab_section,
               const typename ObjectFileReader::Section stabstr_section,
               const bool big_endian,
               Module* module) {
  // A callback object to handle data from the STABS reader.
  StabsToModule handler(module);
  // Find the addresses of the STABS data, and create a STABS reader object.
  // On Linux, STABS entries always have 32-bit values, regardless of the
  // address size of the architecture whose code they're describing, and
  // the strings are always "unitized".
  const uint8_t* stabs = ObjectFileReader::GetSectionPointer(header,
                                                             stab_section);
  const uint8_t* stabstr = ObjectFileReader::GetSectionPointer(header,
                                                               stabstr_section);
  google_breakpad::StabsReader reader(stabs,
                                      ObjectFileReader::GetSectionSize(header, stab_section),
                                      stabstr,
                                      ObjectFileReader::GetSectionSize(header, stabstr_section),
                                      big_endian, 4, true, &handler);
  // Read the STABS data, and do post-processing.
  if (!reader.Process())
    return false;
  handler.Finalize();
  return true;
}
#endif  // NO_STABS_SUPPORT

// A line-to-module loader that accepts line number info parsed by
// dwarf2reader::LineInfo and populates a Module and a line vector
// with the results.
class DumperLineToModule: public DwarfCUToModule::LineToModuleHandler {
 public:
  // Create a line-to-module converter using BYTE_READER.
  explicit DumperLineToModule(dwarf2reader::ByteReader *byte_reader)
      : byte_reader_(byte_reader) { }
  void StartCompilationUnit(const string& compilation_dir) {
    compilation_dir_ = compilation_dir;
  }
  void ReadProgram(const char *program, uint64 length,
                   Module *module, std::vector<Module::Line> *lines) {
    DwarfLineToModule handler(module, compilation_dir_, lines);
    dwarf2reader::LineInfo parser(program, length, byte_reader_, &handler);
    parser.Start();
  }
 private:
  string compilation_dir_;
  dwarf2reader::ByteReader *byte_reader_;
};

template<typename ObjectFileReader>
bool LoadDwarf(const string& dwarf_filename,
               const typename ObjectFileReader::ObjectFileBase header,
               const bool big_endian,
               bool handle_inter_cu_refs,
               Module* module) {
  typedef typename ObjectFileReader::Section Shdr;

  const dwarf2reader::Endianness endianness = big_endian ?
      dwarf2reader::ENDIANNESS_BIG : dwarf2reader::ENDIANNESS_LITTLE;
  dwarf2reader::ByteReader byte_reader(endianness);

  // Construct a context for this file.
  DwarfCUToModule::FileContext file_context(dwarf_filename,
                                            module,
                                            handle_inter_cu_refs);

  // Build a map of the file's sections.
  int num_sections = ObjectFileReader::GetNumberOfSections(header);
  for (int i = 0; i < num_sections; ++i) {
    const Shdr section = ObjectFileReader::FindSectionByIndex(header, i);
    string name = ObjectFileReader::GetSectionName(header, section);
    const char* contents = reinterpret_cast<const char *>(ObjectFileReader::GetSectionPointer(header, section));
    file_context.AddSectionToSectionMap(name, contents,
                                        ObjectFileReader::GetSectionSize(header, section));
  }

  // Parse all the compilation units in the .debug_info section.
  DumperLineToModule line_to_module(&byte_reader);
  dwarf2reader::SectionMap::const_iterator debug_info_entry =
      file_context.section_map().find(".debug_info");
  assert(debug_info_entry != file_context.section_map().end());
  const std::pair<const char*, uint64>& debug_info_section =
      debug_info_entry->second;
  // This should never have been called if the file doesn't have a
  // .debug_info section.
  assert(debug_info_section.first);
  uint64 debug_info_length = debug_info_section.second;
  for (uint64 offset = 0; offset < debug_info_length;) {
    // Make a handler for the root DIE that populates MODULE with the
    // data that was found.
    DwarfCUToModule::WarningReporter reporter(dwarf_filename, offset);
    DwarfCUToModule root_handler(&file_context, &line_to_module, &reporter);
    // Make a Dwarf2Handler that drives the DIEHandler.
    dwarf2reader::DIEDispatcher die_dispatcher(&root_handler);
    // Make a DWARF parser for the compilation unit at OFFSET.
    dwarf2reader::CompilationUnit reader(file_context.section_map(),
                                         offset,
                                         &byte_reader,
                                         &die_dispatcher);
    // Process the entire compilation unit; get the offset of the next.
    offset += reader.Start();
  }
  return true;
}

// Fill REGISTER_NAMES with the register names appropriate to the
// machine architecture, indexed by the register
// numbers used in DWARF call frame information. Return true on
// success, or false if HEADER's machine architecture is not
// supported.
bool DwarfCFIRegisterNames(const char *architecture,
                           std::vector<string>* register_names) {
  if (strcmp(architecture, "x86" ) == 0)
      *register_names = DwarfCFIToModule::RegisterNames::I386();
  else if (strcmp(architecture, "arm" ) == 0)
      *register_names = DwarfCFIToModule::RegisterNames::ARM();
  else if (strcmp(architecture, "mips" ) == 0)
      *register_names = DwarfCFIToModule::RegisterNames::MIPS();
  else if (strcmp(architecture, "x86_64" ) == 0)
      *register_names = DwarfCFIToModule::RegisterNames::X86_64();
  else
      return false;

  return true;
}

template<typename ObjectFileReader>
bool LoadDwarfCFI(const string& dwarf_filename,
                  const typename ObjectFileReader::ObjectFileBase header,
                  const char* section_name,
                  const typename ObjectFileReader::Section section,
                  const bool eh_frame,
                  const typename ObjectFileReader::Section got_section,
                  const typename ObjectFileReader::Section text_section,
                  const bool big_endian,
                  Module* module) {
  // Find the appropriate set of register names for this file's
  // architecture.
  const char *architecture = ObjectFileReader::Architecture(header);
  std::vector<string> register_names;
  if (!DwarfCFIRegisterNames(architecture, &register_names)) {
    return false;
  }

  const dwarf2reader::Endianness endianness = big_endian ?
      dwarf2reader::ENDIANNESS_BIG : dwarf2reader::ENDIANNESS_LITTLE;

  // Find the call frame information and its size.
  const char* cfi = reinterpret_cast<const char *>(ObjectFileReader::GetSectionPointer(header, section));
  size_t cfi_size = ObjectFileReader::GetSectionSize(header, section);

  // Plug together the parser, handler, and their entourages.
  DwarfCFIToModule::Reporter module_reporter(dwarf_filename, section_name);
  DwarfCFIToModule handler(module, register_names, &module_reporter);
  dwarf2reader::ByteReader byte_reader(endianness);

  byte_reader.SetAddressSize(ObjectFileReader::kAddrSize);

  // Provide the base addresses for .eh_frame encoded pointers, if
  // possible.
  byte_reader.SetCFIDataBase(ObjectFileReader::GetSectionRVA(header, section) +
                             ObjectFileReader::GetLoadingAddress(header),
                             cfi);
  if (got_section)
    byte_reader.SetDataBase(ObjectFileReader::GetSectionRVA(header, got_section) +
                              ObjectFileReader::GetLoadingAddress(header));
  if (text_section)
    byte_reader.SetTextBase(ObjectFileReader::GetSectionRVA(header, text_section) +
                             ObjectFileReader::GetLoadingAddress(header));

  dwarf2reader::CallFrameInfo::Reporter dwarf_reporter(dwarf_filename,
                                                       section_name);
  dwarf2reader::CallFrameInfo parser(cfi, cfi_size,
                                     &byte_reader, &handler, &dwarf_reporter,
                                     eh_frame);
  parser.Start();
  return true;
}

bool LoadFile(const string& obj_file, MmapWrapper* map_wrapper,
             const void** header) {
  int obj_fd = open(obj_file.c_str(), O_RDONLY);
  if (obj_fd < 0) {
    fprintf(stderr, "Failed to open file '%s': %s\n",
            obj_file.c_str(), strerror(errno));
    return false;
  }
  FDWrapper obj_fd_wrapper(obj_fd);
  struct stat st;
  if (fstat(obj_fd, &st) != 0 && st.st_size <= 0) {
    fprintf(stderr, "Unable to fstat file '%s': %s\n",
            obj_file.c_str(), strerror(errno));
    return false;
  }
  *header = map_wrapper->set(obj_fd, st.st_size);
  if (!(*header)) {
    fprintf(stderr, "Failed to mmap file '%s': %s\n",
            obj_file.c_str(), strerror(errno));
    return false;
  }
  return true;
}

// Read the .gnu_debuglink and get the debug file name. If anything goes
// wrong, return an empty string.
template<typename ObjectFileReader>
string ReadDebugLink(const char* debuglink,
                     size_t debuglink_size,
                     const string& obj_file,
                     const std::vector<string>& debug_dirs) {
  size_t debuglink_len = strlen(debuglink) + 5;  // '\0' + CRC32.
  debuglink_len = 4 * ((debuglink_len + 3) / 4);  // Round to nearest 4 bytes.

  // Sanity check.
  if (debuglink_len != debuglink_size) {
    fprintf(stderr, "Mismatched .gnu_debuglink string / section size: "
            "%zx %zx\n", debuglink_len, debuglink_size);
    return "";
  }

  bool found = false;
  int debuglink_fd = -1;
  string debuglink_path;
  std::vector<string>::const_iterator it;
  for (it = debug_dirs.begin(); it < debug_dirs.end(); ++it) {
    const string& debug_dir = *it;
    debuglink_path = debug_dir + "/" + debuglink;
    debuglink_fd = open(debuglink_path.c_str(), O_RDONLY);
    if (debuglink_fd >= 0) {
      found = true;
      break;
    }
  }

  if (!found) {
    fprintf(stderr, "Failed to find debug file for '%s' after trying:\n",
            obj_file.c_str());
    for (it = debug_dirs.begin(); it < debug_dirs.end(); ++it) {
      const string debug_dir = *it;
      fprintf(stderr, "  %s/%s\n", debug_dir.c_str(), debuglink);
    }
    return "";
  }

  FDWrapper debuglink_fd_wrapper(debuglink_fd);
  // TODO(thestig) check the CRC-32 at the end of the .gnu_debuglink
  // section.

  return debuglink_path;
}

//
// LoadSymbolsInfo
//
// Holds the state between the two calls to LoadSymbols() in case it's necessary
// to follow the .gnu_debuglink section and load debug information from a
// different file.
//
template<typename ObjectFileReader>
class LoadSymbolsInfo {
 public:
  typedef typename ObjectFileReader::Addr Addr;

  explicit LoadSymbolsInfo(const std::vector<string>& dbg_dirs) :
    debug_dirs_(dbg_dirs),
    has_loading_addr_(false) {}

  // Keeps track of which sections have been loaded so sections don't
  // accidentally get loaded twice from two different files.
  void LoadedSection(const string &section) {
    if (loaded_sections_.count(section) == 0) {
      loaded_sections_.insert(section);
    } else {
      fprintf(stderr, "Section %s has already been loaded.\n",
              section.c_str());
    }
  }

  // The file and linked debug file are expected to have the same preferred
  // loading address.
  void set_loading_addr(Addr addr, const string &filename) {
    if (!has_loading_addr_) {
      loading_addr_ = addr;
      loaded_file_ = filename;
      return;
    }

    if (addr != loading_addr_) {
      fprintf(stderr,
              "file '%s' and debug file '%s' "
              "have different load addresses.\n",
              loaded_file_.c_str(), filename.c_str());
      assert(false);
    }
  }

  // Setters and getters
  const std::vector<string>& debug_dirs() const {
    return debug_dirs_;
  }

  string debuglink_file() const {
    return debuglink_file_;
  }
  void set_debuglink_file(string file) {
    debuglink_file_ = file;
  }

 private:
  const std::vector<string>& debug_dirs_; // Directories in which to
                                          // search for the debug file.

  string debuglink_file_;  // Full path to the debug file.

  bool has_loading_addr_;  // Indicate if LOADING_ADDR_ is valid.

  Addr loading_addr_;  // Saves the preferred loading address from the
                       // first call to LoadSymbols().

  string loaded_file_;  // Name of the file loaded from the first call to
                        // LoadSymbols().

  std::set<string> loaded_sections_;  // Tracks the Loaded sections
                                      // between calls to LoadSymbols().
};

template<typename ObjectFileReader>
bool LoadSymbols(const string& obj_file,
                 const bool big_endian,
                 const typename ObjectFileReader::ObjectFileBase header,
                 const bool read_gnu_debug_link,
                 LoadSymbolsInfo<ObjectFileReader>* info,
                 const DumpOptions& options,
                 Module* module) {
  typedef typename ObjectFileReader::Addr Addr;
  typedef typename ObjectFileReader::Section Shdr;

  Addr loading_addr = ObjectFileReader::GetLoadingAddress(header);
  module->SetLoadAddress(loading_addr);
  info->set_loading_addr(loading_addr, obj_file);

  bool found_debug_info_section = false;
  bool found_usable_info = false;

  if (options.symbol_data != ONLY_CFI) {
#ifndef NO_STABS_SUPPORT
    // Look for STABS debugging information, and load it if present.
    const Shdr stab_section =
        ObjectFileReader::FindSectionByName(".stab", header);
    if (stab_section) {
      const Shdr stabstr_section = ObjectFileReader::FindLinkedSection(header, stab_section);
      if (stabstr_section) {
        found_debug_info_section = true;
        found_usable_info = true;
        info->LoadedSection(".stab");
        if (!LoadStabs<ObjectFileReader>(header, stab_section, stabstr_section,
                                 big_endian, module)) {
          fprintf(stderr, "%s: \".stab\" section found, but failed to load"
                  " STABS debugging information\n", obj_file.c_str());
        }
      }
    }
#endif  // NO_STABS_SUPPORT

    // Look for DWARF debugging information, and load it if present.
    const Shdr dwarf_section =
        ObjectFileReader::FindSectionByName(".debug_info", header);
    if (dwarf_section) {
      found_debug_info_section = true;
      found_usable_info = true;
      info->LoadedSection(".debug_info");
      if (!LoadDwarf<ObjectFileReader>(obj_file, header, big_endian,
                               options.handle_inter_cu_refs, module)) {
        fprintf(stderr, "%s: \".debug_info\" section found, but failed to load "
                "DWARF debugging information\n", obj_file.c_str());
      }
    }
  }

  if (options.symbol_data != NO_CFI) {
    // Dwarf Call Frame Information (CFI) is actually independent from
    // the other DWARF debugging information, and can be used alone.
    const Shdr dwarf_cfi_section =
        ObjectFileReader::FindSectionByName(".debug_frame", header);
    if (dwarf_cfi_section) {
      // Ignore the return value of this function; even without call frame
      // information, the other debugging information could be perfectly
      // useful.
      info->LoadedSection(".debug_frame");
      bool result =
          LoadDwarfCFI<ObjectFileReader>(obj_file, header, ".debug_frame",
                                 dwarf_cfi_section, false, 0, 0, big_endian,
                                 module);
      found_usable_info = found_usable_info || result;
    }

    // Linux C++ exception handling information can also provide
    // unwinding data.
    const Shdr eh_frame_section =
        ObjectFileReader::FindSectionByName(".eh_frame", header);
    if (eh_frame_section) {
      // Pointers in .eh_frame data may be relative to the base addresses of
      // certain sections. Provide those sections if present.
      const Shdr got_section =
          ObjectFileReader::FindSectionByName(".got", header);
      const Shdr text_section =
          ObjectFileReader::FindSectionByName(".text", header);
      info->LoadedSection(".eh_frame");
      // As above, ignore the return value of this function.
      bool result =
          LoadDwarfCFI<ObjectFileReader>(obj_file, header, ".eh_frame",
                                 eh_frame_section, true,
                                 got_section, text_section, big_endian, module);
      found_usable_info = found_usable_info || result;
    }
  }

  if (!found_debug_info_section) {
    fprintf(stderr, "%s: file contains no debugging information"
            " (no \".stab\" or \".debug_info\" sections)\n",
            obj_file.c_str());

    // Failed, but maybe there's a .gnu_debuglink section?
    if (read_gnu_debug_link) {
      const Shdr gnu_debuglink_section
          = ObjectFileReader::FindSectionByName(".gnu_debuglink", header);
      if (gnu_debuglink_section) {
        if (!info->debug_dirs().empty()) {
          const char* debuglink_contents = reinterpret_cast<const char *>
              (ObjectFileReader::GetSectionPointer(header, gnu_debuglink_section));
          string debuglink_file
              = ReadDebugLink<ObjectFileReader>(debuglink_contents,
                                                ObjectFileReader::GetSectionSize(header, gnu_debuglink_section),
                                                obj_file, info->debug_dirs());
          info->set_debuglink_file(debuglink_file);
        } else {
          fprintf(stderr, ".gnu_debuglink section found in '%s', "
                  "but no debug path specified.\n", obj_file.c_str());
        }
      } else {
        fprintf(stderr, "%s does not contain a .gnu_debuglink section.\n",
                obj_file.c_str());
      }
    } else {
      if (options.symbol_data != ONLY_CFI) {
        // The caller doesn't want to consult .gnu_debuglink.
        // See if there are export symbols available.
        bool result = ObjectFileReader::ExportedSymbolsToModule(header, module);
        found_usable_info = found_usable_info || result;
      }

      // Return true if some usable information was found, since
      // the caller doesn't want to use .gnu_debuglink.
      return found_usable_info;
    }

    // No debug info was found, let the user try again with .gnu_debuglink
    // if present.
    return false;
  }

  return true;
}

// Return the non-directory portion of FILENAME: the portion after the
// last slash, or the whole filename if there are no slashes.
string BaseFileName(const string &filename) {
  // Lots of copies!  basename's behavior is less than ideal.
  char *c_filename = strdup(filename.c_str());
  string base = basename(c_filename);
  free(c_filename);
  return base;
}

template<typename ObjectFileReader>
bool ReadSymbolDataFromObjectFile(
    const typename ObjectFileReader::ObjectFileBase header,
    const string& obj_filename,
    const std::vector<string>& debug_dirs,
    const DumpOptions& options,
    Module** out_module) {

  typedef typename ObjectFileReader::Section Shdr;

  *out_module = NULL;

  string identifier = ObjectFileReader::FileIdentifierFromMappedFile(header);
  if (identifier.empty()) {
    fprintf(stderr, "%s: unable to generate file identifier\n",
            obj_filename.c_str());
    return false;
  }

  const char *architecture = ObjectFileReader::Architecture(header);
  if (!architecture) {
    return false;
  }

  // Figure out what endianness this file is.
  bool big_endian;
  if (!ObjectFileReader::Endianness(header, &big_endian))
    return false;

  string name = BaseFileName(obj_filename);
  string os = "windows";
  string id = identifier;

  LoadSymbolsInfo<ObjectFileReader> info(debug_dirs);
  scoped_ptr<Module> module(new Module(name, os, architecture, id));
  if (!LoadSymbols<ObjectFileReader>(obj_filename, big_endian, header,
                             !debug_dirs.empty(), &info,
                             options, module.get())) {
    const string debuglink_file = info.debuglink_file();
    if (debuglink_file.empty())
      return false;

    // Load debuglink file.
    fprintf(stderr, "Found debugging info in %s\n", debuglink_file.c_str());
    MmapWrapper debug_map_wrapper;
    typename ObjectFileReader::ObjectFileBase debug_header = NULL;
    if (!LoadFile(debuglink_file, &debug_map_wrapper,
                 reinterpret_cast<const void**>(&debug_header)))
      return false;

    if (!ObjectFileReader::IsValid(debug_header)) {
      fprintf(stderr, "Not a valid file: %s\n", debuglink_file.c_str());
      return false;
    }

    // Sanity checks to make sure everything matches up.
    const char *debug_architecture =
        ObjectFileReader::Architecture(debug_header);
    if (!debug_architecture) {
      return false;
    }
    if (strcmp(architecture, debug_architecture)) {
      fprintf(stderr, "%s with machine architecture %s does not match "
              "%s with architecture %s\n",
              debuglink_file.c_str(), debug_architecture,
              obj_filename.c_str(), architecture);
      return false;
    }

    bool debug_big_endian;
    if (!ObjectFileReader::Endianness(debug_header, &debug_big_endian))
      return false;
    if (debug_big_endian != big_endian) {
      fprintf(stderr, "%s and %s does not match in endianness\n",
              obj_filename.c_str(), debuglink_file.c_str());
      return false;
    }

    if (!LoadSymbols<ObjectFileReader>(debuglink_file, debug_big_endian,
                               debug_header, false, &info,
                               options, module.get())) {
      return false;
    }
  }

  *out_module = module.release();
  return true;
}

}  // namespace

#endif // COMMON_PECOFF_DUMP_SYMBOLS_INL_H
