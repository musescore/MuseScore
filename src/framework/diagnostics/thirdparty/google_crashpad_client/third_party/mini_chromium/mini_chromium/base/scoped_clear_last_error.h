// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_BASE_SCOPED_CLEAR_LAST_ERROR_H_
#define MINI_CHROMIUM_BASE_SCOPED_CLEAR_LAST_ERROR_H_

#include <errno.h>

#include "base/macros.h"
#include "build/build_config.h"

namespace base {

class ScopedClearLastErrorBase {
 public:
  ScopedClearLastErrorBase() : last_errno_(errno) { errno = 0; }
  ~ScopedClearLastErrorBase() { errno = last_errno_; }

 private:
  const int last_errno_;

  DISALLOW_COPY_AND_ASSIGN(ScopedClearLastErrorBase);
};

#if defined(OS_WIN)

class ScopedClearLastError : public ScopedClearLastErrorBase {
 public:
  ScopedClearLastError();
  ~ScopedClearLastError();

 private:
  const unsigned long last_system_error_;

  DISALLOW_COPY_AND_ASSIGN(ScopedClearLastError);
};

#elif defined(OS_POSIX) || defined(OS_FUCHSIA)

using ScopedClearLastError = ScopedClearLastErrorBase;

#endif  // defined(OS_WIN)

}  // namespace base

#endif  // MINI_CHROMIUM_BASE_SCOPED_CLEAR_LAST_ERROR_H_
