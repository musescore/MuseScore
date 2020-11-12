// -*- mode: c++ -*-

// Copyright (c) 2011, Google Inc.
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

// Author: Jim Blandy <jimb@mozilla.com> <jimb@red-bean.com>

// dump_syms.cc: Create a symbol file for use with minidumps

#include "common/mac/dump_syms.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <mach-o/arch.h>
#include <mach-o/fat.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <ostream>
#include <string>
#include <vector>

#include "common/dwarf/bytereader-inl.h"
#include "common/dwarf/dwarf2reader.h"
#include "common/dwarf_cfi_to_module.h"
#include "common/dwarf_cu_to_module.h"
#include "common/dwarf_line_to_module.h"
#include "common/dwarf_range_list_handler.h"
#include "common/mac/file_id.h"
#include "common/mac/arch_utilities.h"
#include "common/mac/macho_reader.h"
#include "common/module.h"
#include "common/path_helper.h"
#include "common/scoped_ptr.h"
#include "common/stabs_reader.h"
#include "common/stabs_to_module.h"
#include "common/symbol_data.h"

#ifndef CPU_TYPE_ARM
#define CPU_TYPE_ARM (static_cast<cpu_type_t>(12))
#endif //  CPU_TYPE_ARM

#ifndef CPU_TYPE_ARM64
#define CPU_TYPE_ARM64 (static_cast<cpu_type_t>(16777228))
#endif  // CPU_TYPE_ARM64

using dwarf2reader::ByteReader;
using google_breakpad::DwarfCUToModule;
using google_breakpad::DwarfLineToModule;
using google_breakpad::DwarfRangeListHandler;
using google_breakpad::FileID;
using google_breakpad::mach_o::FatReader;
using google_breakpad::mach_o::Section;
using google_breakpad::mach_o::Segment;
using google_breakpad::Module;
using google_breakpad::StabsReader;
using google_breakpad::StabsToModule;
using google_breakpad::scoped_ptr;
using std::make_pair;
using std::pair;
using std::string;
using std::vector;

namespace {
// Return a vector<string> with absolute paths to all the entries
// in directory (excluding . and ..).
vector<string> list_directory(const string& directory) {
  vector<string> entries;
  DIR* dir = opendir(directory.c_str());
  if (!dir) {
    return entries;
  }

  string path = directory;
  if (path[path.length() - 1] != '/') {
    path += '/';
  }

  struct dirent* entry = NULL;
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      entries.push_back(path + entry->d_name);
    }
  }

  closedir(dir);
  return entries;
}
}

