// Copyright (c) 2006, Google Inc.
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

// PDBSourceLineWriter uses a pdb file produced by Visual C++ to output
// a line/address map for use with BasicSourceLineResolver.

#ifndef _PDB_SOURCE_LINE_WRITER_H__
#define _PDB_SOURCE_LINE_WRITER_H__

#include <atlcomcli.h>

#include <hash_map>
#include <string>

#include "common/windows/omap.h"

struct IDiaEnumLineNumbers;
struct IDiaSession;
struct IDiaSymbol;

namespace google_breakpad {

using std::wstring;
using stdext::hash_map;

// A structure that carries information that identifies a pdb file.
struct PDBModuleInfo {
 public:
  // The basename of the pdb file from which information was loaded.
  wstring debug_file;

  // The pdb's identifier.  For recent pdb files, the identifier consists
  // of the pdb's guid, in uppercase hexadecimal form without any dashes
  // or separators, followed immediately by the pdb's age, also in
  // uppercase hexadecimal form.  For older pdb files which have no guid,
  // the identifier is the pdb's 32-bit signature value, in zero-padded
  // hexadecimal form, followed immediately by the pdb's age, in lowercase
  // hexadecimal form.
  wstring debug_identifier;

  // A string identifying the cpu that the pdb is associated with.
  // Currently, this may be "x86" or "unknown".
  wstring cpu;
};

// A structure that carries information that identifies a PE file,
// either an EXE or a DLL.
struct PEModuleInfo {
  // The basename of the PE file.
  wstring code_file;

  // The PE file's code identifier, which consists of its timestamp
  // and file size concatenated together into a single hex string.
  // (The fields IMAGE_OPTIONAL_HEADER::SizeOfImage and
  // IMAGE_FILE_HEADER::TimeDateStamp, as defined in the ImageHlp
  // documentation.) This is not well documented, if it's documented
  // at all, but it's what symstore does and what DbgHelp supports.
  wstring code_identifier;
};

class PDBSourceLineWriter {
 public:
  enum FileFormat {
    PDB_FILE,  // a .pdb file containing debug symbols
    EXE_FILE,  // a .exe or .dll file
    ANY_FILE   // try PDB_FILE and then EXE_FILE
  };

  explicit PDBSourceLineWriter();
  ~PDBSourceLineWriter();

  // Opens the given file.  For executable files, the corresponding pdb
  // file must be available; Open will be if it is not.
  // If there is already a pdb file open, it is automatically closed.
  // Returns true on success.
  bool Open(const wstring &file, FileFormat format);

  // Locates the pdb file for the given executable (exe or dll) file,
  // and opens it.  If there is already a pdb file open, it is automatically
  // closed.  Returns true on success.
  bool OpenExecutable(const wstring &exe_file);

  // Writes a map file from the current pdb file to the given file stream.
  // Returns true on success.
  bool WriteMap(FILE *map_file);

  // Closes the current pdb file and its associated resources.
  void Close();

  // Retrieves information about the module's debugging file.  Returns
  // true on success and false on failure.
  bool GetModuleInfo(PDBModuleInfo *info);

  // Retrieves information about the module's PE file.  Returns
  // true on success and false on failure.
  bool GetPEInfo(PEModuleInfo *info);

  // Sets uses_guid to true if the opened file uses a new-style CodeView
  // record with a 128-bit GUID, or false if the opened file uses an old-style
  // CodeView record.  When no GUID is available, a 32-bit signature should be
  // used to identify the module instead.  If the information cannot be
  // determined, this method returns false.
  bool UsesGUID(bool *uses_guid);

 private:
  // Outputs the line/address pairs for each line in the enumerator.
  // Returns true on success.
  bool PrintLines(IDiaEnumLineNumbers *lines);

