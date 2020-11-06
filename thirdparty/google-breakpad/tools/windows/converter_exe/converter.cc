// Copyright 2019 Google Inc. All rights reserved.
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

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "diaguids.lib")
#pragma comment(lib, "imagehlp.lib")

#include <cassert>
#include <cstdio>
#include <ctime>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "tools/windows/converter_exe/escaping.h"
#include "tools/windows/converter_exe/http_download.h"
#include "tools/windows/converter_exe/tokenizer.h"
#include "common/windows/http_upload.h"
#include "common/windows/string_utils-inl.h"
#include "tools/windows/converter/ms_symbol_server_converter.h"

using strings::WebSafeBase64Unescape;
using strings::WebSafeBase64Escape;

namespace {

using std::map;
using std::string;
using std::vector;
using std::wstring;
using crash::HTTPDownload;
using crash::Tokenizer;
using google_breakpad::HTTPUpload;
using google_breakpad::MissingSymbolInfo;
using google_breakpad::MSSymbolServerConverter;
using google_breakpad::WindowsStringUtils;

const char* kMissingStringDelimiters = "|";
const char* kLocalCachePath = "c:\\symbols";
const char* kNoExeMSSSServer = "http://msdl.microsoft.com/download/symbols/";

// Windows stdio doesn't do line buffering.  Use this function to flush after
// writing to stdout and stderr so that a log will be available if the
// converter crashes.
static int FprintfFlush(FILE* file, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int retval = vfprintf(file, format, arguments);
  va_end(arguments);
  fflush(file);
  return retval;
}

static string CurrentDateAndTime() {
  const string kUnknownDateAndTime = R"(????-??-?? ??:??:??)";

  time_t current_time;
  time(&current_time);

  // localtime_s is safer but is only available in MSVC8.  Use localtime
  // in earlier environments.
  struct tm* time_pointer;
#if _MSC_VER >= 1400  // MSVC 2005/8
  struct tm time_struct;
  time_pointer =& time_struct;
  if (localtime_s(time_pointer,& current_time) != 0) {
    return kUnknownDateAndTime;
  }
#else  // _MSC_VER >= 1400
  time_pointer = localtime(&current_time);
  if (!time_pointer) {
    return kUnknownDateAndTime;
  }
#endif  // _MSC_VER >= 1400

  char buffer[256];
  if (!strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_pointer)) {
    return kUnknownDateAndTime;
  }

  return string(buffer);
}

// ParseMissingString turns |missing_string| into a MissingSymbolInfo
// structure.  It returns true on success, and false if no such conversion
// is possible.
static bool ParseMissingString(const string& missing_string,
                               MissingSymbolInfo* missing_info) {
  assert(missing_info);

  vector<string> tokens;
  Tokenizer::Tokenize(kMissingStringDelimiters, missing_string,& tokens);
  if (tokens.size() != 5) {
    return false;
  }

  missing_info->debug_file = tokens[0];
  missing_info->debug_identifier = tokens[1];
  missing_info->version = tokens[2];
  missing_info->code_file = tokens[3];
  missing_info->code_identifier = tokens[4];

  return true;
}

// StringMapToWStringMap takes each element in a map that associates
// (narrow) strings to strings and converts the keys and values to wstrings.
// Returns true on success and false on failure, printing an error message.
static bool StringMapToWStringMap(const map<string, string>& smap,
                                  map<wstring, wstring>* wsmap) {
  assert(wsmap);
  wsmap->clear();

  for (map<string, string>::const_iterator iterator = smap.begin();
       iterator != smap.end();
       ++iterator) {
    wstring key;
    if (!WindowsStringUtils::safe_mbstowcs(iterator->first,& key)) {
      FprintfFlush(stderr,
                   "StringMapToWStringMap: safe_mbstowcs failed for key %s\n",
                   iterator->first.c_str());
      return false;
    }

    wstring value;
    if (!WindowsStringUtils::safe_mbstowcs(iterator->second,& value)) {
      FprintfFlush(stderr, "StringMapToWStringMap: safe_mbstowcs failed "
                           "for value %s\n",
                   iterator->second.c_str());
      return false;
    }

    wsmap->insert(make_pair(key, value));
  }

  return true;
}

