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

#include <gtest/gtest.h>

#include "io/file.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "libmscore/masterscore.h"
#include "libmscore/excerpt.h"

#include "modularity/ioc.h"
#include "importexport/mei/imeiconfiguration.h"
#include "importexport/mei/internal/meireader.h"
#include "importexport/mei/internal/meiwriter.h"

using namespace mu;
using namespace mu::engraving;

static const String MEI_DIR(u"data/");
static const String TMP_DIR(u"tmp/");

namespace mu::iex::mei {
class Mei_Tests : public ::testing::Test
{
public:
    void meiReadTest(const char* file,  const char* ext);

    // Change this to true to re-generate reference files
    inline static bool s_generateReferenceFile = false;
};

void Mei_Tests::meiReadTest(const char* file, const char* ext)
{
    String fileName = String::fromUtf8(file);

    auto importFunc = [](MasterScore* score, const io::path_t& path) -> Err {
        MeiReader meiReader;
        return meiReader.import(score, path);
    };

    auto exportFunc = [](Score* score, const io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    // Load the .mei file
    MasterScore* score = ScoreRW::readScore(MEI_DIR + fileName + u"." + String::fromUtf8(ext), false, importFunc);
    EXPECT_TRUE(score);

    // Flag to be turned on to generate the test reference .mscx files from the .mei
    if (s_generateReferenceFile) {
        bool res = ScoreRW::saveScore(score, ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mscx");
        EXPECT_TRUE(res);
        return;
    }

    // Compare with the reference MuseScore file
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));

    // Save the .mei file for round trip testing
    bool output = ScoreRW::saveScore(score,  fileName + u".test.mei", exportFunc);
    EXPECT_TRUE(output);
    delete score;

    // Potentially directly compare the mei files
    // This currently passes inconsitently because of some order differences in the xml elements
    // If this can be fixed, we can used that for testing and not re-import the mei file again
    // EXPECT_TRUE(ScoreComp::compareFiles(fileName + u".test.mei", ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mei"))

    // Reload the generated .mei file
    MasterScore* score2 = ScoreRW::readScore(fileName + u".test.mei", true, importFunc);
    EXPECT_TRUE(score2);

    // Compare with the reference MuseScore file
    EXPECT_TRUE(ScoreComp::saveCompareScore(score2, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));
    delete score2;
}

TEST_F(Mei_Tests, meiAccid01) {
    meiReadTest("accid-01", "mei");
}
}
