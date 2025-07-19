// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_BASE_CHECK_H_
#define MINI_CHROMIUM_BASE_CHECK_H_

#include "base/logging.h"

#define CHECK(condition) \
    LAZY_STREAM(LOG_STREAM(FATAL), !(condition)) \
    << "Check failed: " # condition << ". "

#define PCHECK(condition) \
    LAZY_STREAM(PLOG_STREAM(FATAL), !(condition)) \
    << "Check failed: " # condition << ". "

#define DCHECK(condition) \
    LAZY_STREAM(LOG_STREAM(FATAL), DCHECK_IS_ON() ? !(condition) : false) \
    << "Check failed: " # condition << ". "

#define DPCHECK(condition) \
    LAZY_STREAM(PLOG_STREAM(FATAL), DCHECK_IS_ON() ? !(condition) : false) \
    << "Check failed: " # condition << ". "

#endif  // MINI_CHROMIUM_BASE_CHECK_H_
