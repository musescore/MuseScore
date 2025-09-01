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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "dom/masterscore.h"
#include "dom/staff.h"
#include "dom/chord.h"
#include "dom/note.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String CAPO_DATA_DIR(u"capo_data/");

class Engraving_CapoTests : public ::testing::Test
{
};

TEST_F(Engraving_CapoTests, modeChange)
{
    String readFile(CAPO_DATA_DIR + u"capo_mode_changes.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->updateCapo();
    score->doLayout();

    auto firstMeasure = score->firstMeasure();
    EXPECT_TRUE(firstMeasure);

    auto chRestSeg = firstMeasure->first(mu::engraving::SegmentType::ChordRest);
    EXPECT_TRUE(chRestSeg);

    auto chordStd = chRestSeg->element(0);
    EXPECT_TRUE(chordStd->isChord());

    auto chordTab = chRestSeg->element(4);
    EXPECT_TRUE(chordTab->isChord());

    Staff* staffStd = chordStd->staff();
    Staff* staffTab = chordTab->staff();

    auto capoParams = staffStd->capo(chRestSeg->tick());
    EXPECT_TRUE(capoParams.active);

    const Note* noteStd = toChord(chordStd)->notes()[0];
    const Note* noteTab = toChord(chordTab)->notes()[0];

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 1);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::STANDARD_ONLY;
    staffStd->insertCapoParams(noteStd->tick(), capoParams);
    staffTab->insertCapoParams(noteStd->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd->pitch(), 61);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 1);

    EXPECT_EQ(noteTab->pitch(), 61);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::TAB_ONLY;
    staffStd->insertCapoParams(noteStd->tick(), capoParams);
    staffTab->insertCapoParams(noteStd->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 0);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 0);

    delete score;
}

TEST_F(Engraving_CapoTests, addDelete)
{
    String readFile(CAPO_DATA_DIR + u"capo_add_delete.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->updateCapo();
    score->doLayout();

    auto firstMeasure = score->firstMeasure();
    EXPECT_TRUE(firstMeasure);

    auto secondMeasure = firstMeasure->nextMeasure();
    EXPECT_TRUE(secondMeasure);

    auto chRestSeg1 = firstMeasure->first(mu::engraving::SegmentType::ChordRest);
    EXPECT_TRUE(chRestSeg1);
    auto chRestSeg2 = secondMeasure->first(mu::engraving::SegmentType::ChordRest);
    EXPECT_TRUE(chRestSeg2);

    auto chordStd1 = chRestSeg1->element(0);
    EXPECT_TRUE(chordStd1->isChord());

    auto chordTab1 = chRestSeg1->element(4);
    EXPECT_TRUE(chordTab1->isChord());

    auto chordStd2 = chRestSeg2->element(0);
    EXPECT_TRUE(chordStd1->isChord());

    auto chordTab2 = chRestSeg2->element(4);
    EXPECT_TRUE(chordTab1->isChord());

    Staff* staffStd = chordStd1->staff();
    Staff* staffTab = chordTab1->staff();

    auto capoParams = staffStd->capo(chRestSeg1->tick());
    EXPECT_TRUE(capoParams.active);

    const Note* noteStd1 = toChord(chordStd1)->notes()[0];
    const Note* noteTab1 = toChord(chordTab1)->notes()[0];
    const Note* noteStd2 = toChord(chordStd2)->notes()[0];
    const Note* noteTab2 = toChord(chordTab2)->notes()[0];

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 60);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 1);

    EXPECT_EQ(noteTab2->pitch(), 60);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 1);

    staffStd->insertCapoParams(noteStd2->tick(), capoParams);
    staffTab->insertCapoParams(noteStd2->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 60);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 1);

    EXPECT_EQ(noteTab2->pitch(), 60);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::STANDARD_ONLY;
    staffStd->insertCapoParams(noteStd2->tick(), capoParams);
    staffTab->insertCapoParams(noteStd2->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 61);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 1);

    EXPECT_EQ(noteTab2->pitch(), 61);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::TAB_ONLY;
    staffStd->insertCapoParams(noteStd2->tick(), capoParams);
    staffTab->insertCapoParams(noteStd2->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 60);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 0);

    EXPECT_EQ(noteTab2->pitch(), 60);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 0);

    staffStd->removeCapoParams(noteStd2->tick());
    staffTab->removeCapoParams(noteStd2->tick());
    score->doLayout();

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 60);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 1);

    EXPECT_EQ(noteTab2->pitch(), 60);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 1);

    staffStd->removeCapoParams(noteStd1->tick());
    staffTab->removeCapoParams(noteStd1->tick());
    score->doLayout();

    EXPECT_EQ(noteStd1->pitch(), 60);
    EXPECT_EQ(noteStd1->string(), 1);
    EXPECT_EQ(noteStd1->fret(), 1);

    EXPECT_EQ(noteTab1->pitch(), 60);
    EXPECT_EQ(noteTab1->string(), 1);
    EXPECT_EQ(noteTab1->fret(), 1);

    EXPECT_EQ(noteStd2->pitch(), 60);
    EXPECT_EQ(noteStd2->string(), 1);
    EXPECT_EQ(noteStd2->fret(), 1);

    EXPECT_EQ(noteTab2->pitch(), 60);
    EXPECT_EQ(noteTab2->string(), 1);
    EXPECT_EQ(noteTab2->fret(), 1);

    delete score;
}

TEST_F(Engraving_CapoTests, ignoredString)
{
    String readFile(CAPO_DATA_DIR + u"capo_mode_changes.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->updateCapo();
    score->doLayout();

    auto firstMeasure = score->firstMeasure();
    EXPECT_TRUE(firstMeasure);

    auto chRestSeg = firstMeasure->first(mu::engraving::SegmentType::ChordRest);
    EXPECT_TRUE(chRestSeg);

    auto chordStd = chRestSeg->element(0);
    EXPECT_TRUE(chordStd->isChord());

    auto chordTab = chRestSeg->element(4);
    EXPECT_TRUE(chordTab->isChord());

    Staff* staffStd = chordStd->staff();
    Staff* staffTab = chordTab->staff();

    auto capoParams = staffStd->capo(chRestSeg->tick());
    EXPECT_TRUE(capoParams.active);

    const Note* noteStd = toChord(chordStd)->notes()[0];
    const Note* noteTab = toChord(chordTab)->notes()[0];

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 1);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::STANDARD_ONLY;
    capoParams.ignoredStrings.clear();
    capoParams.ignoredStrings.insert(1);
    staffStd->insertCapoParams(noteStd->tick(), capoParams);
    staffTab->insertCapoParams(noteStd->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 1);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 1);

    capoParams.transposeMode = CapoParams::TransposeMode::TAB_ONLY;
    staffStd->insertCapoParams(noteStd->tick(), capoParams);
    staffTab->insertCapoParams(noteStd->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 1);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 1);

    capoParams.ignoredStrings.clear();
    staffStd->insertCapoParams(noteStd->tick(), capoParams);
    staffTab->insertCapoParams(noteStd->tick(), capoParams);
    score->doLayout();

    EXPECT_EQ(noteStd->pitch(), 60);
    EXPECT_EQ(noteStd->string(), 1);
    EXPECT_EQ(noteStd->fret(), 0);

    EXPECT_EQ(noteTab->pitch(), 60);
    EXPECT_EQ(noteTab->string(), 1);
    EXPECT_EQ(noteTab->fret(), 0);

    delete score;
}