// MissingSymbolInfoToParameters turns a MissingSymbolInfo structure into a
// map of parameters suitable for passing to HTTPDownload or HTTPUpload.
// Returns true on success and false on failure, printing an error message.
static bool MissingSymbolInfoToParameters(const MissingSymbolInfo& missing_info,
                                          map<wstring, wstring>* wparameters) {
  assert(wparameters);

  map<string, string> parameters;
  string encoded_param;
  // Indicate the params are encoded.
  parameters["encoded"] = "true";  // The string value here does not matter.

  WebSafeBase64Escape(missing_info.code_file,& encoded_param);
  parameters["code_file"] = encoded_param;

  WebSafeBase64Escape(missing_info.code_identifier,& encoded_param);
  parameters["code_identifier"] = encoded_param;

  WebSafeBase64Escape(missing_info.debug_file,& encoded_param);
  parameters["debug_file"] = encoded_param;

  WebSafeBase64Escape(missing_info.debug_identifier,& encoded_param);
  parameters["debug_identifier"] = encoded_param;

  if (!missing_info.version.empty()) {
    // The version is optional.
    WebSafeBase64Escape(missing_info.version,& encoded_param);
    parameters["version"] = encoded_param;
  }

  WebSafeBase64Escape("WinSymConv",& encoded_param);
  parameters["product"] = encoded_param;

  if (!StringMapToWStringMap(parameters, wparameters)) {
    // StringMapToWStringMap will have printed an error.
    return false;
  }

  return true;
}

// UploadSymbolFile sends |converted_file| as identified by |missing_info|
// to the symbol server rooted at |upload_symbol_url|.  Returns true on
// success and false on failure, printing an error message.
static bool UploadSymbolFile(const wstring& upload_symbol_url,
                             const MissingSymbolInfo& missing_info,
                             const string& converted_file) {
  map<wstring, wstring> parameters;
  if (!MissingSymbolInfoToParameters(missing_info,& parameters)) {
    // MissingSymbolInfoToParameters or a callee will have printed an error.
    return false;
  }

  wstring converted_file_w;

  if (!WindowsStringUtils::safe_mbstowcs(converted_file,& converted_file_w)) {
    FprintfFlush(stderr, "UploadSymbolFile: safe_mbstowcs failed for %s\n",
                 converted_file.c_str());
    return false;
  }
  map<wstring, wstring> files;
  files[L"symbol_file"] = converted_file_w;

  FprintfFlush(stderr, "Uploading %s\n", converted_file.c_str());
  if (!HTTPUpload::SendMultipartPostRequest(
      upload_symbol_url, parameters,
      files, NULL, NULL, NULL)) {
    FprintfFlush(stderr, "UploadSymbolFile: HTTPUpload::SendRequest failed "
                         "for %s %s %s\n",
                 missing_info.debug_file.c_str(),
                 missing_info.debug_identifier.c_str(),
                 missing_info.version.c_str());
    return false;
  }

  return true;
}

// SendFetchFailedPing informs the symbol server based at
// |fetch_symbol_failure_url| that the symbol file identified by
// |missing_info| could authoritatively not be located.  Returns
// true on success and false on failure.
static bool SendFetchFailedPing(const wstring& fetch_symbol_failure_url,
                                const MissingSymbolInfo& missing_info) {
  map<wstring, wstring> parameters;
  if (!MissingSymbolInfoToParameters(missing_info,& parameters)) {
    // MissingSymbolInfoToParameters or a callee will have printed an error.
    return false;
  }

  string content;
  if (!HTTPDownload::Download(fetch_symbol_failure_url,
                             & parameters,
                             & content,
                              NULL)) {
    FprintfFlush(stderr, "SendFetchFailedPing: HTTPDownload::Download failed "
                         "for %s %s %s\n",
                 missing_info.debug_file.c_str(),
                 missing_info.debug_identifier.c_str(),
                 missing_info.version.c_str());
    return false;
  }

  return true;
}

