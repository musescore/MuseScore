// Copyright 2014 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "util/mac/mac_util.h"

#include <Availability.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/mac/scoped_ioobject.h"
#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "base/strings/sys_string_conversions.h"
#include "build/build_config.h"
#include "util/mac/sysctl.h"

extern "C" {
// Private CoreFoundation internals. See 10.9.2 CF-855.14/CFPriv.h and
// CF-855.14/CFUtilities.c. These are marked for weak import because they’re
// private and subject to change.

#define WEAK_IMPORT __attribute__((weak_import))

// Don’t call these functions directly, call them through the
// TryCFCopy*VersionDictionary() helpers to account for the possibility that
// they may not be present at runtime.
CFDictionaryRef _CFCopySystemVersionDictionary() WEAK_IMPORT;
CFDictionaryRef _CFCopyServerVersionDictionary() WEAK_IMPORT;

// Don’t use these constants with CFDictionaryGetValue() directly, use them with
// the TryCFDictionaryGetValue() wrapper to account for the possibility that
// they may not be present at runtime.
extern const CFStringRef _kCFSystemVersionProductNameKey WEAK_IMPORT;
extern const CFStringRef _kCFSystemVersionProductVersionKey WEAK_IMPORT;
extern const CFStringRef _kCFSystemVersionProductVersionExtraKey WEAK_IMPORT;
extern const CFStringRef _kCFSystemVersionBuildVersionKey WEAK_IMPORT;

#undef WEAK_IMPORT

}  // extern "C"

namespace {

#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_13_4
// Returns the running system’s Darwin major version. Don’t call this, it’s an
// implementation detail and its result is meant to be cached by
// MacOSVersionNumber().
//
// This is very similar to Chromium’s base/mac/mac_util.mm
// DarwinMajorVersionInternal().
int DarwinMajorVersion() {
  // base::OperatingSystemVersionNumbers calls Gestalt(), which is a
  // higher-level function than is needed. It might perform unnecessary
  // operations. On 10.6, it was observed to be able to spawn threads (see
  // https://crbug.com/53200). It might also read files or perform other
  // blocking operations. Actually, nobody really knows for sure just what
  // Gestalt() might do, or what it might be taught to do in the future.
  //
  // uname(), on the other hand, is implemented as a simple series of sysctl()
  // system calls to obtain the relevant data from the kernel. The data is
  // compiled right into the kernel, so no threads or blocking or other funny
  // business is necessary.

  utsname uname_info;
  int rv = uname(&uname_info);
  PCHECK(rv == 0) << "uname";

  DCHECK_EQ(strcmp(uname_info.sysname, "Darwin"), 0) << "unexpected sysname "
                                                     << uname_info.sysname;

  char* dot = strchr(uname_info.release, '.');
  CHECK(dot);

  int darwin_major_version = 0;
  CHECK(base::StringToInt(
      base::StringPiece(uname_info.release, dot - uname_info.release),
      &darwin_major_version));

  return darwin_major_version;
}
#endif  // DT < 10.13.4

// Helpers for the weak-imported private CoreFoundation internals.

CFDictionaryRef TryCFCopySystemVersionDictionary() {
  if (_CFCopySystemVersionDictionary) {
    return _CFCopySystemVersionDictionary();
  }
  return nullptr;
}

CFDictionaryRef TryCFCopyServerVersionDictionary() {
  if (_CFCopyServerVersionDictionary) {
    return _CFCopyServerVersionDictionary();
  }
  return nullptr;
}

const void* TryCFDictionaryGetValue(CFDictionaryRef dictionary,
                                    const void* value) {
  if (value) {
    return CFDictionaryGetValue(dictionary, value);
  }
  return nullptr;
}

// Converts |version| to a triplet of version numbers on behalf of
// MacOSVersionNumber() and MacOSVersionComponents(). Returns true on success.
// If |version| does not have the expected format, returns false. |version| must
// be in the form "10.9.2" or just "10.9". In the latter case, |bugfix| will be
// set to 0.
bool StringToVersionNumbers(const std::string& version,
                            int* major,
                            int* minor,
                            int* bugfix) {
  size_t first_dot = version.find_first_of('.');
  if (first_dot == 0 || first_dot == std::string::npos ||
      first_dot == version.length() - 1) {
    LOG(ERROR) << "version has unexpected format";
    return false;
  }
  if (!base::StringToInt(base::StringPiece(&version[0], first_dot), major)) {
    LOG(ERROR) << "version has unexpected format";
    return false;
  }

  size_t second_dot = version.find_first_of('.', first_dot + 1);
  if (second_dot == version.length() - 1) {
    LOG(ERROR) << "version has unexpected format";
    return false;
  } else if (second_dot == std::string::npos) {
    second_dot = version.length();
  }

  if (!base::StringToInt(base::StringPiece(&version[first_dot + 1],
                                           second_dot - first_dot - 1),
                         minor)) {
    LOG(ERROR) << "version has unexpected format";
    return false;
  }

  if (second_dot == version.length()) {
    *bugfix = 0;
  } else if (!base::StringToInt(
                 base::StringPiece(&version[second_dot + 1],
                                   version.length() - second_dot - 1),
                 bugfix)) {
    LOG(ERROR) << "version has unexpected format";
    return false;
  }

  return true;
}

std::string IORegistryEntryDataPropertyAsString(io_registry_entry_t entry,
                                                CFStringRef key) {
  base::ScopedCFTypeRef<CFTypeRef> property(
      IORegistryEntryCreateCFProperty(entry, key, kCFAllocatorDefault, 0));
  CFDataRef data = base::mac::CFCast<CFDataRef>(property);
  if (data && CFDataGetLength(data) > 0) {
    return reinterpret_cast<const char*>(CFDataGetBytePtr(data));
  }

  return std::string();
}

}  // namespace

