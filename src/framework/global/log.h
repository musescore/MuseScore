//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

#endif // MU_FRAMEWORK_LOG_H