// Returns true if it's safe to make an external request for the symbol
// file described in missing_info. It's considered safe to make an
// external request unless the symbol file's debug_file string matches
// the given blacklist regular expression.
// The debug_file name is used from the MissingSymbolInfo struct,
// matched against the blacklist_regex.
static bool SafeToMakeExternalRequest(const MissingSymbolInfo& missing_info,
                                      std::regex blacklist_regex) {
  string file_name = missing_info.debug_file;
  // Use regex_search because we want to match substrings.
  if (std::regex_search(file_name, blacklist_regex)) {
    FprintfFlush(stderr, "Not safe to make external request for file %s\n",
                 file_name.c_str());
    return false;
  }

  return true;
}

// Converter options derived from command line parameters.
struct ConverterOptions {
  ConverterOptions()
      : report_fetch_failures(true) {
  }

  ~ConverterOptions() {
  }

  // Names of MS Symbol Supplier Servers that are internal to Google, and may
  // have symbols for any request.
  vector<string> full_internal_msss_servers;

  // Names of MS Symbol Supplier Servers that are internal to Google, and
  // shouldn't be checked for symbols for any .exe files.
  vector<string> full_external_msss_servers;

  // Names of MS Symbol Supplier Servers that are external to Google, and may
  // have symbols for any request.
  vector<string> no_exe_internal_msss_servers;

  // Names of MS Symbol Supplier Servers that are external to Google, and
  // shouldn't be checked for symbols for any .exe files.
  vector<string> no_exe_external_msss_servers;

  // Temporary local storage for symbols.
  string local_cache_path;

  // URL for uploading symbols.
  wstring upload_symbols_url;

  // URL to fetch list of missing symbols.
  wstring missing_symbols_url;

  // URL to report symbol fetch failure.
  wstring fetch_symbol_failure_url;

  // Are symbol fetch failures reported.
  bool report_fetch_failures;

  // File containing the list of missing symbols.  Fetch failures are not
  // reported if such file is provided.
  string missing_symbols_file;

  // Regex used to blacklist files to prevent external symbol requests.
  // Owned and cleaned up by this struct.
  std::regex blacklist_regex;

 private:
  // DISABLE_COPY_AND_ASSIGN
  ConverterOptions(const ConverterOptions&);
  ConverterOptions& operator=(const ConverterOptions&);
};

