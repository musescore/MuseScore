/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "importexport/lyricsexport/internal/lrcwriter.h"

using namespace mu::engraving;

static const String LRC_DIR(u"data/");

namespace mu::iex::lrcexport {
class Lrc_Tests : public ::testing::Test
{
public:
    void lrcTest(const char* file, bool enhancedLrc);

    /// Set to true to re-generate the MuseScore reference test files
    static constexpr bool s_generateReferenceFile = false;
};

void Lrc_Tests::lrcTest(const char* file, bool enhancedLrc)
{
    String fileName = String::fromUtf8(file);

    auto exportFunc = [](Score* score, const muse::io::path_t& path, bool enhancedLrc) -> bool {
        LRCWriter lrcWriter;
        return lrcWriter.writeScore(score, path, enhancedLrc);
    };

    // Load the file
    MasterScore* score = ScoreRW::readScore(LRC_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);

    // Flag to be turned on to generate the test reference lrc files
    if constexpr (s_generateReferenceFile) {
        bool res = exportFunc(score, ScoreRW::rootPath() + u"/" + LRC_DIR + fileName + u"_ref.lrc", enhancedLrc);
        EXPECT_TRUE(res);
        return;
    }

    // Generate lrc export file
    EXPECT_TRUE(exportFunc(score, fileName + u".test.lrc", enhancedLrc));
    delete score;

    // Compare the lrc files
    EXPECT_TRUE(ScoreComp::compareFiles(fileName + u".test.lrc", ScoreRW::rootPath() + u"/" + LRC_DIR + fileName + u"_ref.lrc"));
}

TEST_F(Lrc_Tests, lrc_simple_test1) {
    lrcTest("lrc_simple_test1", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test1) {
    lrcTest("lrc_enhanced_test1", true);
}

TEST_F(Lrc_Tests, lrc_simple_test2) {
    lrcTest("lrc_simple_test2", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test2) {
    lrcTest("lrc_enhanced_test2", true);
}

TEST_F(Lrc_Tests, lrc_simple_test3) {
    lrcTest("lrc_simple_test3", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test3) {
    lrcTest("lrc_enhanced_test3", true);
}

TEST_F(Lrc_Tests, lrc_simple_test4) {
    lrcTest("lrc_simple_test4", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test4) {
    lrcTest("lrc_enhanced_test4", true);
}

TEST_F(Lrc_Tests, lrc_simple_test5) {
    lrcTest("lrc_simple_test5", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test5) {
    lrcTest("lrc_enhanced_test5", true);
}

TEST_F(Lrc_Tests, lrc_simple_test6) {
    lrcTest("lrc_simple_test6", false);
}

TEST_F(Lrc_Tests, lrc_enhanced_test6) {
    lrcTest("lrc_enhanced_test6", true);
}
}
