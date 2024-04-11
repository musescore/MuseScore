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
#ifndef MU_LOGREMOVER_H
#define MU_LOGREMOVER_H

#include "muse_framework_config.h"

#ifdef MUSE_ENABLE_UNIT_TESTS
#include <gtest/gtest_prod.h>
#endif

#include "types/string.h"
#include "types/datetime.h"
#include "io/path.h"

namespace muse {
class LogRemover
{
public:

    static void removeLogs(const io::path_t& logsDir, int olderThanDays, const String& pattern);

private:

#ifdef MUSE_ENABLE_UNIT_TESTS
    FRIEND_TEST(Global_LogRemoverTests, ParseDate);
#endif

    static void scanDir(const io::path_t& logsDir, io::paths_t& files);
    static Date parseDate(const String& fileName);
    static void removeFiles(const io::paths_t& files);
};
}

#endif // MU_LOGREMOVER_H