// ConverMissingSymbolFile takes a single MissingSymbolInfo structure and
// attempts to locate it from the symbol servers provided in the
// |options.*_msss_servers| arguments.  "Full" servers are those that will be
// queried for all symbol files; "No-EXE" servers will only be queried for
// modules whose missing symbol data indicates are not main program executables.
// Results will be sent to the |options.upload_symbols_url| on success or
// |options.fetch_symbol_failure_url| on failure, and the local cache will be
// stored at |options.local_cache_path|.  Because nothing can be done even in
// the event of a failure, this function returns no value, although it
// may result in error messages being printed.
static void ConvertMissingSymbolFile(const MissingSymbolInfo& missing_info,
                                     const ConverterOptions& options) {
  string time_string = CurrentDateAndTime();
  FprintfFlush(stdout, "converter: %s: attempting %s %s %s\n",
               time_string.c_str(),
               missing_info.debug_file.c_str(),
               missing_info.debug_identifier.c_str(),
               missing_info.version.c_str());

  // The first lookup is always to internal symbol servers.
  // Always ask the symbol servers identified as "full."
  vector<string> msss_servers = options.full_internal_msss_servers;

  // If the file is not an .exe file, also ask an additional set of symbol
  // servers, such as Microsoft's public symbol server.
  bool is_exe = false;

  if (missing_info.code_file.length() >= 4) {
    string code_extension =
        missing_info.code_file.substr(missing_info.code_file.size() - 4);

    // Firefox is a special case: .dll-only servers should be consulted for
    // its symbols.  This enables us to get its symbols from Mozilla's
    // symbol server when crashes occur in Google extension code hosted by a
    // Firefox process.
    if (_stricmp(code_extension.c_str(), ".exe") == 0 &&
        _stricmp(missing_info.code_file.c_str(), "firefox.exe") != 0) {
      is_exe = true;
    }
  }

  if (!is_exe) {
    msss_servers.insert(msss_servers.end(),
                        options.no_exe_internal_msss_servers.begin(),
                        options.no_exe_internal_msss_servers.end());
  }

  // If there are any suitable internal symbol servers, make a request.
  MSSymbolServerConverter::LocateResult located =
      MSSymbolServerConverter::LOCATE_FAILURE;
  string converted_file;
  if (msss_servers.size() > 0) {
    // Attempt to fetch the symbol file and convert it.
    FprintfFlush(stderr, "Making internal request for %s (%s)\n",
                   missing_info.debug_file.c_str(),
                   missing_info.debug_identifier.c_str());
    MSSymbolServerConverter converter(options.local_cache_path, msss_servers);
    located = converter.LocateAndConvertSymbolFile(missing_info,
                                                   false,  // keep_symbol_file
                                                   false,  // keep_pe_file
                                                  & converted_file,
                                                   NULL,   // symbol_file
                                                   NULL);  // pe_file
    switch (located) {
      case MSSymbolServerConverter::LOCATE_SUCCESS:
        FprintfFlush(stderr, "LocateResult = LOCATE_SUCCESS\n");
        // Upload it.  Don't bother checking the return value.  If this
        // succeeds, it should disappear from the missing symbol list.
        // If it fails, something will print an error message indicating
        // the cause of the failure, and the item will remain on the
        // missing symbol list.
        UploadSymbolFile(options.upload_symbols_url, missing_info,
                         converted_file);
        remove(converted_file.c_str());

        // Note: this does leave some directories behind that could be
        // cleaned up.  The directories inside options.local_cache_path for
        // debug_file/debug_identifier can be removed at this point.
        break;

      case MSSymbolServerConverter::LOCATE_NOT_FOUND:
        FprintfFlush(stderr, "LocateResult = LOCATE_NOT_FOUND\n");
        // The symbol file definitively did not exist. Fall through,
        // so we can attempt an external query if it's safe to do so.
        break;

      case MSSymbolServerConverter::LOCATE_RETRY:
        FprintfFlush(stderr, "LocateResult = LOCATE_RETRY\n");
        // Fall through in case we should make an external request.
        // If not, or if an external request fails in the same way,
        // we'll leave the entry in the symbol file list and
        // try again on a future pass.  Print a message so that there's
        // a record.
        break;

      case MSSymbolServerConverter::LOCATE_FAILURE:
        FprintfFlush(stderr, "LocateResult = LOCATE_FAILURE\n");
        // LocateAndConvertSymbolFile printed an error message.
        break;

      default:
        FprintfFlush(
            stderr,
            "FATAL: Unexpected return value '%d' from "
            "LocateAndConvertSymbolFile()\n",
            located);
        assert(0);
        break;
    }
  } else {
    // No suitable internal symbol servers.  This is fine because the converter
    // is mainly used for downloading and converting of external symbols.
  }

  // Make a request to an external server if the internal request didn't
  // succeed, and it's safe to do so.
  if (located != MSSymbolServerConverter::LOCATE_SUCCESS &&
      SafeToMakeExternalRequest(missing_info, options.blacklist_regex)) {
    msss_servers = options.full_external_msss_servers;
    if (!is_exe) {
      msss_servers.insert(msss_servers.end(),
                          options.no_exe_external_msss_servers.begin(),
                          options.no_exe_external_msss_servers.end());
    }
    if (msss_servers.size() > 0) {
      FprintfFlush(stderr, "Making external request for %s (%s)\n",
                   missing_info.debug_file.c_str(),
                   missing_info.debug_identifier.c_str());
      MSSymbolServerConverter external_converter(options.local_cache_path,
                                                 msss_servers);
      located = external_converter.LocateAndConvertSymbolFile(
          missing_info,
          false,  // keep_symbol_file
          false,  // keep_pe_file
         & converted_file,
          NULL,   // symbol_file
          NULL);  // pe_file
    } else {
      FprintfFlush(stderr, "ERROR: No suitable external symbol servers.\n");
    }
  }

  // Final handling for this symbol file is based on the result from the
  // external request (if performed above), or on the result from the
  // previous internal lookup.
  switch (located) {
    case MSSymbolServerConverter::LOCATE_SUCCESS:
      FprintfFlush(stderr, "LocateResult = LOCATE_SUCCESS\n");
      // Upload it.  Don't bother checking the return value.  If this
      // succeeds, it should disappear from the missing symbol list.
      // If it fails, something will print an error message indicating
      // the cause of the failure, and the item will remain on the
      // missing symbol list.
      UploadSymbolFile(options.upload_symbols_url, missing_info,
                       converted_file);
      remove(converted_file.c_str());

      // Note: this does leave some directories behind that could be
      // cleaned up.  The directories inside options.local_cache_path for
      // debug_file/debug_identifier can be removed at this point.
      break;

    case MSSymbolServerConverter::LOCATE_NOT_FOUND:
      // The symbol file definitively didn't exist.  Inform the server.
      // If this fails, something will print an error message indicating
      // the cause of the failure, but there's really nothing more to
      // do.  If this succeeds, the entry should be removed from the
      // missing symbols list.
      if (!options.report_fetch_failures) {
        FprintfFlush(stderr, "SendFetchFailedPing skipped\n");
      } else if (SendFetchFailedPing(options.fetch_symbol_failure_url,
                                     missing_info)) {
        FprintfFlush(stderr, "SendFetchFailedPing succeeded\n");
      } else {
        FprintfFlush(stderr, "SendFetchFailedPing failed\n");
      }
      break;

    case MSSymbolServerConverter::LOCATE_RETRY:
      FprintfFlush(stderr, "LocateResult = LOCATE_RETRY\n");
      // Nothing to do but leave the entry in the symbol file list and
      // try again on a future pass.  Print a message so that there's
      // a record.
      FprintfFlush(stderr, "ConvertMissingSymbolFile: deferring retry "
                           "for %s %s %s\n",
                   missing_info.debug_file.c_str(),
                   missing_info.debug_identifier.c_str(),
                   missing_info.version.c_str());
      break;

    case MSSymbolServerConverter::LOCATE_FAILURE:
      FprintfFlush(stderr, "LocateResult = LOCATE_FAILURE\n");
      // LocateAndConvertSymbolFile printed an error message.

      // This is due to a bad debug file name, so fetch failed.
      if (!options.report_fetch_failures) {
        FprintfFlush(stderr, "SendFetchFailedPing skipped\n");
      } else if (SendFetchFailedPing(options.fetch_symbol_failure_url,
                                     missing_info)) {
        FprintfFlush(stderr, "SendFetchFailedPing succeeded\n");
      } else {
        FprintfFlush(stderr, "SendFetchFailedPing failed\n");
      }
      break;

    default:
      FprintfFlush(
          stderr,
          "FATAL: Unexpected return value '%d' from "
          "LocateAndConvertSymbolFile()\n",
          located);
      assert(0);
      break;
  }
}


