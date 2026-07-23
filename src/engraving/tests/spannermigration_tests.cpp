/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

// ONE-OFF MIGRATION TOOL — not part of the permanent test suite.
// Converts a single fixture that is marked version="5.00" but still uses the
// pre-EID inline spanner format. read500's reading code has been temporarily
// reverted (see git commit 3f8a08bbfd) to the pre-EID connector-based reader
// so that it can still parse these files; the writer is unmodified
// (current/new EID format), so a plain load+resave upgrades the fixture in
// place.
//
// Driven one file at a time (via the MIGRATE_FILE env var, an absolute path)
// from an external shell loop so that a hard abort() on one problem fixture
// doesn't take the rest of the batch down with it.

#include <gtest/gtest.h>

#include "utils/scorerw.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

TEST(Engraving_SpannerMigration, ConvertOldFormatFixture)
{
    const char* envPath = std::getenv("MIGRATE_FILE");
    if (!envPath || !*envPath) {
        GTEST_SKIP() << "MIGRATE_FILE not set";
    }

    String absPath = String::fromUtf8(envPath);
    LOGI() << "=== migrating: " << absPath << " ===";

    MasterScore* score = ScoreRW::readScore(absPath, /*isAbsolutePath*/ true);
    ASSERT_TRUE(score) << absPath.toStdString();

    bool saveOk = ScoreRW::saveScore(score, absPath);
    EXPECT_TRUE(saveOk) << absPath.toStdString();

    delete score;
}