namespace google_breakpad {

bool DumpSymbols::Read(const string& filename) {
  struct stat st;
  if (stat(filename.c_str(), &st) == -1) {
    fprintf(stderr, "Could not access object file %s: %s\n",
            filename.c_str(), strerror(errno));
    return false;
  }

  input_pathname_ = filename;

  // Does this filename refer to a dSYM bundle?
  string contents_path = input_pathname_ + "/Contents/Resources/DWARF";
  if (S_ISDIR(st.st_mode) &&
      access(contents_path.c_str(), F_OK) == 0) {
    // If there's one file under Contents/Resources/DWARF then use that,
    // otherwise bail out.
    const vector<string> entries = list_directory(contents_path);
    if (entries.size() == 0) {
      fprintf(stderr, "Unable to find DWARF-bearing file in bundle: %s\n",
              input_pathname_.c_str());
      return false;
    }
    if (entries.size() > 1) {
      fprintf(stderr, "Too many DWARF files in bundle: %s\n",
              input_pathname_.c_str());
      return false;
    }

    object_filename_ = entries[0];
  } else {
    object_filename_ = input_pathname_;
  }

  // Read the file's contents into memory.
  bool read_ok = true;
  string error;
  if (stat(object_filename_.c_str(), &st) != -1) {
    FILE* f = fopen(object_filename_.c_str(), "rb");
    if (f) {
      contents_.reset(new uint8_t[st.st_size]);
      off_t total = 0;
      while (total < st.st_size && !feof(f)) {
        size_t read = fread(&contents_[0] + total, 1, st.st_size - total, f);
        if (read == 0) {
          if (ferror(f)) {
            read_ok = false;
            error = strerror(errno);
          }
          break;
        }
        total += read;
      }
      fclose(f);
    } else {
      error = strerror(errno);
    }
  }

  if (!read_ok) {
    fprintf(stderr, "Error reading object file: %s: %s\n",
            object_filename_.c_str(),
            error.c_str());
    return false;
  }

  // Get the list of object files present in the file.
  FatReader::Reporter fat_reporter(object_filename_);
  FatReader fat_reader(&fat_reporter);
  if (!fat_reader.Read(&contents_[0],
                       st.st_size)) {
    return false;
  }

  // Get our own copy of fat_reader's object file list.
  size_t object_files_count;
  const SuperFatArch* object_files =
    fat_reader.object_files(&object_files_count);
  if (object_files_count == 0) {
    fprintf(stderr, "Fat binary file contains *no* architectures: %s\n",
            object_filename_.c_str());
    return false;
  }
  object_files_.resize(object_files_count);
  memcpy(&object_files_[0], object_files,
         sizeof(SuperFatArch) * object_files_count);

  return true;
}

bool DumpSymbols::SetArchitecture(cpu_type_t cpu_type,
                                  cpu_subtype_t cpu_subtype) {
  // Find the best match for the architecture the user requested.
  const SuperFatArch* best_match = FindBestMatchForArchitecture(
      cpu_type, cpu_subtype);
  if (!best_match) return false;

  // Record the selected object file.
  selected_object_file_ = best_match;
  return true;
}

bool DumpSymbols::SetArchitecture(const std::string& arch_name) {
  bool arch_set = false;
  const NXArchInfo* arch_info =
      google_breakpad::BreakpadGetArchInfoFromName(arch_name.c_str());
  if (arch_info) {
    arch_set = SetArchitecture(arch_info->cputype, arch_info->cpusubtype);
  }
  return arch_set;
}

SuperFatArch* DumpSymbols::FindBestMatchForArchitecture(
    cpu_type_t cpu_type, cpu_subtype_t cpu_subtype) {
  // Check if all the object files can be converted to struct fat_arch.
  bool can_convert_to_fat_arch = true;
  vector<struct fat_arch> fat_arch_vector;
  for (vector<SuperFatArch>::const_iterator it = object_files_.begin();
       it != object_files_.end();
       ++it) {
    struct fat_arch arch;
    bool success = it->ConvertToFatArch(&arch);
    if (!success) {
      can_convert_to_fat_arch = false;
      break;
    }
    fat_arch_vector.push_back(arch);
  }

  // If all the object files can be converted to struct fat_arch, use
  // NXFindBestFatArch.
  if (can_convert_to_fat_arch) {
    const struct fat_arch* best_match
      = NXFindBestFatArch(cpu_type, cpu_subtype, &fat_arch_vector[0],
                          static_cast<uint32_t>(fat_arch_vector.size()));

    for (size_t i = 0; i < fat_arch_vector.size(); ++i) {
      if (best_match == &fat_arch_vector[i])
        return &object_files_[i];
    }
    assert(best_match == NULL);
    return NULL;
  }

  // Check for an exact match with cpu_type and cpu_subtype.
  for (vector<SuperFatArch>::iterator it = object_files_.begin();
       it != object_files_.end();
       ++it) {
    if (static_cast<cpu_type_t>(it->cputype) == cpu_type &&
        static_cast<cpu_subtype_t>(it->cpusubtype) == cpu_subtype)
      return &*it;
  }

  // No exact match found.
  // TODO(erikchen): If it becomes necessary, we can copy the implementation of
  // NXFindBestFatArch, located at
  // http://web.mit.edu/darwin/src/modules/cctools/libmacho/arch.c.
  fprintf(stderr, "Failed to find an exact match for an object file with cpu "
      "type: %d and cpu subtype: %d. Furthermore, at least one object file is "
      "larger than 2**32.\n", cpu_type, cpu_subtype);
  return NULL;
}

string DumpSymbols::Identifier() {
  FileID file_id(object_filename_.c_str());
  unsigned char identifier_bytes[16];
  cpu_type_t cpu_type = selected_object_file_->cputype;
  cpu_subtype_t cpu_subtype = selected_object_file_->cpusubtype;
  if (!file_id.MachoIdentifier(cpu_type, cpu_subtype, identifier_bytes)) {
    fprintf(stderr, "Unable to calculate UUID of mach-o binary %s!\n",
            object_filename_.c_str());
    return "";
  }

  char identifier_string[40];
  FileID::ConvertIdentifierToString(identifier_bytes, identifier_string,
                                    sizeof(identifier_string));

  string compacted(identifier_string);
  for(size_t i = compacted.find('-'); i != string::npos;
      i = compacted.find('-', i))
    compacted.erase(i, 1);

  return compacted;
}

// A range handler that accepts rangelist data parsed by
// dwarf2reader::RangeListReader and populates a range vector (typically
// owned by a function) with the results.
class DumpSymbols::DumperRangesHandler:
      public DwarfCUToModule::RangesHandler {
 public:
  DumperRangesHandler(dwarf2reader::ByteReader* reader) :
      reader_(reader) { }

  bool ReadRanges(
      enum dwarf2reader::DwarfForm form, uint64_t data,
      dwarf2reader::RangeListReader::CURangesInfo* cu_info,
      vector<Module::Range>* ranges) {
    DwarfRangeListHandler handler(ranges);
    dwarf2reader::RangeListReader range_list_reader(reader_, cu_info,
                                                    &handler);
    return range_list_reader.ReadRanges(form, data);
  }

 private:
  dwarf2reader::ByteReader* reader_;
};

// A line-to-module loader that accepts line number info parsed by
// dwarf2reader::LineInfo and populates a Module and a line vector
// with the results.
class DumpSymbols::DumperLineToModule:
      public DwarfCUToModule::LineToModuleHandler {
 public:
  // Create a line-to-module converter using BYTE_READER.
  DumperLineToModule(dwarf2reader::ByteReader* byte_reader)
      : byte_reader_(byte_reader) { }

  void StartCompilationUnit(const string& compilation_dir) {
    compilation_dir_ = compilation_dir;
  }

  void ReadProgram(const uint8_t* program, uint64_t length,
                   const uint8_t* string_section,
                   uint64_t string_section_length,
                   const uint8_t* line_string_section,
                   uint64_t line_string_section_length,
                   Module* module, vector<Module::Line>* lines) {
    DwarfLineToModule handler(module, compilation_dir_, lines);
    dwarf2reader::LineInfo parser(program, length, byte_reader_,
                                  nullptr, 0, nullptr, 0, &handler);
    parser.Start();
  }
 private:
  string compilation_dir_;
  dwarf2reader::ByteReader* byte_reader_;  // WEAK
};

bool DumpSymbols::CreateEmptyModule(scoped_ptr<Module>& module) {
  // Select an object file, if SetArchitecture hasn't been called to set one
  // explicitly.
  if (!selected_object_file_) {
    // If there's only one architecture, that's the one.
    if (object_files_.size() == 1)
      selected_object_file_ = &object_files_[0];
    else {
      // Look for an object file whose architecture matches our own.
      const NXArchInfo* local_arch = NXGetLocalArchInfo();
      if (!SetArchitecture(local_arch->cputype, local_arch->cpusubtype)) {
        fprintf(stderr, "%s: object file contains more than one"
                " architecture, none of which match the current"
                " architecture; specify an architecture explicitly"
                " with '-a ARCH' to resolve the ambiguity\n",
                object_filename_.c_str());
        return false;
      }
    }
  }

  assert(selected_object_file_);

  // Find the name of the selected file's architecture, to appear in
  // the MODULE record and in error messages.
  const NXArchInfo* selected_arch_info =
      google_breakpad::BreakpadGetArchInfoFromCpuType(
          selected_object_file_->cputype, selected_object_file_->cpusubtype);

  const char* selected_arch_name = selected_arch_info->name;
  if (strcmp(selected_arch_name, "i386") == 0)
    selected_arch_name = "x86";

  // Produce a name to use in error messages that includes the
  // filename, and the architecture, if there is more than one.
  selected_object_name_ = object_filename_;
  if (object_files_.size() > 1) {
    selected_object_name_ += ", architecture ";
    selected_object_name_ + selected_arch_name;
  }

  // Compute a module name, to appear in the MODULE record.
  string module_name = google_breakpad::BaseName(object_filename_);

  // Choose an identifier string, to appear in the MODULE record.
  string identifier = Identifier();
  if (identifier.empty())
    return false;
  identifier += "0";

  // Create a module to hold the debugging information.
  module.reset(new Module(module_name,
                          "mac",
                          selected_arch_name,
                          identifier));
  return true;
}

void DumpSymbols::ReadDwarf(google_breakpad::Module* module,
                            const mach_o::Reader& macho_reader,
                            const mach_o::SectionMap& dwarf_sections,
                            bool handle_inter_cu_refs) const {
  // Build a byte reader of the appropriate endianness.
  ByteReader byte_reader(macho_reader.big_endian()
                         ? dwarf2reader::ENDIANNESS_BIG
                         : dwarf2reader::ENDIANNESS_LITTLE);

  // Construct a context for this file.
  DwarfCUToModule::FileContext file_context(selected_object_name_,
                                            module,
                                            handle_inter_cu_refs);

  // Build a dwarf2reader::SectionMap from our mach_o::SectionMap.
  for (mach_o::SectionMap::const_iterator it = dwarf_sections.begin();
       it != dwarf_sections.end(); ++it) {
    file_context.AddSectionToSectionMap(
        it->first,
        it->second.contents.start,
        it->second.contents.Size());
  }

  // Find the __debug_info section.
  dwarf2reader::SectionMap::const_iterator debug_info_entry =
      file_context.section_map().find("__debug_info");
  // There had better be a __debug_info section!
  if (debug_info_entry == file_context.section_map().end()) {
    fprintf(stderr, "%s: __DWARF segment of file has no __debug_info section\n",
            selected_object_name_.c_str());
    return;
  }
  const std::pair<const uint8_t*, uint64_t>& debug_info_section =
      debug_info_entry->second;

  // Build a line-to-module loader for the root handler to use.
  DumperLineToModule line_to_module(&byte_reader);

  // .debug_ranges and .debug_rngslists reader
  DumperRangesHandler ranges_handler(&byte_reader);

  // Walk the __debug_info section, one compilation unit at a time.
  uint64_t debug_info_length = debug_info_section.second;
  for (uint64_t offset = 0; offset < debug_info_length;) {
    // Make a handler for the root DIE that populates MODULE with the
    // debug info.
    DwarfCUToModule::WarningReporter reporter(selected_object_name_,
                                              offset);
    DwarfCUToModule root_handler(&file_context, &line_to_module,
                                 &ranges_handler, &reporter);
    // Make a Dwarf2Handler that drives our DIEHandler.
    dwarf2reader::DIEDispatcher die_dispatcher(&root_handler);
    // Make a DWARF parser for the compilation unit at OFFSET.
    dwarf2reader::CompilationUnit dwarf_reader(selected_object_name_,
                                               file_context.section_map(),
                                               offset,
                                               &byte_reader,
                                               &die_dispatcher);
    // Process the entire compilation unit; get the offset of the next.
    offset += dwarf_reader.Start();
  }
}

bool DumpSymbols::ReadCFI(google_breakpad::Module* module,
                          const mach_o::Reader& macho_reader,
                          const mach_o::Section& section,
                          bool eh_frame) const {
  // Find the appropriate set of register names for this file's
  // architecture.
  vector<string> register_names;
  switch (macho_reader.cpu_type()) {
    case CPU_TYPE_X86:
      register_names = DwarfCFIToModule::RegisterNames::I386();
      break;
    case CPU_TYPE_X86_64:
      register_names = DwarfCFIToModule::RegisterNames::X86_64();
      break;
    case CPU_TYPE_ARM:
      register_names = DwarfCFIToModule::RegisterNames::ARM();
      break;
    case CPU_TYPE_ARM64:
      register_names = DwarfCFIToModule::RegisterNames::ARM64();
      break;
    default: {
      const NXArchInfo* arch = google_breakpad::BreakpadGetArchInfoFromCpuType(
          macho_reader.cpu_type(), macho_reader.cpu_subtype());
      fprintf(stderr, "%s: cannot convert DWARF call frame information for ",
              selected_object_name_.c_str());
      if (arch)
        fprintf(stderr, "architecture '%s'", arch->name);
      else
        fprintf(stderr, "architecture %d,%d",
                macho_reader.cpu_type(), macho_reader.cpu_subtype());
      fprintf(stderr, " to Breakpad symbol file: no register name table\n");
      return false;
    }
  }

  // Find the call frame information and its size.
  const uint8_t* cfi = section.contents.start;
  size_t cfi_size = section.contents.Size();

  // Plug together the parser, handler, and their entourages.
  DwarfCFIToModule::Reporter module_reporter(selected_object_name_,
                                             section.section_name);
  DwarfCFIToModule handler(module, register_names, &module_reporter);
  dwarf2reader::ByteReader byte_reader(macho_reader.big_endian() ?
                                       dwarf2reader::ENDIANNESS_BIG :
                                       dwarf2reader::ENDIANNESS_LITTLE);
  byte_reader.SetAddressSize(macho_reader.bits_64() ? 8 : 4);
  // At the moment, according to folks at Apple and some cursory
  // investigation, Mac OS X only uses DW_EH_PE_pcrel-based pointers, so
  // this is the only base address the CFI parser will need.
  byte_reader.SetCFIDataBase(section.address, cfi);

  dwarf2reader::CallFrameInfo::Reporter dwarf_reporter(selected_object_name_,
                                                       section.section_name);
  dwarf2reader::CallFrameInfo parser(cfi, cfi_size,
                                     &byte_reader, &handler, &dwarf_reporter,
                                     eh_frame);
  parser.Start();
  return true;
}

// A LoadCommandHandler that loads whatever debugging data it finds into a
// Module.
class DumpSymbols::LoadCommandDumper:
      public mach_o::Reader::LoadCommandHandler {
 public:
  // Create a load command dumper handling load commands from READER's
  // file, and adding data to MODULE.
  LoadCommandDumper(const DumpSymbols& dumper,
                    google_breakpad::Module* module,
                    const mach_o::Reader& reader,
                    SymbolData symbol_data,
                    bool handle_inter_cu_refs)
      : dumper_(dumper),
        module_(module),
        reader_(reader),
        symbol_data_(symbol_data),
        handle_inter_cu_refs_(handle_inter_cu_refs) { }

  bool SegmentCommand(const mach_o::Segment& segment);
  bool SymtabCommand(const ByteBuffer& entries, const ByteBuffer& strings);

 private:
  const DumpSymbols& dumper_;
  google_breakpad::Module* module_;  // WEAK
  const mach_o::Reader& reader_;
  const SymbolData symbol_data_;
  const bool handle_inter_cu_refs_;
};

bool DumpSymbols::LoadCommandDumper::SegmentCommand(const Segment& segment) {
  mach_o::SectionMap section_map;
  if (!reader_.MapSegmentSections(segment, &section_map))
    return false;

  if (segment.name == "__TEXT") {
    module_->SetLoadAddress(segment.vmaddr);
    if (symbol_data_ != NO_CFI) {
      mach_o::SectionMap::const_iterator eh_frame =
          section_map.find("__eh_frame");
      if (eh_frame != section_map.end()) {
        // If there is a problem reading this, don't treat it as a fatal error.
        dumper_.ReadCFI(module_, reader_, eh_frame->second, true);
      }
    }
    return true;
  }

  if (segment.name == "__DWARF") {
    if (symbol_data_ != ONLY_CFI) {
      dumper_.ReadDwarf(module_, reader_, section_map, handle_inter_cu_refs_);
    }
    if (symbol_data_ != NO_CFI) {
      mach_o::SectionMap::const_iterator debug_frame
          = section_map.find("__debug_frame");
      if (debug_frame != section_map.end()) {
        // If there is a problem reading this, don't treat it as a fatal error.
        dumper_.ReadCFI(module_, reader_, debug_frame->second, false);
      }
    }
  }

  return true;
}

bool DumpSymbols::LoadCommandDumper::SymtabCommand(const ByteBuffer& entries,
                                                   const ByteBuffer& strings) {
  StabsToModule stabs_to_module(module_);
  // Mac OS X STABS are never "unitized", and the size of the 'value' field
  // matches the address size of the executable.
  StabsReader stabs_reader(entries.start, entries.Size(),
                           strings.start, strings.Size(),
                           reader_.big_endian(),
                           reader_.bits_64() ? 8 : 4,
                           true,
                           &stabs_to_module);
  if (!stabs_reader.Process())
    return false;
  stabs_to_module.Finalize();
  return true;
}

bool DumpSymbols::ReadSymbolData(Module** out_module) {
  scoped_ptr<Module> module;
  if (!CreateEmptyModule(module))
    return false;

  // Parse the selected object file.
  mach_o::Reader::Reporter reporter(selected_object_name_);
  mach_o::Reader reader(&reporter);
  if (!reader.Read(&contents_[0]
                   + selected_object_file_->offset,
                   selected_object_file_->size,
                   selected_object_file_->cputype,
                   selected_object_file_->cpusubtype))
    return false;

  // Walk its load commands, and deal with whatever is there.
  LoadCommandDumper load_command_dumper(*this, module.get(), reader,
                                        symbol_data_, handle_inter_cu_refs_);
  if (!reader.WalkLoadCommands(&load_command_dumper))
    return false;

  *out_module = module.release();

  return true;
}

bool DumpSymbols::WriteSymbolFile(std::ostream& stream) {
  Module* module = NULL;

  if (ReadSymbolData(&module) && module) {
    bool res = module->Write(stream, symbol_data_);
    delete module;
    return res;
  }

  return false;
}

// Read the selected object file's debugging information, and write out the
// header only to |stream|. Return true on success; if an error occurs, report
// it and return false.
bool DumpSymbols::WriteSymbolFileHeader(std::ostream& stream) {
  scoped_ptr<Module> module;
  if (!CreateEmptyModule(module))
    return false;

  return module->Write(stream, symbol_data_);
}

}  // namespace google_breakpad