namespace crashpad {

int MacOSVersionNumber() {
  static int macos_version_number = []() {
    // kern.osproductversion is a lightweight way to get the operating system
    // version from the kernel without having to open any files or spin up any
    // threads, but it’s only available in macOS 10.13.4 and later.
    std::string macos_version_number_string = ReadStringSysctlByName(
        "kern.osproductversion",
        __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_13_4);
    if (!macos_version_number_string.empty()) {
      int major;
      int minor;
      int bugfix;
      if (StringToVersionNumbers(
              macos_version_number_string, &major, &minor, &bugfix)) {
        DCHECK_GE(major, 10);
        DCHECK_LE(major, 99);
        DCHECK_GE(minor, 0);
        DCHECK_LE(minor, 99);
        DCHECK_GE(bugfix, 0);
        DCHECK_LE(bugfix, 99);
        return major * 1'00'00 + minor * 1'00 + bugfix;
      }
    }

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_13_4
    // On macOS 10.13.4 and later, the sysctlbyname above should have been
    // successful.
    NOTREACHED();
    return -1;
#else  // DT >= 10.13.4
    // The Darwin major version is always 4 greater than the macOS minor version
    // for Darwin versions beginning with 6, corresponding to Mac OS X 10.2,
    // through Darwin 19, corresponding to macOS 10.15.
    int darwin_major_version = DarwinMajorVersion();
    DCHECK_GE(darwin_major_version, 6);
    DCHECK_LE(darwin_major_version, 19);

    int macos_version_number = 10'00'00 + (darwin_major_version - 4) * 1'00;

    // On macOS 10.13.4 and later, the sysctlbyname above should have been
    // successful.
    DCHECK_LT(macos_version_number, 10'13'04);

    return macos_version_number;
#endif  // DT >= 10.13.4
  }();

  return macos_version_number;
}

bool MacOSVersionComponents(int* major,
                            int* minor,
                            int* bugfix,
                            std::string* build,
                            bool* server,
                            std::string* version_string) {
  base::ScopedCFTypeRef<CFDictionaryRef> dictionary(
      TryCFCopyServerVersionDictionary());
  if (dictionary) {
    *server = true;
  } else {
    dictionary.reset(TryCFCopySystemVersionDictionary());
    if (!dictionary) {
      LOG(ERROR) << "_CFCopySystemVersionDictionary failed";
      return false;
    }
    *server = false;
  }

  bool success = true;

  CFStringRef version_cf = base::mac::CFCast<CFStringRef>(
      TryCFDictionaryGetValue(dictionary, _kCFSystemVersionProductVersionKey));
  std::string version;
  if (!version_cf) {
    LOG(ERROR) << "version_cf not found";
    success = false;
  } else {
    version = base::SysCFStringRefToUTF8(version_cf);
    if (!StringToVersionNumbers(version, major, minor, bugfix)) {
      success = false;
    } else {
      DCHECK_GE(*major, 10);
      DCHECK_LE(*major, 99);
      DCHECK_GE(*minor, 0);
      DCHECK_LE(*minor, 99);
      DCHECK_GE(*bugfix, 0);
      DCHECK_LE(*bugfix, 99);
    }
  }

  CFStringRef build_cf = base::mac::CFCast<CFStringRef>(
      TryCFDictionaryGetValue(dictionary, _kCFSystemVersionBuildVersionKey));
  if (!build_cf) {
    LOG(ERROR) << "build_cf not found";
    success = false;
  } else {
    build->assign(base::SysCFStringRefToUTF8(build_cf));
  }

  CFStringRef product_cf = base::mac::CFCast<CFStringRef>(
      TryCFDictionaryGetValue(dictionary, _kCFSystemVersionProductNameKey));
  std::string product;
  if (!product_cf) {
    LOG(ERROR) << "product_cf not found";
    success = false;
  } else {
    product = base::SysCFStringRefToUTF8(product_cf);
  }

  // This key is not required, and in fact is normally not present.
  CFStringRef extra_cf = base::mac::CFCast<CFStringRef>(TryCFDictionaryGetValue(
      dictionary, _kCFSystemVersionProductVersionExtraKey));
  std::string extra;
  if (extra_cf) {
    extra = base::SysCFStringRefToUTF8(extra_cf);
  }

  if (!product.empty() || !version.empty() || !build->empty()) {
    if (!extra.empty()) {
      version_string->assign(base::StringPrintf("%s %s %s (%s)",
                                                product.c_str(),
                                                version.c_str(),
                                                extra.c_str(),
                                                build->c_str()));
    } else {
      version_string->assign(base::StringPrintf(
          "%s %s (%s)", product.c_str(), version.c_str(), build->c_str()));
    }
  }

  return success;
}

void MacModelAndBoard(std::string* model, std::string* board_id) {
  base::mac::ScopedIOObject<io_service_t> platform_expert(
      IOServiceGetMatchingService(kIOMasterPortDefault,
                                  IOServiceMatching("IOPlatformExpertDevice")));
  if (platform_expert) {
    model->assign(
        IORegistryEntryDataPropertyAsString(platform_expert, CFSTR("model")));
#if defined(ARCH_CPU_X86_FAMILY)
    CFStringRef kBoardProperty = CFSTR("board-id");
#elif defined(ARCH_CPU_ARM64)
    // TODO(https://crashpad.chromium.org/bug/352): When production arm64
    // hardware is available, determine whether board-id works and switch to it
    // if feasible, otherwise, determine whether target-type remains a viable
    // alternative.
    CFStringRef kBoardProperty = CFSTR("target-type");
#endif
    board_id->assign(IORegistryEntryDataPropertyAsString(platform_expert,
                                                         kBoardProperty));
  } else {
    model->clear();
    board_id->clear();
  }
}

}  // namespace crashpad
