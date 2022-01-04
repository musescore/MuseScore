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

#include "thirdparty/haw_profiler/src/profiler.h"

#ifndef HAW_LOGGER_QT_SUPPORT
#define HAW_LOGGER_QT_SUPPORT
#endif
#include "thirdparty/haw_logger/logger/log_base.h"

#define IF_ASSERT_FAILED_X(cond, msg) if (!(cond)) { \
        LOGE() << "\"ASSERT FAILED!\":" << msg << __FILE__ << __LINE__; \
        Q_ASSERT(cond); \
} \
    if (!(cond)) \

#define IF_ASSERT_FAILED(cond) IF_ASSERT_FAILED_X(cond, #cond)

#define IF_FAILED(cond) if (!(cond)) { \
        LOGE() << "\"FAILED!\":" << #cond << __FILE__ << __LINE__; \
} \
    if (!(cond)) \

#define UNUSED(x) (void)x;

#define UNREACHABLE \
    LOGE() << "\"UNREACHABLE!\":" << __FILE__ << __LINE__; \
    Q_ASSERT_X(false, "UNREACHABLE", "UNREACHABLE was reached"); \

#endif // MU_FRAMEWORK_LOG_H
