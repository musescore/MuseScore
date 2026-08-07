/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/masterscore.h"

#include "engraving/editing/reset.h"
#include "engraving/editing/transaction/transaction.h"

#include "types/translatablestring.h"
#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String RWUNDORESET_DATA_DIR("readwriteundoreset_data/");

class Engraving_ReadWriteUndoResetTests : public ::testing::Test
{
};

TEST_F(Engraving_ReadWriteUndoResetTests, testReadWriteResetPositions)
{
    std::vector<const char16_t*> files = {
        u"barlines",
        u"slurs",
        u"mmrestBarlineTextLinks" // see issue #296426
    };

    for (const char16_t* file : files) {
        String readFile(RWUNDORESET_DATA_DIR + file + u".mscx");
        String writeFile(String(file) + u"-undoreset-test.mscx");

        MasterScore* score = ScoreRW::readScore(readFile);
        EXPECT_TRUE(score);
        score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Reset all positions"), [&](Transaction& tx) {
            Reset::resetAllPositions(tx, score);
        });
        score->undoRedo(/* undo */ true, nullptr);
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, readFile));

        delete score;
    }
}

//---------------------------------------------------------
//   testMMRestLinksRecreateMMRest
///   For barlines links with MM rests a separate test is
///   needed: in this test score, if creating MM rests from
///   scratch, <linked> tags in BarLines may have appeared
///   before <linkedMain> tags, so they were not able to
///   link and prevented text elements from linking as well.
///
///   See issue #296426
//---------------------------------------------------------

TEST_F(Engraving_ReadWriteUndoResetTests, testMMRestLinksRecreateMMRest)
{
    const String file("mmrestBarlineTextLinks");

    String readFile(RWUNDORESET_DATA_DIR + file + u".mscx");
    String writeFile(file + u"-recreate-mmrest-test.mscx");
    String disableMMRestRefFile(RWUNDORESET_DATA_DIR + file + u"-disable-mmrest-ref.mscx");
    String recreateMMRestRefFile(RWUNDORESET_DATA_DIR + file + u"-recreate-mmrest-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);

    // Regenerate MM rests from scratch:
    // 1) turn MM rests off
    score->startCmd(TranslatableString::untranslatable("Read/write/undo/reset tests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, false);
    score->endCmd();

    // 2) save/close/reopen the score
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, disableMMRestRefFile));
    delete score;
    score = ScoreRW::readScore(writeFile, true);
    ASSERT_TRUE(score);

    // 3) turn MM rests back on
    score->startCmd(TranslatableString::untranslatable("Read/write/undo/reset tests"));
    score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, recreateMMRestRefFile));

    delete score;
}

TEST_F(Engraving_ReadWriteUndoResetTests, testWatermarkStyleProperties)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    // 1. Verify default values
    EXPECT_FALSE(score->style().styleB(Sid::watermarkEnabled));
    EXPECT_EQ(score->style().styleI(Sid::watermarkType), 0);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"DRAFT"));
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkOpacity), 0.20);
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkAngle), -45.0);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkImagePath), String(u""));
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkImageScale), 1.0);

    // 2. Modify properties
    score->style().set(Sid::watermarkEnabled, true);
    score->style().set(Sid::watermarkType, 1);
    score->style().set(Sid::watermarkText, String(u"CONFIDENTIAL"));
    score->style().set(Sid::watermarkOpacity, 0.50);
    score->style().set(Sid::watermarkAngle, 30.0);
    score->style().set(Sid::watermarkImagePath, String(u"file://logo.png"));
    score->style().set(Sid::watermarkImageScale, 2.5);

    // 3. Verify modified values
    EXPECT_TRUE(score->style().styleB(Sid::watermarkEnabled));
    EXPECT_EQ(score->style().styleI(Sid::watermarkType), 1);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"CONFIDENTIAL"));
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkOpacity), 0.50);
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkAngle), 30.0);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkImagePath), String(u"file://logo.png"));
    EXPECT_DOUBLE_EQ(score->style().styleD(Sid::watermarkImageScale), 2.5);

    delete score;
}

TEST_F(Engraving_ReadWriteUndoResetTests, testWatermarkSerialization)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    // 1. Set non-default watermark style values
    score->style().set(Sid::watermarkEnabled, true);
    score->style().set(Sid::watermarkType, 1);
    score->style().set(Sid::watermarkText, String(u"CONFIDENTIAL"));
    score->style().set(Sid::watermarkOpacity, 0.50);
    score->style().set(Sid::watermarkAngle, 30.0);
    score->style().set(Sid::watermarkImagePath, String(u"file://logo.png"));
    score->style().set(Sid::watermarkImageScale, 2.5);

    // 2. Save score to XML
    const String filename(u"watermark-serialization-test.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score, filename));
    delete score;

    // 3. Read it back
    MasterScore* loadedScore = ScoreRW::readScore(filename, true);
    ASSERT_TRUE(loadedScore);

    // 4. Verify deserialized watermark style values
    EXPECT_TRUE(loadedScore->style().styleB(Sid::watermarkEnabled));
    EXPECT_EQ(loadedScore->style().styleI(Sid::watermarkType), 1);
    EXPECT_EQ(loadedScore->style().styleSt(Sid::watermarkText), String(u"CONFIDENTIAL"));
    EXPECT_DOUBLE_EQ(loadedScore->style().styleD(Sid::watermarkOpacity), 0.50);
    EXPECT_DOUBLE_EQ(loadedScore->style().styleD(Sid::watermarkAngle), 30.0);
    EXPECT_EQ(loadedScore->style().styleSt(Sid::watermarkImagePath), String(u"file://logo.png"));
    EXPECT_DOUBLE_EQ(loadedScore->style().styleD(Sid::watermarkImageScale), 2.5);

    delete loadedScore;
}

TEST_F(Engraving_ReadWriteUndoResetTests, testWatermarkUndoRedo)
{
    MasterScore* score = ScoreRW::readScore(u"test.mscx");
    ASSERT_TRUE(score);

    // Verify initial default
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"DRAFT"));

    // 1. Modify watermarkText via Undo Command
    score->startCmd(TranslatableString::untranslatable("Change Watermark Text"));
    score->undoChangeStyleVal(Sid::watermarkText, String(u"CONFIDENTIAL"));
    score->endCmd();

    // Verify it changed
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"CONFIDENTIAL"));

    // 2. Undo
    score->undoRedo(true, nullptr);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"DRAFT"));

    // 3. Redo
    score->undoRedo(false, nullptr);
    EXPECT_EQ(score->style().styleSt(Sid::watermarkText), String(u"CONFIDENTIAL"));

    delete score;
}


