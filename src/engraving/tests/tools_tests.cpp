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
#include "dom/measure.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String TOOLS_DATA_DIR("tools_data/");

class Engraving_ToolsTests : public ::testing::Test
{
public:
    void changeEnharmonic(bool);
};

TEST_F(Engraving_ToolsTests, undoSlashFill)
{
    String readFile(TOOLS_DATA_DIR + "undoSlashFill.mscx");
    String writeFile1("undoSlashFill01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoSlashFill01-ref.mscx");
    String writeFile2("undoSlashFill02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoSlashFill02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);

    // select
    Segment* s = score->firstMeasure()->findSegment(SegmentType::ChordRest, Fraction(2, 4));
    score->selection().setRange(s, score->lastSegment(), 0, 2);

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdSlashFill();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ToolsTests, undoSlashRhythm)
{
    String readFile(TOOLS_DATA_DIR + "undoSlashRhythm.mscx");
    String writeFile1("undoSlashRhythm01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoSlashRhythm01-ref.mscx");
    String writeFile2("undoSlashRhythm02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoSlashRhythm02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdSlashRhythm();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ToolsTests, undoResequenceAlpha)
{
    String readFile(TOOLS_DATA_DIR + "undoResequenceAlpha.mscx");
    String writeFile1("undoResequenceAlpha01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoResequenceAlpha01-ref.mscx");
    String writeFile2("undoResequenceAlpha02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoResequenceAlpha02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    score->doLayout();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdResequenceRehearsalMarks();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ToolsTests, undoResequenceNumeric)
{
    String readFile(TOOLS_DATA_DIR + "undoResequenceNumeric.mscx");
    String writeFile1("undoResequenceNumeric01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoResequenceNumeric01-ref.mscx");
    String writeFile2("undoResequenceNumeric02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoResequenceNumeric02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    score->doLayout();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdResequenceRehearsalMarks();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ToolsTests, undoResequenceMeasure)
{
    String readFile(TOOLS_DATA_DIR + "undoResequenceMeasure.mscx");
    String writeFile1("undoResequenceMeasure01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoResequenceMeasure01-ref.mscx");
    String writeFile2("undoResequenceMeasure02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoResequenceMeasure02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    score->doLayout();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdResequenceRehearsalMarks();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ToolsTests, undoResequencePart)
{
    String readFile(TOOLS_DATA_DIR + "undoResequencePart.mscx");
    String writeFile1("undoResequencePart01-test.mscx");
    String reference1(TOOLS_DATA_DIR + "undoResequencePart01-ref.mscx");
    String writeFile2("undoResequencePart02-test.mscx");
    String reference2(TOOLS_DATA_DIR + "undoResequencePart02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    score->doLayout();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
    score->cmdResequenceRehearsalMarks();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

void Engraving_ToolsTests::changeEnharmonic(bool both)
{
    String readFile(TOOLS_DATA_DIR + u"change-enharmonic.mscx");
    MasterScore* score = ScoreRW::readScore(readFile);
    String mode = both ? u"both" : u"current";
    score->doLayout();
    score->cmdSelectAll();
    for (int i = 1; i < 6; ++i) {
        score->startCmd(TranslatableString::untranslatable("Engraving tools tests"));
        score->changeEnharmonicSpelling(both);
        score->endCmd();
        String prefix = u"change-enharmonic-" + mode + u"-0" + (u'0' + i);
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, prefix + u"-test.mscx", TOOLS_DATA_DIR + prefix + u"-ref.mscx"));
    }
}

TEST_F(Engraving_ToolsTests, changeEnharmonicBoth)
{
    changeEnharmonic(true);
}

TEST_F(Engraving_ToolsTests, changeEnharmonicCurrent)
{
    changeEnharmonic(false);
}