// Reads the contents of file |file_name| and populates |contents|.
// Returns true on success.
static bool ReadFile(string file_name, string* contents) {
  char buffer[1024 * 8];
  FILE* fp = fopen(file_name.c_str(), "rt");
  if (!fp) {
    return false;
  }
  contents->clear();
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    contents->append(buffer);
  }
  fclose(fp);
  return true;
}

// ConvertMissingSymbolsList obtains a missing symbol list from
// |options.missing_symbols_url| or |options.missing_symbols_file| and calls
// ConvertMissingSymbolFile for each missing symbol file in the list.
static bool ConvertMissingSymbolsList(const ConverterOptions& options) {
  // Set param to indicate requesting for encoded response.
  map<wstring, wstring> parameters;
  parameters[L"product"] = L"WinSymConv";
  parameters[L"encoded"] = L"true";
  // Get the missing symbol list.
  string missing_symbol_list;
  if (!options.missing_symbols_file.empty()) {
    if (!ReadFile(options.missing_symbols_file,& missing_symbol_list)) {
      return false;
    }
  } else if (!HTTPDownload::Download(options.missing_symbols_url,& parameters,
                                    & missing_symbol_list, NULL)) {
    return false;
  }

  // Tokenize the content into a vector.
  vector<string> missing_symbol_lines;
  Tokenizer::Tokenize("\n", missing_symbol_list,& missing_symbol_lines);

  FprintfFlush(stderr, "Found %d missing symbol files in list.\n",
               missing_symbol_lines.size() - 1);  // last line is empty.
  int convert_attempts = 0;
  for (vector<string>::const_iterator iterator = missing_symbol_lines.begin();
       iterator != missing_symbol_lines.end();
       ++iterator) {
    // Decode symbol line.
    const string& encoded_line = *iterator;
    // Skip lines that are blank.
    if (encoded_line.empty()) {
      continue;
    }

    string line;
    if (!WebSafeBase64Unescape(encoded_line,& line)) {
      // If decoding fails, assume the line is not encoded.
      // This is helpful when the program connects to a debug server without
      // encoding.
      line = encoded_line;
    }

    FprintfFlush(stderr, "\nLine: %s\n", line.c_str());

    // Turn each element into a MissingSymbolInfo structure.
    MissingSymbolInfo missing_info;
    if (!ParseMissingString(line,& missing_info)) {
      FprintfFlush(stderr, "ConvertMissingSymbols: ParseMissingString failed "
                           "for %s from %ws\n",
                   line.c_str(), options.missing_symbols_url.c_str());
      continue;
    }

    ++convert_attempts;
    ConvertMissingSymbolFile(missing_info, options);
  }

  // Say something reassuring, since ConvertMissingSymbolFile was never called
  // and therefore never reported any progress.
  if (convert_attempts == 0) {
    string current_time = CurrentDateAndTime();
    FprintfFlush(stdout, "converter: %s: nothing to convert\n",
                 current_time.c_str());
  }

  return true;
}

