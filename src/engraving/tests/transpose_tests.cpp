/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/masterscore.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String TRANSPOSE_DATA_DIR("transpose_data/");

class Engraving_TransposeTests : public ::testing::Test
{
public:
    void undoTransposeTest(String scoreName)
    {
        String readFile(TRANSPOSE_DATA_DIR + scoreName + ".mscx");
        String writeFile1(scoreName + "01-test.mscx");
        String reference1(TRANSPOSE_DATA_DIR + scoreName + "01-ref.mscx");
        String writeFile2(scoreName + "02-test.mscx");
        String reference2(TRANSPOSE_DATA_DIR + scoreName + "02-ref.mscx");

        MasterScore* score = ScoreRW::readScore(readFile);

        // select all
        score->cmdSelectAll();

        // transpose major second up
        score->startCmd(TranslatableString::untranslatable("Engraving transpose tests"));
        score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4,
                         true, true, true);
        score->endCmd();
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

        // undo
        EditData ed;
        score->undoStack()->undo(&ed);
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

        delete score;
    }

    void undoDiatonicTransposeTest(String scoreName)
    {
        String readFile(TRANSPOSE_DATA_DIR + scoreName + ".mscx");
        String writeFile1(scoreName + "01-test.mscx");
        String reference1(TRANSPOSE_DATA_DIR + scoreName + "01-ref.mscx");
        String writeFile2(scoreName + "02-test.mscx");
        String reference2(TRANSPOSE_DATA_DIR + scoreName + "02-ref.mscx");

        MasterScore* score = ScoreRW::readScore(readFile);
        score->doLayout();

        // select all
        score->cmdSelectAll();

        // transpose diatonic fourth down
        score->startCmd(TranslatableString::untranslatable("Engraving transpose tests"));
        score->transpose(TransposeMode::DIATONICALLY, TransposeDirection::DOWN, Key::C, 3,
                         true, false, false);
        score->endCmd();
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

        // undo
        EditData ed;
        score->undoStack()->undo(&ed);
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

        delete score;
    }
};

TEST_F(Engraving_TransposeTests, undoTranspose)
{
    undoTransposeTest(u"undoTranspose");
}

TEST_F(Engraving_TransposeTests, undoTransposeChordSymbols)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    undoTransposeTest(u"undoTransposeChordSymbols");
    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_TransposeTests, undoDiatonicTransposeChordSymbols)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    undoDiatonicTransposeTest(u"undoDiatonicTransposeChordSymbols");
    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_TransposeTests, undoDiatonicTranspose)
{
    undoDiatonicTransposeTest(u"undoDiatonicTranspose");
}