  // Outputs a function address and name, followed by its source line list.
  // block can be the same object as function, or it can be a reference
  // to a code block that is lexically part of this function, but
  // resides at a separate address.
  // Returns true on success.
  bool PrintFunction(IDiaSymbol *function, IDiaSymbol *block);

  // Outputs all functions as described above.  Returns true on success.
  bool PrintFunctions();

  // Outputs all of the source files in the session's pdb file.
  // Returns true on success.
  bool PrintSourceFiles();

  // Outputs all of the frame information necessary to construct stack
  // backtraces in the absence of frame pointers.  Returns true on success.
  bool PrintFrameData();

  // Outputs a single public symbol address and name, if the symbol corresponds
  // to a code address.  Returns true on success.  If symbol is does not
  // correspond to code, returns true without outputting anything.
  bool PrintCodePublicSymbol(IDiaSymbol *symbol);

  // Outputs a line identifying the PDB file that is being dumped, along with
  // its uuid and age.
  bool PrintPDBInfo();

  // Outputs a line identifying the PE file corresponding to the PDB
  // file that is being dumped, along with its code identifier,
  // which consists of its timestamp and file size.
  bool PrintPEInfo();

  // Returns true if this filename has already been seen,
  // and an ID is stored for it, or false if it has not.
  bool FileIDIsCached(const wstring &file) {
    return unique_files_.find(file) != unique_files_.end();
  };

  // Cache this filename and ID for later reuse.
  void CacheFileID(const wstring &file, DWORD id) {
    unique_files_[file] = id;
  };

  // Store this ID in the cache as a duplicate for this filename.
  void StoreDuplicateFileID(const wstring &file, DWORD id) {
    hash_map<wstring, DWORD>::iterator iter = unique_files_.find(file);
    if (iter != unique_files_.end()) {
      // map this id to the previously seen one
      file_ids_[id] = iter->second;
    }
  };

  // Given a file's unique ID, return the ID that should be used to
  // reference it. There may be multiple files with identical filenames
  // but different unique IDs. The cache attempts to coalesce these into
  // one ID per unique filename.
  DWORD GetRealFileID(DWORD id) {
    hash_map<DWORD, DWORD>::iterator iter = file_ids_.find(id);
    if (iter == file_ids_.end())
      return id;
    return iter->second;
  };

  // Find the PE file corresponding to the loaded PDB file, and
  // set the code_file_ member. Returns false on failure.
  bool FindPEFile();

  // Returns the function name for a symbol.  If possible, the name is
  // undecorated.  If the symbol's decorated form indicates the size of
  // parameters on the stack, this information is returned in stack_param_size.
  // Returns true on success.  If the symbol doesn't encode parameter size
  // information, stack_param_size is set to -1.
  static bool GetSymbolFunctionName(IDiaSymbol *function, BSTR *name,
                                    int *stack_param_size);

  // Returns the number of bytes of stack space used for a function's
  // parameters.  function must have the tag SymTagFunction.  In the event of
  // a failure, returns 0, which is also a valid number of bytes.
  static int GetFunctionStackParamSize(IDiaSymbol *function);

  // The filename of the PE file corresponding to the currently-open
  // pdb file.
  wstring code_file_;

  // The session for the currently-open pdb file.
  CComPtr<IDiaSession> session_;

  // The current output file for this WriteMap invocation.
  FILE *output_;

  // There may be many duplicate filenames with different IDs.
  // This maps from the DIA "unique ID" to a single ID per unique
  // filename.
  hash_map<DWORD, DWORD> file_ids_;
  // This maps unique filenames to file IDs.
  hash_map<wstring, DWORD> unique_files_;

  // This is used for calculating post-transform symbol addresses and lengths.
  ImageMap image_map_;

  // Disallow copy ctor and operator=
  PDBSourceLineWriter(const PDBSourceLineWriter&);
  void operator=(const PDBSourceLineWriter&);
};

}  // namespace google_breakpad

#endif  // _PDB_SOURCE_LINE_WRITER_H__
