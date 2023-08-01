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

using namespace mu;
using namespace mu::engraving;

static const String MEI_DIR(u"data/");

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

    MasterScore* score = ScoreRW::readScore(MEI_DIR + fileName + u"." + String::fromUtf8(ext), false, importFunc);
    EXPECT_TRUE(score);

    // Flag to be turned on to generate the test reference .mscx files from the .mei
    if (s_generateReferenceFile) {
        bool res = ScoreRW::saveScore(score, ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mscx");
        EXPECT_TRUE(res);
        return;
    }

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));
    delete score;
}

TEST_F(Mei_Tests, meiAccid01) {
    meiReadTest("accid-01", "mei");
}
}
