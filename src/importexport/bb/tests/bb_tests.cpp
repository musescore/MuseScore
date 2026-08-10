/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "engraving/engravingerrors.h"
#include "engraving/dom/masterscore.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::iex::bb {
extern engraving::Err importBB(MasterScore*, const QString& name);
}

static const String BIAB_DIR("data/");

class Bb_Tests : public ::testing::Test
{
public:
    void biabReadTest(const char* file);
};

//---------------------------------------------------------
//   biabReadTest
//   read a Band-in-a-Box file, write to a MuseScore file and verify against reference
//   every "xxx" test requires a *.SGU file and a *.mscx file:
//          xxx.SGU      is the SGU file
//          xxx-ref.mscx is the corresponding (correct) mscore file
//---------------------------------------------------------

void Bb_Tests::biabReadTest(const char* file)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::iex::bb::importBB(score, path.toQString());
    };

    String fileName = String::fromUtf8(file);
    MasterScore* score = ScoreRW::readScore(BIAB_DIR + fileName + u".SGU", false, importFunc);
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u"-test.mscx", BIAB_DIR + fileName + u"-ref.mscx"));
    delete score;
}

TEST_F(Bb_Tests, biabChords) {
    biabReadTest("chords");
}