// usage prints the usage message.  It returns 1 as a convenience, to be used
// as a return value from main.
static int usage(const char* program_name) {
  FprintfFlush(stderr,
      "usage: %s [options]\n"
      "    -f  <full_msss_server>     MS servers to ask for all symbols\n"
      "    -n  <no_exe_msss_server>   same, but prevent asking for EXEs\n"
      "    -l  <local_cache_path>     Temporary local storage for symbols\n"
      "    -s  <upload_url>           URL for uploading symbols\n"
      "    -m  <missing_symbols_url>  URL to fetch list of missing symbols\n"
      "    -mf <missing_symbols_file> File containing the list of missing\n"
      "                               symbols.  Fetch failures are not\n"
      "                               reported if such file is provided.\n"
      "    -t  <fetch_failure_url>    URL to report symbol fetch failure\n"
      "    -b  <regex>                Regex used to blacklist files to\n"
      "                               prevent external symbol requests\n"
      " Note that any server specified by -f or -n that starts with \\filer\n"
      " will be treated as internal, and all others as external.\n",
      program_name);

  return 1;
}

// "Internal" servers consist only of those whose names start with
// the literal string "\\filer\".
static bool IsInternalServer(const string& server_name) {
  if (server_name.find("\\\\filer\\") == 0) {
    return true;
  }
  return false;
}

// Adds a server with the given name to the list of internal or external
// servers, as appropriate.
static void AddServer(const string& server_name,
                      vector<string>* internal_servers,
                      vector<string>* external_servers) {
  if (IsInternalServer(server_name)) {
    internal_servers->push_back(server_name);
  } else {
    external_servers->push_back(server_name);
  }
}

}  // namespace

