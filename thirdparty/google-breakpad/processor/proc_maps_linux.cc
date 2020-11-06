// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "google_breakpad/processor/proc_maps_linux.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>

#include "common/using_std_string.h"
#include "processor/logging.h"

#if defined(OS_ANDROID) && !defined(__LP64__)
// In 32-bit mode, Bionic's inttypes.h defines PRI/SCNxPTR as an
// unsigned long int, which is incompatible with Bionic's stdint.h
// defining uintptr_t as an unsigned int:
// https://code.google.com/p/android/issues/detail?id=57218
#undef SCNxPTR
#define SCNxPTR "x"
#endif

namespace google_breakpad {

bool ParseProcMaps(const string& input,
                   std::vector<MappedMemoryRegion>* regions_out) {
  std::vector<MappedMemoryRegion> regions;

  // This isn't async safe nor terribly efficient, but it doesn't need to be at
  // this point in time.

  // Split the string by newlines.
  std::vector<string> lines;
  string l = "";
  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] != '\n' && input[i] != '\r') {
      l.push_back(input[i]);
    } else if (l.size() > 0) {
      lines.push_back(l);
      l.clear();
    }
  }
  if (l.size() > 0) {
    BPLOG(ERROR) << "Input doesn't end in newline";
    return false;
  }

  for (size_t i = 0; i < lines.size(); ++i) {
    MappedMemoryRegion region;
    const char* line = lines[i].c_str();
    char permissions[5] = {'\0'};  // Ensure NUL-terminated string.
    int path_index = 0;

    // Sample format from man 5 proc:
    //
    // address           perms offset  dev   inode   pathname
    // 08048000-08056000 r-xp 00000000 03:0c 64593   /usr/sbin/gpm
    //
    // The final %n term captures the offset in the input string, which is used
    // to determine the path name. It *does not* increment the return value.
    // Refer to man 3 sscanf for details.
    if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %4c %" SCNx64" %hhx:%hhx %"
               SCNd64 " %n", &region.start, &region.end, permissions,
               &region.offset, &region.major_device, &region.minor_device,
               &region.inode, &path_index) < 7) {
      BPLOG(ERROR) << "sscanf failed for line: " << line;
      return false;
    }

    region.permissions = 0;

    if (permissions[0] == 'r')
      region.permissions |= MappedMemoryRegion::READ;
    else if (permissions[0] != '-')
      return false;

    if (permissions[1] == 'w')
      region.permissions |= MappedMemoryRegion::WRITE;
    else if (permissions[1] != '-')
      return false;

    if (permissions[2] == 'x')
      region.permissions |= MappedMemoryRegion::EXECUTE;
    else if (permissions[2] != '-')
      return false;

    if (permissions[3] == 'p')
      region.permissions |= MappedMemoryRegion::PRIVATE;
    else if (permissions[3] != 's' && permissions[3] != 'S')  // Shared memory.
      return false;

    // Pushing then assigning saves us a string copy.
    regions.push_back(region);
    regions.back().path.assign(line + path_index);
    regions.back().line.assign(line);
  }

  regions_out->swap(regions);
  return true;
}

}  // namespace google_breakpad
