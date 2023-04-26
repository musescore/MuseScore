/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_FRAMEWORK_LOG_H
#define MU_FRAMEWORK_LOG_H

#include <cassert>

#include "thirdparty/haw_profiler/src/profiler.h"
#include "thirdparty/haw_logger/logger/log_base.h"

#define DO_ASSERT_X(cond, msg) \
    LOGE() << "\"ASSERT FAILED!\": " << msg << ", file: " << __FILE__ << ", line: " << __LINE__; \
    assert(cond); \
    (void)(cond); \

#define DO_ASSERT(cond) DO_ASSERT_X(cond, #cond)

#define ASSERT_X(msg) DO_ASSERT_X(false, msg)

#define IF_ASSERT_FAILED_X(cond, msg) if (!(cond)) { \
        DO_ASSERT_X(cond, msg) \
} \
    if (!(cond)) \

#define IF_ASSERT_FAILED(cond) IF_ASSERT_FAILED_X(cond, #cond)

#define IF_FAILED(cond) if (!(cond)) { \
        LOGE() << "\"FAILED!\": " << #cond << ", file: " << __FILE__ << ", line: " << __LINE__; \
} \
    if (!(cond)) \

#define UNUSED(x) (void)x;

#define UNREACHABLE \
    LOGE() << "\"UNREACHABLE!\": " << ", file: " << __FILE__ << ", line: " << __LINE__; \
    ASSERT_X("UNREACHABLE was reached"); \


#if __has_cpp_attribute(fallthrough)
#define MU_FALLTHROUGH() [[fallthrough]]
#else
#define MU_FALLTHROUGH() (void)0
#endif

#endif // MU_FRAMEWORK_LOG_H