int main(int argc, char** argv) {
  string time_string = CurrentDateAndTime();
  FprintfFlush(stdout, "converter: %s: starting\n", time_string.c_str());

  ConverterOptions options;
  options.report_fetch_failures = true;

  // All arguments are paired.
  if (argc % 2 != 1) {
    return usage(argv[0]);
  }

  string blacklist_regex_str;
  bool have_any_msss_servers = false;
  for (int argi = 1; argi < argc; argi += 2) {
    string option = argv[argi];
    string value = argv[argi + 1];

    if (option == "-f") {
      AddServer(value,& options.full_internal_msss_servers,
               & options.full_external_msss_servers);
      have_any_msss_servers = true;
    } else if (option == "-n") {
      AddServer(value,& options.no_exe_internal_msss_servers,
               & options.no_exe_external_msss_servers);
      have_any_msss_servers = true;
    } else if (option == "-l") {
      if (!options.local_cache_path.empty()) {
        return usage(argv[0]);
      }
      options.local_cache_path = value;
    } else if (option == "-s") {
      if (!WindowsStringUtils::safe_mbstowcs(value,
                                            & options.upload_symbols_url)) {
        FprintfFlush(stderr, "main: safe_mbstowcs failed for %s\n",
                     value.c_str());
        return 1;
      }
    } else if (option == "-m") {
      if (!WindowsStringUtils::safe_mbstowcs(value,
                                            & options.missing_symbols_url)) {
        FprintfFlush(stderr, "main: safe_mbstowcs failed for %s\n",
                     value.c_str());
        return 1;
      }
    } else if (option == "-mf") {
      options.missing_symbols_file = value;
      printf("Getting the list of missing symbols from a file.  Fetch failures"
             " will not be reported.\n");
      options.report_fetch_failures = false;
    } else if (option == "-t") {
      if (!WindowsStringUtils::safe_mbstowcs(
          value,
         & options.fetch_symbol_failure_url)) {
        FprintfFlush(stderr, "main: safe_mbstowcs failed for %s\n",
                     value.c_str());
        return 1;
      }
    } else if (option == "-b") {
      blacklist_regex_str = value;
    } else {
      return usage(argv[0]);
    }
  }

  if (blacklist_regex_str.empty()) {
    FprintfFlush(stderr, "No blacklist specified.\n");
    return usage(argv[0]);
  }

  // Compile the blacklist regular expression for later use.
  options.blacklist_regex = std::regex(blacklist_regex_str.c_str(),
      std::regex_constants::icase);

  // Set the defaults.  If the user specified any MSSS servers, don't use
  // any default.
  if (!have_any_msss_servers) {
    AddServer(kNoExeMSSSServer,& options.no_exe_internal_msss_servers,
       & options.no_exe_external_msss_servers);
  }

  if (options.local_cache_path.empty()) {
    options.local_cache_path = kLocalCachePath;
  }

  if (options.upload_symbols_url.empty()) {
    FprintfFlush(stderr, "No upload symbols URL specified.\n");
    return usage(argv[0]);
  }
  if (options.missing_symbols_url.empty() &&
      options.missing_symbols_file.empty()) {
    FprintfFlush(stderr, "No missing symbols URL or file specified.\n");
    return usage(argv[0]);
  }
  if (options.fetch_symbol_failure_url.empty()) {
    FprintfFlush(stderr, "No fetch symbol failure URL specified.\n");
    return usage(argv[0]);
  }

  FprintfFlush(stdout,
               "# of Symbol Servers (int/ext): %d/%d full, %d/%d no_exe\n",
               options.full_internal_msss_servers.size(),
               options.full_external_msss_servers.size(),
               options.no_exe_internal_msss_servers.size(),
               options.no_exe_external_msss_servers.size());

  if (!ConvertMissingSymbolsList(options)) {
    return 1;
  }

  time_string = CurrentDateAndTime();
  FprintfFlush(stdout, "converter: %s: finished\n", time_string.c_str());
  return 0;
}
