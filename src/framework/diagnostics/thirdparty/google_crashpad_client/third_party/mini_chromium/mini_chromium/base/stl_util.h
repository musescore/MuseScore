// Copyright 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_BASE_STL_UTIL_H_
#define MINI_CHROMIUM_BASE_STL_UTIL_H_

#include <sys/types.h>

namespace base {

template <typename T, size_t N>
constexpr size_t size(const T (&array)[N]) noexcept {
  return N;
}

}  // namespace base

#endif  // MINI_CHROMIUM_BASE_STL_UTIL_H_
