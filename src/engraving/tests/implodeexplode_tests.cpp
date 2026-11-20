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
#include "engraving/editing/implodeexplode.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String IMPLODEEXP_DATA_DIR("implode_explode_data/");

class Engraving_ImplodeExplodeTests : public ::testing::Test
{
public:
    void testUndoExplode(String fileName, bool undoRestoresScore = true);
    void testUndoImplode(String fileName);
};

void Engraving_ImplodeExplodeTests::testUndoExplode(String fileName, bool undoRestoresScore)
{
    String readFile(IMPLODEEXP_DATA_DIR + fileName + ".mscx");
    String beforeFile(fileName + "-before.mscx");
    String writeFile1(fileName + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + fileName + "01-ref.mscx");
    String writeFile2(fileName + "02-test.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    EXPECT_TRUE(ScoreRW::saveScore(score, beforeFile));

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    ImplodeExplode::explode(score);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();
    if (undoRestoresScore) {
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, beforeFile, /*compareWithSavedFile*/ true));
    } else {
        // undo doesn't restore this score exactly yet
        EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, IMPLODEEXP_DATA_DIR + fileName + "02-ref.mscx"));
    }

    delete score;
}

void Engraving_ImplodeExplodeTests::testUndoImplode(String filename)
{
    String readFile(IMPLODEEXP_DATA_DIR + filename + ".mscx");
    String beforeFile(filename + "-before.mscx");
    String writeFile1(filename + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + filename + "01-ref.mscx");
    String writeFile2(filename + "02-test.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    EXPECT_TRUE(ScoreRW::saveScore(score, beforeFile));

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    ImplodeExplode::implode(score);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, beforeFile, /*compareWithSavedFile*/ true));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, undoExplode)
{
    testUndoExplode(u"undoExplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplode)
{
    testUndoImplode(u"undoImplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplodeVoice)
{
    testUndoImplode(u"undoImplodeVoice");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeScore)
{
    testUndoImplode(u"implodeScore");
}

TEST_F(Engraving_ImplodeExplodeTests, explodeDynamics)
{
    testUndoExplode(u"explodeDynamics", /*undoRestoresScore*/ false);
}

TEST_F(Engraving_ImplodeExplodeTests, implodeDynamics)
{
    testUndoImplode(u"implodeDynamics");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeArticulations)
{
    testUndoImplode(u"implodeArticulations");
}
